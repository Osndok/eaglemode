//------------------------------------------------------------------------------
// emShowStdDlg.cpp
//
// Copyright (C) 2006-2010 Oliver Hamann.
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

#include <emCore/emRes.h>
#include <emCore/emToolkit.h>
#include <emCore/emGUIFramework.h>


class emStdDlg : public emTkDialog {

public:

	emStdDlg(emContext & parentContext, int argc, char * argv[]);

protected:

	virtual void Finished(int result);

private:

	void PrintHelp();

	emTkLabel * Label;
	emTkTextField * TextField;
};


emStdDlg::emStdDlg(emContext & parentContext, int argc, char * argv[])
	: emTkDialog(parentContext)
{
	emTkTiling * t;
	int i;

	Label=NULL;
	TextField=NULL;

	i=1;
	if (i+1<argc && strcmp(argv[i],"-geometry")==0) {
		SetWinPosViewSize(argv[i+1]);
		i+=2;
	}

	GetContentTiling()->SetFixedColumnCount(1);
	GetContentTiling()->SetPrefChildTallness(0.4);
	GetContentTiling()->SetPrefChildTallness(0.2,-1);

	if (i+3==argc && strcmp(argv[i],"message")==0) {
		SetRootTitle(argv[i+1]);
		Label=new emTkLabel(GetContentTiling(),"l",argv[i+2]);
		AddOKButton();
	}
	else if (i+3==argc && strcmp(argv[i],"confirm")==0) {
		SetRootTitle(argv[i+1]);
		Label=new emTkLabel(GetContentTiling(),"l",argv[i+2]);
		AddOKCancelButtons();
	}
	else if (
		i+4==argc &&
		(strcmp(argv[i],"edit")==0 || strcmp(argv[i],"pwedit")==0)
	) {
		SetRootTitle(argv[i+1]);
		AddOKCancelButtons();
		Label=new emTkLabel(GetContentTiling(),"l",argv[i+2]);
		t=new emTkTiling(GetContentTiling(),"t");
		t->SetOuterSpace(0.05,0.4,0.05,0.4);
		TextField=new emTkTextField(t,"e");
		TextField->SetEditable();
		TextField->SetText(argv[i+3]);
		TextField->SelectAll(false);
		TextField->ActivateLater();
		if (strcmp(argv[i],"pwedit")==0) TextField->SetPasswordMode();
	}
	else {
		PrintHelp();
		exit(2);
	}

	SetWindowIcon(emGetInsResImage(GetRootContext(),"icons","em-dialog48.tga"));
}


void emStdDlg:: Finished(int result)
{
	if (result==POSITIVE) {
		if (TextField) printf("%s\n",TextField->GetText().Get());
		GetScheduler().InitiateTermination(0);
	}
	else {
		GetScheduler().InitiateTermination(1);
	}
}


void emStdDlg::PrintHelp()
{
	printf("Usage: emShowStdDlg [<options>] <command>\n");
	printf("Options:\n");
	printf("  -geometry <geometry>\n");
	printf("Commands:\n");
	printf("  message <title> <text>\n");
	printf("  confirm <title> <text>\n");
	printf("  edit <title> <text> <edited text>\n");
	printf("  pwedit <title> <text> <edited text>\n");
	printf("Exit status:\n");
	printf("  0 Ok button pressed, resulting edited text printed to stdout.\n");
	printf("  1 Cancel button pressed or window closed, nothing printed.\n");
	printf("  2 Error with command syntax.\n");
}


MAIN_OR_WINMAIN_HERE

#if defined(__GNUC__) && defined(_WIN32)
	int _CRT_glob=0;
#endif

static int wrapped_main(int argc, char * argv[])
{
	emInitLocale();

	emGUIFramework framework;

	emStdDlg dlg(framework.GetRootContext(),argc,argv);

	return framework.Run();
}
