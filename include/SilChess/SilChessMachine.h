//------------------------------------------------------------------------------
// SilChessMachine.h
//
// Copyright (C) 2000-2005,2007-2008 Oliver Hamann.
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

#ifndef SilChessMachine_h
#define SilChessMachine_h


class SilChessMachine {

public:

	SilChessMachine();
	SilChessMachine(const SilChessMachine & machine);
	~SilChessMachine();

	SilChessMachine & operator = (const SilChessMachine & machine);

	class Move {
	public:
		signed char X1,Y1,X2,Y2;
		void ToString(char * str) const;
		bool FromString(const char * str);
		bool operator == (const Move & m) const;
	};

	void StartNewGame();

	enum {
		DEFAULT_SEARCH_DEPTH = 2,
		MAX_SEARCH_DEPTH     = 8
	};
	int GetSearchDepth() const;
	void SetSearchDepth(int searchDepth);
		// 0 means to find random moves.

	bool IsHumanWhite() const;
	void SetHumanWhite(bool humanWhite);

	bool IsWhiteOn() const;
	bool IsHumanOn() const;

	int GetMoveCount() const;
	const Move & GetMove(int index) const;

	int GetField(int x, int y) const;
		// 0=empty, 1=white pawn, 2=white knight, 3=white bishop,
		// 4=white rook, 5=white queen, 6=white king, 7=black pawn,
		// 8=black knight ... 12=black king.

	bool IsLegalMove(const Move & m) const;

	void DoMove(const Move & m);
	void UndoMove();

	bool SearchMove(Move * pResult);

	void StartSearching(bool cloneEngine=true);
	bool ContinueSearching();
	bool EndSearching(Move * pResult=NULL);
	bool IsSearching() const;

	bool IsCheck() const;
	bool IsMate() const;
	bool IsDraw() const;
	bool IsEndless() const;
	int GetValue() const;

	bool Save(const char * filename) const;
	bool Load(const char * filename);

	enum Charset {
		CS_ASCII,
		CS_ASCII2,
		CS_ANSI,
		CS_DOS,
		CS_MINI
	};
	void Print(Charset charset, const char * lastMove) const;

	void GeneticTraining();

private:

	enum PieceValues {
		PawnValue   =  2,
		KnightValue =  5,
		BishopValue =  5,
		RookValue   = 10,
		QueenValue  = 20,
		KingValue   = 40
	};

	enum PieceTypeFlags {
		TF_Pawn   = (1<<0),
		TF_Knight = (1<<1),
		TF_Bishop = (1<<2),
		TF_Rook   = (1<<3),
		TF_Queen  = (1<<4),
		TF_King   = (1<<5),
		TF_White  = (1<<6),
		TF_Black  = (1<<7)
	};

	enum PieceStateFlags {
		SF_CanCastle = (1<<0)
	};

	struct Piece {

		int Type;
			// Zero if unused, otherwise something like TF_Pawn|TF_White

		int X,Y;
			// Position (origin is at upper-left)

		int Value;
			// Constant value of the piece, e.g. PawnValue.

		int State;
			// Only for king and rook: SF_CanCastle = never moved.

		Piece * N[16];
			// Next piece for each direction (0 = right, 4 = bottom...).
	};

	Piece Pieces[32];
		// 0-15 white, 16-31 black. Type=0 if no longer on board.

	Piece * Board[8*8];
		// (origin is at upper-left)

	int SearchDepth;

	int HumanSide;
		// TF_White or TF_Black

	int Turn;
		// How is on: TF_White or TF_Black

	enum { MAX_MOVE_COUNT = 2048 };
	Move Moves[MAX_MOVE_COUNT];
	int MoveCount;

	bool CachedInfoValid;
	bool CachedIsCheck,CachedIsMate,CachedIsDraw;
	int CachedValue;

	struct SearchStackEntry {
		int Depth,Alpha,Beta,Count,Index,Found;
		Move Moves[512];
	};
	SearchStackEntry SearchStack[MAX_SEARCH_DEPTH+1];
	SearchStackEntry * SearchStackTop;
	int FoundVals[512];
	SilChessMachine * SearchMachine;

	enum ValueFactorIndex {
		VFI_Piece          =  0,
		VFI_PayingTurn     =  1,
		VFI_PayingTurnOppo =  2,
		VFI_Threats        =  3,
		VFI_Mobility       =  4,
		VFI_Ties           =  5,
		VFI_Center         =  6,
		VFI_KingCover      =  7,
		VFI_KingMobility   =  8,
		VFI_KingNotCentered=  9,
		VFI_KingCheck      = 10,
		VFI_PawnBeside     = 11,
		VFI_PawnOnward     = 12,
		VFI_PawnHeaven     = 13,
		VFI_Count          = 14
	};
	int ValFac[VFI_Count];
		// Factors for valuation.

	int ValRangeForRandom;

	struct TBIntEntry {
		int * Ptr; // NULL for start-mark
		int Val;
	};
	struct TBPtrEntry {
		Piece * * Ptr; // NULL for start-mark
		Piece * Val;
	};
	enum {
		MAX_TB_INT_ENTRIES =  11*(MAX_SEARCH_DEPTH+10),
		MAX_TB_PTR_ENTRIES = 118*(MAX_SEARCH_DEPTH+10)
	};
	TBIntEntry TBIntBuf[MAX_TB_INT_ENTRIES];
	TBPtrEntry TBPtrBuf[MAX_TB_PTR_ENTRIES];
	TBIntEntry * TBIntTop;
	TBPtrEntry * TBPtrTop;

	void UpdateCachedInfo();

	void SortMoves(Move * m, int count) const;

	int Value() const;

	int ValuePiece(const Piece & p) const;
	int ValuePayingHit(const Piece & p) const;
	int ValueThreats(const Piece & p) const;
	int ValueMobility(const Piece & p) const;
	int ValueTies(const Piece & p) const;
	int ValueCenter(const Piece & p) const;
	int ValueKing(const Piece & P) const;
	int ValuePawn(const Piece & p) const;

	bool IsAnyLegalMove() const;
	bool IsCheck(bool invertTurn) const;
	int EnumeratePossibleMoves(Move * buf) const;
	bool IsThreatened(int x, int y, int tside) const;

	void TBDoMove(const Move & m);
		// Perform a move while adding all changes to the take-back
		// buffer.

	void TBLinkPiece(Piece & p);
	void TBUnlinkPiece(Piece & p);
		// Link or unlink a piece while adding all changes to the
		// take-back buffer.

	void TBClear();
		// Clear the take-back buffer.

	void TBStart();
		// Add a start-mark to the take-back buffer.

	void TBSetInt(int & rInt, int val);
	void TBSetPtr(Piece * & rPtr, Piece * val);
		// Set an integer or a pointer while adding the change to the
		// take-back buffer.

	void TakeBack();
		// Take back changes until start-mark.

	void CalcNeighbours(int x, int y, Piece * * n) const;

	static int Random(int min, int max);

	void PrintASCII(bool flipped, const char * lastMove) const;
	void PrintASCII2(bool flipped, const char * lastMove) const;
	void PrintANSI(bool flipped, const char * lastMove) const;
	void PrintDOS(bool flipped, const char * lastMove) const;
	void PrintMINI(bool flipped, const char * lastMove) const;
};

inline int SilChessMachine::GetSearchDepth() const
{
	return SearchDepth;
}

inline bool SilChessMachine::IsHumanWhite() const
{
	return HumanSide==TF_White;
}

inline void SilChessMachine::SetHumanWhite(bool humanWhite)
{
	HumanSide = humanWhite ? TF_White : TF_Black;
}

inline bool SilChessMachine::IsWhiteOn() const
{
	return Turn==TF_White;
}

inline bool SilChessMachine::IsHumanOn() const
{
	return Turn==HumanSide;
}

inline int SilChessMachine::GetMoveCount() const
{
	return MoveCount;
}

inline const SilChessMachine::Move & SilChessMachine::GetMove(int index) const
{
	return Moves[index];
}

inline bool SilChessMachine::IsSearching() const
{
	return SearchStackTop!=NULL;
}

inline void SilChessMachine::TBClear()
{
	TBIntTop=TBIntBuf;
	TBPtrTop=TBPtrBuf;
}

inline void SilChessMachine::TBStart()
{
	TBIntTop->Ptr=NULL;
	TBIntTop++;
	TBPtrTop->Ptr=NULL;
	TBPtrTop++;
}

inline void SilChessMachine::TBSetInt(int & rInt, int val)
{
	TBIntTop->Ptr=&rInt;
	TBIntTop->Val=rInt;
	TBIntTop++;
	rInt=val;
}

inline void SilChessMachine::TBSetPtr(Piece * & rPtr, Piece * val)
{
	TBPtrTop->Ptr=&rPtr;
	TBPtrTop->Val=rPtr;
	TBPtrTop++;
	rPtr=val;
}


#endif
