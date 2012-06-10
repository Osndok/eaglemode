//------------------------------------------------------------------------------
// emMain.cpp
//
// Copyright (C) 2005-2011 Oliver Hamann.
//
// Homepage: http://eaglemode.sourceforge.net/
//
// This program is free software: you can redistribute it and/or modify it under
// the terms of the GNU General Public License version 3 as published by the
// Free Software Foundation.
//
// This program is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
// FOR A PARTICULAR PURPOSE. See the GNU General Public License version 3 for
// more details.
//
// You should have received a copy of the GNU General Public License version 3
// along with this program. If not, see <http://www.gnu.org/licenses/>.
//------------------------------------------------------------------------------

#include <emCore/emFileModel.h>
#include <emCore/emInstallInfo.h>
#include <emCore/emRes.h>
#include <emCore/emMiniIpc.h>
#include <emCore/emGUIFramework.h>
#include <emMain/emMainWindow.h>


#if defined(__linux__) && !defined(ANDROID)
	#include <signal.h>
	#include <execinfo.h>
	#include <unistd.h>

	static void CrashHandler(int sig)
	{
		static void * bt[1024];
		const char * p;

		if (sig==SIGFPE) p="\n--- Arithmetic Exception ---\n";
		else p="\n--- Segmentation Fault ---\n";
		if (write(STDERR_FILENO,p,strlen(p))<0) _exit(128+sig);
		p="Stack Trace (first two entries normally by signal handling):\n";
		if (write(STDERR_FILENO,p,strlen(p))<0) _exit(128+sig);
		backtrace_symbols_fd(
			bt,
			backtrace(bt,sizeof(bt)/sizeof(void*)),
			STDERR_FILENO
		);
		_exit(128+sig);
	}

	static void __attribute((constructor)) InstallCrashHandler(void)
	{
		signal(SIGFPE,CrashHandler);
		signal(SIGSEGV,CrashHandler);
	}
#endif


//==============================================================================
//================================ class emMain ================================
//==============================================================================

class emMain : public emEngine, private emMiniIpcServer {

public:

	emMain(emContext & context, bool serve=true);

	virtual ~emMain();

	static emString CalcServerName();

	void NewWindow(int argc, const char * const argv[]);

protected:

	virtual bool Cycle();

private:

	virtual void OnReception(int argc, const char * const argv[]);

	void CreateFSDialog();
	bool IsFSDialogDone();
	void SetFSDialogDone();
	emString GetLAPath();

	emContext & Context;

	bool FSDialogDone;
	emTkDialog * FSDialog;
	emArray<emString> FSArgs;
};



emMain::emMain(emContext & context, bool serve)
	: emEngine(context.GetScheduler()),
	emMiniIpcServer(context.GetScheduler()), Context(context)

{
	emString name;

	FSDialogDone=false;
	FSDialog=NULL;
	if (serve) {
		name=CalcServerName();
		emDLog("emMain: MiniIPC server name is \"%s\".",name.Get());
		StartServing(name);
	}
}


emMain::~emMain()
{
}


emString emMain::CalcServerName()
{
	const char * p, * q;
	emString h,d,s;

	p=getenv("DISPLAY");
	if (!p) p="";
	q=strrchr(p,':');
	if (!q) h=emString(p);
	else {
		h=emString(p,q-p);
		q++;
		p=strrchr(q,'.');
		if (!p) d=q;
		else {
			d=emString(q,p-q);
			p++;
			s=emString(p);
		}
	}
	if (h=="localhost") h="";
	if (h=="127.0.0.1") h="";
	if (h==emGetHostName()) h="";
	if (d=="") d="0";
	if (s=="") s="0";

	return emString::Format("eaglemode_on_%s:%s.%s",h.Get(),d.Get(),s.Get());
}


void emMain::NewWindow(int argc, const char * const argv[])
{
	const char * opt, * optGeometry, * optCEColor, * optVisit;
	bool optFullscreen,optUndecorated;
	emMainWindow * w;
	emColor col;
	int i;

	if (!IsFSDialogDone()) {
		if (!FSDialog) {
			FSArgs.SetCount(argc,true);
			for (i=0; i<argc; i++) FSArgs.Set(i,emString(argv[i]));
			CreateFSDialog();
		}
		return;
	}

	optGeometry=NULL;
	optCEColor=NULL;
	optFullscreen=false;
	optUndecorated=false;
	optVisit=NULL;
	for (i=0; i<argc; ) {
		opt=argv[i++];
		while (opt[0]=='-' && opt[1]=='-') opt++;
		if (strcmp(opt,"-geometry")==0 && i<argc) {
			optGeometry=argv[i++];
		}
		else if (strcmp(opt,"-cecolor")==0 && i<argc) {
			optCEColor=argv[i++];
		}
		else if (strcmp(opt,"-fullscreen")==0) {
			optFullscreen=true;
		}
		else if (strcmp(opt,"-undecorated")==0) {
			optUndecorated=true;
		}
		else if (strcmp(opt,"-visit")==0 && i<argc) {
			optVisit=argv[i++];
		}
		else if (strcmp(opt,"-setenv")==0 && i+1<argc) {
#			if defined(_WIN32) && !defined(__CYGWIN__)
				SetEnvironmentVariable(argv[i],argv[i+1]);
#			else
				setenv(argv[i],argv[i+1],1);
#			endif
			i+=2;
		}
		else {
			emWarning("emMain::NewWindow: Illegal option: \"%s\".",opt);
		}
	}

	w=new emMainWindow(Context);
	if (optUndecorated) {
		w->SetWindowFlags(w->GetWindowFlags()|emWindow::WF_UNDECORATED);
	}
	if (optGeometry) {
		if (!w->SetWinPosViewSize(optGeometry)) {
			emWarning(
				"emMain::NewWindow: Illegal geometry option value: \"%s\".",
				optGeometry
			);
		}
	}
	if (optCEColor) {
		try {
			col.TryParse(optCEColor);
		}
		catch (emString errorMessage) {
			emWarning(
				"emMain::NewWindow: Could not interpret cecolor option value: %s",
				errorMessage.Get()
			);
			col=0;
		}
		if (col.GetAlpha()) w->GetMainPanel().SetControlEdgesColor(col);
	}
	if (optFullscreen) {
		w->SetWindowFlags(w->GetWindowFlags()|emWindow::WF_FULLSCREEN);
	}
	if (optVisit) {
		w->GetContentView().SeekFullsized(optVisit,false);
	}
}


bool emMain::Cycle()
{
	emArray<const char *> args;
	int i;

	if (FSDialog && IsSignaled(FSDialog->GetFinishSignal())) {
		if (FSDialog->GetResult()==emTkDialog::POSITIVE) {
			delete FSDialog;
			FSDialog=NULL;
			SetFSDialogDone();
			args.SetCount(FSArgs.GetCount(),true);
			for (i=0; i<args.GetCount(); i++) args.Set(i,FSArgs[i].Get());
			NewWindow(args.GetCount(),args.Get());
			FSArgs.Empty(true);
		}
		else {
			emEngine::GetScheduler().InitiateTermination(-1);
		}
	}
	return false;
}


void emMain::OnReception(int argc, const char * const argv[])
{
	emString str;
	int i;

	if (argc>=1 && strcmp(argv[0],"NewWindow")==0) {
		NewWindow(argc-1,argv+1);
	}
	else if (argc==1 && strcmp(argv[0],"ReloadFiles")==0) {
		Signal(emFileModel::AcquireUpdateSignalModel(Context.GetRootContext())->Sig);
	}
	else {
		str.Empty();
		for (i=0; i<argc; i++) { str+=" "; str+=argv[i]; }
		emWarning("emMain: Illegal MiniIpc request:%s",str.Get());
	}
}


void emMain::CreateFSDialog()
{
	static const char * const textFormat=
		"This seems to be your first start of Eagle Mode. Before continuing,\n"
		"you really should have read at least these documents on Eagle Mode:\n"
		"\n"
		"  * License\n"
		"  * System Requirements\n"
		"  * General User Guide (especially the Navigation chapter)\n"
		"\n"
		"For reading the documents, please open this file in a web-browser:\n"
		"\n"
		"  %s\n"
		"\n"
		"Or visit the homepage (may not be compatible if this version of\n"
		"Eagle Mode is out of date):\n"
		"\n"
		"  http://eaglemode.sourceforge.net/\n"
		"\n"
		"By continuing, you are accepting the license."
	;
	emTkLabel * label;
	emString docIndexPath;
	emString text;

	if (FSDialog) return;

	docIndexPath=emGetInstallPath(EM_IDT_HTML_DOC,"emMain","index.html");
	if (!emIsExistingPath(docIndexPath)) {
		emFatalError("File not found: %s",docIndexPath.Get());
	}
	text=emString::Format(textFormat,docIndexPath.Get());

	FSDialog=new emTkDialog(
		Context,
		emView::VF_ROOT_SAME_TALLNESS|emView::VF_NO_ZOOM,
		0
	);
	FSDialog->SetRootTitle("Eagle Mode");
	FSDialog->SetWindowIcon(
		emGetInsResImage(Context.GetRootContext(),"icons","eaglemode48.tga")
	);
	FSDialog->AddPositiveButton("Continue");
	FSDialog->AddNegativeButton("Exit");
	FSDialog->GetButton(1)->ActivateLater();
	label=new emTkLabel(FSDialog->GetContentTiling(),"text");
	label->SetCaption(text);
	AddWakeUpSignal(FSDialog->GetFinishSignal());
}


bool emMain::IsFSDialogDone()
{
	emArray<char> buf;

	if (FSDialogDone) return true;
	try {
		buf=emTryLoadFile(GetLAPath());
	}
	catch (emString) {
		return false;
	}
	if (emString(buf.Get(),buf.GetCount())!="yes\n") {
		return false;
	}
	FSDialogDone=true;
	return true;
}


void emMain::SetFSDialogDone()
{
	if (FSDialogDone) return;
	try {
		emTryMakeDirectories(emGetParentPath(GetLAPath()));
		emTrySaveFile(GetLAPath(),"yes\n",4);
	}
	catch (emString errorMessage) {
		emFatalError("%s",errorMessage.Get());
	}
	FSDialogDone=true;
}


emString emMain::GetLAPath()
{
	return emGetInstallPath(EM_IDT_USER_CONFIG,"emMain","LicenseAccepted");
}


//==============================================================================
//=========================== main/WinMain function ============================
//==============================================================================

MAIN_OR_WINMAIN_HERE

static int wrapped_main(int argc, char * argv[])
{
	emArray<const char *> forwardArgs, sendArgs;
	const char * opt, * id;
	bool optNoClient,optNoServer,optReload;
	int i;

#	if defined(_WIN32)
		SetErrorMode(SEM_FAILCRITICALERRORS|SEM_NOOPENFILEERRORBOX);
#	endif

	emInitLocale();

	optNoClient=false;
	optNoServer=false;
	optReload=false;
	for (i=1; i<argc; ) {
		opt=argv[i++];
		while (opt[0]=='-' && opt[1]=='-') opt++;
		if (strcmp(opt,"-help")==0 || strcmp(opt,"-h")==0) {
			printf(
				"Usage:\n"
				"  %s [<option>]...\n"
				"Options:\n"
				"  -help                   Print this help and exit.\n"
				"  -version                Print version and exit.\n"
				"  -dlog                   Enable debug messages (for this process).\n"
				"  -noclient               Force to run the window by the new process,\n"
				"                          instead of trying to join with another process\n"
				"                          of same user, host and display.\n"
				"  -noserver               Do not allow joining with this process.\n"
				"  -geometry <geometry>    Set geometry of window (e.g. \"700x500+10+10\").\n"
				"  -cecolor <color>        Set color of unused areas beside control view\n"
				"                          (could be used to indicate root privileges).\n"
				"  -fullscreen             Show the window in fullscreen mode.\n"
				"  -undecorated            Show the window without decorations.\n"
				"  -visit <panel identity> Panel to be visited initially.\n"
				"  -reload                 Just tell the server process to reload files.\n",
				argv[0]
			);
			return 0;
		}
		else if (strcmp(opt,"-version")==0) {
			printf(
				"Eagle Mode %s\n",
				emGetVersion()
			);
			return 0;
		}
		else if (strcmp(opt,"-dlog")==0) {
			emEnableDLog();
		}
		else if (strcmp(opt,"-noclient")==0) {
			optNoClient=true;
		}
		else if (strcmp(opt,"-noserver")==0) {
			optNoServer=true;
		}
		else if (strcmp(opt,"-geometry")==0 && i<argc) {
			forwardArgs.Add(opt);
			forwardArgs.Add(argv[i++]);
		}
		else if (strcmp(opt,"-cecolor")==0 && i<argc) {
			forwardArgs.Add(opt);
			forwardArgs.Add(argv[i++]);
		}
		else if (strcmp(opt,"-fullscreen")==0) {
			forwardArgs.Add(opt);
		}
		else if (strcmp(opt,"-undecorated")==0) {
			forwardArgs.Add(opt);
		}
		else if (strcmp(opt,"-visit")==0 && i<argc) {
			forwardArgs.Add(opt);
			forwardArgs.Add(argv[i++]);
		}
		else if (strcmp(opt,"-reload")==0) {
			optReload=true;
		}
		else {
			fprintf(stderr,"Illegal option: %s\n",opt);
			return 1;
		}
	}

	if (optReload) {
		sendArgs=forwardArgs;
		sendArgs.Insert(0,"ReloadFiles");
		try {
			emMiniIpcClient::TrySend(
				emMain::CalcServerName(),
				sendArgs.GetCount(),
				sendArgs.Get()
			);
		}
		catch (emString) {
			fprintf(stderr,"Failed to find server process.\n");
			return 1;
		}
		return 0;
	}

	if (!optNoClient) {
		sendArgs=forwardArgs;
		sendArgs.Insert(0,"NewWindow");
		if ((id=getenv("DESKTOP_STARTUP_ID"))!=NULL) {
			sendArgs.Add("-setenv");
			sendArgs.Add("DESKTOP_STARTUP_ID");
			sendArgs.Add(id);
		}
		try {
			emMiniIpcClient::TrySend(
				emMain::CalcServerName(),
				sendArgs.GetCount(),
				sendArgs.Get()
			);
			return 0;
		}
		catch (emString) {
		}
	}

	emGUIFramework framework;

	framework.EnableAutoTermination();

	emMain mn(framework.GetRootContext(),!optNoServer);
	mn.NewWindow(forwardArgs.GetCount(),forwardArgs.Get());

	return framework.Run();
}
