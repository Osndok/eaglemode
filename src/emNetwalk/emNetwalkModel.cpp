//------------------------------------------------------------------------------
// emNetwalkModel.cpp
//
// Copyright (C) 2010-2011 Oliver Hamann.
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

#include <emNetwalk/emNetwalkModel.h>


emRef<emNetwalkModel> emNetwalkModel::Acquire(
	emContext & context, const emString & name, bool common
)
{
	EM_IMPL_ACQUIRE(emNetwalkModel,context,name,common)
}


const char * emNetwalkModel::GetFormatName() const
{
	return "emNetwalk";
}


void emNetwalkModel::SetAutoMark(bool autoMark, bool saveFile)
{
	if (AutoMark!=autoMark) {
		AutoMark=autoMark;
		AutoMarkPiece=-1;
		if (saveFile) Save(true);
	}
}


int emNetwalkModel::GetPiece(int x, int y) const
{
	int w,h;

	w=Width.Get();
	h=Height.Get();
	if (Borderless.Get()) {
		x%=w;
		if (x<0) x+=w;
		y%=h;
		if (y<0) y+=h;
	}
	else {
		if (x<0 || x>=w || y<0 || y>=h) return PF_BLOCKED;
	}
	return Raster[y*w+x].Get();
}


void emNetwalkModel::TrySetup(
	int width, int height, bool borderless, bool noFourWayJunctions,
	int complexity, bool digMode, bool autoMark, bool saveFile
) throw(emString)
{
	emArray<char> undoBuf;
	int i;

	SaveToMem(undoBuf);
	if (width<2) width=2;
	if (height<2) height=2;
	Width=width;
	Height=height;
	Borderless=borderless;
	NoFourWayJunctions=noFourWayJunctions;
	Complexity=complexity;
	DigMode=digMode;
	AutoMark=autoMark;
	Finished=false;
	PenaltyPoints=0;
	CurrentPiece=-1;
	AutoMarkPiece=-1;
	Raster.SetCount(width*height);
	for (i=1;;i++) {
		Invent();
		if (Solver(this).IsUniqueSolution()) {
			emDLog("emNetwalkModel::Setup: Invented %d setups for finding one with unique solution",i);
			break;
		}
		if (i>1000) {
			TryLoadFromMem(undoBuf);
			throw emString("Could not find any setup with unique solution.");
		}
	}
	Shuffle();
	Fill();
	Dig(true);
	if (saveFile) Save(true);
}


void emNetwalkModel::MarkOrUnmark(int x, int y, bool saveFile)
{
	int w,h;

	w=Width.Get();
	h=Height.Get();
	if (Borderless.Get()) {
		x%=w;
		if (x<0) x+=w;
		y%=h;
		if (y<0) y+=h;
	}
	else {
		if (x<0 || x>=w || y<0 || y>=h) return;
	}
	XorPiece(y*w+x,PF_MARKED);
	if (saveFile) Save(true);
}


void emNetwalkModel::UnmarkAll(bool saveFile)
{
	bool changed;
	int i;

	changed=false;
	for (i=Raster.GetCount()-1; i>=0; i--) {
		if ((GetPiece(i)&PF_MARKED)!=0) {
			XorPiece(i,PF_MARKED);
			changed=true;
		}
	}
	if (AutoMarkPiece>=0) {
		AutoMarkPiece=-1;
		changed=true;
	}
	if (changed && saveFile) Save(true);
}


void emNetwalkModel::Rotate(int x, int y, int angle, bool saveFile)
{
	int w,h,i,p;

	if (IsFinished()) return;
	w=Width.Get();
	h=Height.Get();
	if (Borderless.Get()) {
		x%=w;
		if (x<0) x+=w;
		y%=h;
		if (y<0) y+=h;
	}
	else {
		if (x<0 || x>=w || y<0 || y>=h) return;
	}
	i=y*w+x;
	p=Raster[i].Get();
	if (p&(PF_BLOCKED|PF_MARKED)) return;
	p=RawRotate(p,angle);
	if (CurrentPiece.Get()!=i) {
		if (p&PF_TOUCHED) PenaltyPoints.Set(PenaltyPoints.Get()+1);
		CurrentPiece.Set(i);
	}
	p|=PF_TOUCHED;
	Raster[i].Set(p);
	Fill();
	Dig(true);
	if (AutoMark) {
		if (AutoMarkPiece!=-1 && AutoMarkPiece!=i) OrPiece(AutoMarkPiece,PF_MARKED);
		AutoMarkPiece=i;
		AutoMarkToSave=saveFile;
		AutoMarkTimer.Stop(true);
		AutoMarkTimer.Start(1000);
	}
	if (saveFile) Save(true);
}


void emNetwalkModel::Scroll(int dx, int dy, bool saveFile)
{
	emArray<int> arr;
	int i,j,cp,ap,n,x,y,w,h;

	w=GetWidth();
	h=GetHeight();
	n=Raster.GetCount();
	arr.SetCount(n);
	for (i=0; i<n; i++) arr.Set(i,GetPiece(i));
	dx=dx%w; if (dx<0) dx+=w;
	dy=dy%h; if (dy<0) dy+=h;
	cp=CurrentPiece.Get();
	ap=AutoMarkPiece;
	for (i=0; i<n; i++) {
		x=(i+dx)%w;
		y=(i/w+dy)%h;
		j=y*w+x;
		SetPiece(j,arr[i]);
		if (cp==i) CurrentPiece.Set(j);
		if (ap==i) AutoMarkPiece=j;
	}
	if (saveFile) Save(true);
}


emNetwalkModel::emNetwalkModel(emContext & context, const emString & name)
	: emRecFileModel(context,name),
	emStructRec(),
	Width(this,"Width",2,2,INT_MAX),
	Height(this,"Height",2,2,INT_MAX),
	Borderless(this,"Borderless"),
	NoFourWayJunctions(this,"NoFourWayJunctions"),
	Complexity(this,"Complexity",1,1,5),
	DigMode(this,"DigMode"),
	AutoMark(this,"AutoMark"),
	Finished(this,"Finished"),
	PenaltyPoints(this,"PenaltyPoints"),
	CurrentPiece(this,"CurrentPiece",-1),
	Raster(this,"Raster",4,INT_MAX),
	AutoMarkTimer(GetScheduler()),
	AutoMarkPiece(-1),
	AutoMarkToSave(false)
{
	PostConstruct(*this);
	AddWakeUpSignal(AutoMarkTimer.GetSignal());
}


emNetwalkModel::~emNetwalkModel()
{
}


bool emNetwalkModel::TryContinueLoading() throw(emString)
{
	if (!emRecFileModel::TryContinueLoading()) return false;
	if (Width.Get()*Height.Get()!=Raster.GetCount()) {
		throw emString("file content not consistent");
	}
	return true;
}


bool emNetwalkModel::Cycle()
{
	bool busy;

	busy=emRecFileModel::Cycle();
	if (IsSignaled(AutoMarkTimer.GetSignal())) {
		if (AutoMark && AutoMarkPiece!=-1) {
			if ((GetPiece(AutoMarkPiece)&PF_MARKED)==0) {
				OrPiece(AutoMarkPiece,PF_MARKED);
				if (AutoMarkToSave) Save(true);
			}
		}
		AutoMarkPiece=-1;
	}
	return busy;
}

int emNetwalkModel::GetNeigborIndex(int index, int angle) const
{
	int w,h,x,y;

	w=Width.Get();
	h=Height.Get();
	x=index%w;
	y=index/w;
	switch (angle&3) {
	case 0:
		x++;
		if (x<w) break;
		if (!Borderless.Get()) return -1;
		x=0;
		break;
	case 1:
		y++;
		if (y<h) break;
		if (!Borderless.Get()) return -1;
		y=0;
		break;
	case 2:
		x--;
		if (x>=0) break;
		if (!Borderless.Get()) return -1;
		x=w-1;
		break;
	case 3:
		y--;
		if (y>=0) break;
		if (!Borderless.Get()) return -1;
		y=h-1;
		break;
	}
	return y*w+x;
}


bool emNetwalkModel::IsConnected(int index, int angle) const
{
	return (GetPiece(index)&A2PF[angle&3])!=0;
}


void emNetwalkModel::Connect(int index, int angle)
{
	int index2;

	index2=GetNeigborIndex(index,angle);
	if (index2<0) return;
	OrPiece(index,A2PF[angle&3]);
	OrPiece(index2,A2PF[(angle+2)&3]);
}


void emNetwalkModel::Invent()
{
	static const int PR1[]={100,93,78,70,50};
	static const int PR2[]={100,93,78,60,0};
	emArray<int> arr1,arr2;
	int as[4];
	int w,h,a,i,j,k,ac,f,pr1,pr2;

	i=Complexity.Get()-1;
	if (i<0) i=0; else if (i>4) i=4;
	pr1=PR1[i];
	pr2=PR2[i];
	for (i=Raster.GetCount()-1; i>=0; i--) SetPiece(i,0);
	w=GetWidth();
	h=GetHeight();
	if (!NoFourWayJunctions.Get() && w>2 && h>2) {
		if (Borderless.Get()) i=emGetIntRandom(0,w*h-1);
		else i=emGetIntRandom(1,h-2)*w+emGetIntRandom(1,w-2);
		SetPiece(i,PF_CONMASK);
		for (a=3; a>=0; a--) {
			j=GetNeigborIndex(i,a);
			SetPiece(j,A2PF[(a+2)&3]);
			arr1.Add(j);
		}
	}
	else {
		i=emGetIntRandom(0,w*h-1);
		arr1.Add(i);
	}
	for (;;) {
		if (
			arr1.GetCount()>0 &&
			(arr2.GetCount()==0 || emGetIntRandom(0,100)<pr1)
		) {
			k=emGetIntRandom(0,arr1.GetCount()-1);
			i=arr1[k];
			arr1.Remove(k);
			for (a=3, ac=-1, f=0; a>=0; a--) {
				if (IsConnected(i,a)) ac=a;
				else {
					j=GetNeigborIndex(i,a);
					if (j>=0 && GetPiece(j)==0) as[f++]=a;
				}
			}
			if (f>0) {
				if (
					ac>=0 &&
					(j=GetNeigborIndex(i,ac+2))>=0 &&
					GetPiece(j)==0 &&
					emGetIntRandom(0,100)<pr2
				) {
					a=(ac+2)&3;
				}
				else {
					a=as[emGetIntRandom(0,f-1)];
				}
				Connect(i,a);
				arr1.Add(GetNeigborIndex(i,a));
				if (ac==-1) arr1.Add(i); else arr2.Add(i);
			}
			else {
				OrPiece(i,PF_TARGET);
			}
		}
		else if (arr2.GetCount()>0) {
			k=emGetIntRandom(0,arr2.GetCount()-1);
			i=arr2[k];
			for (a=3, f=0; a>=0; a--) {
				if (IsConnected(i,a)) continue;
				j=GetNeigborIndex(i,a);
				if (j>=0 && GetPiece(j)==0) as[f++]=a;
			}
			if (f>0) {
				a=as[emGetIntRandom(0,f-1)];
				Connect(i,a);
				arr1.Add(GetNeigborIndex(i,a));
			}
			if (f<=1 || NoFourWayJunctions.Get()) arr2.Remove(k);
		}
		else break;
	}
	i=emGetIntRandom(0,w*h-1);
	AndPiece(i,~PF_TARGET);
	OrPiece(i,PF_SOURCE);
}


void emNetwalkModel::Shuffle()
{
	int i;

	for (i=Raster.GetCount()-1; i>=0; i--) {
		SetPiece(i,RawRotate(GetPiece(i),emGetIntRandom(0,3)));
	}
}


void emNetwalkModel::Fill()
{
	emArray<int> stack;
	int i,f,a,j;

	for (i=Raster.GetCount()-1; i>=0; i--) {
		f=GetPiece(i);
		SetPiece(i,f&~PF_FILLED);
		if (f&PF_SOURCE) {
			OrPiece(i,PF_FILLED);
			stack.Add(i);
		}
	}
	while (stack.GetCount()>0) {
		i=stack[stack.GetCount()-1];
		stack.Remove(stack.GetCount()-1);
		for (a=3; a>=0; a--) {
			if (!IsConnected(i,a)) continue;
			j=GetNeigborIndex(i,a);
			if (j<0) continue;
			if (GetPiece(j)&PF_FILLED) continue;
			if (!IsConnected(j,a+2)) continue;
			OrPiece(j,PF_FILLED);
			stack.Add(j);
		}
	}
	for (i=Raster.GetCount()-1; i>=0; i--) {
		f=GetPiece(i);
		if ((f&PF_FILLED)==0 && (f&PF_CONMASK)!=0) break;
	}
	Finished.Set(i<0);
}


void emNetwalkModel::Dig(bool reset)
{
	int i,j,a;

	for (i=Raster.GetCount()-1; i>=0; i--) {
		if (DigMode.Get() && (GetPiece(i)&PF_FILLED)==0) {
			for (a=3; a>=0; a--) {
				j=GetNeigborIndex(i,a);
				if (j<0) continue;
				if ((GetPiece(j)&PF_FILLED)==0) continue;
				if (IsConnected(j,a+2)) break;
			}
			if (a<0) {
				if (reset) OrPiece(i,PF_BLOCKED);
				continue;
			}
		}
		AndPiece(i,~PF_BLOCKED);
	}
}


int emNetwalkModel::RawRotate(int piece, int angle)
{
	int c;

	for (angle&=3; angle; angle--) {
		c=piece&PF_CONMASK;
		piece&=~PF_CONMASK;
		if (c&PF_EAST ) piece|=PF_SOUTH;
		if (c&PF_SOUTH) piece|=PF_WEST;
		if (c&PF_WEST ) piece|=PF_NORTH;
		if (c&PF_NORTH) piece|=PF_EAST;
	}
	return piece;
}


emNetwalkModel::Solver::Solver(emNetwalkModel * model)
{
	int i,n,p,a;

	PieceCount=model->GetWidth()*model->GetHeight();
	Pieces=new Piece[PieceCount];
	Groups=new Group[PieceCount];
	for (n=0; (1<<n)<PieceCount; n++);
	n=PieceCount*(n+30)+100;
	TBBuf=new TBEntry[n];
	TBTop=TBBuf;
	TBEnd=TBBuf+n;
	for (i=0; i<PieceCount; i++) {
		p=model->GetPiece(i);
		Pieces[i].OrigDirs=0;
		for (a=0; a<4; a++) {
			if (p&A2PF[a]) Pieces[i].OrigDirs|=(1<<a);
			Pieces[i].Neighbor[a]=model->GetNeigborIndex(i,a);
		}
	}
}


emNetwalkModel::Solver::~Solver()
{
	delete [] Pieces;
	delete [] Groups;
	delete [] TBBuf;
}


bool emNetwalkModel::Solver::IsUniqueSolution()
{
	int i,a,d,found,cost;

	GroupCount=PieceCount;
	for (i=0; i<PieceCount; i++) {
		Pieces[i].Dirs=Pieces[i].OrigDirs;
		Pieces[i].Placed=0;
		Pieces[i].Group=i;
		Pieces[i].NextPiece=-1;
		Pieces[i].FrontRing=-1;
		Groups[i].FirstPiece=i;
		Groups[i].PieceCount=1;
		Groups[i].OpenCount=0;
		for (a=3; a>=0; a--) {
			if (Pieces[i].Dirs&(1<<a)) Groups[i].OpenCount++;
		}
	}
	FrontRing=-1;
	Current=0;
	cost=0;
	found=0;
	TBClear();
	TBStart();
L_TRACK_DOWN:
	cost++;
	if (cost>10000) return false;
	PlacePiece(Current);
	do {
		if (CheckPiece(Current)) {
			TBStart();
			if (TBEnd-TBTop<100+PieceCount) {
				emFatalError("emNetwalkModel::Solver: TBBuf too small");
			}
			if (UpdateGroups(Current)) {
				TBSet(Current,FindAndGetBestNext());
				if (Current>=0) goto L_TRACK_DOWN;
				if (GroupCount==1) {
					if (found>0) return false;
					found++;
				}
			}
L_TRACK_BACK:
			TakeBack();
		}
		d=Pieces[Current].Dirs;
		d=((d<<1)|(d>>3))&0x0F;
		Pieces[Current].Dirs=d;
	} while (d!=Pieces[Current].OrigDirs);
	if (Current>0) goto L_TRACK_BACK;
	return found==1;
}


bool emNetwalkModel::Solver::CheckPiece(int i) const
{
	int j,a,d,m,d2,m2;

	d=Pieces[i].Dirs;
	for (a=3; a>=0; a--) {
		j=Pieces[i].Neighbor[a];
		if (j<0) {
			if (d&(1<<a)) return false;
		}
		else if (Pieces[j].Placed) {
			d2=Pieces[j].Dirs;
			m2=1<<((a+2)&3);
			if (d2&m2) {
				m=(1<<a);
				if ((d&m)==0) return false;
				if (d2==m2 && d==m) return false;
			}
			else {
				if (d&(1<<a)) return false;
			}
		}
	}
	return true;
}


void emNetwalkModel::Solver::PlacePiece(int i)
{
	int a,j;

	TBSet(Pieces[i].Placed,1);
	for (a=3; a>=0; a--) {
		j=Pieces[i].Neighbor[a];
		if (j<0) continue;
		if (Pieces[j].Placed) continue;
		if (Pieces[j].FrontRing>=0) continue;
		if (FrontRing>=0) {
			TBSet(Pieces[j].FrontRing,Pieces[FrontRing].FrontRing);
			TBSet(Pieces[FrontRing].FrontRing,j);
		}
		else {
			TBSet(Pieces[j].FrontRing,j);
			TBSet(FrontRing,j);
		}
	}
}


bool emNetwalkModel::Solver::UpdateGroups(int i)
{
	int a,j,k,g,g2,n,t;

	for (a=3; a>=0; a--) {
		if ((Pieces[i].Dirs&(1<<a))==0) continue;
		j=Pieces[i].Neighbor[a];
		if (!Pieces[j].Placed) continue;
		g=Pieces[j].Group;
		g2=Pieces[i].Group;
		if (g==g2) return false;
		if (Groups[g].PieceCount<Groups[g2].PieceCount) { t=g; g=g2; g2=t; }
		n=Groups[g].OpenCount+Groups[g2].OpenCount-2;
		if (n<=0 && GroupCount>2) return false;
		TBSet(Groups[g].OpenCount,n);
		TBSet(Groups[g].PieceCount,Groups[g].PieceCount+Groups[g2].PieceCount);
		TBSet(GroupCount,GroupCount-1);
		for (k=Groups[g2].FirstPiece;;) {
			TBSet(Pieces[k].Group,g);
			t=Pieces[k].NextPiece;
			if (t<0) break;
			k=t;
		}
		TBSet(Pieces[k].NextPiece,Groups[g].FirstPiece);
		TBSet(Groups[g].FirstPiece,Groups[g2].FirstPiece);
	}
	return true;
}


int emNetwalkModel::Solver::FindAndGetBestNext()
{
	int i,j,d,n,nBest,iBest;

	if (FrontRing<0) return -1;
	iBest=FrontRing;
	nBest=5;
	for (i=FrontRing;;) {
		j=Pieces[i].FrontRing;
		n=0;
		do {
			if (CheckPiece(j)) n++;
			d=Pieces[j].Dirs;
			d=((d<<1)|(d>>3))&0x0F;
			Pieces[j].Dirs=d;
		} while (d!=Pieces[j].OrigDirs);
		if (nBest>n) {
			nBest=n;
			iBest=i;
			if (n<=1) break;
		}
		i=j;
		if (i==FrontRing) break;
	}
	i=iBest;
	j=Pieces[i].FrontRing;
	if (i==j) {
		TBSet(FrontRing,-1);
	}
	else {
		if (FrontRing!=i) TBSet(FrontRing,i);
		TBSet(Pieces[i].FrontRing,Pieces[j].FrontRing);
	}
	TBSet(Pieces[j].FrontRing,-1);
	return j;
}


void emNetwalkModel::Solver::TakeBack()
{
	TBEntry * t;

	for (t=TBTop-1; t->Ptr; t--) *(t->Ptr)=t->Val;
	TBTop=t;
}


const int emNetwalkModel::A2PF[4]={
	PF_EAST, PF_SOUTH, PF_WEST, PF_NORTH
};
