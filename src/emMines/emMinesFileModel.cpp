//------------------------------------------------------------------------------
// emMinesFileModel.cpp
//
// Copyright (C) 2005-2009 Oliver Hamann.
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

#include <emMines/emMinesFileModel.h>


emRef<emMinesFileModel> emMinesFileModel::Acquire(
	emContext & context, const emString & name, bool common
)
{
	EM_IMPL_ACQUIRE(emMinesFileModel,context,name,common)
}


const char * emMinesFileModel::GetFormatName() const
{
	return "emMines";
}


void emMinesFileModel::StartGame(int level, bool saveFile)
{
	if (level<1) level=1;
	else if (level>5) level=5;
	StartGame(
		LevelParams[level-1].SizeX,
		LevelParams[level-1].SizeY,
		LevelParams[level-1].SizeZ,
		LevelParams[level-1].MineCount,
		saveFile
	);
}


void emMinesFileModel::StartGame(
	int sizeX, int sizeY, int sizeZ, int mineCount, bool saveFile
)
{
	int i,j,k,x,y,fcnt;

	ExtraDataValid=false;
	SizeX=sizeX;
	SizeY=sizeY;
	SizeZ=sizeZ;

	// Because they may have been clipped by the above assignments.
	sizeX=SizeX;
	sizeY=SizeY;
	sizeZ=SizeZ;

	fcnt=sizeX*sizeY*sizeZ;
	Fields.SetCount(fcnt);
	for (i=0; i<fcnt; i++) Fields[i].Set(0);

	if (mineCount>fcnt) mineCount=fcnt;
	for (i=0; i<mineCount; i++) {
		j=emGetIntRandom(0,fcnt-i-1);
		for (k=0; ; k++) {
			if (Fields[k].Get()==0) {
				j--;
				if (j<0) {
					Fields[k].Set(1);
					break;
				}
			}
		}
	}
	if (SizeZ>1) {
		for (y=0; y<SizeY; y++) for (x=0; x<SizeX; x++) {
			if (IsMine(x,y,0)) SetMark(x,y,0,false);
			else OpenField(x,y,0,false);
		}
	}
	if (saveFile) Save(true);
}


int emMinesFileModel::DetectLevel() const
{
	int level;

	if (!ExtraDataValid) ((emMinesFileModel*)this)->CalcExtraData();
	for (level=1; level<5; level++) {
		if (
			LevelParams[level-1].SizeX>=SizeX &&
			LevelParams[level-1].SizeY>=SizeY &&
			LevelParams[level-1].SizeZ>=SizeZ &&
			LevelParams[level-1].MineCount>=MineCount
		) break;
	}
	return level;
}


bool emMinesFileModel::IsGameWon() const
{
	if (!ExtraDataValid) ((emMinesFileModel*)this)->CalcExtraData();
	return GameWon;
}


bool emMinesFileModel::IsGameLost() const
{
	if (!ExtraDataValid) ((emMinesFileModel*)this)->CalcExtraData();
	return GameLost;
}


int emMinesFileModel::GetFieldCount() const
{
	if (!ExtraDataValid) ((emMinesFileModel*)this)->CalcExtraData();
	return FieldCount;
}


int emMinesFileModel::GetMineCount() const
{
	if (!ExtraDataValid) ((emMinesFileModel*)this)->CalcExtraData();
	return MineCount;
}


int emMinesFileModel::GetMarkCount() const
{
	if (!ExtraDataValid) ((emMinesFileModel*)this)->CalcExtraData();
	return MarkCount;
}


int emMinesFileModel::GetOpenCount() const
{
	if (!ExtraDataValid) ((emMinesFileModel*)this)->CalcExtraData();
	return OpenCount;
}


void emMinesFileModel::OpenField(int x, int y, int z, bool saveFile)
{
	int x1,y1,z1,x2,y2,z2,f;

	f=GetField(x,y,z);
	if ((f&2)!=0) return;
	f|=2;
	f&=~4;
	SetField(x,y,z,f);
	if (GetSurroundings(x,y,z)==0) {
		z1=z-1; if (z1<0) z1=0;
		z2=z+2; if (z2>SizeZ) z2=SizeZ;
		y1=y-1; if (y1<0) y1=0;
		y2=y+2; if (y2>SizeY) y2=SizeY;
		x1=x-1; if (x1<0) x1=0;
		x2=x+2; if (x2>SizeX) x2=SizeX;
		for (z=z1; z<z2; z++) {
			for (y=y1; y<y2; y++) {
				for (x=x1; x<x2; x++) {
					OpenField(x,y,z,false);
				}
			}
		}
	}
	if (saveFile) Save(true);
}


void emMinesFileModel::SetMark(int x, int y, int z, bool saveFile)
{
	int f;

	f=GetField(x,y,z);
	if ((f&6)==0) {
		f|=4;
		SetField(x,y,z,f);
		if (saveFile) Save(true);
	}
}


void emMinesFileModel::RemoveMark(int x, int y, int z, bool saveFile)
{
	int f;

	f=GetField(x,y,z);
	if ((f&4)!=0) {
		f&=~4;
		SetField(x,y,z,f);
		if (saveFile) Save(true);
	}
}


void emMinesFileModel::InvertMark(int x, int y, int z, bool saveFile)
{
	int f;

	f=GetField(x,y,z);
	if ((f&4)==0) SetMark(x,y,z,saveFile);
	else RemoveMark(x,y,z,saveFile);
}


int emMinesFileModel::GetSurroundings(int x, int y, int z) const
{
	return
		(GetField(x-1,y-1,z-1)&1) +
		(GetField(x  ,y-1,z-1)&1) +
		(GetField(x+1,y-1,z-1)&1) +
		(GetField(x-1,y  ,z-1)&1) +
		(GetField(x  ,y  ,z-1)&1) +
		(GetField(x+1,y  ,z-1)&1) +
		(GetField(x-1,y+1,z-1)&1) +
		(GetField(x  ,y+1,z-1)&1) +
		(GetField(x+1,y+1,z-1)&1) +
		(GetField(x-1,y-1,z  )&1) +
		(GetField(x  ,y-1,z  )&1) +
		(GetField(x+1,y-1,z  )&1) +
		(GetField(x-1,y  ,z  )&1) +
		(GetField(x+1,y  ,z  )&1) +
		(GetField(x-1,y+1,z  )&1) +
		(GetField(x  ,y+1,z  )&1) +
		(GetField(x+1,y+1,z  )&1) +
		(GetField(x-1,y-1,z+1)&1) +
		(GetField(x  ,y-1,z+1)&1) +
		(GetField(x+1,y-1,z+1)&1) +
		(GetField(x-1,y  ,z+1)&1) +
		(GetField(x  ,y  ,z+1)&1) +
		(GetField(x+1,y  ,z+1)&1) +
		(GetField(x-1,y+1,z+1)&1) +
		(GetField(x  ,y+1,z+1)&1) +
		(GetField(x+1,y+1,z+1)&1)
	;
}


emMinesFileModel::emMinesFileModel(emContext & context, const emString & name)
	: emRecFileModel(context,name),
	emStructRec(),
	SizeX(this,"SizeX",1,1,20),
	SizeY(this,"SizeY",1,1,20),
	SizeZ(this,"SizeZ",1,1,20),
	Fields(this,"Fields",1,20*20*20)
{
	ExtraDataValid=false;
	PostConstruct(*this);
	SetListenedRec(this);
}


emMinesFileModel::~emMinesFileModel()
{
}


int emMinesFileModel::GetField(int x, int y, int z) const
{
	int i;

	if (
		((unsigned)x)>=(unsigned)SizeX ||
		((unsigned)y)>=(unsigned)SizeY ||
		((unsigned)z)>=(unsigned)SizeZ
	) return 0;
	i=(z*SizeY+y)*SizeX+x;
	if (i>=Fields.GetCount()) return 0;
	return Fields[i].Get();
}


void emMinesFileModel::SetField(int x, int y, int z, int f)
{
	int i;

	if (
		((unsigned)x)>=(unsigned)SizeX ||
		((unsigned)y)>=(unsigned)SizeY ||
		((unsigned)z)>=(unsigned)SizeZ
	) return;
	i=(z*SizeY+y)*SizeX+x;
	if (i>=Fields.GetCount()) Fields.SetCount(i+1);
	Fields[i].Set(f);
}


void emMinesFileModel::OnRecChanged()
{
	ExtraDataValid=false;
}


void emMinesFileModel::CalcExtraData()
{
	int x, y, z, f;

	FieldCount=SizeX*SizeY*SizeZ;
	MineCount=0;
	OpenCount=0;
	MarkCount=0;
	GameWon=false;
	GameLost=false;
	for (z=0; z<SizeZ; z++) for (y=0; y<SizeY; y++) for (x=0; x<SizeX; x++) {
		f=GetField(x,y,z);
		if (f&1) {
			MineCount++;
			if (f&2) GameLost=true;
		}
		if (f&2) OpenCount++;
		if (f&4) MarkCount++;
	}
	if (!GameLost && OpenCount==FieldCount-MineCount) GameWon=true;
	ExtraDataValid=true;
}


const emMinesFileModel::LevelParamsStruct emMinesFileModel::LevelParams[5]={
	{  4, 4, 3,   9 }, // 5.33 fields per mine
	{  6, 4, 4,  19 }, // 5.05 fields per mine
	{  8, 6, 5,  50 }, // 4.8  fields per mine
	{ 12, 8, 6, 125 }, // 4.61 fields per mine
	{ 14,10, 8, 250 }  // 4.48 fields per mine
};
