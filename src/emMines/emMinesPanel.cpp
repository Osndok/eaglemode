//------------------------------------------------------------------------------
// emMinesPanel.cpp
//
// Copyright (C) 2005-2008 Oliver Hamann.
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

#include <emMines/emMinesControlPanel.h>
#include <emMines/emMinesPanel.h>


emMinesPanel::emMinesPanel(
	ParentArg parent, const emString & name, emMinesFileModel * fileModel
)
	: emFilePanel(parent,name,fileModel,true)
{
	Mdl=fileModel;
	HaveControlPanel=IsVFSGood();
	CursorX=-1;
	CursorY=-1;
	CursorZ=-1;
	AddWakeUpSignal(Mdl->GetChangeSignal());
	AddWakeUpSignal(GetVirFileStateSignal());
	PrepareTransformation();
}


emMinesPanel::~emMinesPanel()
{
}


emString emMinesPanel::GetTitle()
{
	return "Mines";
}


void emMinesPanel::GetEssenceRect(
	double * pX, double * pY, double * pW, double * pH
)
{
	*pX=EssenceX;
	*pY=EssenceY;
	*pW=EssenceW;
	*pH=EssenceH;
}


bool emMinesPanel::Cycle()
{
	bool vfsGood;

	if (IsSignaled(GetVirFileStateSignal())) {
		vfsGood=IsVFSGood();
		if (HaveControlPanel!=vfsGood) {
			HaveControlPanel=vfsGood;
			InvalidateControlPanel();
		}
	}

	if (
		IsSignaled(GetVirFileStateSignal()) ||
		IsSignaled(Mdl->GetChangeSignal())
	) {
		PrepareTransformation();
		InvalidatePainting();
	}

	return emFilePanel::Cycle();
}


void emMinesPanel::Notice(NoticeFlags flags)
{
	emFilePanel::Notice(flags);
	if ((flags&NF_VIEWING_CHANGED)!=0) {
		CursorX=-1;
		CursorY=-1;
		CursorZ=-1;
		PrepareTransformation();
	}
}


void emMinesPanel::Input(
	emInputEvent & event, const emInputState & state, double mx, double my
)
{
	int x,y,z,cx,cy,cz;
	double dx,dy,d,dmin;

	if (!IsViewed() || !IsVFSGood()) {
		CursorX=-1;
		CursorY=-1;
		CursorZ=-1;
		emFilePanel::Input(event,state,mx,my);
		return;
	}

	cx=-1;
	cy=-1;
	cz=-1;
	if (
		mx>=0 && mx<1.0 && my>=0 && my<GetHeight() &&
		IsViewed() &&
		PanelToViewX(mx)>=GetClipX1() && PanelToViewX(mx)<GetClipX2() &&
		PanelToViewY(my)>=GetClipY1() && PanelToViewY(my)<GetClipY2() &&
		!Mdl->IsGameWon() && !Mdl->IsGameLost()
	) {
		dmin=(TransX(1,0)-TransX(0,0))*0.5;
		dmin*=dmin;
		for (z=Mdl->GetSizeZ()-1; z>=0 && z>CameraZ+0.5; z--) {
			for (y=Mdl->GetSizeY()-1; y>=0; y--) {
				for (x=Mdl->GetSizeX()-1; x>=0; x--) {
					dx=mx-TransX(x,z);
					dy=my-TransY(y,z);
					d=dx*dx+dy*dy;
					if (d<dmin) {
						dmin=d; cx=x; cy=y; cz=z;
					}
				}
			}
		}
	}
	if (CursorX!=cx || CursorY!=cy || CursorZ!=cz) {
		CursorX=cx;
		CursorY=cy;
		CursorZ=cz;
		InvalidatePainting();
	}

	switch (event.GetKey()) {
	case EM_KEY_LEFT_BUTTON:
		if (state.IsNoMod()) {
			if (IsCursorValid() && !Mdl->IsGameWon() && !Mdl->IsGameLost()) {
				Mdl->OpenField(CursorX,CursorY,CursorZ);
			}
			Focus();
			event.Eat();
		}
		break;
	case EM_KEY_RIGHT_BUTTON:
		if (state.IsNoMod()) {
			if (IsCursorValid() && !Mdl->IsGameWon() && !Mdl->IsGameLost()) {
				Mdl->InvertMark(CursorX,CursorY,CursorZ);
			}
			Focus();
			event.Eat();
		}
		break;
	case EM_KEY_N:
		if (state.IsCtrlMod()) {
			Mdl->StartGame(
				Mdl->GetSizeX(),
				Mdl->GetSizeY(),
				Mdl->GetSizeZ(),
				Mdl->GetMineCount()
			);
			event.Eat();
		}
		break;
	case EM_KEY_1:
		if (state.IsCtrlMod()) {
			Mdl->StartGame(1);
			event.Eat();
		}
		break;
	case EM_KEY_2:
		if (state.IsCtrlMod()) {
			Mdl->StartGame(2);
			event.Eat();
		}
		break;
	case EM_KEY_3:
		if (state.IsCtrlMod()) {
			Mdl->StartGame(3);
			event.Eat();
		}
		break;
	case EM_KEY_4:
		if (state.IsCtrlMod()) {
			Mdl->StartGame(4);
			event.Eat();
		}
		break;
	case EM_KEY_5:
		if (state.IsCtrlMod()) {
			Mdl->StartGame(5);
			event.Eat();
		}
		break;
	default:
		break;
	}

	emFilePanel::Input(event,state,mx,my);
}


bool emMinesPanel::IsOpaque()
{
	if (IsVFSGood()) return true;
	else return emFilePanel::IsOpaque();
}


void emMinesPanel::Paint(const emPainter & painter, emColor canvasColor)
{
	emColor color;
	double tx,ty,tz,tw,th;
	int x,y,z,cx,cy,sx,sy,sz;

	if (!IsVFSGood()) {
		emFilePanel::Paint(painter,canvasColor);
		return;
	}

	painter.Clear(
		Mdl->IsGameWon()?emColor(34,34,102):
			Mdl->IsGameLost()?emColor(102,17,0):
				emColor(emColor::BLACK),
		canvasColor
	);

	sx=Mdl->GetSizeX();
	sy=Mdl->GetSizeY();
	sz=Mdl->GetSizeZ();
	cx=(int)ceil(CameraX);
	cy=(int)ceil(CameraY);
	if (cx<0) cx=0; else if (cx>sx-1) cx=sx-1;
	if (cy<0) cy=0; else if (cy>sy-1) cy=sy-1;

	for (z=sz-1; z>=0 && z>CameraZ+0.5; z--) {
		color.SetHSVA(
			60.0F*(z%6),
			55.0F,
			400.0F/(z+4),
			255
		);
		for (y=0; y<cy; y++) {
			for (x=0; x<cx; x++) {
				PaintField(painter,x,y,z,color);
			}
		}
		for (y=0; y<cy; y++) {
			for (x=sx-1; x>=cx; x--) {
				PaintField(painter,x,y,z,color);
			}
		}
		for (y=sy-1; y>=cy; y--) {
			for (x=0; x<cx; x++) {
				PaintField(painter,x,y,z,color);
			}
		}
		for (y=sy-1; y>=cy; y--) {
			for (x=sx-1; x>=cx; x--) {
				PaintField(painter,x,y,z,color);
			}
		}
	}

	if (IsCursorValid()) {
		color=emColor(255,255,255,192);
		PaintField(painter,CursorX,CursorY,CursorZ,color);
	}

	if (Mdl->IsGameWon() || Mdl->IsGameLost()) {
		tz=-1.0;
		if (tz>CameraZ+0.5) {
			tx=TransX(0,tz);
			ty=TransY(0,tz);
			tw=TransX(Mdl->GetSizeX()-1,tz)-tx;
			th=TransY(Mdl->GetSizeY()-1,tz)-ty;
			tx+=0.125*tw;
			ty+=0.125*th;
			tw*=0.75;
			th*=0.75;
			painter.PaintTextBoxed(
				tx,
				ty,
				tw,
				th,
				Mdl->IsGameLost() ? "Game over" : "Success!",
				th,
				Mdl->IsGameLost() ? emColor(255,0,0,128) : emColor(0,0,255,128),
				0,
				EM_ALIGN_CENTER,
				EM_ALIGN_CENTER,
				1.0
			);
		}
	}
}


emPanel * emMinesPanel::CreateControlPanel(
	ParentArg parent, const emString & name
)
{
	if (HaveControlPanel) {
		return new emMinesControlPanel(parent,name,Mdl);
	}
	else {
		return emFilePanel::CreateControlPanel(parent,name);
	}
}


void emMinesPanel::PaintField(
	const emPainter & painter, int x, int y, int z, emColor color
)
{
	static const double br=0.002;
	static const double fr=0.08;
	int xybeams;

	if (z+1<Mdl->GetSizeZ()) PaintZBeam(painter,x,y,z+fr,z+0.5,br,color);

	xybeams=0;
	if (x>0) {
		if (x-fr<=CameraX) PaintXBeam(painter,x-0.5,y,z,x-fr,br,color);
		else xybeams|=1;
	}
	if (x+1<Mdl->GetSizeX()) {
		if (x+fr>=CameraX) PaintXBeam(painter,x+fr,y,z,x+0.5,br,color);
		else xybeams|=2;
	}
	if (y>0) {
		if (y-fr<=CameraY) PaintYBeam(painter,x,y-0.5,z,y-fr,br,color);
		else xybeams|=4;
	}
	if (y+1<Mdl->GetSizeY()) {
		if (y+fr>=CameraY) PaintYBeam(painter,x,y+fr,z,y+0.5,br,color);
		else xybeams|=8;
	}

	if (Mdl->IsOpen(x,y,z)) {
		if (Mdl->IsMine(x,y,z)) PaintExplodingField(painter,x,y,z,fr);
		else PaintOpenField(painter,x,y,z,fr,Mdl->GetSurroundings(x,y,z),color);
	}
	else if (Mdl->IsMarked(x,y,z)) PaintMarkedField(painter,x,y,z,fr,color);
	else PaintClosedField(painter,x,y,z,fr,color);

	if ((xybeams&1)!=0) PaintXBeam(painter,x-0.5,y,z,x-fr,br,color);
	if ((xybeams&2)!=0) PaintXBeam(painter,x+fr,y,z,x+0.5,br,color);
	if ((xybeams&4)!=0) PaintYBeam(painter,x,y-0.5,z,y-fr,br,color);
	if ((xybeams&8)!=0) PaintYBeam(painter,x,y+fr,z,y+0.5,br,color);
	if (z>0) PaintZBeam(painter,x,y,z-0.5,z-fr,br,color);
}


void emMinesPanel::PaintClosedField(
	const emPainter & painter, double x, double y, double z, double r,
	emColor color
)
{
	double x11,y11,x12,y12,x21,y21,x22,y22;
	double xy[4*2];
	emColor cl,cr,ct,cb;

	cl=color.GetLighted(-20.0f);
	cr=color.GetLighted(-30.0f);
	ct=color.GetLighted(-10.0f);
	cb=color.GetLighted(-40.0f);

	x11=TransX(x-r,z-r);
	y11=TransY(y-r,z-r);
	x12=TransX(x-r,z+r);
	y12=TransY(y-r,z+r);
	x21=TransX(x+r,z-r);
	y21=TransY(y+r,z-r);
	x22=TransX(x+r,z+r);
	y22=TransY(y+r,z+r);

	painter.PaintRect(x11,y11,x21-x11,y21-y11,color);
	if (x12<x11) {
		xy[0]=x11; xy[1]=y11;
		xy[2]=x11; xy[3]=y21;
		xy[4]=x12; xy[5]=y22;
		xy[6]=x12; xy[7]=y12;
		painter.PaintPolygon(xy,4,cl);
		painter.PaintEdgeCorrection(x11,y11,x11,y21,color,cl);
	}
	else if (x22>x21) {
		xy[0]=x21; xy[1]=y11;
		xy[2]=x21; xy[3]=y21;
		xy[4]=x22; xy[5]=y22;
		xy[6]=x22; xy[7]=y12;
		painter.PaintPolygon(xy,4,cr);
		painter.PaintEdgeCorrection(x21,y21,x21,y11,color,cr);
	}
	if (y12<y11) {
		xy[0]=x11; xy[1]=y11;
		xy[2]=x21; xy[3]=y11;
		xy[4]=x22; xy[5]=y12;
		xy[6]=x12; xy[7]=y12;
		painter.PaintPolygon(xy,4,ct);
		painter.PaintEdgeCorrection(x21,y11,x11,y11,color,ct);
		if (x12<x11) painter.PaintEdgeCorrection(x11,y11,x12,y12,cl,ct);
		else if (x22>x21) painter.PaintEdgeCorrection(x22,y12,x21,y11,cr,ct);
	}
	else if (y22>y21) {
		xy[0]=x11; xy[1]=y21;
		xy[2]=x21; xy[3]=y21;
		xy[4]=x22; xy[5]=y22;
		xy[6]=x12; xy[7]=y22;
		painter.PaintPolygon(xy,4,cb);
		painter.PaintEdgeCorrection(x11,y21,x21,y21,color,cb);
		if (x12<x11) painter.PaintEdgeCorrection(x12,y22,x11,y21,cl,cb);
		else if (x22>x21) painter.PaintEdgeCorrection(x21,y21,x22,y22,cr,cb);
	}
}


void emMinesPanel::PaintMarkedField(
	const emPainter & painter, double x, double y, double z, double r,
	emColor color
)
{
	static const float light[8]={-45,-40,-30,-20,-5,-10,-20,-30};
	double x1[8],y1[8],x2[8],y2[8],x3[8],y3[8];
	double xy[8*2];
	double nx,ny;
	int i,k,v1,v2;
	emColor c1[8],c2[8];
	emColor cwarn;

	cwarn=color.GetBlended(emColor(255,0,0,color.GetAlpha()),25.0f);

	for (i=0; i<8; i++) {
		nx=r*cos((i+0.5)*(M_PI*0.25));
		ny=r*sin((i+0.5)*(M_PI*0.25));
		x1[i]=TransX(x+nx*0.448,z-r      );
		y1[i]=TransY(y+ny*0.448,z-r      );
		x2[i]=TransX(x+nx*1.082,z-r*0.414);
		y2[i]=TransY(y+ny*1.082,z-r*0.414);
		x3[i]=TransX(x+nx*1.082,z+r*0.414);
		y3[i]=TransY(y+ny*1.082,z+r*0.414);
		if ((i&1)!=0) {
			c1[i]=color.GetLighted(light[i]*0.5F);
			c2[i]=cwarn.GetLighted(light[i]);
		}
		else {
			c1[i]=cwarn.GetLighted(light[i]*0.5F);
			c2[i]=color.GetLighted(light[i]);
		}
	}

	v1=0;
	v2=0;
	for (i=0; i<8; i++) {
		k=(i+1)&7;
		xy[0]=x2[i]; xy[1]=y2[i];
		xy[2]=x2[k]; xy[3]=y2[k];
		xy[4]=x3[k]; xy[5]=y3[k];
		xy[6]=x3[i]; xy[7]=y3[i];
		if ((xy[0]-xy[2])*(xy[7]-xy[1])+(xy[3]-xy[1])*(xy[6]-xy[0])>0.0) {
			painter.PaintPolygon(xy,4,c2[i]);
			v2|=1<<i;
		}
		xy[0]=x1[i]; xy[1]=y1[i];
		xy[2]=x1[k]; xy[3]=y1[k];
		xy[4]=x2[k]; xy[5]=y2[k];
		xy[6]=x2[i]; xy[7]=y2[i];
		if ((xy[0]-xy[2])*(xy[7]-xy[1])+(xy[3]-xy[1])*(xy[6]-xy[0])>0.0) {
			painter.PaintPolygon(xy,4,c1[i]);
			v1|=1<<i;
		}
	}
	for (i=0; i<8; i++) {
		xy[i*2]=x1[i];
		xy[i*2+1]=y1[i];
	}
	painter.PaintPolygon(xy,8,color);

	for (i=0; i<8; i++) {
		k=(i+1)&7;
		if ((v2&(1<<i))!=0 && (v2&(1<<k))!=0) {
			if (!k) painter.PaintEdgeCorrection(x3[k],y3[k],x2[k],y2[k],c2[k],c2[i]);
			else painter.PaintEdgeCorrection(x2[k],y2[k],x3[k],y3[k],c2[i],c2[k]);
		}
		if ((v1&(1<<i))!=0) {
			if ((v1&(1<<k))!=0) {
				if (!k) painter.PaintEdgeCorrection(x2[k],y2[k],x1[k],y1[k],c1[k],c1[i]);
				else painter.PaintEdgeCorrection(x1[k],y1[k],x2[k],y2[k],c1[i],c1[k]);
			}
			painter.PaintEdgeCorrection(x1[i],y1[i],x1[k],y1[k],c1[i],color);
			if ((v2&(1<<i))!=0) {
				painter.PaintEdgeCorrection(x2[i],y2[i],x2[k],y2[k],c2[i],c1[i]);
			}
		}
	}
}


void emMinesPanel::PaintOpenField(
	const emPainter & painter, double x, double y, double z, double r,
	int number, emColor color
)
{
	double x1,y1,x2,y2;
	char numstr[64];

	x1=TransX(x-r*1.2,z+r*0.1);
	y1=TransY(y-r*1.2,z+r*0.1);
	x2=TransX(x+r*1.2,z+r*0.1);
	y2=TransY(y+r*1.4,z+r*0.1);
	sprintf(numstr,"%d",number);
	painter.PaintTextBoxed(
		x1,y1,x2-x1,y2-y1,
		numstr,
		y2-y1,
		color.GetLighted(-25.0f)
	);

	x1=TransX(x-r*1.2,z-r*0.1);
	y1=TransY(y-r*1.2,z-r*0.1);
	x2=TransX(x+r*1.2,z-r*0.1);
	y2=TransY(y+r*1.4,z-r*0.1);
	painter.PaintTextBoxed(
		x1,y1,x2-x1,y2-y1,
		numstr,
		y2-y1,
		color
	);
}


void emMinesPanel::PaintExplodingField(
	const emPainter & painter, double x, double y, double z, double r
)
{
	static const struct { double x, y, z; } vertex[18]={
		{1347.62,-575.08,-864.97},
		{364.42,263.00,-316.07},
		{129.39,1088.80,-881.94},
		{-460.32,176.19,-316.07},
		{-1427.27,-577.16,-896.03},
		{-257.24,-525.68,-316.07},
		{-62.15,-1552.24,-896.03},
		{222.94,-491.38,-316.07},
		{0.00,0.00,0.00},
		{2190.00,-734.54,-426.79},
		{588.33,433.44,-179.60},
		{284.21,1851.23,-432.53},
		{-601.94,512.80,-179.60},
		{-2446.35,-754.52,-459.44},
		{-493.83,-856.07,-179.60},
		{-102.35,-2555.93,-459.44},
		{489.14,-875.85,-179.60},
		{0.00,0.00,0.00}
	};
	static const emColor colors[2]={
		emColor(255,255,0,192),
		emColor(255,0,0,255)
	};
	static const struct { int vi[3]; int ci; } poly[16]={
		{{15,16,17},0},
		{{17,16,9},0},
		{{17,9,10},0},
		{{11,17,10},0},
		{{11,12,17},0},
		{{13,17,12},0},
		{{17,13,14},0},
		{{14,15,17},0},
		{{6,7,8},1},
		{{8,7,0},1},
		{{8,0,1},1},
		{{2,8,1},1},
		{{2,3,8},1},
		{{4,8,3},1},
		{{8,4,5},1},
		{{5,6,8},1}
	};
	double xy[3*2];
	int i,j;

	for (i=0; i<16; i++) {
		for (j=0; j<3; j++) {
			xy[j*2]=TransX(
				x+r*vertex[poly[i].vi[j]].x*0.004,
				z+r*vertex[poly[i].vi[j]].z*0.004
			);
			xy[j*2+1]=TransY(
				y+r*vertex[poly[i].vi[j]].y*0.004,
				z+r*vertex[poly[i].vi[j]].z*0.004
			);
		}
		painter.PaintPolygon(xy,3,colors[poly[i].ci]);
	}
}


void emMinesPanel::PaintXBeam(
	const emPainter & painter, double x, double y, double z, double x2,
	double r, emColor color
)
{
	double x11,y11,x12,y12,x21,y21,x22,y22;
	double xy[4*2];

	x11=TransX(x  ,z-r);
	y11=TransY(y-r,z-r);
	x12=TransX(x  ,z+r);
	y12=TransY(y-r,z+r);
	x21=TransX(x2 ,z-r);
	y21=TransY(y+r,z-r);
	x22=TransX(x2 ,z+r);
	y22=TransY(y+r,z+r);
	painter.PaintRect(x11,y11,x21-x11,y21-y11,color);
	if (y12<y11) {
		xy[0]=x11; xy[1]=y11;
		xy[2]=x21; xy[3]=y11;
		xy[4]=x22; xy[5]=y12;
		xy[6]=x12; xy[7]=y12;
		painter.PaintPolygon(xy,4,color.GetLighted(-10.0f));
	}
	else if (y22>y21) {
		xy[0]=x11; xy[1]=y21;
		xy[2]=x21; xy[3]=y21;
		xy[4]=x22; xy[5]=y22;
		xy[6]=x12; xy[7]=y22;
		painter.PaintPolygon(xy,4,color.GetLighted(-40.0f));
	}
}


void emMinesPanel::PaintYBeam(
	const emPainter & painter, double x, double y, double z, double y2,
	double r, emColor color
)
{
	double x11,y11,x12,y12,x21,y21,x22,y22;
	double xy[4*2];

	x11=TransX(x-r,z-r);
	y11=TransY(y  ,z-r);
	x12=TransX(x-r,z+r);
	y12=TransY(y  ,z+r);
	x21=TransX(x+r,z-r);
	y21=TransY(y2 ,z-r);
	x22=TransX(x+r,z+r);
	y22=TransY(y2 ,z+r);
	painter.PaintRect(x11,y11,x21-x11,y21-y11,color);
	if (x12<x11) {
		xy[0]=x11; xy[1]=y11;
		xy[2]=x11; xy[3]=y21;
		xy[4]=x12; xy[5]=y22;
		xy[6]=x12; xy[7]=y12;
		painter.PaintPolygon(xy,4,color.GetLighted(-20.0f));
	}
	else if (x22>x21) {
		xy[0]=x21; xy[1]=y11;
		xy[2]=x21; xy[3]=y21;
		xy[4]=x22; xy[5]=y22;
		xy[6]=x22; xy[7]=y12;
		painter.PaintPolygon(xy,4,color.GetLighted(-30.0f));
	}
}


void emMinesPanel::PaintZBeam(
	const emPainter & painter, double x, double y, double z, double z2,
	double r, emColor color
)
{
	double x11,y11,x12,y12,x21,y21,x22,y22;
	double xy[4*2];

	x11=TransX(x-r,z );
	y11=TransY(y-r,z );
	x12=TransX(x-r,z2);
	y12=TransY(y-r,z2);
	x21=TransX(x+r,z );
	y21=TransY(y+r,z );
	x22=TransX(x+r,z2);
	y22=TransY(y+r,z2);
	if (x12<x11) {
		xy[0]=x11; xy[1]=y11;
		xy[2]=x11; xy[3]=y21;
		xy[4]=x12; xy[5]=y22;
		xy[6]=x12; xy[7]=y12;
		painter.PaintPolygon(xy,4,color.GetLighted(-20.0f));
	}
	else if (x22>x21) {
		xy[0]=x21; xy[1]=y11;
		xy[2]=x21; xy[3]=y21;
		xy[4]=x22; xy[5]=y22;
		xy[6]=x22; xy[7]=y12;
		painter.PaintPolygon(xy,4,color.GetLighted(-30.0f));
	}
	if (y12<y11) {
		xy[0]=x11; xy[1]=y11;
		xy[2]=x21; xy[3]=y11;
		xy[4]=x22; xy[5]=y12;
		xy[6]=x12; xy[7]=y12;
		painter.PaintPolygon(xy,4,color.GetLighted(-10.0f));
	}
	else if (y22>y21) {
		xy[0]=x11; xy[1]=y21;
		xy[2]=x21; xy[3]=y21;
		xy[4]=x22; xy[5]=y22;
		xy[6]=x12; xy[7]=y22;
		painter.PaintPolygon(xy,4,color.GetLighted(-40.0f));
	}
}


bool emMinesPanel::IsCursorValid() const
{
	return
		CursorX>=0 && CursorX<Mdl->GetSizeX() &&
		CursorY>=0 && CursorY<Mdl->GetSizeY() &&
		CursorZ>=0 && CursorZ<Mdl->GetSizeZ()
	;
}


double emMinesPanel::TransX(double fieldX, double fieldZ) const
{
	return (fieldX-CameraX)/(fieldZ-CameraZ)*TrScale+TrX0;
}


double emMinesPanel::TransY(double fieldY, double fieldZ) const
{
	return (fieldY-CameraY)/(fieldZ-CameraZ)*TrScale+TrY0;
}


void emMinesPanel::PrepareTransformation()
{
	double h,k,s;

	if (!IsViewed() || !IsVFSGood()) {
		EssenceX=0.0;
		EssenceY=0.0;
		EssenceW=1.0;
		EssenceH=GetHeight();
		CameraX=0;
		CameraY=0;
		CameraZ=1000;
		TrX0=0;
		TrY0=0;
		TrScale=1.0;
		return;
	}

	h=GetHeight();
	s=emMin(h/Mdl->GetSizeY(),1.0/Mdl->GetSizeX())*0.9;

	EssenceW=(Mdl->GetSizeX()-0.6)*s;
	EssenceH=(Mdl->GetSizeY()-0.6)*s;
	EssenceX=(1.0-EssenceW)*0.5;
	EssenceY=(h-EssenceH)*0.5;

	TrX0=ViewToPanelX(GetView().GetCurrentX()+GetView().GetCurrentWidth()*0.5);
	TrY0=ViewToPanelY(GetView().GetCurrentY()+GetView().GetCurrentHeight()*0.5);

	CameraX=TrX0/s+(Mdl->GetSizeX()-1-1.0/s)*0.5;
	CameraY=TrY0/s+(Mdl->GetSizeY()-1-h/s)*0.5;
	k=emMax(
		PanelToViewDeltaX(EssenceW)/GetView().GetCurrentWidth(),
		PanelToViewDeltaY(EssenceH)/GetView().GetCurrentHeight()
	);
	CameraZ=(Mdl->GetSizeX()*Mdl->GetSizeY())*0.5/k*0.21;
	TrScale=CameraZ*s;
	if (k>1.0) {
		TrScale*=k/(2.0-1.0/k);
		CameraZ*=1.0-(1.0-1.0/k)*log(k)*0.5;
	}
	CameraZ=-CameraZ;
}
