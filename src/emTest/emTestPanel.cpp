//------------------------------------------------------------------------------
// emTestPanel.cpp
//
// Copyright (C) 2005-2009,2011 Oliver Hamann.
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

#include <emTest/emTestPanel.h>
#include <emCore/emVarModel.h>


emTestPanel::emTestPanel(ParentArg parent, const emString & name)
	: emPanel(parent,name)
{
	DefaultBgColor=0x001C38FF;

	BgColor=emVarModel<emColor>::GetAndRemove(
		GetView(),
		"emTestPanel - BgColor of " + GetIdentity(),
		DefaultBgColor
	);

	EnableAutoExpansion();
	SetAutoExpansionThreshold(900.0);
}


emTestPanel::~emTestPanel()
{
	if (BgColor!=DefaultBgColor) {
		emVarModel<emColor>::Set(
			GetView(),
			"emTestPanel - BgColor of " + GetIdentity(),
			BgColor,
			10
		);
	}
}


emString emTestPanel::GetTitle()
{
	return "Test Panel";
}


bool emTestPanel::Cycle()
{
	if (BgColorField && IsSignaled(BgColorField->GetColorSignal())) {
		BgColor=BgColorField->GetColor();
		UpdateControlPanel();
		InvalidatePainting();
		InvalidateChildrenLayout();
	}
	return false;
}


void emTestPanel::Notice(NoticeFlags flags)
{
	UpdateControlPanel();
	InvalidatePainting();
}


void emTestPanel::Input(
	emInputEvent & event, const emInputState & state, double mx, double my
)
{
	emString log;
	int i,k;

	log=emString::Format(
		"EVENT: key=%d chars=\"%s\" repeat=%d variant=%d STATE: pressed=",
		(int)event.GetKey(),
		event.GetChars().Get(),
		event.GetRepeat(),
		event.GetVariant()
	);
	for (i=0, k=0; i<256; i++) {
		if (state.Get((emInputKey)i)) {
			if (k) log+=",";
			log+=emString::Format("%d",i);
			k=1;
		}
	}
	log+=emString::Format(" mouse=%g,%g",mx,my);
	if (InputLog.GetCount()>19) InputLog.RemoveFirst();
	InputLog.Add(log);
	InvalidatePainting();

	emPanel::Input(event,state,mx,my);
}


bool emTestPanel::IsOpaque()
{
	return BgColor.IsOpaque();
}


void emTestPanel::Paint(const emPainter & painter, emColor canvasColor)
{
	const emString * pstr;
	emColor fgCol;
	emString str;
	double h;
	double xy[128*2];
	int i;

	if (IsFocused()) fgCol=emColor(255,136,136);
	else if (IsInFocusedPath()) fgCol=emColor(187,136,136);
	else if (IsVisited()) fgCol=emColor(255,255,136);
	else if (IsInVisitedPath()) fgCol=emColor(187,187,136);
	else fgCol=emColor(136,136,136);

	h=GetHeight();
	painter.PaintRect(0,0,1,h,BgColor,canvasColor);
	painter.PaintRectOutline(0.01,0.01,1.0-0.02,h-0.02,0.02,fgCol,BgColor);

	painter.PaintTextBoxed(
		0.02,0.02,0.49,0.07,
		"Test Panel",
		0.1,
		fgCol,
		BgColor,
		EM_ALIGN_TOP_LEFT
	);

	str="State:";
	if (IsFocused()) str+=" Focused";
	if (IsInFocusedPath()) str+=" InFocusedPath";
	if (IsVisited()) str+=" Visited";
	if (IsInVisitedPath()) str+=" InVisitedPath";
	if (IsViewFocused()) str+=" ViewFocused";
	painter.PaintTextBoxed(0.05,0.4,0.9,0.05,str,0.05,fgCol,BgColor,EM_ALIGN_LEFT);

	str=emString::Format(
		"Pri=%f MemLim=%lu",
		GetUpdatePriority(),
		(unsigned long)GetMemoryLimit()
	);
	painter.PaintTextBoxed(0.05,0.45,0.9,0.1,str,0.1,fgCol,BgColor,EM_ALIGN_LEFT);

	for (pstr=InputLog.GetFirst(), i=0; pstr; pstr=InputLog.GetNext(pstr), i++) {
		painter.PaintText(
			0.05,
			0.57+i*0.008,
			pstr->Get(),
			0.008,
			1.0,
			0x8888BBFF,
			BgColor
		);
	}

	painter.PaintTextBoxed(
		0.25,0.8,0.05,0.05,
		"Text Test\n"
		"\t<-tab\n"
		"tab->\t<-tab",
		0.1,
		fgCol,
		BgColor,
		EM_ALIGN_CENTER,
		EM_ALIGN_LEFT,
		0.2,
		true,
		0.1
	);
	painter.PaintRect(0.25,0.8,0.05,0.05,emColor(255,0,0,32));

	xy[0]=0.7; xy[1]=0.6;
	xy[2]=0.6; xy[3]=0.7;
	xy[4]=0.8; xy[5]=0.8;
	painter.PaintPolygon(xy,3,fgCol,BgColor);

	xy[ 0]=0.90; xy[ 1]=0.90;
	xy[ 2]=0.94; xy[ 3]=0.90;
	xy[ 4]=0.94; xy[ 5]=0.94;
	xy[ 6]=0.90; xy[ 7]=0.94;
	xy[ 8]=0.90; xy[ 9]=0.90;
	xy[10]=0.91; xy[11]=0.91;
	xy[12]=0.93; xy[13]=0.91;
	xy[14]=0.93; xy[15]=0.93;
	xy[16]=0.91; xy[17]=0.93;
	xy[18]=0.91; xy[19]=0.91;
	painter.PaintPolygon(xy,10,emColor(255,255,255,128),BgColor);

	xy[ 0]=0.80; xy[ 1]=0.90;
	xy[ 2]=0.84; xy[ 3]=0.90;
	xy[ 4]=0.84; xy[ 5]=0.94;
	xy[ 6]=0.80; xy[ 7]=0.94;
	xy[ 8]=0.80; xy[ 9]=0.90;
	xy[10]=0.81; xy[11]=0.91;
	xy[12]=0.81; xy[13]=0.93;
	xy[14]=0.83; xy[15]=0.93;
	xy[16]=0.83; xy[17]=0.91;
	xy[18]=0.81; xy[19]=0.91;
	painter.PaintPolygon(xy,10,emColor(255,255,255),BgColor);

	for (i=0; i<64; i++) {
		xy[i*2]=sin(M_PI*i/32.0)*0.05+0.65;
		xy[i*2+1]=cos(M_PI*i/32.0)*0.05+0.85;
	}
	painter.PaintPolygon(xy,64,emColor(255,255,0),BgColor);

	for (i=0; i<64; i++) {
		xy[i*2]=sin(M_PI*i/32.0)*0.05+0.55;
		xy[i*2+1]=cos(M_PI*i/32.0)*0.05+0.85;
	}
	emPainter(
		painter,
		0.51*painter.GetScaleX()+painter.GetOriginX(),
		0.81*painter.GetScaleY()+painter.GetOriginY(),
		0.59*painter.GetScaleX()+painter.GetOriginX(),
		0.89*painter.GetScaleY()+painter.GetOriginY()
	).PaintPolygon(xy,64,emColor(0,255,0),BgColor);

	for (i=0; i<64; i++) {
		xy[i*2]=sin(M_PI*i/32.0)*0.06+0.6;
		xy[i*2+1]=cos(M_PI*i/32.0)*0.04+0.86;
	}
	painter.PaintPolygon(xy,64,emColor(255,0,0,92));

	xy[0]=0.6;  xy[1]=0.9;
	xy[2]=0.5;  xy[3]=0.92;
	xy[4]=0.65; xy[5]=0.95;
	painter.PaintPolygon(xy,3,emColor(187,255,255),BgColor);

	xy[0]=0.6;  xy[1]=0.96;
	xy[2]=0.5;  xy[3]=0.92;
	xy[4]=0.65; xy[5]=0.95;
	painter.PaintPolygon(xy,3,emColor(255,0,0),BgColor);

	xy[0]=0.45; xy[1]=0.9;
	xy[2]=0.35; xy[3]=0.92;
	xy[4]=0.5;  xy[5]=0.95;
	painter.PaintPolygon(xy,3,emColor(187,255,255));

	xy[0]=0.45; xy[1]=0.96;
	xy[2]=0.35; xy[3]=0.92;
	xy[4]=0.5;  xy[5]=0.95;
	painter.PaintPolygon(xy,3,emColor(255,0,0));

	xy[0]=0.6;   xy[1]=0.6;
	xy[2]=0.602; xy[3]=0.6;
	xy[4]=0.502; xy[5]=0.7;
	painter.PaintPolygon(xy,3,emColor(187,136,255,192));

	xy[0]=0.7;   xy[1]=0.55;
	xy[2]=0.702; xy[3]=0.55;
	xy[4]=0.802; xy[5]=0.9;
	xy[6]=0.8;   xy[7]=0.9;
	painter.PaintPolygon(xy,4,emColor(136,187,255,192));

	xy[0]=0.8; xy[1]=0.55;
	xy[2]=0.9; xy[3]=0.55;
	xy[4]=0.8; xy[5]=0.8;
	xy[6]=0.9; xy[7]=0.8;
	painter.PaintPolygon(xy,4,emColor(136,187,255,192));

	painter.PaintEllipse(0.05,0.80,0.01,0.01,0xFFFFFFFF,BgColor);
	painter.PaintEllipse(0.06,0.80,0.02,0.01,0xFFFFFFFF,BgColor);
	painter.PaintEllipse(0.09,0.80,0.005,0.01,0xFFFFFFFF,BgColor);
	painter.PaintEllipse(0.10,0.80,0.01,0.01,45,350,0xFFFFFFFF,BgColor);
	painter.PaintEllipse(0.11,0.80,0.02,0.01,45,-350,0xFFFFFFFF,BgColor);
	painter.PaintEllipse(0.13,0.80,0.005,0.01,245,50,0xFFFFFFFF,BgColor);
	painter.PaintEllipse(0.14,0.80,0.01,0.01,245,-50,0xFFFFFFFF,BgColor);

	painter.PaintRectOutline(0.05,0.82,0.01,0.01,0.001,0xFFFFFFFF,BgColor);
	painter.PaintRectOutline(0.07,0.82,0.02,0.01,0.001,0xFFFFFFFF,BgColor);
	painter.PaintRectOutline(0.10,0.82,0.01,0.01,0.008,0xFFFFFFFF,BgColor);
	painter.PaintRectOutline(0.13,0.82,0.01,0.01,0.011,0xFFFFFFFF,BgColor);

	painter.PaintRoundRect(0.05,0.84,0.01,0.01,0.001,0.001,0xFFFFFFFF,BgColor);
	painter.PaintRoundRect(0.07,0.84,0.02,0.01,0.001,0.002,0xFFFFFFFF,BgColor);
	painter.PaintRoundRect(0.10,0.84,0.01,0.01,0.003,0.002,0xFFFFFFFF,BgColor);
	painter.PaintRoundRect(0.13,0.84,0.01,0.01,0.001,0.011,0xFFFFFFFF,BgColor);
	painter.PaintRoundRect(0.15,0.84,0.01,0.01,0.000,0.00,0xFFFFFFFF,BgColor);

	painter.PaintEllipseOutline(0.05,0.86,0.01,0.01,0.003,0xFFFFFFFF,BgColor);
	painter.PaintEllipseOutline(0.065,0.86,0.02,0.01,0.001,0xFFFFFFFF,BgColor);
	painter.PaintEllipseOutline(0.09,0.86,0.005,0.01,0.0001,0xFFFFFFFF,BgColor);
	painter.PaintEllipseOutline(0.10,0.86,0.01,0.01,90,225,0.001,0xFFFFFFFF,BgColor);
	painter.PaintEllipseOutline(0.11,0.86,0.02,0.01,45,-350,0.0001,0xFFFFFFFF,BgColor);
	painter.PaintEllipseOutline(0.13,0.86,0.005,0.01,245,50,0.001,0xFFFFFFFF,BgColor);
	painter.PaintEllipseOutline(0.14,0.86,0.01,0.01,245,-50,0.001,0xFFFFFFFF,BgColor);

	painter.PaintRoundRectOutline(0.05,0.88,0.01,0.01,0.001,0.001,0.001,0xFFFFFFFF,BgColor);
	painter.PaintRoundRectOutline(0.07,0.88,0.02,0.01,0.001,0.002,0.001,0xFFFFFFFF,BgColor);
	painter.PaintRoundRectOutline(0.10,0.88,0.01,0.01,0.003,0.002,0.003,0xFFFFFFFF,BgColor);
	painter.PaintRoundRectOutline(0.13,0.88,0.01,0.01,0.001,0.011,0.0001,0xFFFFFFFF,BgColor);
	painter.PaintRoundRectOutline(0.15,0.88,0.01,0.01,-0.0004,-0.0004,0.001,0xFFFFFFFF,BgColor);


	painter.PaintLine(0.050,0.90,0.060,0.91,0.0005,emPainter::LC_FLAT,emPainter::LC_FLAT,0xFFFFFFFF,BgColor);
	painter.PaintLine(0.051,0.90,0.061,0.91,0.0005,emPainter::LC_SQUARE,emPainter::LC_SQUARE,0xFFFFFFFF,BgColor);
	painter.PaintLine(0.052,0.90,0.062,0.91,0.0005,emPainter::LC_ROUND,emPainter::LC_ROUND,0xFFFFFFFF,BgColor);
	painter.PaintLine(0.053,0.90,0.063,0.91,0.0005,emPainter::LC_FLAT,emPainter::LC_ROUND,0xFFFFFFFF,BgColor);
	painter.PaintLine(0.054,0.90,0.064,0.91,0.0005,emPainter::LC_ROUND,emPainter::LC_SQUARE,0xFFFFFFFF,BgColor);
	painter.PaintLine(0.055,0.90,0.065,0.91,0.0005,emPainter::LC_SQUARE,emPainter::LC_FLAT,0xFFFFFFFF,BgColor);
	painter.PaintLine(0.056,0.90,0.066,0.91,0.0005,emPainter::LC_ROUND,emPainter::LC_FLAT,0xFFFFFFFF,BgColor);
	painter.PaintLine(0.057,0.90,0.067,0.91,0.0005,emPainter::LC_SQUARE,emPainter::LC_ROUND,0xFFFFFFFF,BgColor);
	painter.PaintLine(0.058,0.90,0.068,0.91,0.0005,emPainter::LC_FLAT,emPainter::LC_SQUARE,0xFFFFFFFF,BgColor);

	painter.PaintLine(0.070,0.90,0.070,0.91,0.0005,emPainter::LC_FLAT,emPainter::LC_FLAT,0xFFFFFFFF,BgColor);
	painter.PaintLine(0.071,0.90,0.071,0.91,0.0005,emPainter::LC_SQUARE,emPainter::LC_SQUARE,0xFFFFFFFF,BgColor);
	painter.PaintLine(0.072,0.90,0.072,0.91,0.0005,emPainter::LC_ROUND,emPainter::LC_ROUND,0xFFFFFFFF,BgColor);
	painter.PaintLine(0.073,0.90,0.073,0.91,0.0005,emPainter::LC_FLAT,emPainter::LC_ROUND,0xFFFFFFFF,BgColor);
	painter.PaintLine(0.074,0.90,0.074,0.91,0.0005,emPainter::LC_ROUND,emPainter::LC_SQUARE,0xFFFFFFFF,BgColor);
	painter.PaintLine(0.075,0.90,0.075,0.91,0.0005,emPainter::LC_SQUARE,emPainter::LC_FLAT,0xFFFFFFFF,BgColor);
	painter.PaintLine(0.076,0.90,0.076,0.91,0.0005,emPainter::LC_ROUND,emPainter::LC_FLAT,0xFFFFFFFF,BgColor);
	painter.PaintLine(0.077,0.90,0.077,0.91,0.0005,emPainter::LC_SQUARE,emPainter::LC_ROUND,0xFFFFFFFF,BgColor);
	painter.PaintLine(0.078,0.90,0.078,0.91,0.0005,emPainter::LC_FLAT,emPainter::LC_SQUARE,0xFFFFFFFF,BgColor);

	painter.PaintLine(0.080,0.90,0.080,0.90,0.0005,emPainter::LC_FLAT,emPainter::LC_FLAT,0xFFFFFFFF,BgColor);
	painter.PaintLine(0.081,0.90,0.081,0.90,0.0005,emPainter::LC_SQUARE,emPainter::LC_SQUARE,0xFFFFFFFF,BgColor);
	painter.PaintLine(0.082,0.90,0.082,0.90,0.0005,emPainter::LC_ROUND,emPainter::LC_ROUND,0xFFFFFFFF,BgColor);
	painter.PaintLine(0.083,0.90,0.083,0.90,0.0005,emPainter::LC_FLAT,emPainter::LC_ROUND,0xFFFFFFFF,BgColor);
	painter.PaintLine(0.084,0.90,0.084,0.90,0.0005,emPainter::LC_ROUND,emPainter::LC_SQUARE,0xFFFFFFFF,BgColor);
	painter.PaintLine(0.085,0.90,0.085,0.90,0.0005,emPainter::LC_SQUARE,emPainter::LC_FLAT,0xFFFFFFFF,BgColor);
	painter.PaintLine(0.086,0.90,0.086,0.90,0.0005,emPainter::LC_ROUND,emPainter::LC_FLAT,0xFFFFFFFF,BgColor);
	painter.PaintLine(0.087,0.90,0.087,0.90,0.0005,emPainter::LC_SQUARE,emPainter::LC_ROUND,0xFFFFFFFF,BgColor);
	painter.PaintLine(0.088,0.90,0.088,0.90,0.0005,emPainter::LC_FLAT,emPainter::LC_SQUARE,0xFFFFFFFF,BgColor);

	painter.PaintLine(0.090,0.91,0.100,0.90,0.0005,emPainter::LC_FLAT,emPainter::LC_FLAT,0xFFFFFFFF,BgColor);
	painter.PaintLine(0.091,0.91,0.101,0.90,0.0005,emPainter::LC_SQUARE,emPainter::LC_SQUARE,0xFFFFFFFF,BgColor);
	painter.PaintLine(0.092,0.91,0.102,0.90,0.0005,emPainter::LC_ROUND,emPainter::LC_ROUND,0xFFFFFFFF,BgColor);
	painter.PaintLine(0.093,0.91,0.103,0.90,0.0005,emPainter::LC_FLAT,emPainter::LC_ROUND,0xFFFFFFFF,BgColor);
	painter.PaintLine(0.094,0.91,0.104,0.90,0.0005,emPainter::LC_ROUND,emPainter::LC_SQUARE,0xFFFFFFFF,BgColor);
	painter.PaintLine(0.095,0.91,0.105,0.90,0.0005,emPainter::LC_SQUARE,emPainter::LC_FLAT,0xFFFFFFFF,BgColor);
	painter.PaintLine(0.096,0.91,0.106,0.90,0.0005,emPainter::LC_ROUND,emPainter::LC_FLAT,0xFFFFFFFF,BgColor);
	painter.PaintLine(0.097,0.91,0.107,0.90,0.0005,emPainter::LC_SQUARE,emPainter::LC_ROUND,0xFFFFFFFF,BgColor);
	painter.PaintLine(0.098,0.91,0.108,0.90,0.0005,emPainter::LC_FLAT,emPainter::LC_SQUARE,0xFFFFFFFF,BgColor);

	painter.PaintLine(0.130,0.90,0.110,0.91,0.0002,emPainter::LC_FLAT,emPainter::LC_FLAT,0xFFFFFFFF,BgColor);
	painter.PaintLine(0.131,0.90,0.111,0.91,0.0002,emPainter::LC_SQUARE,emPainter::LC_SQUARE,0xFFFFFFFF,BgColor);
	painter.PaintLine(0.132,0.90,0.112,0.91,0.0002,emPainter::LC_ROUND,emPainter::LC_ROUND,0xFFFFFFFF,BgColor);
	painter.PaintLine(0.133,0.90,0.113,0.91,0.0002,emPainter::LC_FLAT,emPainter::LC_ROUND,0xFFFFFFFF,BgColor);
	painter.PaintLine(0.134,0.90,0.114,0.91,0.0002,emPainter::LC_ROUND,emPainter::LC_SQUARE,0xFFFFFFFF,BgColor);
	painter.PaintLine(0.135,0.90,0.115,0.91,0.0002,emPainter::LC_SQUARE,emPainter::LC_FLAT,0xFFFFFFFF,BgColor);
	painter.PaintLine(0.136,0.90,0.116,0.91,0.0002,emPainter::LC_ROUND,emPainter::LC_FLAT,0xFFFFFFFF,BgColor);
	painter.PaintLine(0.137,0.90,0.117,0.91,0.0002,emPainter::LC_SQUARE,emPainter::LC_ROUND,0xFFFFFFFF,BgColor);
	painter.PaintLine(0.138,0.90,0.118,0.91,0.0002,emPainter::LC_FLAT,emPainter::LC_SQUARE,0xFFFFFFFF,BgColor);

	xy[0]=0.06; xy[1]=0.80;
	xy[2]=0.10; xy[3]=0.85;
	xy[4]=0.08; xy[5]=0.91;
	painter.PaintPolygonOutline(xy,3,0.0002,emColor(255,0,0));
}


void emTestPanel::AutoExpand()
{
	TkT=new TkTestGrp(*this,"TkTestGrp");
	TP1=new emTestPanel(*this,"1");
	TP2=new emTestPanel(*this,"2");
	TP3=new emTestPanel(*this,"3");
	TP4=new emTestPanel(*this,"4");
	BgColorField=new emTkColorField(
		*this,"BgColorField",
		"Background Color",emString(),emImage(),
		BgColor,true,true
	);
	AddWakeUpSignal(BgColorField->GetColorSignal());
}


void emTestPanel::LayoutChildren()
{
	if (TkT) TkT->Layout(0.2,0.15,0.3,0.12,BgColor);
	if (TP1) TP1->Layout(0.70,0.05,0.12,0.12,BgColor);
	if (TP2) TP2->Layout(0.83,0.05,0.12,0.12,BgColor);
	if (TP3) TP3->Layout(0.70,0.18,0.12,0.12,BgColor);
	if (TP4) TP4->Layout(0.83,0.18,0.12,0.12,BgColor);
	if (BgColorField) BgColorField->Layout(0.775,0.34,0.1,0.02,BgColor);
}


emPanel * emTestPanel::CreateControlPanel(
	ParentArg parent, const emString & name
)
{
	ControlPanel=new emTkLabel(parent,name);
	UpdateControlPanel();
	return ControlPanel;
}


void emTestPanel::UpdateControlPanel()
{
	if (ControlPanel) {
		ControlPanel->SetCaption(
			emString::Format(
				"This is just a test\n"
				"\n"
				"Panel Identity: %s\n"
				"BgColor: 0x%08X",
				GetIdentity().Get(),
				(int)BgColor
			)
		);
	}
}


emTestPanel::TkTest::TkTest(ParentArg parent, const emString & name)
	: emTkGroup(parent,name)
{
	emTkGroup * grp;
	emTkTiling * tlng;
	emTkButton * bt;
	emTkTextField * tf;
	emTkColorField * cf;
	emTkScalarField * sf;
	emTkTunnel * tunnel;
	emString str;
	int i;

	SetCaption("Toolkit Test");
	SetPrefChildTallness(0.3);


	grp=new emTkGroup(this,"buttons","Buttons");
	grp->SetBorderScaling(2.5);
		bt=new emTkButton(grp,"b1","Button");
		bt=new emTkButton(grp,"b2","Long Desc");
		str="";
		for (i=0; i<100; i++) {
			str+="This is a looooooooooooooooooooooooooooooooooooooooooooooooooooooong description of the button.\n";
		}
		bt->SetDescription(str);
		bt=new emTkButton(grp,"b3","NoEOI");
		bt->SetNoEOI();

	grp=new emTkGroup(this,"checkbuttons","Check Buttons and Boxes");
	grp->SetBorderScaling(2.5);
		new emTkCheckButton(grp,"c1","Check Button");
		new emTkCheckButton(grp,"c2","Check Button");
		new emTkCheckButton(grp,"c3","Check Button");
		new emTkCheckBox(grp,"c4","Check Box");
		new emTkCheckBox(grp,"c5","Check Box");
		new emTkCheckBox(grp,"c6","Check Box");

	grp=new emTkRadioButton::Group(this,"radiobuttons","Radio Buttons and Boxes");
	grp->SetBorderScaling(2.5);
		new emTkRadioButton(grp,"r1","Radio Button");
		new emTkRadioButton(grp,"r2","Radio Button");
		new emTkRadioButton(grp,"r3","Radio Button");
		new emTkRadioBox(grp,"r4","Radio Box");
		new emTkRadioBox(grp,"r5","Radio Box");
		new emTkRadioBox(grp,"r6","Radio Box");

	grp=new emTkGroup(this,"textfields","Text Fields");
	grp->SetBorderScaling(2.5);
		tf=new emTkTextField(
			grp,"tf1",
			"Read-Only","This is a read-only text field.",emImage(),
			"Read-Only"
		);
		tf=new emTkTextField(
			grp,"tf2",
			"Editable","This is an editable text field.",emImage(),
			"Editable",true
		);
		tf=new emTkTextField(
			grp,"tf3",
			"Password","This is an editable password text field.",emImage(),
			"Password",true
		);
		tf->SetPasswordMode();
		tf=new emTkTextField(
			grp,"mltf1",
			"Multi-Line","This is an editable multi-line text field.",emImage(),
			"first line\nsecond line\n...",true
		);
		tf->SetMultiLineMode();

	grp=new emTkGroup(this,"scalarfields","Scalar Fields");
	grp->SetBorderScaling(2.5);
	grp->SetPrefChildTallness(0.1);

		sf=new emTkScalarField(grp,"sf1","Read-Only");

		sf=new emTkScalarField(grp,"sf2","Editable");
		sf->SetEditable();

		sf=new emTkScalarField(grp,"sf3");
		sf->SetEditable();
		sf->SetMinMaxValues(-1000,1000);
		sf->SetScaleMarkIntervals(1000,100,10,5,1,0);

		sf=new emTkScalarField(grp,"sf4","Level");
		sf->SetEditable();
		sf->SetTextBoxTallness(0.25);
		sf->SetMinMaxValues(1,5);
		sf->SetValue(3);
		sf->SetTextOfValueFunc(TextOfLevelValue,NULL);

		SFLen=new emTkScalarField(grp,"sf5","Play Length");
		SFLen->SetEditable();
		SFLen->SetMinMaxValues(0,24*3600*1000);
		SFLen->SetValue(4*3600*1000);
		SFLen->SetScaleMarkIntervals(60*60*1000,15*60*1000,5*60*1000,60*1000,10*1000,1000,100,10,1,0);
		SFLen->SetTextOfValueFunc(TextOfTimeValue,NULL);
		AddWakeUpSignal(SFLen->GetValueSignal());

		SFPos=new emTkScalarField(grp,"sf6","Play Position");
		SFPos->SetEditable();
		SFPos->SetMinMaxValues(0,SFLen->GetValue());
		SFPos->SetScaleMarkIntervals(60*60*1000,15*60*1000,5*60*1000,60*1000,10*1000,1000,100,10,1,0);
		SFPos->SetTextOfValueFunc(TextOfTimeValue,NULL);

	grp=new emTkGroup(this,"colorfields","Color Fields");
	grp->SetBorderScaling(2.5);
	grp->SetPrefChildTallness(0.4);

		cf=new emTkColorField(grp,"cf1","Read-Only");
		cf->SetColor(0xBB2222FF);

		cf=new emTkColorField(grp,"cf2","Editable");
		cf->SetColor(0x22BB22FF);
		cf->SetEditable();

		cf=new emTkColorField(grp,"cf3","Editable, Alpha Enabled");
		cf->SetColor(0x2222BBFF);
		cf->SetEditable();
		cf->SetAlphaEnabled();

	grp=new emTkGroup(this,"tunnels","Tunnels");
	grp->SetBorderScaling(2.5);
	grp->SetPrefChildTallness(0.4);

		tunnel=new emTkTunnel(grp,"t1","Tunnel");
		new emTkButton(tunnel,"e","End Of Tunnel");

		tunnel=new emTkTunnel(grp,"t2","Deeper Tunnel");
		tunnel->SetDepth(30.0);
		new emTkGroup(tunnel,"e","End Of Tunnel");

		tunnel=new emTkTunnel(grp,"t3","Square End");
		tunnel->SetChildTallness(1.0);
		new emTkGroup(tunnel,"e","End Of Tunnel");

		tunnel=new emTkTunnel(grp,"t4","Square End, Zero Depth");
		tunnel->SetChildTallness(1.0);
		tunnel->SetDepth(0.0);
		new emTkGroup(tunnel,"e","End Of Tunnel");

	grp=new emTkGroup(this,"dlgs","Dialogs");
	grp->SetBorderScaling(2.5);
	grp->SetFixedColumnCount(1);
		tlng=new emTkTiling(grp,"tlng");
		tlng->SetPrefChildTallness(0.1);
			CbTopLev=new emTkCheckBox(tlng,"tl","Top-Level");
			CbPZoom=new emTkCheckBox(tlng,"VF_POPUP_ZOOM","VF_POPUP_ZOOM");
			CbPZoom->SetChecked();
			CbModal=new emTkCheckBox(tlng,"WF_MODAL","WF_MODAL");
			CbModal->SetChecked();
			CbUndec=new emTkCheckBox(tlng,"WF_UNDECORATED","WF_UNDECORATED");
			CbPopup=new emTkCheckBox(tlng,"WF_POPUP","WF_POPUP");
			CbFull=new emTkCheckBox(tlng,"WF_FULLSCREEN","WF_FULLSCREEN");
		BtCreateDlg=new emTkButton(grp,"bt","Create Test Dialog");
		AddWakeUpSignal(BtCreateDlg->GetClickSignal());
}


emTestPanel::TkTest::~TkTest()
{
}


bool emTestPanel::TkTest::Cycle()
{
	emContext * ctx;
	emTkDialog * dlg;
	emView::ViewFlags vFlags;
	emWindow::WindowFlags wFlags;

	if (IsSignaled(SFLen->GetValueSignal())) {
		SFPos->SetMaxValue(SFLen->GetValue());
	}
	if (IsSignaled(BtCreateDlg->GetClickSignal())) {
		ctx=&GetView();
		if (CbTopLev->IsChecked()) ctx=&GetRootContext();
		vFlags=emView::VF_ROOT_SAME_TALLNESS;
		if (CbPZoom->IsChecked()) vFlags|=emView::VF_POPUP_ZOOM;
		wFlags=0;
		if (CbModal->IsChecked()) wFlags|=emWindow::WF_MODAL;
		if (CbUndec->IsChecked()) wFlags|=emWindow::WF_UNDECORATED;
		if (CbPopup->IsChecked()) wFlags|=emWindow::WF_POPUP;
		if (CbFull->IsChecked()) wFlags|=emWindow::WF_FULLSCREEN;
		dlg=new emTkDialog(*ctx,vFlags,wFlags);
		dlg->AddNegativeButton("Close");
		dlg->EnableAutoDeletion();
		dlg->SetRootTitle("Test Dialog");
		new TkTest(dlg->GetContentTiling(),"test");
	}
	return false;
}


void emTestPanel::TkTest::TextOfTimeValue(
	char * buf, int bufSize, emInt64 value, emUInt64 markInterval, void * context
)
{
	int h,m,s,ms;

	h=(int)(value/3600000);
	m=(int)((value/60000)%60);
	s=(int)((value/1000)%60);
	ms=(int)(value%1000);
	if (markInterval<10) {
		snprintf(buf,bufSize,"%02d:%02d:%02d\n.%03d",h,m,s,ms);
	}
	else if (markInterval<100) {
		snprintf(buf,bufSize,"%02d:%02d:%02d\n.%02d",h,m,s,ms/10);
	}
	else if (markInterval<1000) {
		snprintf(buf,bufSize,"%02d:%02d:%02d\n.%01d",h,m,s,ms/100);
	}
	else if (markInterval<60*1000) {
		snprintf(buf,bufSize,"%02d:%02d:%02d",h,m,s);
	}
	else {
		snprintf(buf,bufSize,"%02d:%02d",h,m);
	}
	buf[bufSize-1]=0;
}


void emTestPanel::TkTest::TextOfLevelValue(
	char * buf, int bufSize, emInt64 value, emUInt64 markInterval, void * context
)
{
	snprintf(buf,bufSize,"Level %d",(int)value);
	buf[bufSize-1]=0;
}


emTestPanel::TkTestGrp::TkTestGrp(ParentArg parent, const emString & name)
	: emTkGroup(parent,name)
{
	SetCaption("Toolkit Test");
	EnableAutoExpansion();
	SetAutoExpansionThreshold(900.0);
}


void emTestPanel::TkTestGrp::AutoExpand()
{
	emTkSplitter * sp, * sp1, * sp2;
	TkTest * t1a, * t1b, * t2a, * t2b;

	sp=new emTkSplitter(this,"sp");
	sp1=new emTkSplitter(sp,"sp1");
	sp1->SetVertical(true);
	sp2=new emTkSplitter(sp,"sp2");
	sp2->SetVertical(true);
	sp->SetPos(0.8);
	sp1->SetPos(0.8);
	sp2->SetPos(0.8);

	t1a=new TkTest(sp1,"t1a");
	t1b=new TkTest(sp1,"t1b");
	t2a=new TkTest(sp2,"t2a");
	t2b=new TkTest(sp2,"t2b");

	t2b->SetEnableSwitch(false);
	t2b->SetCaption("Disabled");
}
