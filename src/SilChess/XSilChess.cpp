//------------------------------------------------------------------------------
// XSilChess.cpp - X11 version of SilChess
//
// Copyright (C) 2001-2005,2007-2009 Oliver Hamann.
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <Xm/XmAll.h>
#include <X11/IntrinsicP.h>
#include <SilChess/SilChessRayTracer.h>


//==============================================================================
//============================== XSilChessWindow ===============================
//==============================================================================

class XSilChessWindow {

public:

	XSilChessWindow(XtAppContext app, Widget toplevel, Visual * vsl,
	               int vsldepth, Colormap cmap);

	~XSilChessWindow();

private:

	static void HandleCallback(Widget widget, XtPointer client_data,
	                           XtPointer call_data);

	static void HandleEvent(Widget widget, XtPointer data,
	                        XEvent * event, Boolean *);

	void HandleCallbackOrEvent(Widget widget,
	                           XmAnyCallbackStruct * cbs,
	                           XEvent * event);

	void MousePress(int x, int y);

	void DoSearching();

	void UpdateStatusBar();

	void UpdateMovesList();

	void UpdateView();

	void UpdateDepthMenu();

	void DoPainting();

	void PaintSel();

	SilChessMachine * Machine;
	int SelX,SelY;
	char OverwriteFile[1024];

	bool IsSearching,AbortSearching;
	bool NeedPainting,IsPainting;
	bool HintWanted,HintValid;
	SilChessMachine::Move Hint;

	XtAppContext App;
	Display * Disp;
	Visual * Vsl, * DlgVsl;
	int VslDepth, DlgVslDepth;
	Colormap CMap, DlgCMap;
	int PixelSize,RedMask,GreenMask,BlueMask;
	Widget TopLevel,MainWin,MainMenu,
	       FileMenu,BFileLoad,BFileSave,BFileExit,BFile,
	       GameMenu,BGameNew,BGameFlip,BGameUndo,BGameList,BGame,
	       DepthMenu,BDepth[SilChessMachine::MAX_SEARCH_DEPTH+1],
	       CompMenu,BCompHint,BCompDepth,BComp,HelpMenu,BHelp,
	       BHelpAbout,MainForm,StatusFrame,StatusLabel,ViewFrame,
	       ViewArea,LoadDialog,SaveDialog,OverwriteDialog,ErrorBox,
	       ListDialogPopup,ListDialog,LDClose,LDScroll,LDList,
	       AboutDialog;
	Dimension ViewWidth,ViewHeight;
	Window ViewWin;
	GC ViewGC;
	SilChessRayTracer RT;

	static const char * const AboutText;
};


XSilChessWindow::XSilChessWindow(XtAppContext app, Widget toplevel,
                                 Visual * vsl, int vsldepth, Colormap cmap)
{
	char tmp[512];
	Arg al[10];
	int i;
	XmString xms;

	// Initialize member variables
	App=app;
	TopLevel=toplevel;
	Disp=XtDisplay(TopLevel);
	Vsl=vsl;
	VslDepth=vsldepth;
	CMap=cmap;
	DlgVsl=DefaultVisual(Disp,XScreenNumberOfScreen(XtScreen(TopLevel)));
	DlgVslDepth=DefaultDepth(Disp,XScreenNumberOfScreen(XtScreen(TopLevel)));
	DlgCMap=DefaultColormap(Disp,XScreenNumberOfScreen(XtScreen(TopLevel)));
	PixelSize=(VslDepth<=8 ? 1 : (VslDepth<=16 ? 2 : 4));
	RedMask=Vsl->red_mask;
	GreenMask=Vsl->green_mask;
	BlueMask=Vsl->blue_mask;
	SelX=SelY-1;
	IsSearching=false;
	AbortSearching=false;
	NeedPainting=false;
	IsPainting=false;
	HintWanted=false;
	HintValid=false;

	// Create main window
	MainWin=XtVaCreateManagedWidget(
		"mainWin",xmMainWindowWidgetClass,TopLevel,
		(char*)NULL
	);

	// Create main menu bar
	MainMenu=XmCreateMenuBar(MainWin,(char*)"mainMenu",NULL,0);
	XtManageChild(MainMenu);

	// Create menu item: file
	XtSetArg(al[0],XmNvisual,Vsl);
	XtSetArg(al[1],XmNdepth,VslDepth);
	XtSetArg(al[2],XmNcolormap,CMap);
	FileMenu=XmCreatePulldownMenu(MainMenu,(char*)"fileMenu",al,3);
	BFile=XtVaCreateManagedWidget(
		"file",xmCascadeButtonWidgetClass,MainMenu,
		XmNsubMenuId,FileMenu,
		(char*)NULL
	);

	// Create menu item: file/load
	BFileLoad=XtVaCreateManagedWidget(
		"load",xmPushButtonWidgetClass,FileMenu,
		(char*)NULL
	);
	XtAddCallback(BFileLoad,XmNactivateCallback,HandleCallback,this);

	// Create menu item: file/save
	BFileSave=XtVaCreateManagedWidget(
		"save",xmPushButtonWidgetClass,FileMenu,
		(char*)NULL
	);
	XtAddCallback(BFileSave,XmNactivateCallback,HandleCallback,this);

	// Create menu item: file/exit
	XtVaCreateManagedWidget(
		"separator",xmSeparatorWidgetClass,FileMenu,
		(char*)NULL
	);
	BFileExit=XtVaCreateManagedWidget(
		"exit",xmPushButtonWidgetClass,FileMenu,
		(char*)NULL
	);
	XtAddCallback(BFileExit,XmNactivateCallback,HandleCallback,this);

	// Create menu item: game
	XtSetArg(al[0],XmNvisual,Vsl);
	XtSetArg(al[1],XmNdepth,VslDepth);
	XtSetArg(al[2],XmNcolormap,CMap);
	GameMenu=XmCreatePulldownMenu(MainMenu,(char*)"gameMenu",al,3);
	BGame=XtVaCreateManagedWidget(
		"game",xmCascadeButtonWidgetClass,MainMenu,
		XmNsubMenuId,GameMenu,
		(char*)NULL
	);

	// Create menu item: game/new
	BGameNew=XtVaCreateManagedWidget(
		"new",xmPushButtonWidgetClass,GameMenu,
		(char*)NULL
	);
	XtAddCallback(BGameNew,XmNactivateCallback,HandleCallback,this);

	// Create menu item: game/flip
	BGameFlip=XtVaCreateManagedWidget(
		"flip",xmPushButtonWidgetClass,GameMenu,
		(char*)NULL
	);
	XtAddCallback(BGameFlip,XmNactivateCallback,HandleCallback,this);

	// Create menu item: game/undo
	BGameUndo=XtVaCreateManagedWidget(
		"undo",xmPushButtonWidgetClass,GameMenu,
		(char*)NULL
	);
	XtAddCallback(BGameUndo,XmNactivateCallback,HandleCallback,this);

	// Create menu item: game/list
	BGameList=XtVaCreateManagedWidget(
		"list",xmPushButtonWidgetClass,GameMenu,
		(char*)NULL
	);
	XtAddCallback(BGameList,XmNactivateCallback,HandleCallback,this);

	// Create menu item: computer
	XtSetArg(al[0],XmNvisual,Vsl);
	XtSetArg(al[1],XmNdepth,VslDepth);
	XtSetArg(al[2],XmNcolormap,CMap);
	CompMenu=XmCreatePulldownMenu(MainMenu,(char*)"compMenu",al,3);
	BComp=XtVaCreateManagedWidget(
		"comp",xmCascadeButtonWidgetClass,MainMenu,
		XmNsubMenuId,CompMenu,
		(char*)NULL
	);

	// Create menu item: computer/hint
	BCompHint=XtVaCreateManagedWidget(
		"hint",xmPushButtonWidgetClass,CompMenu,
		(char*)NULL
	);
	XtAddCallback(BCompHint,XmNactivateCallback,HandleCallback,this);

	// Create menu item: computer/depth
	XtSetArg(al[0],XmNvisual,Vsl);
	XtSetArg(al[1],XmNdepth,VslDepth);
	XtSetArg(al[2],XmNcolormap,CMap);
	XtSetArg(al[3],XmNradioBehavior,True);
	DepthMenu=XmCreatePulldownMenu(CompMenu,(char*)"depthMenu",al,4);
	BCompDepth=XtVaCreateManagedWidget(
		"depth",xmCascadeButtonWidgetClass,CompMenu,
		XmNsubMenuId,DepthMenu,
		(char*)NULL
	);

	// Create menu items: computer/depth/1...
	for (i=0; i<=SilChessMachine::MAX_SEARCH_DEPTH; i++) {
		sprintf(tmp,"%d",i);
		BDepth[i]=XtVaCreateManagedWidget(
			tmp,xmToggleButtonWidgetClass,DepthMenu,
			(char*)NULL
		);
		XtAddCallback(BDepth[i],XmNvalueChangedCallback,HandleCallback,this);
	}

	// Create menu item: help
	XtSetArg(al[0],XmNvisual,Vsl);
	XtSetArg(al[1],XmNdepth,VslDepth);
	XtSetArg(al[2],XmNcolormap,CMap);
	HelpMenu=XmCreatePulldownMenu(MainMenu,(char*)"helpMenu",al,3);
	BHelp=XtVaCreateManagedWidget(
		"help",xmCascadeButtonWidgetClass,MainMenu,
		XmNsubMenuId,HelpMenu,
		(char*)NULL
	);
	XtVaSetValues(MainMenu,XmNmenuHelpWidget,BHelp,(char*)NULL);

	// Create menu item: help/about
	BHelpAbout=XtVaCreateManagedWidget(
		"about",xmPushButtonWidgetClass,HelpMenu,
		(char*)NULL
	);
	XtAddCallback(BHelpAbout,XmNactivateCallback,HandleCallback,this);

	// Create a parent for status bar and view
	MainForm=XtVaCreateManagedWidget(
		"mainForm",xmFormWidgetClass,MainWin,
		(char*)NULL
	);

	// Create status line
	StatusFrame=XtVaCreateManagedWidget(
		"statusFrame",xmFrameWidgetClass,MainForm,
		XmNleftAttachment,XmATTACH_FORM,
		XmNrightAttachment,XmATTACH_FORM,
		XmNtopAttachment,XmATTACH_FORM,
		(char*)NULL
	);
	StatusLabel=XtVaCreateManagedWidget(
		"statusLabel",xmLabelWidgetClass,StatusFrame,
		XmNalignment, XmALIGNMENT_BEGINNING,
		(char*)NULL
	);

	// Create the chess board view
	ViewFrame=XtVaCreateManagedWidget(
		"viewFrame",xmFrameWidgetClass,MainForm,
		XmNtopAttachment,XmATTACH_WIDGET,
		XmNtopWidget,StatusFrame,
		XmNleftAttachment,XmATTACH_FORM,
		XmNrightAttachment,XmATTACH_FORM,
		XmNbottomAttachment,XmATTACH_FORM,
		(char*)NULL
	);
	ViewArea=XtVaCreateManagedWidget(
		"viewArea",xmDrawingAreaWidgetClass,ViewFrame,
		XmNtopAttachment,XmATTACH_FORM,
		XmNleftAttachment,XmATTACH_FORM,
		XmNrightAttachment,XmATTACH_FORM,
		XmNbottomAttachment,XmATTACH_FORM,
		(char*)NULL
	);
	XtAddCallback(ViewArea,XmNexposeCallback,HandleCallback,this);
	XtAddCallback(ViewArea,XmNresizeCallback,HandleCallback,this);
	XtAddEventHandler(
		ViewArea,ButtonPressMask|ButtonMotionMask|ButtonReleaseMask|
		StructureNotifyMask,False,HandleEvent,this
	);
	XtVaGetValues(ViewArea,XmNwidth,&ViewWidth,(char*)NULL);
	XtVaGetValues(ViewArea,XmNheight,&ViewHeight,(char*)NULL);
	ViewWin=0;
	ViewGC=NULL;

	// Create dialog: load game
	XtSetArg(al[0],XmNvisual,DlgVsl);
	XtSetArg(al[1],XmNdepth,DlgVslDepth);
	XtSetArg(al[2],XmNcolormap,DlgCMap);
	XtSetArg(al[3],XmNautoUnmanage,True);
	LoadDialog=XmCreateFileSelectionDialog(TopLevel,(char*)"loadDialog",al,4);
	XtAddCallback(LoadDialog,XmNokCallback,HandleCallback,this);
	XtUnmanageChild(XmFileSelectionBoxGetChild(LoadDialog,XmDIALOG_HELP_BUTTON));

	// Create dialog: save game
	XtSetArg(al[0],XmNvisual,DlgVsl);
	XtSetArg(al[1],XmNdepth,DlgVslDepth);
	XtSetArg(al[2],XmNcolormap,DlgCMap);
	XtSetArg(al[3],XmNautoUnmanage, True);
	SaveDialog=XmCreateFileSelectionDialog(TopLevel,(char*)"saveDialog",al,4);
	XtAddCallback(SaveDialog,XmNokCallback,HandleCallback,this);
	XtUnmanageChild(XmFileSelectionBoxGetChild(SaveDialog,XmDIALOG_HELP_BUTTON));

	// Create dialog: file exists, overwrite?
	XtSetArg(al[0],XmNvisual,DlgVsl);
	XtSetArg(al[1],XmNdepth,DlgVslDepth);
	XtSetArg(al[2],XmNcolormap,DlgCMap);
	XtSetArg(al[3],XmNautoUnmanage, True);
	OverwriteDialog=XmCreateWarningDialog(TopLevel,(char*)"overwriteDialog",al,4);
	XtUnmanageChild(XmMessageBoxGetChild(OverwriteDialog,XmDIALOG_HELP_BUTTON));
	XtAddCallback(OverwriteDialog,XmNokCallback,HandleCallback,this);

	// Create dialog: error message
	XtSetArg(al[0],XmNvisual,DlgVsl);
	XtSetArg(al[1],XmNdepth,DlgVslDepth);
	XtSetArg(al[2],XmNcolormap,DlgCMap);
	XtSetArg(al[3],XmNautoUnmanage, True);
	ErrorBox=XmCreateWarningDialog(TopLevel,(char*)"errorBox",al,4);
	XtUnmanageChild(XmMessageBoxGetChild(ErrorBox,XmDIALOG_CANCEL_BUTTON));
	XtUnmanageChild(XmMessageBoxGetChild(ErrorBox,XmDIALOG_HELP_BUTTON));

	// Create dialog: list of moves
	ListDialogPopup=XtVaCreateWidget(
		"listDialog_popup",xmDialogShellWidgetClass,TopLevel,
		XmNvisual,DlgVsl,
		XmNdepth,DlgVslDepth,
		XmNcolormap,DlgCMap,
		(char*)NULL
	);
	ListDialog=XtVaCreateWidget(
		"listDialog",xmFormWidgetClass,ListDialogPopup,
		(char*)NULL
	);
	LDClose=XtVaCreateManagedWidget(
		"close",xmPushButtonWidgetClass,ListDialog,
		XmNleftAttachment,XmATTACH_FORM,
		XmNrightAttachment,XmATTACH_FORM,
		XmNbottomAttachment,XmATTACH_FORM,
		(char*)NULL
	);
	XtAddCallback(LDClose,XmNactivateCallback,HandleCallback,this);
	LDScroll=XtVaCreateManagedWidget(
		"scroll",xmScrolledWindowWidgetClass,ListDialog,
		XmNscrollingPolicy,XmAUTOMATIC,
		XmNscrollBarDisplayPolicy,XmAS_NEEDED,
		XmNleftAttachment,XmATTACH_FORM,
		XmNrightAttachment,XmATTACH_FORM,
		XmNtopAttachment,XmATTACH_FORM,
		XmNbottomAttachment,XmATTACH_WIDGET,
		XmNbottomWidget,LDClose,
		(char*)NULL
	);
	LDList=XtVaCreateManagedWidget(
		"list",xmLabelGadgetClass,LDScroll,
		XmNalignment,XmALIGNMENT_BEGINNING,
		(char*)NULL
	);

	// Create dialog: about
	xms=XmStringCreateLtoR((char*)AboutText,XmFONTLIST_DEFAULT_TAG);
	XtSetArg(al[0],XmNvisual,DlgVsl);
	XtSetArg(al[1],XmNdepth,DlgVslDepth);
	XtSetArg(al[2],XmNcolormap,DlgCMap);
	XtSetArg(al[3],XmNautoUnmanage,True);
	XtSetArg(al[4],XmNmessageString,xms);
	XtSetArg(al[5],XmNmessageAlignment,XmALIGNMENT_CENTER);
	AboutDialog=XmCreateMessageDialog(TopLevel,(char*)"aboutDialog",al,6);
	XmStringFree(xms);
	XtUnmanageChild(XmMessageBoxGetChild(AboutDialog,XmDIALOG_CANCEL_BUTTON));
	XtUnmanageChild(XmMessageBoxGetChild(AboutDialog,XmDIALOG_HELP_BUTTON));

	// Set main window areas
	XmMainWindowSetAreas(MainWin,MainMenu,NULL,NULL,NULL,MainForm);

	// Create chess machine
	Machine = new SilChessMachine();

	// Setup ray tracer
	RT.SetViewSize(ViewWidth,ViewHeight);
	RT.SetWorld(Machine);

	// Update all
	UpdateStatusBar();
	UpdateMovesList();
	UpdateView();
	UpdateDepthMenu();
}


XSilChessWindow::~XSilChessWindow()
{
	delete Machine;
}


void XSilChessWindow::HandleCallback(Widget widget, XtPointer client_data,
                                     XtPointer call_data)
{
	((XSilChessWindow*)client_data)->HandleCallbackOrEvent(
		widget,
		(XmAnyCallbackStruct*)call_data,
		NULL
	);
}


void XSilChessWindow::HandleEvent(Widget widget, XtPointer data,
                                  XEvent * event, Boolean *)
{
	((XSilChessWindow*)data)->HandleCallbackOrEvent(
		widget,
		NULL,
		event
	);
}


void XSilChessWindow::HandleCallbackOrEvent(Widget widget,
                                            XmAnyCallbackStruct * cbs,
                                            XEvent * event)
{
	XGCValues gcval;
	const char * file_name;
	XmString xms;
	char tmp[512];
	int i;

	if (widget==BFileLoad) {
		XtUnmanageChild(LoadDialog);
		XmFileSelectionDoSearch(LoadDialog,NULL);
		XtManageChild(LoadDialog);
	}
	else if (widget==BFileSave) {
		XtUnmanageChild(SaveDialog);
		XmFileSelectionDoSearch(SaveDialog,NULL);
		XtManageChild(SaveDialog);
	}
	else if (widget==BFileExit) {
		exit(0);
	}
	else if (widget==BGameNew) {
		AbortSearching=true;
		HintWanted=false;
		HintValid=false;
		Machine->StartNewGame();
		RT.SetWorld(Machine);
		UpdateStatusBar();
		UpdateMovesList();
		UpdateView();
	}
	else if (widget==BGameFlip) {
		AbortSearching=true;
		HintWanted=false;
		HintValid=false;
		Machine->SetHumanWhite(!Machine->IsHumanWhite());
		RT.SetWorld(Machine);
		UpdateStatusBar();
		UpdateView();
	}
	else if (widget==BGameUndo) {
		AbortSearching=true;
		HintWanted=false;
		HintValid=false;
		Machine->UndoMove();
		if (!Machine->IsHumanOn()) Machine->UndoMove();
		RT.SetWorld(Machine);
		UpdateStatusBar();
		UpdateMovesList();
		UpdateView();
	}
	else if (widget==BGameList) {
		XtUnmanageChild(ListDialog);
		XtManageChild(ListDialog);
	}
	else if (widget==LoadDialog) {
		AbortSearching=true;
		HintWanted=false;
		HintValid=false;
		file_name=XmTextGetString(
			XmFileSelectionBoxGetChild(widget,XmDIALOG_TEXT)
		);
		if (!Machine->Load(file_name)) {
			XtUnmanageChild(ErrorBox);
			sprintf(tmp,"Failed to load '%s'",file_name);
			xms=XmStringCreateSimple(tmp);
			XtVaSetValues(ErrorBox,XmNmessageString,xms,(char*)NULL);
			XmStringFree(xms);
			XtManageChild(ErrorBox);
		}
		RT.SetWorld(Machine);
		UpdateStatusBar();
		UpdateMovesList();
		UpdateView();
		UpdateDepthMenu();
	}
	else if (widget==SaveDialog) {
		file_name=XmTextGetString(
			XmFileSelectionBoxGetChild(widget,XmDIALOG_TEXT)
		);
		if (access(file_name,F_OK)!=-1) {
			XtUnmanageChild(OverwriteDialog);
			sprintf(tmp,"OK to overwrite '%s'?",file_name);
			xms=XmStringCreateSimple(tmp);
			XtVaSetValues(OverwriteDialog,XmNmessageString,xms,(char*)NULL);
			XmStringFree(xms);
			XtManageChild(OverwriteDialog);
			strcpy(OverwriteFile,file_name);
		}
		else if (!Machine->Save(file_name)) {
			XtUnmanageChild(ErrorBox);
			sprintf(tmp,"Failed to save '%s'",file_name);
			xms=XmStringCreateSimple(tmp);
			XtVaSetValues(ErrorBox,XmNmessageString,xms,(char*)NULL);
			XmStringFree(xms);
			XtManageChild(ErrorBox);
		}
	}
	else if (widget==OverwriteDialog) {
		file_name=OverwriteFile;
		if (!Machine->Save(file_name)) {
			XtUnmanageChild(ErrorBox);
			sprintf(tmp,"Failed to save '%s'",file_name);
			xms=XmStringCreateSimple(tmp);
			XtVaSetValues(ErrorBox,XmNmessageString,xms,(char*)NULL);
			XmStringFree(xms);
			XtManageChild(ErrorBox);
		}
	}
	else if (widget==ViewArea) {
		if (cbs!=NULL && cbs->reason==XmCR_EXPOSE &&
		    cbs->event && cbs->event->xexpose.count==0) {
			UpdateView();
		}
		else if (cbs!=NULL && cbs->reason==XmCR_RESIZE) {
			XtVaGetValues(ViewArea,XmNwidth,&ViewWidth,(char*)NULL);
			XtVaGetValues(ViewArea,XmNheight,&ViewHeight,(char*)NULL);
			RT.SetViewSize(ViewWidth,ViewHeight);
			UpdateView();
		}
		else if (event!=NULL && event->type==ButtonPress) {
			MousePress(event->xbutton.x,event->xbutton.y);
		}
		else if (event!=NULL && event->type==MapNotify && !ViewWin) {
			ViewWin=XtWindow(ViewArea);
			ViewGC=XtGetGC(ViewArea,0,&gcval);
		}
	}
	else if (widget==BCompHint) {
		AbortSearching=true;
		HintWanted=true;
		HintValid=false;
		UpdateStatusBar();
	}
	else if (widget==BHelpAbout) {
		XtUnmanageChild(AboutDialog);
		XtManageChild(AboutDialog);
	}
	else if (widget==LDClose) {
		XtUnmanageChild(ListDialog);
	}
	else {
		for (i=0; i<=SilChessMachine::MAX_SEARCH_DEPTH; i++) {
			if (widget==BDepth[i] && cbs!=NULL) {
				if (((XmToggleButtonCallbackStruct*)cbs)->set) {
					AbortSearching=true;
					Machine->SetSearchDepth(i);
					UpdateStatusBar();
				}
			}
		}
	}
	do {
		DoPainting();
		DoSearching();
	} while(NeedPainting && !IsPainting && ViewWin);
}


void XSilChessWindow::MousePress(int x, int y)
{
	int i;
	SilChessMachine::Move m;

	RT.View2Board(x,y,&x,&y);
	if (x<0 || y<0 || x>7 || y>7 || (x==SelX && y==SelY)) {
		if (SelX!=-1 || SelY!=-1) {
			SelX=SelY=-1;
			UpdateView();
		}
		return;
	}
	i=Machine->GetField(x,y);
	if (i!=0 && ((i<7) == Machine->IsWhiteOn())) {
		if (SelX!=x || SelY!=y) {
			if (SelX==-1 || SelY==-1) {
				SelX=x;
				SelY=y;
				PaintSel();
			}
			else {
				SelX=x;
				SelY=y;
				UpdateView();
			}
		}
		return;
	}
	if (SelX!=-1 && SelY!=-1) {
		m.X1=SelX;
		m.Y1=SelY;
		m.X2=x;
		m.Y2=y;
		if (Machine->IsLegalMove(m)) {
			AbortSearching=true;
			HintWanted=false;
			HintValid=false;
			Machine->DoMove(m);
			SelX=SelY=-1;
			RT.SetWorld(Machine);
			UpdateStatusBar();
			UpdateMovesList();
			UpdateView();
		}
		else {
			SelX=SelY=-1;
			UpdateStatusBar();
			UpdateView();
		}
	}
}


void XSilChessWindow::DoSearching()
{
	XEvent e;
	SilChessMachine::Move m;
	bool res;

	if (IsPainting) return;
	if (IsSearching) return;
	IsSearching=true;
	while ((!Machine->IsHumanOn() || HintWanted) &&
	       !Machine->IsMate() && !Machine->IsDraw() && !Machine->IsEndless()) {
		AbortSearching=false;
		Machine->StartSearching();
		for (;;) {
			if (Machine->ContinueSearching()) break;
			while (XtAppPending(App)) {
				XtAppNextEvent(App,&e);
				XtDispatchEvent(&e);
			}
			if (AbortSearching) break;
		}
		res=Machine->EndSearching(&m);
		if (!AbortSearching && res) {
			if (!Machine->IsHumanOn()) {
				Machine->DoMove(m);
				RT.SetWorld(Machine);
				UpdateStatusBar();
				UpdateMovesList();
				UpdateView();
			}
			else if (HintWanted) {
				HintWanted=false;
				Hint=m;
				HintValid=true;
				UpdateStatusBar();
			}
		}
	}
	IsSearching=false;
}


void XSilChessWindow::UpdateStatusBar()
{
	XmString xms;
	char tmp[512];

	tmp[0]=0;
	if (Machine->GetMoveCount()>0) {
		Machine->GetMove(Machine->GetMoveCount()-1).ToString(tmp+strlen(tmp));
		sprintf(tmp+strlen(tmp)," <%d>  ",Machine->GetValue());
	}
	if (Machine->IsMate()) strcat(tmp,"MATE!");
	else if (Machine->IsDraw()) strcat(tmp,"DRAW!");
	else if (Machine->IsEndless()) strcat(tmp,"ENDLESS!");
	else {
		if (Machine->IsCheck()) strcat(tmp,"check! ");
		if (!Machine->IsHumanOn()) {
			sprintf(tmp+strlen(tmp),"searching (%d)...",Machine->GetSearchDepth());
		}
		else if (HintWanted) {
			sprintf(tmp+strlen(tmp),"searching hint (%d)...",Machine->GetSearchDepth());
		}
		else {
			if (HintValid) {
				strcat(tmp,"hint: ");
				Hint.ToString(tmp+strlen(tmp));
				strcat(tmp,", ");
			}
			strcat(tmp,"your move? ");
		}
	}
	xms=XmStringCreateLtoR(tmp,XmFONTLIST_DEFAULT_TAG);
	XtVaSetValues(StatusLabel,XmNlabelString,xms,(char*)NULL);
	XmStringFree(xms);
}


void XSilChessWindow::UpdateMovesList()
{
	XmString xms;
	Widget clpwin,scrbar;
	char * buf, * p;
	int i,smax,ssiz;
	Position x,y;
	Dimension h,hc;

	buf=(char*)malloc(65536);
	p=buf;
	for (i=0; i<Machine->GetMoveCount(); i++) {
		if ((i&1)==0) p+=sprintf(p,"%03d",i/2+1);
		*p++=' ';
		Machine->GetMove(i).ToString(p);
		p+=4;
		if ((i&1)!=0 || i==Machine->GetMoveCount()-1) *p++='\n';
	}
	*p=0;
	if (p==buf) strcpy(buf,"<empty>");
	xms=XmStringCreateLtoR(buf,XmFONTLIST_DEFAULT_TAG);
	free(buf);

	XtVaSetValues(LDList,XmNlabelString,xms,(char*)NULL);
	XmStringFree(xms);

	// Make last entry visible
	clpwin=NULL;
	XtVaGetValues(LDScroll,XmNclipWindow,&clpwin,(char*)NULL);
	if (clpwin) {
		XtVaGetValues(LDList,XmNx,&x,XmNy,&y,XmNheight,&h,(char*)NULL);
		XtVaGetValues(clpwin,XmNheight,&hc,(char*)NULL);
		if (((Position)hc-h)<y) {
			XtMoveWidget(LDList,x,hc-h);
			scrbar=NULL;
			XtVaGetValues(LDScroll,XmNverticalScrollBar,&scrbar,(char*)NULL);
			if (scrbar) {
				XtVaGetValues(scrbar,XmNmaximum,&smax,XmNsliderSize,&ssiz,(char*)NULL);
				XtVaSetValues(scrbar,XmNvalue,smax-ssiz,(char*)NULL);
			}
		}
	}
}


void XSilChessWindow::UpdateView()
{
	NeedPainting=true;
}


void XSilChessWindow::UpdateDepthMenu()
{
	int i;

	for (i=0; i<SilChessMachine::MAX_SEARCH_DEPTH; i++) {
		XtVaSetValues(
			BDepth[i],
			XmNset,
			Machine->GetSearchDepth()==i ? True : False,
			(char*)NULL
		);
	}
}


void XSilChessWindow::DoPainting()
{
	XEvent e;
	char * data;
	XImage * img;
	int i,y,msk;

	if (IsPainting) return;
	if (!ViewWin) return;
	IsPainting=true;
	y=0;
	while (NeedPainting) {
		NeedPainting=false;
		RT.SetWorld(Machine);
		data=(char*)malloc(PixelSize*ViewWidth+1000000);
		img=XCreateImage(Disp,Vsl,VslDepth,ZPixmap,0,data,
		                 ViewWidth,1,8*PixelSize,0);
		for (msk=0x3ff; msk<ViewHeight-1; msk=(msk<<1)|1);
		for (i=0, y%=ViewHeight; i<ViewHeight; i++) {
			while (XtAppPending(App)!=0) {
				XtAppNextEvent(App,&e);
				XtDispatchEvent(&e);
			}
			if (NeedPainting) break;
			RT.RenderScanline(y,data,PixelSize,RedMask,GreenMask,BlueMask);
			XPutImage(Disp,ViewWin,ViewGC,img,0,0,0,y,ViewWidth,1);
			if (i==ViewHeight-1 || (i&7)==0) PaintSel();
			do { y=(y+269779)&msk; } while (y>=ViewHeight);
		}
		PaintSel();
		XFree(img);
		free(data);
	}
	IsPainting=false;
}


void XSilChessWindow::PaintSel()
{
	int i,tx,ty;
	int mvx[4],mvy[4];

	if (SelX<0 || SelY<0 || !ViewWin) return;
	RT.Board2View(SelX-0.5,SelY-0.5,&(mvx[0]),&(mvy[0]));
	RT.Board2View(SelX-0.5,SelY+0.5,&(mvx[1]),&(mvy[1]));
	RT.Board2View(SelX+0.5,SelY+0.5,&(mvx[2]),&(mvy[2]));
	RT.Board2View(SelX+0.5,SelY-0.5,&(mvx[3]),&(mvy[3]));
	XSetForeground(Disp,ViewGC,0);
	for (i=0; i<4; i++) {
		tx=mvx[(i+1)&3]+(mvx[i]-mvx[(i+1)&3])/6;
		ty=mvy[(i+1)&3]+(mvy[i]-mvy[(i+1)&3])/6;
		XDrawLine(Disp,ViewWin,ViewGC,tx,ty,mvx[(i+1)&3],mvy[(i+1)&3]);
		tx=mvx[i]+(mvx[(i+1)&3]-mvx[i])/6;
		ty=mvy[i]+(mvy[(i+1)&3]-mvy[i])/6;
		XDrawLine(Disp,ViewWin,ViewGC,mvx[i],mvy[i],tx,ty);
	}
}


const char * const XSilChessWindow::AboutText=
	"XSilChess 3.1\n\n"
	"Copyright (C) 2001-2009 Oliver Hamann\n"
;


//==============================================================================
//============================= SCDefaultResources =============================
//==============================================================================

static const char * SCDefaultResources[]= {
	"XSilChess*.background: #c0c0c0",
	"XSilChess*.foreground: black",
	"XSilChess.title: XSilChess",
	"XSilChess.minWidth: 190",
	"XSilChess.minHeight: 176",
	"XSilChess.mainWin.width: 460",
	"XSilChess.mainWin.height: 350",
	"XSilChess.mainWin.mainMenu.shadowThickness: 1",
	"XSilChess.mainWin.mainMenu.file.labelString: File",
	"XSilChess.mainWin.mainMenu.file.mnemonic: F",
	"XSilChess.mainWin.mainMenu*.fileMenu.load.labelString: Load...",
	"XSilChess.mainWin.mainMenu*.fileMenu.load.mnemonic: L",
	"XSilChess.mainWin.mainMenu*.fileMenu.load.accelerator: Ctrl<Key>L",
	"XSilChess.mainWin.mainMenu*.fileMenu.load.acceleratorText: Ctrl+L",
	"XSilChess.mainWin.mainMenu*.fileMenu.save.labelString: Save...",
	"XSilChess.mainWin.mainMenu*.fileMenu.save.mnemonic: S",
	"XSilChess.mainWin.mainMenu*.fileMenu.save.accelerator: Ctrl<Key>S",
	"XSilChess.mainWin.mainMenu*.fileMenu.save.acceleratorText: Ctrl+S",
	"XSilChess.mainWin.mainMenu*.fileMenu.exit.labelString: Exit",
	"XSilChess.mainWin.mainMenu*.fileMenu.exit.mnemonic: x",
	"XSilChess.mainWin.mainMenu*.fileMenu.exit.accelerator: Ctrl<Key>Q",
	"XSilChess.mainWin.mainMenu*.fileMenu.exit.acceleratorText: Ctrl+Q",
	"XSilChess.mainWin.mainMenu.game.labelString: Game",
	"XSilChess.mainWin.mainMenu.game.mnemonic: G",
	"XSilChess.mainWin.mainMenu*.gameMenu.new.labelString: New Game",
	"XSilChess.mainWin.mainMenu*.gameMenu.new.mnemonic: N",
	"XSilChess.mainWin.mainMenu*.gameMenu.new.accelerator: Ctrl<Key>N",
	"XSilChess.mainWin.mainMenu*.gameMenu.new.acceleratorText: Ctrl+N",
	"XSilChess.mainWin.mainMenu*.gameMenu.flip.labelString: Flip Sides",
	"XSilChess.mainWin.mainMenu*.gameMenu.flip.mnemonic: F",
	"XSilChess.mainWin.mainMenu*.gameMenu.flip.accelerator: Ctrl<Key>F",
	"XSilChess.mainWin.mainMenu*.gameMenu.flip.acceleratorText: Ctrl+F",
	"XSilChess.mainWin.mainMenu*.gameMenu.undo.labelString: Undo Move",
	"XSilChess.mainWin.mainMenu*.gameMenu.undo.mnemonic: U",
	"XSilChess.mainWin.mainMenu*.gameMenu.undo.accelerator: Ctrl<Key>Z",
	"XSilChess.mainWin.mainMenu*.gameMenu.undo.acceleratorText: Ctrl+Z",
	"XSilChess.mainWin.mainMenu*.gameMenu.list.labelString: Show Moves...",
	"XSilChess.mainWin.mainMenu*.gameMenu.list.mnemonic: M",
	"XSilChess.mainWin.mainMenu*.gameMenu.list.accelerator: Ctrl<Key>M",
	"XSilChess.mainWin.mainMenu*.gameMenu.list.acceleratorText: Ctrl+M",
	"XSilChess.mainWin.mainMenu.comp.labelString: Computer",
	"XSilChess.mainWin.mainMenu.comp.mnemonic: C",
	"XSilChess.mainWin.mainMenu*.compMenu.hint.labelString: Give Hint",
	"XSilChess.mainWin.mainMenu*.compMenu.hint.mnemonic: H",
	"XSilChess.mainWin.mainMenu*.compMenu.hint.accelerator: Ctrl<Key>H",
	"XSilChess.mainWin.mainMenu*.compMenu.hint.acceleratorText: Ctrl+H",
	"XSilChess.mainWin.mainMenu*.compMenu.depth.labelString: Search Depth",
	"XSilChess.mainWin.mainMenu*.compMenu.depth.mnemonic: D",
	"XSilChess.mainWin.mainMenu.help.labelString: Help",
	"XSilChess.mainWin.mainMenu.help.mnemonic: H",
	"XSilChess.mainWin.mainMenu*.helpMenu.about.labelString: About...",
	"XSilChess.mainWin.mainMenu*.helpMenu.about.mnemonic: A",
	"XSilChess.mainWin.mainForm.statusFrame.shadowType: SHADOW_IN",
	"XSilChess.mainWin.mainForm.statusFrame.shadowThickness: 1",
	"XSilChess.mainWin.mainForm.viewFrame.shadowType: SHADOW_IN",
	"XSilChess.mainWin.mainForm.viewFrame.shadowThickness: 0",
	"XSilChess.mainWin.mainForm.viewFrame.viewArea.background: #4d3319",
	"XSilChess.loadDialog_popup.loadDialog.dialogTitle: Load Game",
	"XSilChess.loadDialog_popup.loadDialog.pattern: *.silchess",
	"XSilChess.saveDialog_popup.saveDialog.dialogTitle: Save Game",
	"XSilChess.saveDialog_popup.saveDialog.pattern: *.silchess",
	"XSilChess.overwriteDialog_popup.overwriteDialog.dialogTitle: File exists",
	"XSilChess.errorBox_popup.errorBox.dialogTitle: Error",
	"XSilChess.listDialog_popup.title: List of Moves",
	"XSilChess.listDialog_popup.minWidth: 96",
	"XSilChess.listDialog_popup.minHeight: 96",
	"XSilChess.listDialog_popup.listDialog.close.labelString: Close",
	"XSilChess.aboutDialog_popup.aboutDialog.dialogTitle: About",
	NULL
};


//==============================================================================
//==================================== main ====================================
//==============================================================================

int main(int argc, char * argv[])
{
	XtAppContext App;
	XSilChessWindow * MainWindow;
	Widget TopLevel;
	Colormap colormap;
	Visual * visual;
	XVisualInfo * vil;
	XVisualInfo vit;
	Display * display;
	int vic,i,depth;

	TopLevel=XtVaAppInitialize(
		&App,"XSilChess",NULL,0,&argc,argv,
		(char**)SCDefaultResources,(char*)NULL
	);
	display=XtDisplay(TopLevel);
	memset(&vit,0,sizeof(vit));
	vit.screen=XScreenNumberOfScreen(XtScreen(TopLevel));
	vil=XGetVisualInfo(display,VisualScreenMask,&vit,&vic);
	visual=NULL;
	depth=0;
	for (i=0; i<vic; i++) {
		if (vil[i].c_class==TrueColor) {
			visual=vil[i].visual;
			depth=vil[i].depth;
			break;
		}
	}
	if (visual==NULL) {
		fprintf(stderr,"ERROR: no true color visual available\n");
		return 1;
	}
	colormap=XCreateColormap(display,DefaultRootWindow(display),visual,AllocNone);
	XtVaSetValues(
		TopLevel,
		XmNvisual,visual,
		XmNdepth,depth,
		XmNcolormap,colormap,
		(char*)NULL
	);
	MainWindow=new XSilChessWindow(App,TopLevel,visual,depth,colormap);
	XtRealizeWidget(TopLevel);
	XtAppMainLoop(App);
	delete MainWindow;
	return 0;
}
