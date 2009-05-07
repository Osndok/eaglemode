//------------------------------------------------------------------------------
// emTreeDumpUtil.cpp
//
// Copyright (C) 2007-2009 Oliver Hamann.
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

#include <emCore/emInstallInfo.h>
#include <emCore/emFileModel.h>
#include <emCore/emPanel.h>
#include <emTreeDump/emTreeDumpUtil.h>

#if defined(__GNUC__) && ((__GNUC__==3 && __GNUC_MINOR__>=1) || __GNUC__>=4)
#	define HAVE_CXXABI_DEMANGLE
#endif

#if defined(HAVE_CXXABI_DEMANGLE)
#	include <cxxabi.h>
#endif


static emString emTreeDump_GetClassName(const type_info & t)
{
#if defined(HAVE_CXXABI_DEMANGLE)
	emString name;
	char * buf;
	int status;

	buf=abi::__cxa_demangle(t.name(),NULL,0,&status);
	if (buf!=NULL && status==0) name=buf;
	else name=t.name();
	if (buf) free(buf);
	return name;
#else
	return t.name();
#endif
}


static int emTreeDump_CmpModelsForSorting(
	emModel * const * obj1, emModel * const * obj2, void * context
)
{
	emString c1,c2;
	int i;

	c1=emTreeDump_GetClassName(typeid(**obj1));
	c2=emTreeDump_GetClassName(typeid(**obj2));
	i=strcmp(c1,c2);
	if (!i) {
		i=strcmp(
			(*obj1)->GetName().Get(),
			(*obj2)->GetName().Get()
		);
	}
	return i;
}


void emTreeDumpFromObject(emEngine * object, emTreeDumpRec * rec)
{
	emEngine * asEngine;
	emContext * asContext;
	emView * asView;
	emWindow * asWindow;
	emPanel * asPanel;
	emModel * asModel;
	emFileModel * asFileModel;
	emString className,title,text;
	emModel * * arrayOfCommon;
	emContext * childContext;
	emPanel * childPanel;
	int i,n,commonCount,privateCount;
	double d1,d2,d3,d4;
	char tmp[64];

	className=emTreeDump_GetClassName(typeid(*object));
	title=className;
	text=emString::Format(
		"Class: %s\n"
		"Address: %p",
		className.Get(),
		(void*)object
	);

	asEngine=dynamic_cast<emEngine*>(object);
	if (asEngine) {
		rec->BgColor=emColor(0,0,0);
		rec->FgColor=emColor(238,238,238);
		rec->Frame=emTreeDumpRec::FRAME_RECTANGLE;
		text+=emString::Format(
			"\nEngine Priority: %d",
			(int)asEngine->GetEnginePriority()
		);
	}

	asContext=dynamic_cast<emContext*>(object);
	if (asContext) {
		title=asContext->GetParentContext() ? "Context:\n" : "Root Context:\n";
		title+=className;
		rec->BgColor=emColor(119,119,119);
		rec->FgColor=emColor(238,238,238);
		rec->Frame=emTreeDumpRec::FRAME_ELLIPSE;
		asContext->GetModelInfo(&commonCount,&privateCount,&arrayOfCommon);
		emSortArray(
			arrayOfCommon,
			commonCount,
			emTreeDump_CmpModelsForSorting,
			(void*)NULL
		);
		n=rec->Children.GetCount();
		rec->Children.SetCount(n+commonCount);
		for (i=0; i<commonCount; i++) {
			emTreeDumpFromObject(arrayOfCommon[i],&rec->Children[n+i]);
		}
		delete [] arrayOfCommon;
		text+=emString::Format(
			"\nCommon Models: %d"
			"\nPrivate Models: %d (not listed)",
			commonCount,
			privateCount
		);
		childContext=asContext->GetFirstChildContext();
		while (childContext) {
			n=rec->Children.GetCount();
			rec->Children.SetCount(n+1);
			emTreeDumpFromObject(childContext,&rec->Children[n]);
			childContext=childContext->GetNextContext();
		}
	}

	asView=dynamic_cast<emView*>(object);
	if (asView) {
		title="View (Context):\n" + className;
		rec->BgColor=emColor(68,136,136);
		rec->FgColor=asView->IsFocused() ? emColor(238,238,68) : emColor(238,238,238);
		rec->Frame=emTreeDumpRec::FRAME_ROUND_RECT;

		text+="\nView Flags: ";
		i=0;
		if ((asView->GetViewFlags()&emView::VF_POPUP_ZOOM         )!=0) {
			if (i) text+=", "; else i=1;
			text+="VF_POPUP_ZOOM";
		}
		if ((asView->GetViewFlags()&emView::VF_ROOT_SAME_TALLNESS )!=0) {
			if (i) text+=", "; else i=1;
			text+="VF_ROOT_SAME_TALLNESS";
		}
		if ((asView->GetViewFlags()&emView::VF_NO_ZOOM            )!=0) {
			if (i) text+=", "; else i=1;
			text+="VF_NO_ZOOM";
		}
		if ((asView->GetViewFlags()&emView::VF_NO_USER_NAVIGATION )!=0) {
			if (i) text+=", "; else i=1;
			text+="VF_NO_USER_NAVIGATION";
		}
		if ((asView->GetViewFlags()&emView::VF_NO_FOCUS_HIGHLIGHT )!=0) {
			if (i) text+=", "; else i=1;
			text+="VF_NO_FOCUS_HIGHLIGHT";
		}
		if ((asView->GetViewFlags()&emView::VF_NO_ACTIVE_HIGHLIGHT)!=0) {
			if (i) text+=", "; else i=1;
			text+="VF_NO_ACTIVE_HIGHLIGHT";
		}
		if ((asView->GetViewFlags()&emView::VF_EGO_MODE           )!=0) {
			if (i) text+=", "; else i=1;
			text+="VF_EGO_MODE";
		}
		if ((asView->GetViewFlags()&emView::VF_STRESS_TEST        )!=0) {
			if (i) text+=", "; else i=1;
			text+="VF_STRESS_TEST";
		}
		if (!i) text+="0";

		text+="\nTitle: ";
		text+=asView->GetTitle();
		text+=emString::Format(
			"\nFocused: %s"
			"\nVisit Adherent: %s"
			"\nPopped Up: %s"
			"\nBackground Color: 0x%08X"
			"\nHome XYWH: %.9G, %.9G, %.9G, %.9G"
			"\nCurrent XYWH: %.9G, %.9G, %.9G, %.9G",
			(asView->IsFocused() ? "yes" : "no"),
			(asView->IsVisitAdherent() ? "yes" : "no"),
			(asView->IsPoppedUp() ? "yes" : "no"),
			(int)asView->GetBackgroundColor(),
			asView->GetHomeX(),
			asView->GetHomeY(),
			asView->GetHomeWidth(),
			asView->GetHomeHeight(),
			asView->GetCurrentX(),
			asView->GetCurrentY(),
			asView->GetCurrentWidth(),
			asView->GetCurrentHeight()
		);
		childPanel=asView->GetRootPanel();
		if (childPanel) {
			n=rec->Children.GetCount();
			rec->Children.SetCount(n+1);
			emTreeDumpFromObject(childPanel,&rec->Children[n]);
		}
	}

	asWindow=dynamic_cast<emWindow*>(object);
	if (asWindow) {
		title="Window (View, Context):\n"+className;
		rec->BgColor=emColor(34,34,136);

		text+="\nWindow Flags: ";
		i=0;
		if ((asWindow->GetWindowFlags()&emWindow::WF_MODAL)!=0) {
			if (i) text+=", "; else i=1;
			text+="WF_MODAL";
		}
		if ((asWindow->GetWindowFlags()&emWindow::WF_UNDECORATED)!=0) {
			if (i) text+=", "; else i=1;
			text+="WF_UNDECORATED";
		}
		if ((asWindow->GetWindowFlags()&emWindow::WF_POPUP)!=0) {
			if (i) text+=", "; else i=1;
			text+="WF_POPUP";
		}
		if ((asWindow->GetWindowFlags()&emWindow::WF_FULLSCREEN)!=0) {
			if (i) text+=", "; else i=1;
			text+="WF_FULLSCREEN";
		}
		if (!i) text+="0";

		text+="\nWMResName: ";
		text+=asWindow->GetWMResName();
	}

	asPanel=dynamic_cast<emPanel*>(object);
	if (asPanel) {
		title="Panel:\n"+className+"\n\""+asPanel->GetName() + "\"";
		if (asPanel->IsViewed()) rec->BgColor=emColor(51,136,51);
		else if (asPanel->IsInViewedPath()) rec->BgColor=emColor(34,85,34);
		else rec->BgColor=emColor(68,85,68);
		if (asPanel->IsInFocusedPath()) rec->FgColor=emColor(238,238,68);
		else if (asPanel->IsInActivePath()) rec->FgColor=emColor(238,238,136);
		else if (asPanel->IsInVisitedPath()) rec->FgColor=emColor(238,170,170);
		else rec->FgColor=emColor(238,238,238);
		rec->Frame=emTreeDumpRec::FRAME_RECTANGLE;
		text+="\nName: "; text+=asPanel->GetName();
		text+="\nCreationNumber: ";
		n=emUInt64ToStr(tmp,sizeof(tmp),asPanel->GetCreationNumber());
		text+=emString(tmp,n);
		text+="\nTitle: "; text+=asPanel->GetTitle();
		text+=emString::Format(
			"\nLayout XYWH: %.9G, %.9G, %.9G, %.9G",
			asPanel->GetLayoutX(),
			asPanel->GetLayoutY(),
			asPanel->GetLayoutWidth(),
			asPanel->GetLayoutHeight()
		);
		text+=emString::Format("\nHeight: %.9G",asPanel->GetHeight());
		asPanel->GetEssenceRect(&d1,&d2,&d3,&d4),
		text+=emString::Format(
			"\nEssence XYWH: %.9G, %.9G, %.9G, %.9G",
			d1,d2,d3,d4
		);
		text+=emString::Format("\nViewed: %s",asPanel->IsViewed()?"yes":"no");
		text+=emString::Format("\nInViewedPath: %s",asPanel->IsInViewedPath()?"yes":"no");
		text+="\nViewed XYWH: ";
		if (asPanel->IsViewed()) {
			text+=emString::Format(
				"%.9G, %.9G, %.9G, %.9G",
				asPanel->GetViewedX(),
				asPanel->GetViewedY(),
				asPanel->GetViewedWidth(),
				asPanel->GetViewedHeight()
			);
		}
		else {
			text+="-";
		}
		text+="\nClip X1Y1X2Y2: ";
		if (asPanel->IsViewed()) {
			text+=emString::Format(
				"%.9G, %.9G, %.9G, %.9G",
				asPanel->GetClipX1(),
				asPanel->GetClipY1(),
				asPanel->GetClipX2(),
				asPanel->GetClipY2()
			);
		}
		else {
			text+="-";
		}
		text+=emString::Format("\nEnableSwitch: %s",asPanel->GetEnableSwitch()?"yes":"no");
		text+=emString::Format("\nEnabled: %s",asPanel->IsEnabled()?"yes":"no");
		text+=emString::Format("\nFocusable: %s",asPanel->IsFocusable()?"yes":"no");
		text+=emString::Format("\nVisited: %s",asPanel->IsVisited()?"yes":"no");
		text+=emString::Format("\nInVisitedPath: %s",asPanel->IsInVisitedPath()?"yes":"no");
		text+=emString::Format("\nActive: %s",asPanel->IsActive()?"yes":"no");
		text+=emString::Format("\nInActivePath: %s",asPanel->IsInActivePath()?"yes":"no");
		text+=emString::Format("\nFocused: %s",asPanel->IsFocused()?"yes":"no");
		text+=emString::Format("\nInFocusedPath: %s",asPanel->IsInFocusedPath()?"yes":"no");
		text+=emString::Format("\nUpdate Priority: %.9G",asPanel->GetUpdatePriority());
		text+=emString::Format("\nMemory Limit: %lu",(unsigned long)asPanel->GetMemoryLimit());
		childPanel=asPanel->GetFirstChild();
		while (childPanel) {
			n=rec->Children.GetCount();
			rec->Children.SetCount(n+1);
			emTreeDumpFromObject(childPanel,&rec->Children[n]);
			childPanel=childPanel->GetNext();
		}
	}

	asModel=dynamic_cast<emModel*>(object);
	if (asModel) {
		title="Common Model:\n"+className+"\n\""+asModel->GetName() + "\"";
		rec->BgColor=emColor(68,0,0);
		rec->FgColor=emColor(187,187,187);
		rec->Frame=emTreeDumpRec::FRAME_HEXAGON;
		text+=emString::Format(
			"\nName: %s"
			"\nMin Common Lifetime: %d",
			 asModel->GetName().Get(),
			 (int)asModel->GetMinCommonLifetime()
		);
	}

	asFileModel=dynamic_cast<emFileModel*>(object);
	if (asFileModel) {
		title="Common File Model:\n"+className+"\n\""+asModel->GetName() + "\"";
		rec->BgColor=emColor(68,0,51);
		rec->FgColor=emColor(187,187,187);
		rec->Frame=emTreeDumpRec::FRAME_HEXAGON;
		text+="\nFile Path: ";
		text+=asFileModel->GetFilePath();
		text+=emString::Format(
			"\nFile State: %s"
			"\nMemory Need: %lu",
			asFileModel->GetFileState()==emFileModel::FS_WAITING ? "FS_WAITING" :
			asFileModel->GetFileState()==emFileModel::FS_LOADING ? "FS_LOADING" :
			asFileModel->GetFileState()==emFileModel::FS_LOADED ? "FS_LOADED" :
			asFileModel->GetFileState()==emFileModel::FS_UNSAVED ? "FS_UNSAVED" :
			asFileModel->GetFileState()==emFileModel::FS_SAVING ? "FS_SAVING" :
			asFileModel->GetFileState()==emFileModel::FS_TOO_COSTLY ? "FS_TOO_COSTLY" :
			asFileModel->GetFileState()==emFileModel::FS_LOAD_ERROR ? "FS_LOAD_ERROR" :
			asFileModel->GetFileState()==emFileModel::FS_SAVE_ERROR ? "FS_SAVE_ERROR" :
			"unknown",
			(unsigned long)asFileModel->GetMemoryNeed()
		);
	}

	rec->Title=title;
	rec->Text=text;
}


void emTreeDumpFromRootContext(emRootContext * rootContext, emTreeDumpRec * rec)
{
	char tmp[256];
	emString tstr;
	emUInt64 cputsc;
	time_t t;
	int i,chr;

	t=time(NULL);
	tstr=ctime_r(&t,tmp);

	for (i=tstr.GetLen(); i>0 && (emByte)tstr[i-1]<=32; i--) {
		tstr=tstr.GetSubString(0,i-1);
	}

	chr=(int)(char)-1;
	cputsc=emGetCPUTSC();

	rec->SetToDefault();
	rec->Title="Tree Dump\nof the top-level objects\nof a running emCore-based program";
	rec->Text=
		"General Info"
		"\n~~~~~~~~~~~~"
		"\n"
		"\nTime       : " + tstr +
		"\nHost Name  : " + emGetHostName() +
		"\nUser Name  : " + emGetUserName() +
		"\nProcess Id : " + emString::Format("%d",emGetProcessId()) +
		"\nCurrent Dir: " + emGetCurrentDirectory() +
		"\nUTF8       : " + (emIsUtf8System() ? "yes" : "no") +
		"\nByte Order : " + emString::Format("%d",(int)EM_BYTE_ORDER) +
		"\nsizeof(ptr): " + emString::Format("%d",(int)sizeof(void*)) +
		"\nsizeof(lng): " + emString::Format("%d",(int)sizeof(long)) +
		"\nchar       : " + emString(chr<0?"signed":"unsigned") +
		"\nCPU-TSC    : " + emString::Format("0x%08x%08x",(emUInt32)(cputsc>>32),(emUInt32)cputsc) +
		"\n"
		"\nPaths of emCore:"
		"\nBin        : " + emGetInstallPath(EM_IDT_BIN        ,"emCore") +
		"\nInclude    : " + emGetInstallPath(EM_IDT_INCLUDE    ,"emCore") +
		"\nLib        : " + emGetInstallPath(EM_IDT_LIB        ,"emCore") +
		"\nHtml Doc   : " + emGetInstallPath(EM_IDT_HTML_DOC   ,"emCore") +
		"\nPs Doc     : " + emGetInstallPath(EM_IDT_PS_DOC     ,"emCore") +
		"\nUser Config: " + emGetInstallPath(EM_IDT_USER_CONFIG,"emCore") +
		"\nHost Config: " + emGetInstallPath(EM_IDT_HOST_CONFIG,"emCore") +
		"\nTmp        : " + emGetInstallPath(EM_IDT_TMP        ,"emCore") +
		"\nRes        : " + emGetInstallPath(EM_IDT_RES        ,"emCore") +
		"\nHome       : " + emGetInstallPath(EM_IDT_HOME       ,"emCore")
	;
	rec->BgColor=emColor(68,68,102);
	rec->FgColor=emColor(187,187,238);
	rec->Frame=emTreeDumpRec::FRAME_RECTANGLE;
	rec->Children.SetCount(1);
	emTreeDumpFromObject(rootContext,&rec->Children[0]);
}


void emTryTreeDumpFileFromRootContext(
	emRootContext * rootContext, const char * filename
) throw(emString)
{
	emTreeDumpRec rec;

	emTreeDumpFromRootContext(rootContext,&rec);
	rec.TrySave(filename);
}


extern "C" {
	bool emTreeDumpFileFromRootContext(
		emRootContext * rootContext, const char * filename,
		emString * errorBuf
	)
	{
		try {
			emTryTreeDumpFileFromRootContext(rootContext,filename);
		}
		catch (emString errorMessage) {
			*errorBuf=errorMessage;
			return false;
		}
		errorBuf->Empty();
		return true;
	}
}
