//------------------------------------------------------------------------------
// emTestPanel.cpp
//
// Copyright (C) 2005-2009,2011,2014-2016,2020-2022 Oliver Hamann.
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
#include <emCore/emRes.h>
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

	TestImage=emGetInsResImage(GetRootContext(),"icons","teddy.tga");

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


emString emTestPanel::GetTitle() const
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


bool emTestPanel::IsOpaque() const
{
	return BgColor.IsOpaque();
}


void emTestPanel::Paint(const emPainter & painter, emColor canvasColor) const
{
	const emString * pstr;
	emColor fgCol;
	emString str;
	double h,a;
	double xy[128*2];
	int i,n;

	if (IsFocused()) fgCol=emColor(255,136,136);
	else if (IsInFocusedPath()) fgCol=emColor(187,136,136);
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

	if (GetViewCondition(VCT_WIDTH)<25.0) return;

	str="State:";
	if (IsFocused()) str+=" Focused";
	if (IsInFocusedPath()) str+=" InFocusedPath";
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
	painter.PaintEllipseSector(0.10,0.80,0.01,0.01,45,350,0xFFFFFFFF,BgColor);
	painter.PaintEllipseSector(0.11,0.80,0.02,0.01,45,-350,0xFFFFFFFF,BgColor);
	painter.PaintEllipseSector(0.13,0.80,0.005,0.01,245,50,0xFFFFFFFF,BgColor);
	painter.PaintEllipseSector(0.14,0.80,0.01,0.01,245,-50,0xFFFFFFFF,BgColor);

	painter.PaintRectOutline(0.05,0.82,0.01,0.01,0.001,0xFFFFFFFF,BgColor);
	painter.PaintRectOutline(0.07,0.82,0.02,0.01,0.001,emDashedStroke(0xFFFFFFFF),BgColor);
	painter.PaintRectOutline(0.10,0.82,0.01,0.01,0.008,0xFFFFFFFF,BgColor);
	painter.PaintRectOutline(0.13,0.82,0.01,0.01,0.011,0xFFFFFFFF,BgColor);

	painter.PaintRoundRect(0.05,0.84,0.01,0.01,0.001,0.001,0xFFFFFFFF,BgColor);
	painter.PaintRoundRect(0.07,0.84,0.02,0.01,0.001,0.002,0xFFFFFFFF,BgColor);
	painter.PaintRoundRect(0.10,0.84,0.01,0.01,0.003,0.002,0xFFFFFFFF,BgColor);
	painter.PaintRoundRect(0.13,0.84,0.01,0.01,0.001,0.011,0xFFFFFFFF,BgColor);
	painter.PaintRoundRect(0.15,0.84,0.01,0.01,0.000,0.00,0xFFFFFFFF,BgColor);

	painter.PaintEllipseOutline(0.05,0.86,0.01,0.01,0.003,0xFFFFFFFF,BgColor);
	painter.PaintEllipseOutline(0.065,0.86,0.02,0.01,0.001,0xFFFFFFFF,BgColor);
	painter.PaintEllipseOutline(0.09,0.86,0.005,0.01,0.00025,emRoundedDottedStroke(0xFFFFFFFF),BgColor);
	painter.PaintEllipseArc(0.10,0.86,0.01,0.01,90,225,0.001,0xFFFFFFFF,emStrokeEnd(),emStrokeEnd(),BgColor);
	painter.PaintEllipseSectorOutline(0.11,0.86,0.02,0.01,45,-320,0.0001,0xFFFFFFFF,BgColor);
	painter.PaintEllipseArc(0.13,0.86,0.005,0.01,245,50,0.001,0xFFFFFFFF,emStrokeEnd(),emStrokeEnd(),BgColor);
	painter.PaintEllipseArc(0.14,0.86,0.01,0.01,245,-50,0.001,0xFFFFFFFF,emStrokeEnd(),emStrokeEnd(),BgColor);
	painter.PaintEllipseArc(0.15,0.86,0.01,0.01,0,-145,0.0001,emRoundedStroke(0xFFFFFFFF),emStrokeEnd::CAP,emStrokeEnd::LINE_ARROW,BgColor);

	painter.PaintRoundRectOutline(0.05,0.88,0.01,0.01,0.001,0.001,0.001,0xFFFFFFFF,BgColor);
	painter.PaintRoundRectOutline(0.07,0.88,0.02,0.01,0.001,0.002,0.001,0xFFFFFFFF,BgColor);
	painter.PaintRoundRectOutline(0.10,0.88,0.01,0.01,0.003,0.002,0.003,0xFFFFFFFF,BgColor);
	painter.PaintRoundRectOutline(0.12,0.88,0.01,0.01,0.001,0.011,0.0001,0xFFFFFFFF,BgColor);
	painter.PaintRoundRectOutline(0.135,0.88,0.01,0.01,0.001,0.001,0.00002,emDashDottedStroke(0xFFFFFFFF),BgColor);
	painter.PaintRoundRectOutline(0.15,0.88,0.01,0.01,-0.0004,-0.0004,0.001,0xFFFFFFFF,BgColor);

	xy[0]=0.05; xy[1]=0.90;
	xy[2]=0.06; xy[3]=0.90;
	xy[4]=0.05; xy[5]=0.91;
	painter.PaintBezier(xy,3,0xFFFFFFFF,BgColor);

	xy[ 0]=0.065; xy[ 1]=0.91;
	xy[ 2]=0.05;  xy[ 3]=0.902;
	xy[ 4]=0.058; xy[ 5]=0.89;
	xy[ 6]=0.065; xy[ 7]=0.900;
	xy[ 8]=0.072; xy[ 9]=0.89;
	xy[10]=0.08;  xy[11]=0.902;
	painter.PaintBezier(xy,6,0xFFFFFFFF,BgColor);

	xy[ 0]=0.085; xy[ 1]=0.91;
	xy[ 2]=0.07;  xy[ 3]=0.902;
	xy[ 4]=0.078; xy[ 5]=0.89;
	xy[ 6]=0.085; xy[ 7]=0.900;
	xy[ 8]=0.092; xy[ 9]=0.89;
	xy[10]=0.10;  xy[11]=0.902;
	painter.PaintBezierOutline(xy,6,0.0002,emRoundedDashedStroke(0xFFFFFFFF),BgColor);

	xy[0]=0.105; xy[1]=0.91;
	xy[2]=0.09;  xy[3]=0.902;
	xy[4]=0.098; xy[5]=0.89;
	xy[6]=0.105; xy[7]=0.900;
	painter.PaintBezierLine(
		xy,4,0.0002,emRoundedDashedStroke(0xFFFFFFFF,1.0,0.5),
		emStrokeEnd(emStrokeEnd::CONTOUR_TRIANGLE,emColor::RED),
		emStrokeEnd::ARROW,BgColor
	);

	n=emStrokeEnd::STROKE+1;
	for (i=0; i<2*n; i++) {
		a=2*M_PI*i/(2*n);
		painter.PaintLine(
			0.117+0.002*cos(a),
			0.903+0.002*sin(a),
			0.117+0.0075*cos(a),
			0.903+0.0075*sin(a),
			0.0001,
			emStroke(0xFFFFFFFF,(i&1)!=0),
			emStrokeEnd::CAP,
			emStrokeEnd((emStrokeEnd::TypeEnum)(i/2),0xFFFFFF40)
		);
	}

	xy[0]=0.13; xy[1]=0.897;
	xy[2]=0.14; xy[3]=0.902;
	xy[4]=0.13; xy[5]=0.906;
	xy[6]=0.137; xy[7]=0.909;
	painter.PaintPolyline(
		xy,4,0.0005,emRoundedStroke(0xFFFFFFFF),
		emStrokeEnd(emStrokeEnd::CONTOUR_ARROW,0),emStrokeEnd::CAP,
		BgColor
	);

	xy[0]=0.06; xy[1]=0.80;
	xy[2]=0.10; xy[3]=0.85;
	xy[4]=0.08; xy[5]=0.91;
	painter.PaintPolygonOutline(xy,3,0.0002,emColor(255,0,0));

	xy[ 0]=0.200; xy[ 1]=0.905;
	xy[ 2]=0.215; xy[ 3]=0.912;
	xy[ 4]=0.230; xy[ 5]=0.900;
	xy[ 6]=0.222; xy[ 7]=0.915;
	xy[ 8]=0.230; xy[ 9]=0.930;
	xy[10]=0.220; xy[11]=0.922;
	xy[12]=0.205; xy[13]=0.935;
	xy[14]=0.212; xy[15]=0.920;
	painter.PaintPolygon(
		xy,8,emLinearGradientTexture(0.23,0.9,0x00FF0080,0.2,0.93,0xFFFF00FF)
	);

	xy[ 0]=0.220; xy[ 1]=0.905;
	xy[ 2]=0.235; xy[ 3]=0.912;
	xy[ 4]=0.250; xy[ 5]=0.900;
	xy[ 6]=0.242; xy[ 7]=0.915;
	xy[ 8]=0.250; xy[ 9]=0.930;
	xy[10]=0.240; xy[11]=0.922;
	xy[12]=0.225; xy[13]=0.935;
	xy[14]=0.232; xy[15]=0.920;
	painter.PaintPolygon(
		xy,8,emRadialGradientTexture(0.21,0.90,0.05,0.035,0xCCCC33FF,0x0000FF60)
	);

	xy[ 0]=0.240; xy[ 1]=0.905;
	xy[ 2]=0.255; xy[ 3]=0.912;
	xy[ 4]=0.270; xy[ 5]=0.900;
	xy[ 6]=0.262; xy[ 7]=0.915;
	xy[ 8]=0.270; xy[ 9]=0.930;
	xy[10]=0.260; xy[11]=0.922;
	xy[12]=0.245; xy[13]=0.935;
	xy[14]=0.252; xy[15]=0.920;
	painter.PaintPolygon(
		xy,8,
		emImageTexture(
			0.0,
			0.0,
			0.002,
			0.002*TestImage.GetHeight()/TestImage.GetWidth(),
			TestImage,192
		)
	);

	painter.PaintRect(
		0.2,0.94,0.02,0.01,
		emLinearGradientTexture(0.207,0.944,0x00000080,0.213,0.946,0x80808080)
	);

	painter.PaintRect(
		0.221,0.94,0.008,0.01,
		emRadialGradientTexture(0.223,0.941,0.004,0.008,0xFF8800FF,0x005500FF)
	);

	painter.PaintEllipse(
		0.23,0.94,0.02,0.01,
		emRadialGradientTexture(0.23,0.94,0.02,0.01,0,0x00cc88FF)
	);

	painter.PaintRect(
		0.26,0.94,0.02,0.01,
		emImageTexture(
			0.26,
			0.94,
			0.001,
			0.001*TestImage.GetHeight()/TestImage.GetWidth(),
			TestImage
		)
	);

	painter.PaintRect(
		0.2625,0.942,0.02,0.01,
		emImageColoredTexture(
			1.0005,
			0.942,
			0.001,
			0.001*TestImage.GetHeight()/TestImage.GetWidth(),
			TestImage,
			0x00FFFFFF,0xFF0000FF
		)
	);

	painter.PaintRect(
		0.275,0.907,0.002,0.002,
		emImageTexture(
			0.2755,0.9075,0.001,0.001,
			TestImage,50,10,110,110,
			255,emTexture::EXTEND_TILED
		)
	);

	painter.PaintRect(
		0.275,0.910,0.002,0.002,
		emImageTexture(
			0.2755,0.9105,0.001,0.001,
			TestImage,50,10,110,110,
			255,emTexture::EXTEND_EDGE
		)
	);

	painter.PaintRect(
		0.275,0.913,0.002,0.002,
		emImageTexture(
			0.2755,0.9135,0.001,0.001,
			TestImage,50,10,110,110,
			255,emTexture::EXTEND_ZERO
		)
	);
}


void emTestPanel::AutoExpand()
{
	TkT=new TkTestGrp(this,"TkTestGrp");
	TP1=new emTestPanel(this,"1");
	TP2=new emTestPanel(this,"2");
	TP3=new emTestPanel(this,"3");
	TP4=new emTestPanel(this,"4");
	BgColorField=new emColorField(
		this,"BgColorField",
		"Background Color",emString(),emImage(),
		BgColor,true,true
	);
	PolyDraw=new PolyDrawPanel(this,"PolyDraw");
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
	if (PolyDraw) PolyDraw->Layout(0.05,0.92,0.08,0.04,BgColor);
}


emPanel * emTestPanel::CreateControlPanel(
	ParentArg parent, const emString & name
)
{
	ControlPanel=new emLabel(parent,name);
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
	: emRasterGroup(parent,name)
{
	emRasterGroup * grp;
	emRasterLayout * rl;
	emButton * bt;
	emTextField * tf;
	emColorField * cf;
	emScalarField * sf;
	emTunnel * tunnel;
	emListBox * listBox;
	emFileSelectionBox * fsb;
	emString str;
	int i;

	SetCaption("Toolkit Test");
	SetPrefChildTallness(0.3);

	grp=new emRasterGroup(this,"buttons","Buttons");
	grp->SetBorderScaling(2.5);
		bt=new emButton(grp,"b1","Button");
		bt=new emButton(grp,"b2","Long Desc");
		str="";
		for (i=0; i<100; i++) {
			str+="This is a looooooooooooooooooooooooooooooooooooooooooooooooooooooong description of the button.\n";
		}
		bt->SetDescription(str);
		bt=new emButton(grp,"b3","NoEOI");
		bt->SetNoEOI();

	grp=new emRasterGroup(this,"checkbuttons","Check Buttons and Boxes");
	grp->SetBorderScaling(2.5);
		new emCheckButton(grp,"c1","Check Button");
		new emCheckButton(grp,"c2","Check Button");
		new emCheckButton(grp,"c3","Check Button");
		new emCheckBox(grp,"c4","Check Box");
		new emCheckBox(grp,"c5","Check Box");
		new emCheckBox(grp,"c6","Check Box");

	grp=new emRadioButton::RasterGroup(this,"radiobuttons","Radio Buttons and Boxes");
	grp->SetBorderScaling(2.5);
		new emRadioButton(grp,"r1","Radio Button");
		new emRadioButton(grp,"r2","Radio Button");
		new emRadioButton(grp,"r3","Radio Button");
		new emRadioBox(grp,"r4","Radio Box");
		new emRadioBox(grp,"r5","Radio Box");
		new emRadioBox(grp,"r6","Radio Box");

	grp=new emRasterGroup(this,"textfields","Text Fields");
	grp->SetBorderScaling(2.5);
		tf=new emTextField(
			grp,"tf1",
			"Read-Only","This is a read-only text field.",emImage(),
			"Read-Only"
		);
		tf=new emTextField(
			grp,"tf2",
			"Editable","This is an editable text field.",emImage(),
			"Editable",true
		);
		tf=new emTextField(
			grp,"tf3",
			"Password","This is an editable password text field.",emImage(),
			"Password",true
		);
		tf->SetPasswordMode();
		tf=new emTextField(
			grp,"mltf1",
			"Multi-Line","This is an editable multi-line text field.",emImage(),
			"first line\nsecond line\n...",true
		);
		tf->SetMultiLineMode();

	grp=new emRasterGroup(this,"scalarfields","Scalar Fields");
	grp->SetBorderScaling(2.5);
	grp->SetPrefChildTallness(0.1);

		sf=new emScalarField(grp,"sf1","Read-Only");

		sf=new emScalarField(grp,"sf2","Editable");
		sf->SetEditable();

		sf=new emScalarField(grp,"sf3");
		sf->SetEditable();
		sf->SetMinMaxValues(-1000,1000);
		sf->SetScaleMarkIntervals(1000,100,10,5,1,0);

		sf=new emScalarField(grp,"sf4","Level");
		sf->SetEditable();
		sf->SetTextBoxTallness(0.25);
		sf->SetMinMaxValues(1,5);
		sf->SetValue(3);
		sf->SetTextOfValueFunc(TextOfLevelValue,NULL);

		SFLen=new emScalarField(grp,"sf5","Play Length");
		SFLen->SetEditable();
		SFLen->SetMinMaxValues(0,24*3600*1000);
		SFLen->SetValue(4*3600*1000);
		SFLen->SetScaleMarkIntervals(60*60*1000,15*60*1000,5*60*1000,60*1000,10*1000,1000,100,10,1,0);
		SFLen->SetTextOfValueFunc(TextOfTimeValue,NULL);
		AddWakeUpSignal(SFLen->GetValueSignal());

		SFPos=new emScalarField(grp,"sf6","Play Position");
		SFPos->SetEditable();
		SFPos->SetMinMaxValues(0,SFLen->GetValue());
		SFPos->SetScaleMarkIntervals(60*60*1000,15*60*1000,5*60*1000,60*1000,10*1000,1000,100,10,1,0);
		SFPos->SetTextOfValueFunc(TextOfTimeValue,NULL);

	grp=new emRasterGroup(this,"colorfields","Color Fields");
	grp->SetBorderScaling(2.5);
	grp->SetPrefChildTallness(0.4);

		cf=new emColorField(grp,"cf1","Read-Only");
		cf->SetColor(0xBB2222FF);

		cf=new emColorField(grp,"cf2","Editable");
		cf->SetColor(0x22BB22FF);
		cf->SetEditable();

		cf=new emColorField(grp,"cf3","Editable, Alpha Enabled");
		cf->SetColor(0x2222BBFF);
		cf->SetEditable();
		cf->SetAlphaEnabled();

	grp=new emRasterGroup(this,"tunnels","Tunnels");
	grp->SetBorderScaling(2.5);
	grp->SetPrefChildTallness(0.4);

		tunnel=new emTunnel(grp,"t1","Tunnel");
		new emButton(tunnel,"e","End Of Tunnel");

		tunnel=new emTunnel(grp,"t2","Deeper Tunnel");
		tunnel->SetDepth(30.0);
		new emRasterGroup(tunnel,"e","End Of Tunnel");

		tunnel=new emTunnel(grp,"t3","Square End");
		tunnel->SetChildTallness(1.0);
		new emRasterGroup(tunnel,"e","End Of Tunnel");

		tunnel=new emTunnel(grp,"t4","Square End, Zero Depth");
		tunnel->SetChildTallness(1.0);
		tunnel->SetDepth(0.0);
		new emRasterGroup(tunnel,"e","End Of Tunnel");

	grp=new emRasterGroup(this,"listboxes","List Boxes");
	grp->SetBorderScaling(2.5);
	grp->SetPrefChildTallness(0.4);

		listBox=new emListBox(grp,"l1","Empty");

		listBox=new emListBox(grp,"l2","Single-Selection");
		for (i=1; i<=7; i++) {
			listBox->AddItem(emString::Format("%d",i),emString::Format("Item %d",i));
		}
		listBox->SetSelectedIndex(0);

		listBox=new emListBox(grp,"l3","Read-Only");
		listBox->SetSelectionType(emListBox::READ_ONLY_SELECTION);
		for (i=1; i<=7; i++) {
			listBox->AddItem(emString::Format("%d",i),emString::Format("Item %d",i));
		}
		listBox->SetSelectedIndex(2);

		listBox=new emListBox(grp,"l4","Multi-Selection");
		listBox->SetSelectionType(emListBox::MULTI_SELECTION);
		for (i=1; i<=7; i++) {
			listBox->AddItem(emString::Format("%d",i),emString::Format("Item %d",i));
		}
		listBox->Select(1);
		listBox->Select(2);
		listBox->Select(3);
		listBox->Select(4);

		listBox=new emListBox(grp,"l5","Toggle-Selection");
		listBox->SetSelectionType(emListBox::TOGGLE_SELECTION);
		for (i=1; i<=7; i++) {
			listBox->AddItem(emString::Format("%d",i),emString::Format("Item %d",i));
		}
		listBox->Select(2);
		listBox->Select(4);

		listBox=new emListBox(grp,"l6","Single Column");
		listBox->SetFixedColumnCount(1);
		for (i=1; i<=7; i++) {
			listBox->AddItem(emString::Format("%d",i),emString::Format("Item %d",i));
		}
		listBox->SetSelectedIndex(0);

		listBox=new CustomListBox(grp,"l7","Custom List Box");
		listBox->SetSelectionType(emListBox::MULTI_SELECTION);
		for (i=1; i<=7; i++) {
			listBox->AddItem(emString::Format("%d",i),emString::Format("Item %d",i));
		}
		listBox->SetSelectedIndex(0);

	grp=new emRasterGroup(this,"dlgs","Test Dialog");
	grp->SetBorderScaling(2.5);
	grp->SetFixedColumnCount(1);
		rl=new emRasterLayout(grp,"rl");
		rl->SetPrefChildTallness(0.1);
			CbTopLev=new emCheckBox(rl,"tl","Top-Level");
			CbPZoom=new emCheckBox(rl,"VF_POPUP_ZOOM","VF_POPUP_ZOOM");
			CbPZoom->SetChecked();
			CbModal=new emCheckBox(rl,"WF_MODAL","WF_MODAL");
			CbModal->SetChecked();
			CbUndec=new emCheckBox(rl,"WF_UNDECORATED","WF_UNDECORATED");
			CbPopup=new emCheckBox(rl,"WF_POPUP","WF_POPUP");
			CbMax=new emCheckBox(rl,"WF_MAXIMIZED","WF_MAXIMIZED");
			CbFull=new emCheckBox(rl,"WF_FULLSCREEN","WF_FULLSCREEN");
		BtCreateDlg=new emButton(grp,"bt","Create Test Dialog");
		AddWakeUpSignal(BtCreateDlg->GetClickSignal());

	grp=new emRasterGroup(this,"fileChoosers","File Selection");
	grp->SetBorderScaling(2.5);
	grp->SetPrefChildTallness(0.3);
		fsb=new emFileSelectionBox(grp,"l8","File Selection Box");
		emArray<emString> filters;
		filters.Add("All Files (*)");
		filters.Add("Image Files (*.bmp *.gif *.jpg *.png *.tga)");
		filters.Add("HTML Files (*.htm *.html)");
		fsb->SetFilters(filters);
		BtOpenFile=new emButton(grp,"openFile","Open...");
		AddWakeUpSignal(BtOpenFile->GetClickSignal());
		BtOpenFiles=new emButton(grp,"openFiles","Open Multi, Allow Dir...");
		AddWakeUpSignal(BtOpenFiles->GetClickSignal());
		BtSaveFile=new emButton(grp,"saveFile","Save As...");
		AddWakeUpSignal(BtSaveFile->GetClickSignal());

	FileDlg=NULL;
}


emTestPanel::TkTest::~TkTest()
{
}


bool emTestPanel::TkTest::Cycle()
{
	emContext * ctx;
	emDialog * dlg;
	emView::ViewFlags vFlags;
	emWindow::WindowFlags wFlags;
	emArray<emString> names;
	emString str;
	int i;

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
		if (CbMax->IsChecked()) wFlags|=emWindow::WF_MAXIMIZED;
		if (CbFull->IsChecked()) wFlags|=emWindow::WF_FULLSCREEN;
		dlg=new emDialog(*ctx,vFlags,wFlags);
		dlg->AddNegativeButton("Close");
		dlg->EnableAutoDeletion();
		dlg->SetRootTitle("Test Dialog");
		new TkTest(dlg->GetContentPanel(),"test");
	}
	if (IsSignaled(BtOpenFile->GetClickSignal())) {
		if (FileDlg) delete FileDlg;
		FileDlg=new emFileDialog(GetView(),emFileDialog::M_OPEN);
		AddWakeUpSignal(FileDlg->GetFinishSignal());
	}
	if (IsSignaled(BtOpenFiles->GetClickSignal())) {
		if (FileDlg) delete FileDlg;
		FileDlg=new emFileDialog(GetView(),emFileDialog::M_OPEN);
		FileDlg->SetMultiSelectionEnabled();
		FileDlg->SetDirectoryResultAllowed();
		AddWakeUpSignal(FileDlg->GetFinishSignal());
	}
	if (IsSignaled(BtSaveFile->GetClickSignal())) {
		if (FileDlg) delete FileDlg;
		FileDlg=new emFileDialog(GetView(),emFileDialog::M_SAVE);
		AddWakeUpSignal(FileDlg->GetFinishSignal());
	}
	if (FileDlg && IsSignaled(FileDlg->GetFinishSignal())) {
		if (FileDlg->GetResult()==emDialog::POSITIVE) {
			names=FileDlg->GetSelectedNames();
			str="File dialog finished with positive result. Would load or save:\n";
			if (names.GetCount() <= 1) {
				str+=FileDlg->GetSelectedPath();
			}
			else {
				for (i=0; i<names.GetCount(); i++) {
					str+=emString("  ")+names[i]+emString("\n");
				}
				str+=emString("From:\n  ")+FileDlg->GetParentDirectory();
			}
			emDialog::ShowMessage(GetView(),"Result",str);
		}
		delete FileDlg;
		FileDlg=NULL;
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
	: emRasterGroup(parent,name)
{
	SetCaption("Toolkit Test");
	SetAutoExpansionThreshold(900.0);
}


void emTestPanel::TkTestGrp::AutoExpand()
{
	emSplitter * sp, * sp1, * sp2;
	TkTest * t1a, * t1b, * t2a, * t2b;

	sp=new emSplitter(this,"sp");
	sp1=new emSplitter(sp,"sp1");
	sp1->SetVertical(true);
	sp2=new emSplitter(sp,"sp2");
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


emTestPanel::CustomItemPanel::CustomItemPanel(
	emListBox & listBox, const emString & name, int itemIndex
) :
	emLinearGroup(listBox,name),
	emListBox::ItemPanelInterface(listBox,itemIndex)
{
	SetBorderScaling(5.0);
	SetHorizontal();
	ItemTextChanged();
	ItemSelectionChanged();
}


emTestPanel::CustomItemPanel::~CustomItemPanel()
{
}


void emTestPanel::CustomItemPanel::Input(
	emInputEvent & event, const emInputState & state, double mx, double my
)
{
	ProcessItemInput(this,event,state);
	emLinearGroup::Input(event,state,mx,my);
}


void emTestPanel::CustomItemPanel::AutoExpand()
{
	emLabel * label;
	emTestPanel::CustomListBox * listBox;
	int i;

	label=new emLabel(this,"t","This is a custom list\nbox item panel (it is\nrecursive...)");
	label->SetLook(GetListBox().GetLook());
	listBox = new emTestPanel::CustomListBox(this,"l","Child List Box");
	listBox->SetLook(GetListBox().GetLook());
	listBox->SetSelectionType(emListBox::MULTI_SELECTION);
	for (i=1; i<=7; i++) {
		listBox->AddItem(emString::Format("%d",i),emString::Format("Item %d",i));
	}
	listBox->SetSelectedIndex(0);
}


void emTestPanel::CustomItemPanel::ItemTextChanged()
{
	SetCaption(GetItemText());
}


void emTestPanel::CustomItemPanel::ItemDataChanged()
{
}


void emTestPanel::CustomItemPanel::ItemSelectionChanged()
{
	emLook look;

	if (IsItemSelected()) {
		look=GetLook();
		look.SetBgColor(emColor(224,80,128));
		SetLook(look);
	}
	else {
		SetLook(GetListBox().GetLook());
	}
}


emTestPanel::CustomListBox::CustomListBox(
	ParentArg parent, const emString & name,
	const emString & caption, const emString & description,
	const emImage & icon, SelectionType selType
) :
	emListBox(parent,name,caption,description,icon,selType)
{
	SetChildTallness(0.4);
	SetAlignment(EM_ALIGN_TOP_LEFT);
	SetStrictRaster();
}


void emTestPanel::CustomListBox::CreateItemPanel(const emString & name, int itemIndex)
{
	new emTestPanel::CustomItemPanel(*this,name,itemIndex);
}


emTestPanel::PolyDrawPanel::PolyDrawPanel(ParentArg parent, const emString & name)
	: emLinearGroup(
		parent,name,"Poly Draw Test",
		"This allows manual testing of various paint functions. Main focus is\n"
		"on strokes an stroke ends, i.e. textures cannot be tested with this.\n"
	)
{
	SetOrientationThresholdTallness(1.0);
}


bool emTestPanel::PolyDrawPanel::Cycle()
{
	if (
		Canvas && (
			IsSignaled(Type->GetCheckSignal()) ||
			IsSignaled(VertexCount->GetTextSignal()) ||
			IsSignaled(WithCanvasColor->GetCheckSignal()) ||
			IsSignaled(FillColor->GetColorSignal()) ||
			IsSignaled(StrokeWidth->GetTextSignal()) ||
			IsSignaled(StrokeColor->GetColorSignal()) ||
			IsSignaled(StrokeRounded->GetCheckSignal()) ||
			IsSignaled(StrokeDashType->GetCheckSignal()) ||
			IsSignaled(DashLengthFactor->GetTextSignal()) ||
			IsSignaled(GapLengthFactor->GetTextSignal()) ||
			IsSignaled(StrokeStartType->GetCheckSignal()) ||
			IsSignaled(StrokeStartInnerColor->GetColorSignal()) ||
			IsSignaled(StrokeStartWidthFactor->GetTextSignal()) ||
			IsSignaled(StrokeStartLengthFactor->GetTextSignal()) ||
			IsSignaled(StrokeEndType->GetCheckSignal()) ||
			IsSignaled(StrokeEndInnerColor->GetColorSignal()) ||
			IsSignaled(StrokeEndWidthFactor->GetTextSignal()) ||
			IsSignaled(StrokeEndLengthFactor->GetTextSignal())
		)
	) {
		Canvas->Setup(
			Type->GetCheckIndex(),
			atoi(VertexCount->GetText()),
			WithCanvasColor->IsChecked(),
			emTexture(FillColor->GetColor()),
			atof(StrokeWidth->GetText()),
			emStroke(
				StrokeColor->GetColor(),
				StrokeRounded->IsChecked(),
				(emStroke::DashTypeEnum)StrokeDashType->GetCheckIndex(),
				atof(DashLengthFactor->GetText()),
				atof(GapLengthFactor->GetText())
			),
			emStrokeEnd(
				(emStrokeEnd::TypeEnum)StrokeStartType->GetCheckIndex(),
				StrokeStartInnerColor->GetColor(),
				atof(StrokeStartWidthFactor->GetText()),
				atof(StrokeStartLengthFactor->GetText())
			),
			emStrokeEnd(
				(emStrokeEnd::TypeEnum)StrokeEndType->GetCheckIndex(),
				StrokeEndInnerColor->GetColor(),
				atof(StrokeEndWidthFactor->GetText()),
				atof(StrokeEndLengthFactor->GetText())
			)
		);
	}

	return false;
}


void emTestPanel::PolyDrawPanel::AutoExpand()
{
	emRasterLayout * controls;
	emLinearGroup * general, * stroke, * strokeStart, * strokeEnd;
	emLinearLayout * ll;

	controls=new emRasterLayout(this,"Controls");
	controls->SetPrefChildTallness(0.6);

	general=new emLinearGroup(controls,"general","General");
	general->SetBorderScaling(2.0);
	general->SetChildWeight(0,2.0);

	stroke=new emLinearGroup(controls,"stroke","Stroke");
	stroke->SetBorderScaling(2.0);
	stroke->SetChildWeight(2,2.0);

	strokeStart=new emLinearGroup(controls,"strokeStart","Stroke Start");
	strokeStart->SetBorderScaling(2.0);
	strokeStart->SetChildWeight(0,2.0);

	strokeEnd=new emLinearGroup(controls,"strokeEnd","Stroke End");
	strokeEnd->SetBorderScaling(2.0);
	strokeEnd->SetChildWeight(0,2.0);

	Type=new emRadioButton::RasterGroup(general,"Method","Method");
	new emRadioBox(*Type,"0","PaintPolygon");
	new emRadioBox(*Type,"1","PaintPolygonOutline");
	new emRadioBox(*Type,"2","PaintPolyline");
	new emRadioBox(*Type,"3","PaintBezier");
	new emRadioBox(*Type,"4","PaintBezierOutline");
	new emRadioBox(*Type,"5","PaintBezierLine");
	new emRadioBox(*Type,"6","PaintLine");
	new emRadioBox(*Type,"7","PaintRect");
	new emRadioBox(*Type,"8","PaintRectOutline");
	new emRadioBox(*Type,"9","PaintEllipse");
	new emRadioBox(*Type,"10","PaintEllipseOutline");
	new emRadioBox(*Type,"11","PaintEllipseSector");
	new emRadioBox(*Type,"12","PaintEllipseSectorOutline");
	new emRadioBox(*Type,"13","PaintEllipseArc");
	new emRadioBox(*Type,"14","PaintRoundRect");
	new emRadioBox(*Type,"15","PaintRoundRectOutline");
	Type->SetBorderScaling(1.5);
	Type->SetPrefChildTallness(0.07);
	Type->SetCheckIndex(0);
	AddWakeUpSignal(Type->GetCheckSignal());

	ll=new emLinearLayout(general,"ll");
	ll->SetHorizontal();

	VertexCount=new emTextField(ll,"VertexCount","Vertex Count");
	VertexCount->SetEditable();
	VertexCount->SetText("9");
	AddWakeUpSignal(VertexCount->GetTextSignal());

	FillColor=new emColorField(ll,"FillColor","Fill Color");
	FillColor->SetEditable();
	FillColor->SetAlphaEnabled();
	FillColor->SetColor(0xFFFFFFFF);
	AddWakeUpSignal(FillColor->GetColorSignal());

	ll=new emLinearLayout(general,"ll2");
	ll->SetHorizontal();

	StrokeWidth=new emTextField(ll,"StrokeWidth","Stroke Width");
	StrokeWidth->SetEditable();
	StrokeWidth->SetText("0.01");
	AddWakeUpSignal(StrokeWidth->GetTextSignal());

	WithCanvasColor=new emCheckBox(ll,"WithCanvasColor","With Canvas Color");
	AddWakeUpSignal(WithCanvasColor->GetCheckSignal());
	WithCanvasColor->SetChecked(false);

	StrokeColor=new emColorField(stroke,"StrokeColor","Color");
	StrokeColor->SetEditable();
	StrokeColor->SetAlphaEnabled();
	StrokeColor->SetColor(0x000000FF);
	AddWakeUpSignal(StrokeColor->GetColorSignal());

	StrokeRounded=new emCheckBox(stroke,"StrokeRounded","Rounded");
	AddWakeUpSignal(StrokeRounded->GetCheckSignal());

	StrokeDashType=new emRadioButton::RasterGroup(stroke,"StrokeDashType","Dash Type");
	new emRadioBox(*StrokeDashType,"0","SOLID");
	new emRadioBox(*StrokeDashType,"1","DASHED");
	new emRadioBox(*StrokeDashType,"2","DOTTED");
	new emRadioBox(*StrokeDashType,"3","DASH_DOTTED");
	StrokeDashType->SetBorderScaling(1.5);
	StrokeDashType->SetPrefChildTallness(0.08);
	StrokeDashType->SetCheckIndex(0);
	AddWakeUpSignal(StrokeDashType->GetCheckSignal());

	ll=new emLinearLayout(stroke,"ll");
	ll->SetHorizontal();

	DashLengthFactor=new emTextField(ll,"DashLengthFactor","Dash Length Factor");
	DashLengthFactor->SetEditable();
	DashLengthFactor->SetText("1.0");
	AddWakeUpSignal(DashLengthFactor->GetTextSignal());

	GapLengthFactor=new emTextField(ll,"GapLengthFactor","Gap Length Factor");
	GapLengthFactor->SetEditable();
	GapLengthFactor->SetText("1.0");
	AddWakeUpSignal(GapLengthFactor->GetTextSignal());

	StrokeStartType=new emRadioButton::RasterGroup(strokeStart,"StrokeStartType","Type");
	new emRadioBox(*StrokeStartType,"0","BUTT");
	new emRadioBox(*StrokeStartType,"1","CAP");
	new emRadioBox(*StrokeStartType,"2","ARROW");
	new emRadioBox(*StrokeStartType,"3","CONTOUR_ARROW");
	new emRadioBox(*StrokeStartType,"4","LINE_ARROW");
	new emRadioBox(*StrokeStartType,"5","TRIANGLE");
	new emRadioBox(*StrokeStartType,"6","CONTOUR_TRIANGLE");
	new emRadioBox(*StrokeStartType,"7","SQUARE");
	new emRadioBox(*StrokeStartType,"8","CONTOUR_SQUARE");
	new emRadioBox(*StrokeStartType,"9","HALF_SQUARE");
	new emRadioBox(*StrokeStartType,"10","CIRCLE");
	new emRadioBox(*StrokeStartType,"11","CONTOUR_CIRCLE");
	new emRadioBox(*StrokeStartType,"12","HALF_CIRCLE");
	new emRadioBox(*StrokeStartType,"13","DIAMOND");
	new emRadioBox(*StrokeStartType,"14","CONTOUR_DIAMOND");
	new emRadioBox(*StrokeStartType,"15","HALF_DIAMOND");
	new emRadioBox(*StrokeStartType,"16","STROKE");
	StrokeStartType->SetBorderScaling(1.5);
	StrokeStartType->SetPrefChildTallness(0.08);
	StrokeStartType->SetCheckIndex(0);
	AddWakeUpSignal(StrokeStartType->GetCheckSignal());

	StrokeStartInnerColor=new emColorField(strokeStart,"StrokeStartInnerColor","Inner Color");
	StrokeStartInnerColor->SetEditable();
	StrokeStartInnerColor->SetAlphaEnabled();
	StrokeStartInnerColor->SetColor(0xEEEEEEFF);
	AddWakeUpSignal(StrokeStartInnerColor->GetColorSignal());

	ll=new emLinearLayout(strokeStart,"ll");
	ll->SetHorizontal();

	StrokeStartWidthFactor=new emTextField(ll,"StrokeStartWidthFactor","Width Factor");
	StrokeStartWidthFactor->SetEditable();
	StrokeStartWidthFactor->SetText("1.0");
	AddWakeUpSignal(StrokeStartWidthFactor->GetTextSignal());

	StrokeStartLengthFactor=new emTextField(ll,"StrokeStartLengthFactor","Length Factor");
	StrokeStartLengthFactor->SetEditable();
	StrokeStartLengthFactor->SetText("1.0");
	AddWakeUpSignal(StrokeStartLengthFactor->GetTextSignal());

	StrokeEndType=new emRadioButton::RasterGroup(strokeEnd,"StrokeEndType","Type");
	new emRadioBox(*StrokeEndType,"0","BUTT");
	new emRadioBox(*StrokeEndType,"1","CAP");
	new emRadioBox(*StrokeEndType,"2","ARROW");
	new emRadioBox(*StrokeEndType,"3","CONTOUR_ARROW");
	new emRadioBox(*StrokeEndType,"4","LINE_ARROW");
	new emRadioBox(*StrokeEndType,"5","TRIANGLE");
	new emRadioBox(*StrokeEndType,"6","CONTOUR_TRIANGLE");
	new emRadioBox(*StrokeEndType,"7","SQUARE");
	new emRadioBox(*StrokeEndType,"8","CONTOUR_SQUARE");
	new emRadioBox(*StrokeEndType,"9","HALF_SQUARE");
	new emRadioBox(*StrokeEndType,"10","CIRCLE");
	new emRadioBox(*StrokeEndType,"11","CONTOUR_CIRCLE");
	new emRadioBox(*StrokeEndType,"12","HALF_CIRCLE");
	new emRadioBox(*StrokeEndType,"13","DIAMOND");
	new emRadioBox(*StrokeEndType,"14","CONTOUR_DIAMOND");
	new emRadioBox(*StrokeEndType,"15","HALF_DIAMOND");
	new emRadioBox(*StrokeEndType,"16","STROKE");
	StrokeEndType->SetBorderScaling(1.5);
	StrokeEndType->SetPrefChildTallness(0.08);
	StrokeEndType->SetCheckIndex(0);
	AddWakeUpSignal(StrokeEndType->GetCheckSignal());

	StrokeEndInnerColor=new emColorField(strokeEnd,"StrokeEndInnerColor","Inner Color");
	StrokeEndInnerColor->SetEditable();
	StrokeEndInnerColor->SetAlphaEnabled();
	StrokeEndInnerColor->SetColor(0xEEEEEEFF);
	AddWakeUpSignal(StrokeEndInnerColor->GetColorSignal());

	ll=new emLinearLayout(strokeEnd,"ll");
	ll->SetHorizontal();

	StrokeEndWidthFactor=new emTextField(ll,"StrokeEndWidthFactor","Width Factor");
	StrokeEndWidthFactor->SetEditable();
	StrokeEndWidthFactor->SetText("1.0");
	AddWakeUpSignal(StrokeEndWidthFactor->GetTextSignal());

	StrokeEndLengthFactor=new emTextField(ll,"StrokeEndLengthFactor","Length Factor");
	StrokeEndLengthFactor->SetEditable();
	StrokeEndLengthFactor->SetText("1.0");
	AddWakeUpSignal(StrokeEndLengthFactor->GetTextSignal());

	Canvas=new CanvasPanel(this,"CanvasPanel");
}


emTestPanel::PolyDrawPanel::CanvasPanel::CanvasPanel(
	ParentArg parent, const emString & name
) :
	emPanel(parent,name),
	Type(0),
	DragIdx(-1),
	ShowHandles(false)
{
}


void emTestPanel::PolyDrawPanel::CanvasPanel::Setup(
	int type, int vertexCount, bool withCanvasColor, const emTexture& texture,
	double strokeWidth, const emStroke& stroke, const emStrokeEnd& strokeStart,
	const emStrokeEnd& strokeEnd
)
{
	int i;

	Type=type;
	if (XY.GetCount()>vertexCount*2) {
		XY.SetCount(vertexCount*2);
		DragIdx=-1;
	}
	else if (XY.GetCount()<vertexCount*2) {
		XY.SetCount(vertexCount*2);
		for (i=0; i<vertexCount; i++) {
			XY.Set(i*2,cos(M_PI*2*i/vertexCount)*0.4+0.5);
			XY.Set(i*2+1,GetHeight()*(sin(M_PI*2*i/vertexCount)*0.4+0.5));
		}
		DragIdx=-1;
	}
	WithCanvasColor=withCanvasColor;
	Texture=texture;
	StrokeWidth=strokeWidth;
	Stroke=stroke;
	StrokeStart=strokeStart;
	StrokeEnd=strokeEnd;
	InvalidatePainting();
}


void emTestPanel::PolyDrawPanel::CanvasPanel::Input(
	emInputEvent & event, const emInputState & state, double mx, double my
)
{
	double dx,dy,x,y,r,bestR;
	int i,bestI;
	bool b;

	if (DragIdx<0 && event.IsLeftButton()) {
		event.Eat();
		Focus();
		bestI=-1;
		bestR=ViewToPanelDeltaX(12.0);
		for (i=0; i<XY.GetCount()/2; i++) {
			dx=XY[i*2]-mx;
			dy=XY[i*2+1]-my;
			r=sqrt(dx*dx+dy*dy);
			if (bestR>r) {
				bestI=i;
				bestR=r;
			}
		}
		if (bestI>=0) {
			DragIdx=bestI;
			DragDX=XY[bestI*2]-mx;
			DragDY=XY[bestI*2+1]-my;
			InvalidatePainting();
		}
	}
	else if (DragIdx>=0 && !state.GetLeftButton()) {
		DragIdx=-1;
		InvalidatePainting();
	}
	else if (DragIdx>=0) {
		x=emMin(emMax(mx+DragDX,0.0),1.0);
		y=emMin(emMax(my+DragDY,0.0),GetHeight());
		if (state.GetShift() || state.GetCtrl() || state.GetAlt()) {
			for (r=0.1; IsViewed() && PanelToViewDeltaX(r)>20.0; r*=0.5);
			x=round(x/r)*r;
			y=round(y/r)*r;
		}
		if (XY[DragIdx*2]!=x || XY[DragIdx*2+1]!=y) {
			XY.Set(DragIdx*2,x);
			XY.Set(DragIdx*2+1,y);
			InvalidatePainting();
		}
	}

	b=DragIdx>=0 || (mx>=0.0 && mx<1.0 && my>=0.0 && my<GetHeight());
	if (ShowHandles!=b) {
		ShowHandles=b;
		InvalidatePainting();
	}

	emPanel::Input(event,state,mx,my);
}


void emTestPanel::PolyDrawPanel::CanvasPanel::Paint(
	const emPainter & painter, emColor canvasColor
) const
{
	double x1,y1,x2,y2,x,y,w,h,sa,ra,r;
	emColor c;
	int i,m;

	if (WithCanvasColor) {
		c=emColor(96,128,160);
		painter.Clear(c,canvasColor);
		canvasColor=c;
	}
	else {
		painter.Clear(
			emLinearGradientTexture(
				0.0,0.0,emColor(80,80,160),
				0.0,GetHeight(),emColor(160,160,80)
			),
			canvasColor
		);
		canvasColor=0;
	}

	x1=y1=x2=y2=x=y=w=h=sa=ra=0.0;
	if (XY.GetCount()>=4) {
		x1=XY[0]; y1=XY[1];
		x2=XY[2]; y2=XY[3];
		x=emMin(x1,x2);
		y=emMin(y1,y2);
		w=fabs(x2-x1);
		h=fabs(y2-y1);
	}
	if (XY.GetCount()>=8) {
		sa=atan2(XY[5]-y-h*0.5,XY[4]-x-w*0.5);
		ra=atan2(XY[7]-y-h*0.5,XY[6]-x-w*0.5)-sa;
		if (ra<0.0) ra+=2.0*M_PI;
		sa*=180.0/M_PI;
		ra*=180.0/M_PI;
	}

	switch (Type) {
	case 0:
		painter.PaintPolygon(XY.Get(),XY.GetCount()/2,Texture,canvasColor);
		break;
	case 1:
		painter.PaintPolygonOutline(XY.Get(), XY.GetCount() / 2, StrokeWidth,
		                            Stroke, canvasColor);
		break;
	case 2:
		painter.PaintPolyline(XY.Get(),XY.GetCount()/2,StrokeWidth,Stroke,
		                      StrokeStart,StrokeEnd,canvasColor);
		break;
	case 3:
		painter.PaintBezier(XY.Get(),XY.GetCount()/2,Texture,canvasColor);
		break;
	case 4:
		painter.PaintBezierOutline(XY.Get(),XY.GetCount()/2,StrokeWidth,
		                           Stroke,canvasColor);
		break;
	case 5:
		painter.PaintBezierLine(XY.Get(),XY.GetCount()/2,StrokeWidth,Stroke,
		                        StrokeStart,StrokeEnd,canvasColor);
		break;
	case 6:
		painter.PaintLine(x1,y1,x2,y2,StrokeWidth,Stroke,
		                  StrokeStart,StrokeEnd,canvasColor);
		break;
	case 7:
		painter.PaintRect(x,y,w,h,Texture,canvasColor);
		break;
	case 8:
		painter.PaintRectOutline(x,y,w,h,StrokeWidth,Stroke,canvasColor);
		break;
	case 9:
		painter.PaintEllipse(x,y,w,h,Texture,canvasColor);
		break;
	case 10:
		painter.PaintEllipseOutline(x,y,w,h,StrokeWidth,Stroke,canvasColor);
		break;
	case 11:
		painter.PaintEllipseSector(x,y,w,h,sa,ra,Texture,canvasColor);
		break;
	case 12:
		painter.PaintEllipseSectorOutline(x,y,w,h,sa,ra,StrokeWidth,
		                                     Stroke,canvasColor);
		break;
	case 13:
		painter.PaintEllipseArc(x,y,w,h,sa,ra,StrokeWidth,Stroke,
		                           StrokeStart,StrokeEnd,canvasColor);
		break;
	case 14:
		painter.PaintRoundRect(x,y,w,h,w*0.2,h*0.2,Texture,canvasColor);
		break;
	case 15:
		painter.PaintRoundRectOutline(x,y,w,h,w*0.2,h*0.2,StrokeWidth,Stroke,canvasColor);
		break;
	}

	if (ShowHandles) {
		r=emMin(ViewToPanelDeltaX(12.0),0.05);
		for (i=0; i<XY.GetCount()/2; i++) {
			c=emColor(0,255,0,128);
			if (
				(Type>=3 && Type<=5 && i%3!=0) ||
				(Type>=6 && i>1)
			) c=emColor(255,255,0,128);
			m=XY.GetCount()/2;
			if (Type>=3 && Type<=4) m-=m%3;
			else if (Type==5) m-=(m+2)%3;
			else if (Type>=11 && Type<=13) m=4;
			else if (Type>=6) m=2;
			if (i>=m) c=emColor(128,128,128,128);
			if (i==DragIdx) c=c.GetBlended(emColor(255,255,255,128),75.0F);
			x=XY[i*2];
			y=XY[i*2+1];
			painter.PaintEllipse(x-r,y-r,2*r,2*r,c);
			painter.PaintEllipseOutline(x-r,y-r,2*r,2*r,r*0.15,emColor(0,0,0,128));
		}
	}

	painter.PaintTextBoxed(
		0.0,GetHeight()-0.03,1.0,0.03,
		"The vertices can be dragged with the left mouse button!\n"
		"(Hold shift for raster)\n",
		0.03,0xFFFFFFFF,0,EM_ALIGN_CENTER,EM_ALIGN_CENTER
	);
}
