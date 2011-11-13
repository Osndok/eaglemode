//------------------------------------------------------------------------------
// emNetwalkModel.h
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

#ifndef emNetwalkModel_h
#define emNetwalkModel_h

#ifndef emRecFileModel_h
#include <emCore/emRecFileModel.h>
#endif


class emNetwalkModel : public emRecFileModel, private emStructRec
{

public:

	static emRef<emNetwalkModel> Acquire(
		emContext & context, const emString & name, bool common=true
	);

	virtual const char * GetFormatName() const;

	int GetWidth() const;
	int GetHeight() const;

	bool IsBorderless() const;
	bool IsNoFourWayJunctions() const;
	int GetComplexity() const;
	bool IsDigMode() const;

	void SetAutoMark(bool autoMark, bool saveFile=true);
	bool IsAutoMark() const;

	bool IsFinished() const;
	int GetPenaltyPoints() const;

	int GetPiece(int x, int y) const;
	enum {
		PF_EAST   =(1<<0),
		PF_SOUTH  =(1<<1),
		PF_WEST   =(1<<2),
		PF_NORTH  =(1<<3),
		PF_SOURCE =(1<<4),
		PF_TARGET =(1<<5),
		PF_FILLED =(1<<6),
		PF_TOUCHED=(1<<7),
		PF_MARKED =(1<<8),
		PF_BLOCKED=(1<<9),
		PF_CONMASK=PF_EAST|PF_SOUTH|PF_WEST|PF_NORTH
	};

	void TrySetup(
		int width, int height, bool borderless, bool noFourWayJunctions,
		int complexity, bool digMode, bool autoMark, bool saveFile=true
	) throw(emString);

	void MarkOrUnmark(int x, int y, bool saveFile=true);
	void UnmarkAll(bool saveFile=true);

	void Rotate(int x, int y, int angle, bool saveFile=true);
	void RotateLeft(int x, int y, bool saveFile=true);
	void RotateRight(int x, int y, bool saveFile=true);

	void Scroll(int dx, int dy, bool saveFile=true);

protected:

	emNetwalkModel(emContext & context, const emString & name);
	virtual ~emNetwalkModel();

	virtual bool TryContinueLoading() throw(emString);

	virtual bool Cycle();

private:

	int GetPiece(int index) const;
	void SetPiece(int index, int piece);
	void XorPiece(int index, int flags);
	void OrPiece(int index, int flags);
	void AndPiece(int index, int flags);

	int GetNeigborIndex(int index, int angle) const;
	bool IsConnected(int index, int angle) const;
	void Connect(int index, int angle);

	void Invent();
	void Shuffle();
	void Fill();
	void Dig(bool reset);

	void DoAutoMark();

	static int RawRotate(int piece, int angle);

	class Solver : public emUncopyable {
	public:
		Solver(emNetwalkModel * model);
		~Solver();
		bool IsUniqueSolution();
	private:
		struct Piece {
			int OrigDirs;
			int Dirs;
			int Placed;
			int Group;
			int NextPiece;
			int FrontRing;
			int Neighbor[4];
		};
		struct Group {
			int FirstPiece;
			int PieceCount;
			int OpenCount;
		};
		struct TBEntry {
			int * Ptr; // NULL for start-mark
			int Val;
		};
		bool CheckPiece(int i) const;
		void PlacePiece(int i);
		bool UpdateGroups(int i);
		int FindAndGetBestNext();
		void TBClear();
		void TBStart();
		void TBSet(int & rInt, int val);
		void TakeBack();
		int PieceCount;
		int GroupCount;
		int FrontRing;
		int Current;
		Piece * Pieces;
		Group * Groups;
		TBEntry * TBBuf, * TBTop, * TBEnd;
	};
	friend class Solver;

	emIntRec Width;
	emIntRec Height;
	emBoolRec Borderless;
	emBoolRec NoFourWayJunctions;
	emIntRec Complexity;
	emBoolRec DigMode;
	emBoolRec AutoMark;
	emBoolRec Finished;
	emIntRec PenaltyPoints;
	emIntRec CurrentPiece;
	emTArrayRec<emIntRec> Raster;

	emTimer AutoMarkTimer;
	int AutoMarkPiece;
	bool AutoMarkToSave;

	static const int A2PF[4];
};

inline int emNetwalkModel::GetWidth() const
{
	return Width;
}

inline int emNetwalkModel::GetHeight() const
{
	return Height;
}

inline bool emNetwalkModel::IsBorderless() const
{
	return Borderless;
}

inline bool emNetwalkModel::IsNoFourWayJunctions() const
{
	return NoFourWayJunctions;
}

inline int emNetwalkModel::GetComplexity() const
{
	return Complexity;
}

inline bool emNetwalkModel::IsDigMode() const
{
	return DigMode;
}

inline bool emNetwalkModel::IsAutoMark() const
{
	return AutoMark;
}

inline bool emNetwalkModel::IsFinished() const
{
	return Finished;
}

inline int emNetwalkModel::GetPenaltyPoints() const
{
	return PenaltyPoints;
}

inline void emNetwalkModel::RotateLeft(int x, int y, bool saveFile)
{
	Rotate(x,y,-1,saveFile);
}

inline void emNetwalkModel::RotateRight(int x, int y, bool saveFile)
{
	Rotate(x,y,1,saveFile);
}

inline int emNetwalkModel::GetPiece(int index) const
{
	return Raster[index].Get();
}

inline void emNetwalkModel::SetPiece(int index, int piece)
{
	Raster[index].Set(piece);
}

inline void emNetwalkModel::XorPiece(int index, int flags)
{
	SetPiece(index,GetPiece(index)^flags);
}

inline void emNetwalkModel::OrPiece(int index, int flags)
{
	SetPiece(index,GetPiece(index)|flags);
}

inline void emNetwalkModel::AndPiece(int index, int flags)
{
	SetPiece(index,GetPiece(index)&flags);
}

inline void emNetwalkModel::Solver::TBClear()
{
	TBTop=TBBuf;
}

inline void emNetwalkModel::Solver::TBStart()
{
	TBTop->Ptr=NULL;
	TBTop++;
}

inline void emNetwalkModel::Solver::TBSet(int & rInt, int val)
{
	TBTop->Ptr=&rInt;
	TBTop->Val=rInt;
	TBTop++;
	rInt=val;
}


#endif
