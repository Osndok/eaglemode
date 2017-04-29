//------------------------------------------------------------------------------
// emTmpConvFramePanel.cpp
//
// Copyright (C) 2006-2009,2016-2017 Oliver Hamann.
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

#include <emTmpConv/emTmpConvFramePanel.h>


emTmpConvFramePanel::emTmpConvFramePanel(
	ParentArg parent, const emString & name, emTmpConvModel * model
)
	: emPanel(parent,name)
{
	double minViewPercentForTriggering,minViewPercentForHolding;

	BGColor=emColor(136,136,136);
	InnerScale=0.2;
	minViewPercentForTriggering=70.0*InnerScale*InnerScale;
	minViewPercentForHolding=minViewPercentForTriggering*0.3;
	InnerPanel=new emTmpConvPanel(
		this,"tmp",model,
		minViewPercentForTriggering,minViewPercentForHolding
	);
	FileManViewConfig=emFileManViewConfig::Acquire(GetView());
	AddWakeUpSignal(FileManViewConfig->GetChangeSignal());
	SetAutoplayHandling(APH_CUTOFF);
	UpdateBgColor();
}


emTmpConvFramePanel::~emTmpConvFramePanel()
{
}


bool emTmpConvFramePanel::Cycle()
{
	bool busy;

	busy=emPanel::Cycle();
	if (IsSignaled(FileManViewConfig->GetChangeSignal())) {
		UpdateBgColor();
	}
	return busy;
}


bool emTmpConvFramePanel::IsOpaque() const
{
	return BGColor.IsOpaque();
}


void emTmpConvFramePanel::Paint(
	const emPainter & painter, emColor canvasColor
) const
{
	double xy[4*2];
	double h,b,d,fx,fy,fw,fh;

	if (!BGColor.IsTotallyTransparent()) {
		painter.Clear(BGColor,canvasColor);
		canvasColor=BGColor;
	}

	h=GetHeight();
	d=0.0; //emMin(1.0,h)*(1.0-InnerScale)*0.005;
	fx=(1.0-InnerScale)*0.5-d;
	fy=(h-h*InnerScale)*0.5-d;
	fw=InnerScale+2*d;
	fh=h*InnerScale+2*d;

	b=fx*0.3;

	xy[0]=b;     xy[1]=0.0;
	xy[2]=1.0-b; xy[3]=0.0;
	xy[4]=fx+fw; xy[5]=fy;
	xy[6]=fx;    xy[7]=fy;
	painter.PaintPolygon(xy,4,0x00000044,canvasColor);

	xy[0]=1.0-b; xy[1]=0.0;
	xy[2]=1.0-b; xy[3]=h;
	xy[4]=fx+fw; xy[5]=fy+fh;
	xy[6]=fx+fw; xy[7]=fy;
	painter.PaintPolygon(xy,4,0xFFFFFF22,canvasColor);

	xy[0]=1.0-b; xy[1]=h;
	xy[2]=b;     xy[3]=h;
	xy[4]=fx;    xy[5]=fy+fh;
	xy[6]=fx+fw; xy[7]=fy+fh;
	painter.PaintPolygon(xy,4,0xFFFFFF44,canvasColor);

	xy[0]=b;     xy[1]=h;
	xy[2]=b;     xy[3]=0.0;
	xy[4]=fx;    xy[5]=fy;
	xy[6]=fx;    xy[7]=fy+fh;
	painter.PaintPolygon(xy,4,0x00000022,canvasColor);

	d=0.2;
	PaintInfo(painter,b*d,h*d,b*(1.0-2*d),h*(1.0-2*d),canvasColor);
	PaintInfo(painter,1.0-b*(1.0-d),h*d,b*(1.0-2*d),h*(1.0-2*d),canvasColor);
}


void emTmpConvFramePanel::LayoutChildren()
{
	double h;

	h=GetHeight();
	InnerPanel->Layout(
		(1.0-InnerScale)*0.5,
		(h-h*InnerScale)*0.5,
		InnerScale,
		h*InnerScale,
		BGColor
	);
}


void emTmpConvFramePanel::UpdateBgColor()
{
	emColor c;

	c=FileManViewConfig->GetTheme().FileContentColor.Get();
	if (c.GetVal()>70.0F) c.SetVal(70.0F);
	if (c.GetVal()<30.0F) c.SetVal(30.0F);
	if (BGColor!=c) {
		BGColor=c;
		InvalidatePainting();
		InvalidateChildrenLayout();
	}
}


void emTmpConvFramePanel::PaintInfo(
	const emPainter & painter, double x, double y, double w, double h,
	emColor canvasColor
) const
{
	static const emColor bgColor=emColor(204,204,204);
	static const emColor frameColor=emColor(238,0,0);
	static const emColor textColor=emColor(0,0,0);
	static const char * const text1=
		"Caution!"
	;
	static const char * const text2=
		"Temporary Conversion -\n"
		"Changes to files herein will get lost!"
	;
	static const char * const text3=
		"This file viewer temporarily converts the original file to another file type\n"
		"in order to show it through an inner file viewer. The original file is kept\n"
		"unchanged.\n"
		"\n"
		"The type of conversion depends on the type of the original file. It can be a\n"
		"true format conversion, an info extraction or an unpacking. In the latter case,\n"
		"the output is a directory tree of unpacked files.\n"
		"\n"
		"You may do anything with the output files, but be aware that they are deleted\n"
		"automatically. Any changes to the files will get lost and even inserted files\n"
		"will be deleted!\n"
		"\n"
		"Normally the files are deleted at the moment you are zooming out. In seldom\n"
		"cases, the files may be deleted and recreated in between by a reconversion,\n"
		"especially when the original file is modified.\n"
		"\n"
		"There is a helpful exception: If you select an unpacked file and zoom out, the\n"
		"whole temporary conversion is kept until deselecting. For example, you could\n"
		"zoom into a ZIP file, select an unpacked file, zoom out and navigate elsewhere,\n"
		"select a directory as the target and execute the copy command.\n"
		"\n"
		"Because the conversion can cost many system resources, you have to zoom in so\n"
		"far before the conversion is triggered. Please do not zoom into a large\n"
		"conversion while not having enough disk space for the temporary files."
	;

	double xy[3*2];
	double s,vw;

	s=h/0.8;
	if (s>w) {
		y+=(h-w*h/s)*0.5;
		s=w;
	}
	else {
		x+=(w-s)*0.5;
	}

	vw=GetViewedWidth()*s;

	if (vw<4.0) return;

	xy[0]=x+s*0.5; xy[1]=y+s*0.1;
	xy[2]=x+s*0.9; xy[3]=y+s*0.7;
	xy[4]=x+s*0.1; xy[5]=y+s*0.7;
	painter.PaintPolygon(xy,3,bgColor,canvasColor);
	painter.PaintPolygonOutline(xy,3,s*0.08,frameColor);

	if (vw<8.0) return;

	painter.PaintTextBoxed(
		x+s*0.4,
		y+s*0.34,
		s*0.2,
		s*0.04,
		text1,
		s*0.04,
		textColor,
		bgColor,
		EM_ALIGN_CENTER,
		EM_ALIGN_CENTER
	);

	painter.PaintTextBoxed(
		x+s*0.34,
		y+s*0.41,
		s*0.32,
		s*0.04,
		text2,
		s*0.04,
		textColor,
		bgColor,
		EM_ALIGN_CENTER,
		EM_ALIGN_CENTER,
		0.8
	);

	if (vw<10.0) return;

	painter.PaintTextBoxed(
		x+s*0.37,
		y+s*0.47,
		s*0.26,
		s*0.18,
		text3,
		s*0.02,
		textColor,
		bgColor,
		EM_ALIGN_TOP,
		EM_ALIGN_LEFT,
		0.8
	);
}
