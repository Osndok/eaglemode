//------------------------------------------------------------------------------
// emTreeDumpRecPanel.cpp
//
// Copyright (C) 2007-2008,2011 Oliver Hamann.
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

#include <emTreeDump/emTreeDumpRecPanel.h>
#include <emTreeDump/emTreeDumpControlPanel.h>
#include <emCore/emFpPlugin.h>


emTreeDumpRecPanel::emTreeDumpRecPanel(
	ParentArg parent, const emString & name, emTreeDumpRec * rec,
	const emString & dir
)
	: emPanel(parent,name)
{
	Rec=rec;
	Dir=dir;
	if (Rec) BgColor=Rec->BgColor;
	else BgColor=0;
	EnableAutoExpansion();
}


emTreeDumpRecPanel::~emTreeDumpRecPanel()
{
}


double emTreeDumpRecPanel::GetBestHeight()
{
	return 2.0/3.0;
}


bool emTreeDumpRecPanel::IsOpaque()
{
	return false;
}


void emTreeDumpRecPanel::Paint(const emPainter & painter, emColor canvasColor)
{
	double xy[6*2];
	double x,y,w,h,t,r;

	if (!Rec) return;

	t=0.02;
	x=t*0.5;
	y=t*0.5;
	w=1.0-t;
	h=GetHeight()-t;
	switch (Rec->Frame.Get()) {
	case emTreeDumpRec::FRAME_RECTANGLE:
		painter.PaintRect(x,y,w,h,BgColor,canvasColor);
		painter.PaintRectOutline(x,y,w,h,t,Rec->FgColor);
		break;
	case emTreeDumpRec::FRAME_ROUND_RECT:
		r=emMin(w,h)*0.2;
		painter.PaintRoundRect(x,y,w,h,r,r,BgColor,canvasColor);
		painter.PaintRoundRectOutline(x,y,w,h,r,r,t,Rec->FgColor);
		break;
	case emTreeDumpRec::FRAME_ELLIPSE:
		painter.PaintEllipse(x,y,w,h,BgColor,canvasColor);
		painter.PaintEllipseOutline(x,y,w,h,t,Rec->FgColor);
		break;
	case emTreeDumpRec::FRAME_HEXAGON:
		r=w*0.2;
		xy[ 0]=x+r;   xy[ 1]=y;
		xy[ 2]=x+w-r; xy[ 3]=y;
		xy[ 4]=x+w;   xy[ 5]=y+h*0.5;
		xy[ 6]=x+w-r; xy[ 7]=y+h;
		xy[ 8]=x+r;   xy[ 9]=y+h;
		xy[10]=x;     xy[11]=y+h*0.5;
		painter.PaintPolygon(xy,6,BgColor,canvasColor);
		painter.PaintPolygonOutline(xy,6,t,Rec->FgColor);
		break;
	default:
		painter.Clear(BgColor,canvasColor);
		break;
	}

	canvasColor=BgColor;

	painter.PaintTextBoxed(
		0.19,0.1,0.62,0.13,
		Rec->Title.Get(),
		0.1,
		Rec->FgColor,
		canvasColor,
		EM_ALIGN_CENTER,
		EM_ALIGN_CENTER
	);

	if (GetViewedHeight()>10.0) {
		painter.PaintTextBoxed(
			0.19,0.26,0.15,0.3,
			Rec->Text.Get(),
			0.01,
			Rec->FgColor,
			canvasColor,
			EM_ALIGN_TOP_LEFT,
			EM_ALIGN_LEFT,
			0.8
		);
	}
}


void emTreeDumpRecPanel::AutoExpand()
{
	emRef<emFpPluginList> pl;
	int i,n;

	if (!Rec) return;

	n=Rec->Files.GetCount();
	if (n) {
		pl=emFpPluginList::Acquire(GetRootContext());
		for (i=0; i<n; i++) {
			pl->CreateFilePanel(
				this,
				emString::Format("%d",i),
				emGetAbsolutePath(Rec->Files.Get(i),Dir)
			);
		}
	}

	for (i=0; i<Rec->Children.GetCount(); i++) {
		new emTreeDumpRecPanel(
			this,
			emString::Format("%d",n+i),
			&Rec->Children[i],
			Dir
		);
	}
}


void emTreeDumpRecPanel::LayoutChildren()
{
	emPanel * p;
	int i,sz,cnt;
	double x,y,w,h,gap,cw,ch,pw,ph;

	for (cnt=0, p=GetFirstChild(); p; cnt++, p=p->GetNext());
	if (cnt>0) {
		for (sz=1; sz*sz<cnt; sz++);
		x=0.355;
		y=0.26;
		w=0.46;
		h=w*GetBestHeight();
		gap=0.2;
		cw=w/(sz-gap);
		ch=h/(sz-gap);
		pw=cw*(1.0-gap);
		ph=ch*(1.0-gap);
		if (cnt<=(sz-1)*sz) x+=cw*0.5;
		for (i=0, p=GetFirstChild(); p; i++, p=p->GetNext()) {
			p->Layout(x+cw*(i/sz),y+ch*(i%sz),pw,ph,BgColor);
		}
	}
}


emPanel * emTreeDumpRecPanel::CreateControlPanel(
	ParentArg parent, const emString & name
)
{
	if (IsActive()) {
		return new emTreeDumpControlPanel(parent,name,GetView(),Rec,Dir);
	}
	else {
		return NULL;
	}
}
