//------------------------------------------------------------------------------
// emMinesFileModel.h
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

#ifndef emMinesFileModel_h
#define emMinesFileModel_h

#ifndef emRecFileModel_h
#include <emCore/emRecFileModel.h>
#endif


class emMinesFileModel :
	public emRecFileModel,
	private emStructRec,
	private emRecListener
{

public:

	static emRef<emMinesFileModel> Acquire(
		emContext & context, const emString & name, bool common=true
	);

	virtual const char * GetFormatName() const;

	void StartGame(int level, bool saveFile=true);

	void StartGame(int sizeX, int sizeY, int sizeZ, int mineCount,
	               bool saveFile=true);

	int DetectLevel() const;

	bool IsGameWon() const;
	bool IsGameLost() const;

	int GetFieldCount() const;
	int GetMineCount() const;
	int GetMarkCount() const;
	int GetOpenCount() const;

	int GetSizeX() const;
	int GetSizeY() const;
	int GetSizeZ() const;

	void OpenField(int x, int y, int z, bool saveFile=true);
	void SetMark(int x, int y, int z, bool saveFile=true);
	void RemoveMark(int x, int y, int z, bool saveFile=true);
	void InvertMark(int x, int y, int z, bool saveFile=true);

	bool IsMine(int x, int y, int z) const;
	bool IsOpen(int x, int y, int z) const;
	bool IsMarked(int x, int y, int z) const;
	int GetSurroundings(int x, int y, int z) const;

protected:

	emMinesFileModel(emContext & context, const emString & name);
	virtual ~emMinesFileModel();

private:

	int GetField(int x, int y, int z) const;
	void SetField(int x, int y, int z, int f);

	virtual void OnRecChanged();

	void CalcExtraData();

	emIntRec SizeX;
	emIntRec SizeY;
	emIntRec SizeZ;

	emTArrayRec<emIntRec> Fields;
		// Index is (z*SizeY+y)*SizeX+x.
		// Value is a set of flags:
		//   bit 0: mine
		//   bit 1: open
		//   bit 2: marked (only if not open)

	bool ExtraDataValid;
	int FieldCount,MineCount,OpenCount,MarkCount;
	bool GameWon,GameLost;

	struct LevelParamsStruct {
		int SizeX;
		int SizeY;
		int SizeZ;
		int MineCount;
	};
	static const LevelParamsStruct LevelParams[5];
};

inline int emMinesFileModel::GetSizeX() const
{
	return SizeX;
}

inline int emMinesFileModel::GetSizeY() const
{
	return SizeY;
}

inline int emMinesFileModel::GetSizeZ() const
{
	return SizeZ;
}

inline bool emMinesFileModel::IsMine(int x, int y, int z) const
{
	return (GetField(x,y,z)&1)!=0;
}

inline bool emMinesFileModel::IsOpen(int x, int y, int z) const
{
	return (GetField(x,y,z)&2)!=0;
}

inline bool emMinesFileModel::IsMarked(int x, int y, int z) const
{
	return (GetField(x,y,z)&4)!=0;
}


#endif
