//------------------------------------------------------------------------------
// emStarFieldPanel.cpp
//
// Copyright (C) 2007-2008 Oliver Hamann.
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

#include <emMain/emStarFieldPanel.h>
#include <emCore/emRes.h>


emStarFieldPanel::emStarFieldPanel(
	ParentArg parent, const emString & name, int depth, emUInt32 randomSeed
)
	: emPanel(parent,name)
{
	emPanel * p;
	double r;
	int i;

	Depth=depth;

	RandomSeed=randomSeed;

	if (Depth<1) StarCount=0;
	else StarCount=(int)(emMin(Depth*3,400)*GetRandom(0.5,1.0));
	Stars=new Star[StarCount];
	for (i=0; i<StarCount; i++) {
		r=MinStarRadius/MinPanelSize*GetRandom(0.5,1.0);
		Stars[i].X=GetRandom(r,1.0-r);
		Stars[i].Y=GetRandom(r,1.0-r);
		Stars[i].Radius=r;
		Stars[i].Color.SetHSVA(
			(float)GetRandom(0.0,360.0),
			(float)GetRandom(0.0,15.0),
			100.0f
		);
	}

	ChildRandomSeed[0]=GetRandom()^0x74fc8324;
	ChildRandomSeed[1]=GetRandom()^0x058f56a9;
	ChildRandomSeed[2]=GetRandom()^0xfc863e37;
	ChildRandomSeed[3]=GetRandom()^0x8bef7891;

	StarShape=emGetInsResImage(GetRootContext(),"emMain","Star.tga",1);

	p=new OverlayPanel(this,"o");
	p->Layout(0.0,0.0,1.0,1.0);

	if (Depth>50 && GetRandom()%11213==0) {
		p=new TicTacToePanel(this,"t");
		p->Layout(0.48,0.48,0.04,0.04);
	}
}


emStarFieldPanel::~emStarFieldPanel()
{
	if (Stars) delete [] Stars;
}


emString emStarFieldPanel::GetTitle()
{
	return "Star Field";
}


void emStarFieldPanel::Notice(NoticeFlags flags)
{
	emPanel::Notice(flags);
	if (flags&(NF_VIEWING_CHANGED|NF_SOUGHT_NAME_CHANGED)) {
		UpdateChildren();
	}
}


bool emStarFieldPanel::IsOpaque()
{
	return true;
}


void emStarFieldPanel::Paint(const emPainter & painter, emColor canvasColor)
{
	if (BgColor!=canvasColor) painter.Clear(BgColor,canvasColor);
}


void emStarFieldPanel::PaintOverlay(const emPainter & painter)
{
	emColor c;
	double r,vr,x,y,d;
	float hue,sat,alpha;
	int i;

	for (i=0; i<StarCount; i++) {
		r=Stars[i].Radius;
		vr=PanelToViewDeltaX(r);
		if (vr>MinStarRadius) {
			if (vr>4.0) {
				hue=Stars[i].Color.GetHue();
				sat=Stars[i].Color.GetSat();
				alpha=sat*18.0f;
				if (alpha>255.0f) alpha=255.0f;
				c.SetHSVA(hue,100.0f,100.0f,(emByte)alpha);
				x=Stars[i].X-r;
				y=Stars[i].Y-r;
				d=r*2;
				painter.PaintShape(x,y,d,d,StarShape,0,c);
				c.SetHSVA(hue,sat-10.0f,100.0f);
				painter.PaintShape(x,y,d,d,StarShape,0,c);
			}
			else {
				r*=0.6;
				vr=PanelToViewDeltaX(r);
				if (vr>1.2) {
					x=Stars[i].X-r;
					y=Stars[i].Y-r;
					d=r*2;
					painter.PaintEllipse(x,y,d,d,Stars[i].Color);
				}
				else {
					r*=0.8862;
					x=Stars[i].X-r;
					y=Stars[i].Y-r;
					d=r*2;
					painter.PaintRect(x,y,d,d,Stars[i].Color);
				}
			}
		}
	}
}


void emStarFieldPanel::UpdateChildren()
{
	const char * soughtName;
	emPanel * p;
	char name[32];
	double x,y,w,h;
	int i;

	soughtName=GetSoughtName();
	for (i=0; i<4; i++) {
		name[0]=(char)('0'+i);
		name[1]=0;
		p=GetChild(name);
		x=(i&1)?0.5:0.0;
		y=(i&2)?0.5:0.0;
		w=0.5;
		h=0.5;
		if (
			(
				soughtName &&
				strcmp(soughtName,name)==0
			) ||
			(
				IsViewed() &&
				GetViewedWidth()>=2*MinPanelSize &&
				GetClipX1()<PanelToViewX(x+w) &&
				GetClipX2()>PanelToViewX(x) &&
				GetClipY1()<PanelToViewY(y+h) &&
				GetClipY2()>PanelToViewY(y)
			)
		) {
			if (!p) {
				p=new emStarFieldPanel(this,name,Depth+1,ChildRandomSeed[i]);
				p->SetFocusable(false);
				p->BeFirst();
				p->Layout(x,y,w,h,BgColor);
			}
		}
		else if (
			p && (IsViewed() || !p->IsInViewedPath()) && (
				!p->IsViewed() || (
					p->GetViewedWidth()<0.9*GetView().GetCurrentWidth() &&
					p->GetViewedHeight()<0.9*GetView().GetCurrentHeight()
					// Otherwise there would be a bug with
					// seeking in a small window.
				)
			)
		) {
			delete p;
		}
	}
}


emUInt32 emStarFieldPanel::GetRandom()
{
	RandomSeed=RandomSeed*1664525+1013904223;
	return RandomSeed;
}


double emStarFieldPanel::GetRandom(double minVal, double maxVal)
{
	return GetRandom()*(maxVal-minVal)/EM_UINT32_MAX+minVal;
}


emStarFieldPanel::OverlayPanel::OverlayPanel(
	emStarFieldPanel * parent, const emString & name
)
	:
	emPanel(parent,name)
{
	SetFocusable(false);
}


void emStarFieldPanel::OverlayPanel::Input(
	emInputEvent & event, const emInputState & state, double mx, double my
)
{
}


bool emStarFieldPanel::OverlayPanel::IsOpaque()
{
	return false;
}


void emStarFieldPanel::OverlayPanel::Paint(
	const emPainter & painter, emColor canvasColor
)
{
	((emStarFieldPanel*)GetParent())->PaintOverlay(painter);
}


emStarFieldPanel::TicTacToePanel::TicTacToePanel(
	emStarFieldPanel * parent, const emString & name
)
	: emPanel(parent,name)
{
	State=0;
	Starter=1;
	RandomSeed=(emUInt32)emGetClockMS();
}


emString emStarFieldPanel::TicTacToePanel::GetTitle()
{
	return "Tic Tac Toe";
}


void emStarFieldPanel::TicTacToePanel::Input(
	emInputEvent & event, const emInputState & state, double mx, double my
)
{
	int ix,iy,i,k,c,s,best,f;

	ix=((int)((mx-0.05)/0.3+1.0))-1;
	iy=((int)((my-0.05)/0.3+1.0))-1;
	if (ix>=0 && iy>=0 && ix<=2 && iy<=2) {
		if (event.GetKey()==EM_KEY_LEFT_BUTTON && CheckState(State)<0) {
			i=iy*3+ix;
			if ((State&(3<<(i*2)))==0) {
				State^=1<<(i*2);
				if (CheckState(State)<0) {
					best=-1;
					k=GetRandom()%9;
					f=GetRandom()%10;
					for (i=0; i<9; i++) if ((State&(3<<((k+i)%9*2)))==0) {
						s=State|(2<<((k+i)%9*2));
						c=DeepCheckState(s,1);
						if (c!=1 || best==-1) best=s;
						if (f==0 || c==2) break;
					}
					State=best;
				}
				InvalidatePainting();
			}
			Focus();
			event.Eat();
		}
	}
	if (event.GetKey()==EM_KEY_RIGHT_BUTTON) {
		State=0;
		Starter^=3;
		if (Starter==2) State=2<<(GetRandom()%9*2);
		InvalidatePainting();
		Focus();
		event.Eat();
	}
	emPanel::Input(event,state,mx,my);
}


bool emStarFieldPanel::TicTacToePanel::IsOpaque()
{
	return false;
}


void emStarFieldPanel::TicTacToePanel::Paint(
	const emPainter & painter, emColor canvasColor
)
{
	double x,y,w,h,d,t;
	emColor col;
	int i,s,c;

	for (i=0; i<9; i++) {
		w=h=0.3;
		x=0.05+i%3*w;
		y=0.05+i/3*h;
		t=0.03;
		c=CheckState(State);
		painter.PaintRectOutline(x,y,w,h,t,0x666699FF);
		s=(State>>(i*2))&3;
		if (s) {
			col=(s==1 ? 0x88FF88FF : 0xFF8888FF);
			d=0.05;
			w-=d*2; h-=d*2;
			x+=d; y+=d;
			if (s==c) t=0.06; else t=0.03;
			painter.PaintLine(x,y,x+w,y+h,t,emPainter::LC_ROUND,emPainter::LC_ROUND,col);
			painter.PaintLine(x+w,y,x,y+h,t,emPainter::LC_ROUND,emPainter::LC_ROUND,col);
		}
	}
	painter.PaintTextBoxed(
		0.0,0.04,1.0,0.02,
		"TIC TAC TOE  -  Left mouse button marks a field, "
		"right mouse button starts new game.",
		0.01,0xFFBB88FF,0,EM_ALIGN_CENTER,EM_ALIGN_CENTER
	);
}


int emStarFieldPanel::TicTacToePanel::DeepCheckState(int state, int turn)
{
	int c,best,i;

	c=CheckState(state);
	if (c>=0) return c;
	best=turn^3;
	for (i=0; i<9; i++) if ((state&(3<<(i*2)))==0) {
		c=DeepCheckState(state|(turn<<(i*2)),turn^3);
		if (c==turn) return c;
		if (c==0) best=0;
	}
	return best;
}


int emStarFieldPanel::TicTacToePanel::CheckState(int state)
{
	int m;

	m=state&0x0003F;
	if (m==0x00015) return 1;
	if (m==0x0002A) return 2;
	m=state&0x00FC0;
	if (m==0x00540) return 1;
	if (m==0x00A80) return 2;
	m=state&0x3F000;
	if (m==0x15000) return 1;
	if (m==0x2A000) return 2;
	m=state&0x030C3;
	if (m==0x01041) return 1;
	if (m==0x02082) return 2;
	m=state&0x0C30C;
	if (m==0x04104) return 1;
	if (m==0x08208) return 2;
	m=state&0x30C30;
	if (m==0x10410) return 1;
	if (m==0x20820) return 2;
	m=state&0x30303;
	if (m==0x10101) return 1;
	if (m==0x20202) return 2;
	m=state&0x03330;
	if (m==0x01110) return 1;
	if (m==0x02220) return 2;
	if (((state|(state>>1))&0x15555)==0x15555) return 0;
	return -1;
}


emUInt32 emStarFieldPanel::TicTacToePanel::GetRandom()
{
	RandomSeed=RandomSeed*1664525+1013904223;
	return RandomSeed;
}


const emColor emStarFieldPanel::BgColor=emColor(0x000000ff);
const double emStarFieldPanel::MinPanelSize=64.0;
const double emStarFieldPanel::MinStarRadius=0.3;
