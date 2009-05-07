//------------------------------------------------------------------------------
// SilChessMachine.cpp
//
// Copyright (C) 2000-2005,2007-2009 Oliver Hamann.
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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <limits.h>
#include <time.h>
#include <SilChess/SilChessMachine.h>


SilChessMachine::SilChessMachine()
{
	SearchDepth=DEFAULT_SEARCH_DEPTH;
	HumanSide=TF_White;
	SearchStackTop=NULL;
	SearchMachine=NULL;
	CachedInfoValid=false;
	StartNewGame();
}


SilChessMachine::SilChessMachine(const SilChessMachine & machine)
{
	SearchDepth=DEFAULT_SEARCH_DEPTH;
	HumanSide=TF_White;
	SearchStackTop=NULL;
	SearchMachine=NULL;
	CachedInfoValid=false;
	StartNewGame();
	*this=machine;
}


SilChessMachine::~SilChessMachine()
{
	EndSearching();
}


SilChessMachine & SilChessMachine::operator = (const SilChessMachine & machine)
{
	int i,j;

	EndSearching();
	CachedInfoValid=false;
	SearchDepth=machine.SearchDepth;
	HumanSide=machine.HumanSide;
	memcpy(Pieces,machine.Pieces,sizeof(Pieces));
	for (i=0; i<32; i++) for (j=0; j<16; j++) if (Pieces[i].N[j]) {
		Pieces[i].N[j]=Pieces+(Pieces[i].N[j]-machine.Pieces);
	}
	for (i=0; i<64; i++) {
		Board[i] = machine.Board[i] ? Pieces+(machine.Board[i]-machine.Pieces) : NULL;
	}
	Turn=machine.Turn;
	memcpy(Moves,machine.Moves,sizeof(Moves));
	MoveCount=machine.MoveCount;
	TBClear();
	memcpy(ValFac,machine.ValFac,sizeof(ValFac));
	return *this;
}


void SilChessMachine::Move::ToString(char * str) const
{
	str[0]=(char)('a'+X1);
	str[1]=(char)('8'-Y1);
	str[2]=(char)('a'+X2);
	str[3]=(char)('8'-Y2);
	str[4]=0;
}


bool SilChessMachine::Move::FromString(const char * str)
{
	int i;

	if (strlen(str)<4) return false;
	if (str[0]>='A' && str[0]<='Z') X1=(signed char)(str[0]-'A');
	else X1=(signed char)(str[0]-'a');
	Y1=(signed char)('8'-str[1]);
	if (str[2]>='A' && str[2]<='Z') X2=(signed char)(str[2]-'A');
	else X2=(signed char)(str[2]-'a');
	Y2=(signed char)('8'-str[3]);
	if (X1<0 || Y1<0 || X2<0 || Y2<0 ||
	    X1>7 || Y1>7 || X2>7 || Y2>7) return false;
	for (i=4; str[i]!=0; i++) if ((unsigned char)str[i]>32) return false;
	return true;
}


bool SilChessMachine::Move::operator == (const Move & m) const
{
	return X1==m.X1 && Y1==m.Y1 && X2==m.X2 && Y2==m.Y2;
}


void SilChessMachine::StartNewGame()
{
	int i;

	EndSearching();
	CachedInfoValid=false;

	// The following factors have been optimized by a genetic algorithm.
	ValFac[VFI_Piece          ]= 116;
	ValFac[VFI_PayingTurn     ]=  90;
	ValFac[VFI_PayingTurnOppo ]=   2;
	ValFac[VFI_Threats        ]=  12;
	ValFac[VFI_Mobility       ]=   6;
	ValFac[VFI_Ties           ]=   2;
	ValFac[VFI_Center         ]=   1;
	ValFac[VFI_KingCover      ]=   2;
	ValFac[VFI_KingMobility   ]=   6;
	ValFac[VFI_KingNotCentered]=   2;
	ValFac[VFI_KingCheck      ]= 112;
	ValFac[VFI_PawnBeside     ]=   6;
	ValFac[VFI_PawnOnward     ]=  26;
	ValFac[VFI_PawnHeaven     ]= 120;

	// The higher this value, the more random are the computer moves.
	ValRangeForRandom=3;

	TBClear();
	memset(Pieces,0,sizeof(Pieces));
	memset(Board,0,sizeof(Board));
	Turn=TF_White;
	MoveCount=0;
	for (i=0; i<16; i++) {
		if (i<8) {
			Pieces[i].Type=TF_Pawn|TF_White;
			Pieces[i].Value=PawnValue;
		}
		else if (i==8 || i==15) {
			Pieces[i].Type=TF_Rook|TF_White;
			Pieces[i].Value=RookValue;
			Pieces[i].State=SF_CanCastle;
		}
		else if (i==9 || i==14) {
			Pieces[i].Type=TF_Knight|TF_White;
			Pieces[i].Value=KnightValue;
		}
		else if (i==10 || i==13) {
			Pieces[i].Type=TF_Bishop|TF_White;
			Pieces[i].Value=BishopValue;
		}
		else if (i==11) {
			Pieces[i].Type=TF_Queen|TF_White;
			Pieces[i].Value=QueenValue;
		}
		else {
			Pieces[i].Type=TF_King|TF_White;
			Pieces[i].Value=KingValue;
			Pieces[i].State=SF_CanCastle;
		}
		Pieces[i].X=i&7;
		Pieces[i].Y=6+(i>>3);
	}
	for (i=16; i<32; i++) {
		Pieces[i]=Pieces[i-16];
		Pieces[i].Type^=TF_White|TF_Black;
		Pieces[i].Y=7-Pieces[i].Y;
	}
	for (i=0; i<32; i++) TBLinkPiece(Pieces[i]);
	TBClear();
}


void SilChessMachine::SetSearchDepth(int searchDepth)
{
	if (searchDepth<0) searchDepth=0;
	if (searchDepth>MAX_SEARCH_DEPTH) searchDepth=MAX_SEARCH_DEPTH;
	if (SearchDepth!=searchDepth) {
		EndSearching();
		SearchDepth=searchDepth;
	}
}


int SilChessMachine::GetField(int x, int y) const
{
	Piece * p;
	int t,r;

	p=Board[y*8+x];
	if (!p) return 0;
	t=p->Type;
	if      (t&TF_Pawn  ) r=1;
	else if (t&TF_Knight) r=2;
	else if (t&TF_Bishop) r=3;
	else if (t&TF_Rook  ) r=4;
	else if (t&TF_Queen ) r=5;
	else                  r=6;
	if (t&TF_Black) r+=6;
	return r;
}


bool SilChessMachine::IsLegalMove(const Move & m) const
{
	Move pm[512];
	int pmc,i;
	bool check;
	SilChessMachine * machine;

	pmc=EnumeratePossibleMoves(pm);
	for (i=0; i<pmc; i++) {
		if (pm[i]==m) {
			machine=(SilChessMachine*)this;
			machine->TBStart();
			machine->TBDoMove(m);
			check=machine->IsCheck(true);
			machine->TakeBack();
			return !check;
		}
	}
	return false;
}


void SilChessMachine::DoMove(const Move & m)
{
	EndSearching();
	TBDoMove(m);
	TBClear();
}


void SilChessMachine::UndoMove()
{
	Move m[MAX_MOVE_COUNT];
	int mc,i;

	if (MoveCount<=0) return;
	EndSearching();
	mc=MoveCount-1;
	memcpy(m,Moves,sizeof(Move)*mc);
	StartNewGame();
	for (i=0; i<mc; i++) DoMove(m[i]);
}


bool SilChessMachine::SearchMove(Move * pResult)
{
	StartSearching(false);
	while (!ContinueSearching()) {}
	return EndSearching(pResult);
}


void SilChessMachine::StartSearching(bool cloneEngine)
{
	SearchStackEntry * top;
	int i;

	EndSearching();

	top=SearchStack;
	SearchStackTop=top;
	top->Depth=SearchDepth;
	top->Alpha=-INT_MAX;
	top->Beta=INT_MAX;
	top->Count=EnumeratePossibleMoves(top->Moves);
	if (top->Depth>1) SortMoves(top->Moves,top->Count);
	top->Index=0;
	for (i=0; i<512; i++) FoundVals[i]=-INT_MAX;

	if (cloneEngine) {
		SearchMachine=new SilChessMachine(*this);
	}
	else {
		SearchMachine=this;
	}
}


bool SilChessMachine::ContinueSearching()
{
	SearchStackEntry * top;
	int v;

	top=SearchStackTop;
	if (!top) return false;
L_Next:
	if (top->Index>=top->Count) {
		if (top>SearchStack) goto L_Pop;
		SearchStackTop=top;
		return true;
	}
	SearchMachine->TBStart();
	SearchMachine->TBDoMove(top->Moves[top->Index]);
	if (!SearchMachine->IsCheck(true)) {
		if (top->Depth>1) {
			top[1].Depth=top->Depth-1;
			top[1].Alpha=-top->Beta;
			top[1].Beta=-top->Alpha;
			top++;
			top->Count=SearchMachine->EnumeratePossibleMoves(top->Moves);
			if (top->Depth>1) SearchMachine->SortMoves(top->Moves,top->Count);
			top->Index=0;
			top->Found=0;
			goto L_Next;
L_Pop:
			if (top->Found>0) v=-top->Alpha;
			else if (SearchMachine->IsCheck(false)) v=INT_MAX;
			else v=0;
			top--;
		}
		else if (top->Depth==1) {
			v=-SearchMachine->Value();
		}
		else {
			v=0;
		}
		if (top>SearchStack) {
			top->Found++;
			if (v>top->Alpha) {
				top->Alpha=v;
				if (v>=top->Beta) {
					SearchMachine->TakeBack();
					goto L_Pop;
				}
			}
		}
		else {
			if (v==-INT_MAX) v=-INT_MAX+1;
			FoundVals[top->Index]=v;
			if (v<=-INT_MAX+2+ValRangeForRandom) v=-INT_MAX+1;
			else v-=1+ValRangeForRandom;
			if (v>top->Alpha) top->Alpha=v;
		}
	}
	SearchMachine->TakeBack();
	top->Index++;
	if (top->Depth<3) goto L_Next;
	SearchStackTop=top;
	return false;
}


bool SilChessMachine::EndSearching(Move * pResult)
{
	SearchStackEntry * top;
	int i,n,m,v;
	bool res;

	res=false;
	top=SearchStackTop;
	if (top) {
		if (top==SearchStack && top->Index==top->Count) {
			n=top->Count;
			v=-INT_MAX;
			for (i=0; i<n; i++) if (v<FoundVals[i]) v=FoundVals[i];
			if (v>-INT_MAX) {
				if (pResult) {
					if (v<=-INT_MAX+1+ValRangeForRandom) v=-INT_MAX+1;
					else v-=ValRangeForRandom;
					for (i=0, m=0; i<n; i++) {
						if (FoundVals[i]>=v) top->Moves[m++]=top->Moves[i];
					}
					*pResult=top->Moves[Random(0,m-1)];
//					fprintf(stderr,"\nfound %d best moves\n",m);
				}
				res=true;
			}
		}
		if (SearchMachine!=this) delete SearchMachine;
		SearchMachine=NULL;
		SearchStackTop=NULL;
	}
	return res;
}


bool SilChessMachine::IsCheck() const
{
	SilChessMachine * m;

	m=(SilChessMachine*)this;
	if (!m->CachedInfoValid) m->UpdateCachedInfo();
	return m->CachedIsCheck;
}


bool SilChessMachine::IsMate() const
{
	SilChessMachine * m;

	m=(SilChessMachine*)this;
	if (!m->CachedInfoValid) m->UpdateCachedInfo();
	return m->CachedIsMate;
}


bool SilChessMachine::IsDraw() const
{
	SilChessMachine * m;

	m=(SilChessMachine*)this;
	if (!m->CachedInfoValid) m->UpdateCachedInfo();
	return m->CachedIsDraw;
}


bool SilChessMachine::IsEndless() const
{
	return MoveCount>MAX_MOVE_COUNT-100;
}


int SilChessMachine::GetValue() const
{
	SilChessMachine * m;

	m=(SilChessMachine*)this;
	if (!m->CachedInfoValid) m->UpdateCachedInfo();
	return m->CachedValue;
}


bool SilChessMachine::Save(const char * filename) const
{
	FILE * f;
	char tmp[256];
	int i;

	if ((f=fopen(filename,"wb"))==NULL) return false;
	fprintf(
		f,
		"_SilChess_\n"
		"search depth: %d\n"
		"human side: %s\n"
		"moves:\n",
		SearchDepth,
		HumanSide==TF_White ? "white" : "black"
	);
	for (i=0; i<GetMoveCount(); i++) {
		GetMove(i).ToString(tmp);
		fprintf(f,"%s\n",tmp);
	}
	fflush(f);
	if (ferror(f)) { fclose(f); return false; }
	fclose(f);
	return true;
}


bool SilChessMachine::Load(const char * filename)
{
	FILE * f;
	char tmp[256];
	int i;
	Move m;

	StartNewGame();
	if ((f=fopen(filename,"rb"))==NULL) return false;
	if (!fgets(tmp,256,f)) tmp[0]=0;
	if (memcmp(tmp,"_SilChess_",10)!=0) goto L_FormatError;
	if (!fgets(tmp,256,f)) tmp[0]=0;
	if (memcmp(tmp,"search depth:",13)!=0) goto L_FormatError;
	for (i=13; tmp[i]!=0 && (unsigned char)tmp[i]<=32; i++);
	SearchDepth=atoi(tmp+i);
	if (SearchDepth<0 || SearchDepth>MAX_SEARCH_DEPTH) goto L_FormatError;
	if (!fgets(tmp,256,f)) tmp[0]=0;
	if (memcmp(tmp,"human side:",11)!=0) goto L_FormatError;
	for (i=11; tmp[i]!=0 && (unsigned char)tmp[i]<=32; i++);
	if (memcmp(tmp+i,"white",5)==0) HumanSide=TF_White;
	else if (memcmp(tmp+i,"black",5)==0) HumanSide=TF_Black;
	else goto L_FormatError;
	if (!fgets(tmp,256,f)) tmp[0]=0;
	if (memcmp(tmp,"moves:",6)!=0) goto L_FormatError;
	while (!feof(f)) {
		tmp[0]=0;
		if (!fgets(tmp,256,f)) tmp[0]=0;
		for (i=0; tmp[i]!=0 && (unsigned char)tmp[i]<=32; i++);
		if (tmp[i]!=0) {
			if (!m.FromString(tmp+i)) goto L_FormatError;
			DoMove(m);
		}
	}
	if (ferror(f)) {
		fclose(f);
		return false;
	}
	fclose(f);
	return true;
L_FormatError:
	fclose(f);
	return false;
}


void SilChessMachine::Print(Charset charset, const char * lastMove) const
{
	switch (charset) {
		case CS_ASCII : PrintASCII (!IsHumanWhite(),lastMove); break;
		case CS_ASCII2: PrintASCII2(!IsHumanWhite(),lastMove); break;
		case CS_ANSI  : PrintANSI  (!IsHumanWhite(),lastMove); break;
		case CS_DOS   : PrintDOS   (!IsHumanWhite(),lastMove); break;
		case CS_MINI  : PrintMINI  (!IsHumanWhite(),lastMove); break;
	}
}


void SilChessMachine::GeneticTraining()
{
	// This is a Genetic Algorithm for optimizing the valuation factors
	// (ValFac[]). Computer players with different genes (=valuation
	// factors) are playing against each other, the best survive and leave
	// their genes to the next generation.
	// Coming to the current factors was an interactive process: generate
	// first factors, modify the code, improve the factors, modify the
	// code, improve the factors and so on. Highly experimental...

	#define CGT_PopCount 12 // >=4
	#define CGT_PlayDepth 1
	#define CGT_PlayMaxMoves 70
	int Pop[VFI_Count][CGT_PopCount];
	int PopTmp[VFI_Count][CGT_PopCount];
	int Fitness[CGT_PopCount];
	Move move;
	int i,j,k,l,m;

	StartNewGame();
	for (j=0; j<CGT_PopCount; j++) for (i=0; i<VFI_Count; i++) {
		m=ValFac[i];
		if (Random(0,7)==0) {
			if (m<=6) m+=Random(-4,4);
			else m+=Random(-128,128)*m/128/5;
		}
//		m=Random(ValFac[i]/4,ValFac[i]*3);
//		m+=Random(-8,8);
		if (m>255) m=255; else if (m<0) m=0;
		Pop[i][j]=m;
	}

L_Loop:

	StartNewGame();
	for (i=0; i<VFI_Count; i++) {
		Pop[i][CGT_PopCount-1]=ValFac[i];
	}

//	for (j=0; j<CGT_PopCount; j++) {
//		Pop[0][j]=ValFac[0];
//		Pop[1][j]=ValFac[1];
//	}


	printf("New Population:\n");
	for (j=0; j<CGT_PopCount; j++) {
		printf("%2d: ",j);
		for (i=0; i<VFI_Count; i++) {
			printf("%4d",Pop[i][j]);
		}
		printf("\n");
	}

	for (i=0; i<CGT_PopCount; i++) Fitness[i]=0;
	for (i=0; i<CGT_PopCount; i++) {
		printf("%2d:",i);
		for (j=0; j<CGT_PopCount; j++) if (i!=j) {
			StartNewGame();
			for (l=0; l<CGT_PlayMaxMoves; l++) {
				for (k=0; k<VFI_Count; k++) ValFac[k]=Pop[k][i];
				SetSearchDepth(i==CGT_PopCount-1 ? CGT_PlayDepth+2 : CGT_PlayDepth);
//				SetSearchDepth(playDepth);
				if (!SearchMove(&move)) break;
				DoMove(move);
				for (k=0; k<VFI_Count; k++) ValFac[k]=Pop[k][j];
				SetSearchDepth(j==CGT_PopCount-1 ? CGT_PlayDepth+2 : CGT_PlayDepth);
//				SetSearchDepth(playDepth);
				if (!SearchMove(&move)) break;
				DoMove(move);
			}
			m=0;
			if (l<CGT_PlayMaxMoves) {
				if (Turn&TF_Black) m+=100; else m-=100;
				printf("M");
			}
			else printf(" ");
			for (k=0; k<32; k++) if (Pieces[k].Type) {
				if (Pieces[k].Type&TF_White) m+=Pieces[k].Value;
				else m-=Pieces[k].Value;
			}
			printf("%4d ",m); fflush(stdout);
			Fitness[i]+=m; // if (m>0) Fitness[i]+=m;
			Fitness[j]-=m; // if (m<0) Fitness[j]-=m;
		}
		printf("\n");
	}
	printf("Fitness:\n");
	for (j=0; j<CGT_PopCount; j++) {
		printf("%2d: ",j);
		for (i=0; i<VFI_Count; i++) {
			printf("%4d",Pop[i][j]);
		}
		printf(" = %d\n",Fitness[j]);
	}

	for (j=0; j<CGT_PopCount; j++) for (i=0; i<VFI_Count; i++) {
		PopTmp[i][j]=Pop[i][j];
	}
	for (j=0; j<CGT_PopCount/2; j++) {
		m=Fitness[0]; k=0;
//		for (i=1; i<CGT_PopCount; i++) {
		for (i=1; i<CGT_PopCount-1; i++) {
			if (m<Fitness[i]) { m=Fitness[i]; k=i; }
		}
		Fitness[k]=INT_MIN;
		for (i=0; i<VFI_Count; i++) {
			Pop[i][j]=PopTmp[i][k];
		}
	}
	if (Fitness[0]!=INT_MIN) {
		for (i=0; i<VFI_Count; i++) {
			Pop[i][CGT_PopCount/2-1]=PopTmp[i][0];
		}
	}

	for (j=CGT_PopCount/2; j<CGT_PopCount ;j++) {
		k=Random(0,CGT_PopCount/2-1);
		l=Random(0,CGT_PopCount/2-2);
		if (l>=k) l++;
		for (i=0; i<VFI_Count; i++) {
			if (Random(0,1)) m=Pop[i][k]; else m=Pop[i][l];
			if (Random(0,7)==0) {
				if (m<=12) m+=Random(-2,2);
				else m+=Random(-128,128)*m/128/10;
			}

			if (m>255) m=255; else if (m<1) m=1;
			Pop[i][j]=m;
		}
	}

	goto L_Loop;
}


void SilChessMachine::UpdateCachedInfo()
{
	CachedIsCheck=IsCheck(false);
	if (!IsAnyLegalMove()) {
		CachedIsMate=CachedIsCheck;
		CachedIsDraw=!CachedIsCheck;
	}
	else {
		CachedIsMate=false;
		CachedIsDraw=false;
	}
	CachedValue=Value();
	CachedInfoValid=true;
}


void SilChessMachine::SortMoves(Move * m, int count) const
{
	int lr[512*2];
	int * p;
	int v[512];
	int vm,tv;
	int i,j;
	Move tpm;

	for (i=0; i<count; i++) {
		((SilChessMachine*)this)->TBStart();
		((SilChessMachine*)this)->TBDoMove(m[i]);
		if (IsCheck(true)) v[i]=INT_MAX;
		else v[i]=Value();
		((SilChessMachine*)this)->TakeBack();
	}

	p=lr; p[0]=0; p[1]=count-1;
	for (;;) {
		i=p[0]; j=p[1]; vm=v[(i+j)/2];
		for (;;) {
			while (v[i]<vm) i++;
			while (v[j]>vm) j--;
			if (i>=j) break;
			if (v[i]!=v[j]) {
				tpm=m[i]; m[i]=m[j]; m[j]=tpm;
				tv =v[i]; v[i]=v[j]; v[j]=tv ;
			}
			i++; j--;
		}
		while (i<p[1] && v[i]==vm) i++;
		while (j>p[0] && v[j]==vm) j--;
		if (i<p[1]) {
			if (j>p[0]) {
				p[2]=p[0];
				p[3]=j;
				p[0]=i;
				p+=2;
			}
			else p[0]=i;
		}
		else if (j>p[0]) p[1]=j;
		else if (p>lr) p-=2;
		else break;
	}
}


int SilChessMachine::Value() const
{
	int v;
	int i;

	// Value the whole situation from sight of the one who is on.

	for (v=0, i=0; i<32; i++) if (Pieces[i].Type) v+=ValuePiece(Pieces[i]);
	return v;
}


int SilChessMachine::ValuePiece(const Piece & p) const
{
	int v;

	// Value the piece from sight of the one who is on.

	v =
		p.Value*ValFac[VFI_Piece] +
		ValuePayingHit(p) +
		ValueThreats(p) +
		ValueMobility(p) +
		ValueTies(p) +
		ValueCenter(p) +
		ValueKing(p) +
		ValuePawn(p)
	;
	if (!(p.Type&Turn)) v=-v;
	return v;
}


int SilChessMachine::ValuePayingHit(const Piece & p) const
{
	Piece * n[16];
	int w[32];
	int v;
	int t,f,i,k;

	// Value how worth it is to hit piece p.

	for (i=0; i<16; i++) n[i]=p.N[i];
	t=(~p.Type)&(TF_White|TF_Black);
	k=0;
	w[k]=p.Value;
	goto L_Loop;

L_Hit:
	k++;
	w[k]=n[i]->Value;
	if (i&1) n[i]=NULL; else n[i]=n[i]->N[i];
	t^=TF_White|TF_Black;

L_Loop:
	if (t==TF_White) {
		if (n[6] && n[6]->Type==(TF_Pawn|TF_White) &&
		    n[6]->Y==p.Y+1) { i=6; goto L_Hit; }
		if (n[2] && n[2]->Type==(TF_Pawn|TF_White) &&
		    n[2]->Y==p.Y+1) { i=2; goto L_Hit; }
	}
	else {
		if (n[10] && n[10]->Type==(TF_Pawn|TF_Black) &&
		    n[10]->Y==p.Y-1) { i=10; goto L_Hit; }
		if (n[14] && n[14]->Type==(TF_Pawn|TF_Black) &&
		    n[14]->Y==p.Y-1) { i=14; goto L_Hit; }
	}

	f=t|TF_Knight;
	if (n[ 1] && n[ 1]->Type==f) { i= 1; goto L_Hit; }
	if (n[ 3] && n[ 3]->Type==f) { i= 3; goto L_Hit; }
	if (n[ 5] && n[ 5]->Type==f) { i= 5; goto L_Hit; }
	if (n[ 7] && n[ 7]->Type==f) { i= 7; goto L_Hit; }
	if (n[ 9] && n[ 9]->Type==f) { i= 9; goto L_Hit; }
	if (n[11] && n[11]->Type==f) { i=11; goto L_Hit; }
	if (n[13] && n[13]->Type==f) { i=13; goto L_Hit; }
	if (n[15] && n[15]->Type==f) { i=15; goto L_Hit; }

	f=t|TF_Bishop;
	if (n[ 2] && n[ 2]->Type==f) { i= 2; goto L_Hit; }
	if (n[ 6] && n[ 6]->Type==f) { i= 6; goto L_Hit; }
	if (n[10] && n[10]->Type==f) { i=10; goto L_Hit; }
	if (n[14] && n[14]->Type==f) { i=14; goto L_Hit; }

	f=t|TF_Rook;
	if (n[ 0] && n[ 0]->Type==f) { i= 0; goto L_Hit; }
	if (n[ 4] && n[ 4]->Type==f) { i= 4; goto L_Hit; }
	if (n[ 8] && n[ 8]->Type==f) { i= 8; goto L_Hit; }
	if (n[12] && n[12]->Type==f) { i=12; goto L_Hit; }

	f=t|TF_Queen;
	if (n[ 0] && n[ 0]->Type==f) { i= 0; goto L_Hit; }
	if (n[ 2] && n[ 2]->Type==f) { i= 2; goto L_Hit; }
	if (n[ 4] && n[ 4]->Type==f) { i= 4; goto L_Hit; }
	if (n[ 6] && n[ 6]->Type==f) { i= 6; goto L_Hit; }
	if (n[ 8] && n[ 8]->Type==f) { i= 8; goto L_Hit; }
	if (n[10] && n[10]->Type==f) { i=10; goto L_Hit; }
	if (n[12] && n[12]->Type==f) { i=12; goto L_Hit; }
	if (n[14] && n[14]->Type==f) { i=14; goto L_Hit; }

	f=t|TF_King;
	if (n[ 0] && n[ 0]->Type==f && n[ 0]->X==p.X+1) { i= 0; goto L_Hit; }
	if (n[ 2] && n[ 2]->Type==f && n[ 2]->X==p.X+1) { i= 2; goto L_Hit; }
	if (n[ 4] && n[ 4]->Type==f && n[ 4]->Y==p.Y+1) { i= 4; goto L_Hit; }
	if (n[ 6] && n[ 6]->Type==f && n[ 6]->Y==p.Y+1) { i= 6; goto L_Hit; }
	if (n[ 8] && n[ 8]->Type==f && n[ 8]->X==p.X-1) { i= 8; goto L_Hit; }
	if (n[10] && n[10]->Type==f && n[10]->X==p.X-1) { i=10; goto L_Hit; }
	if (n[12] && n[12]->Type==f && n[12]->Y==p.Y-1) { i=12; goto L_Hit; }
	if (n[14] && n[14]->Type==f && n[14]->Y==p.Y-1) { i=14; goto L_Hit; }

	v=0;
	while (k>0) {
		k--;
		v=w[k]-v;
		if (v<0) v=0;
	}
	return -v * ValFac[(p.Type&Turn) ? VFI_PayingTurnOppo : VFI_PayingTurn];
}


int SilChessMachine::ValueThreats(const Piece & p) const
{
	const Piece * const * n;
	int v,type;

	// Value how many pieces are threatened or covered by piece p.

	type=p.Type; n=p.N;
	v=0;
	if (type&TF_Pawn) {
		if (type&TF_Black) {
			if (n[2] && n[2]->Y==p.Y+1) v++;
			if (n[6] && n[6]->Y==p.Y+1) v++;
		}
		else {
			if (n[10] && n[10]->Y==p.Y-1) v++;
			if (n[14] && n[14]->Y==p.Y-1) v++;
		}
	}
	else if (type&(TF_Bishop|TF_Rook|TF_Queen)) {
		if (type&(TF_Rook|TF_Queen)) {
			if (n[ 0]) v++;
			if (n[ 4]) v++;
			if (n[ 8]) v++;
			if (n[12]) v++;
		}
		if (type&(TF_Bishop|TF_Queen)) {
			if (n[ 2]) v++;
			if (n[ 6]) v++;
			if (n[10]) v++;
			if (n[14]) v++;
		}
	}
	else if (type&TF_Knight) {
		if (n[ 1]) v++;
		if (n[ 3]) v++;
		if (n[ 5]) v++;
		if (n[ 7]) v++;
		if (n[ 9]) v++;
		if (n[11]) v++;
		if (n[13]) v++;
		if (n[15]) v++;
	}
	else if (type&TF_King) {
		if (n[ 0] && n[ 0]->X==p.X+1) v++;
		if (n[ 2] && n[ 2]->X==p.X+1) v++;
		if (n[ 4] && n[ 4]->Y==p.Y+1) v++;
		if (n[ 6] && n[ 6]->Y==p.Y+1) v++;
		if (n[ 8] && n[ 8]->X==p.X-1) v++;
		if (n[10] && n[10]->X==p.X-1) v++;
		if (n[12] && n[12]->Y==p.Y-1) v++;
		if (n[14] && n[14]->Y==p.Y-1) v++;
	}

	return v * ValFac[VFI_Threats];
}


int SilChessMachine::ValueMobility(const Piece & p) const
{
	const Piece * const * n;
	int v,x,y,type;

	// Value the mobility of piece p.

	type=p.Type; x=p.X; y=p.Y; n=p.N; v=0;
	if (type&TF_Pawn) {
		if (type&TF_Black) {
			if (n[4]) v=n[4]->Y-y-1; else v=7-y;
			if (y==1) { if (v>2) v=2; } else { if (v>1) v=1; }
		}
		else {
			if (n[12]) v=y-n[12]->Y-1; else v=y;
			if (y==6) { if (v>2) v=2; } else { if (v>1) v=1; }
		}
	}
	else if (type&(TF_Bishop|TF_Rook|TF_Queen)) {
		if (type&(TF_Rook|TF_Queen)) {
			if (n[ 0]) v+=n[ 0]->X-x-1; else v+=7-x;
			if (n[ 4]) v+=n[ 4]->Y-y-1; else v+=7-y;
			if (n[ 8]) v+=x-n[ 8]->X-1; else v+=x;
			if (n[12]) v+=y-n[12]->Y-1; else v+=y;
		}
		if (type&(TF_Bishop|TF_Queen)) {
			if (n[ 2]) v+=n[ 2]->X-x-1; else if (x>=y  ) v+=7-x; else v+=7-y;
			if (n[ 6]) v+=x-n[ 6]->X-1; else if (x<=7-y) v+=x  ; else v+=7-y;
			if (n[10]) v+=x-n[10]->X-1; else if (x<=y  ) v+=x  ; else v+=y  ;
			if (n[14]) v+=n[14]->X-x-1; else if (7-x<=y) v+=7-x; else v+=y  ;
		}
	}
	else if (type&TF_Knight) {
		if (x>0) {
			if (y<6 && !n[5]) v++;
			if (y>1 && !n[11]) v++;
			if (x>1) {
				if (y<7 && !n[7]) v++;
				if (y>0 && !n[9]) v++;
			}
		}
		if (x<7) {
			if (y<6 && !n[3]) v++;
			if (y>1 && !n[13]) v++;
			if (x<6) {
				if (y<7 && !n[1]) v++;
				if (y>0 && !n[15]) v++;
			}
		}
	}
	return v * ValFac[VFI_Mobility];
}


int SilChessMachine::ValueTies(const Piece & p) const
{
	const Piece * const * n;
	int v;
	int t;

	// Value ties of the enemy by piece p.

	if (!(p.Type&(TF_Bishop|TF_Rook|TF_Queen))) return 0;
	v=0;
	n=p.N;
	t=p.Type&(TF_White|TF_Black);
	if (p.Type&(TF_Rook|TF_Queen)) {
		if (n[0] && n[0]->N[0] && !(n[0]->Type&t) && !(n[0]->N[0]->Type&t))
			v+=n[0]->Value+n[0]->N[0]->Value;
		if (n[4] && n[4]->N[4] && !(n[4]->Type&t) && !(n[4]->N[4]->Type&t))
			v+=n[4]->Value+n[4]->N[4]->Value;
		if (n[8] && n[8]->N[8] && !(n[8]->Type&t) && !(n[8]->N[8]->Type&t))
			v+=n[8]->Value+n[8]->N[8]->Value;
		if (n[12] && n[12]->N[12] && !(n[12]->Type&t) && !(n[12]->N[12]->Type&t))
			v+=n[12]->Value+n[12]->N[12]->Value;
	}
	if (p.Type&(TF_Bishop|TF_Queen)) {
		if (n[2] && n[2]->N[2] && !(n[2]->Type&t) && !(n[2]->N[2]->Type&t))
			v+=n[2]->Value+n[2]->N[2]->Value;
		if (n[6] && n[6]->N[6] && !(n[6]->Type&t) && !(n[6]->N[6]->Type&t))
			v+=n[6]->Value+n[6]->N[6]->Value;
		if (n[10] && n[10]->N[10] && !(n[10]->Type&t) && !(n[10]->N[10]->Type&t))
			v+=n[10]->Value+n[10]->N[10]->Value;
		if (n[14] && n[14]->N[14] && !(n[14]->Type&t) && !(n[14]->N[14]->Type&t))
			v+=n[14]->Value+n[14]->N[14]->Value;
	}

	return (v * ValFac[VFI_Ties])/2;
}


int SilChessMachine::ValueCenter(const Piece & p) const
{
	int x,y;

	// Value how centered piece p is on the board.

	x=p.X; if (x>3) x=7-x;
	y=p.Y; if (y>3) y=7-y;
	if (x>y) x=y;
	return x * ValFac[VFI_Center];
}


int SilChessMachine::ValueKing(const Piece & p) const
{
	const Piece * const * n;
	int v;
	int t,X,Y,i,j;

	// Value the king: Is he well guarded, centered, mobile and not in check?

	if (!(p.Type&TF_King)) return 0;

	v=0; n=p.N; X=p.X; Y=p.Y;
	t=(~p.Type)&(TF_White|TF_Black);

	i=0;
	if (n[ 0]) { if (n[ 0]->Type&t) i+=7; else i+=n[ 0]->X-X-1; }
	else i+=7-X;
	if (n[ 4]) { if (n[ 4]->Type&t) i+=7; else i+=n[ 4]->Y-Y-1; }
	else i+=7-Y;
	if (n[ 8]) { if (n[ 8]->Type&t) i+=7; else i+=X-n[ 8]->X-1; }
	else i+=X;
	if (n[12]) { if (n[12]->Type&t) i+=7; else i+=Y-n[12]->Y-1; }
	else i+=Y;
	if (n[ 2]) { if (n[ 2]->Type&t) i+=7; else i+=n[ 2]->X-X-1; }
	else if (X>=Y  ) i+=7-X; else i+=7-Y;
	if (n[ 6]) { if (n[ 6]->Type&t) i+=7; else i+=X-n[ 6]->X-1; }
	else if (X<=7-Y) i+=X  ; else i+=7-Y;
	if (n[10]) { if (n[10]->Type&t) i+=7; else i+=X-n[10]->X-1; }
	else if (X<=Y  ) i+=X  ; else i+=Y  ;
	if (n[14]) { if (n[14]->Type&t) i+=7; else i+=n[14]->X-X-1; }
	else if (7-X<=Y) i+=7-X; else i+=Y  ;
	v-=i*ValFac[VFI_KingCover];

	i=0;
	if (X<7) {
		if (!n[0] || n[0]->X>X+1) i++;
		if (Y<7 && (!n[2] || n[2]->X>X+1)) i++;
	}
	if (Y<7) {
		if (!n[4] || n[4]->Y>Y+1) i++;
		if (X>0 && (!n[6] || n[6]->Y>Y+1)) i++;
	}
	if (X>0) {
		if (!n[8] || n[8]->X<X-1) i++;
		if (Y>0 && (!n[10] || n[10]->X<X-1)) i++;
	}
	if (Y>0) {
		if (!n[12] || n[12]->Y<Y-1) i++;
		if (X<7 && (!n[14] || n[14]->Y<Y-1)) i++;
	}
	v+=i*ValFac[VFI_KingMobility];

	i=X; if (i>3) i=7-i;
	j=Y; if (j>3) j=7-j;
	if (i>j) i=j;
	v-=i*ValFac[VFI_KingNotCentered];

	if (IsThreatened(p.X,p.Y,t)) v-=ValFac[VFI_KingCheck];

	return v;
}


int SilChessMachine::ValuePawn(const Piece & p) const
{
	const Piece * const * n;
	int v;
	int t;

	// Value the pawn: Are there no gabs between the pawns? And will
	// he become a queen soon?

	t=p.Type;
	if (!(t&TF_Pawn)) return 0;
	v=0;
	n=p.N;
	if (
		(n[ 0] && n[ 0]->Type==t && n[ 0]->X==p.X+1) ||
		(n[ 2] && n[ 2]->Type==t && n[ 2]->X==p.X+1) ||
		(n[14] && n[14]->Type==t && n[14]->X==p.X+1) ||
		(n[13] && n[13]->Type==t                   ) ||
		(n[ 3] && n[ 3]->Type==t                   )
	) v+=ValFac[VFI_PawnBeside];
	if (t&TF_White) {
		v+=(7-p.Y)*ValFac[VFI_PawnOnward];
		if (p.Y==1) {
			v+=ValFac[VFI_PawnHeaven];
			if (!n[12]) v+=ValFac[VFI_PawnHeaven];
		}
	}
	else {
		v+=p.Y*ValFac[VFI_PawnOnward];
		if (p.Y==6) {
			v+=ValFac[VFI_PawnHeaven];
			if (!n[4]) v+=ValFac[VFI_PawnHeaven];
		}
	}
	return v;
}


bool SilChessMachine::IsAnyLegalMove() const
{
	Move m[512];
	int mc,i;
	bool check;
	SilChessMachine * machine;

	mc=EnumeratePossibleMoves(m);
	machine=(SilChessMachine*)this;
	for (i=0; i<mc; i++) {
		machine->TBStart();
		machine->TBDoMove(m[i]);
		check=machine->IsCheck(true);
		machine->TakeBack();
		if (!check) return true;
	}
	return false;
}


bool SilChessMachine::IsCheck(bool invertTurn) const
{
	int i,side,king;

	if (!invertTurn) {
		king=TF_King|Turn;
		side=Turn^(TF_White|TF_Black);
	}
	else {
		king=TF_King|(Turn^(TF_White|TF_Black));
		side=Turn;
	}
	for (i=0; i<32; i++) {
		if (Pieces[i].Type==king) {
			return IsThreatened(Pieces[i].X,Pieces[i].Y,side);
		}
	}
	return false;
}


int SilChessMachine::EnumeratePossibleMoves(Move * buf) const
{
	const Piece * p, * pend;
	Move * m;
	int x1,y1,x2,y2,x,y;

#	define SCMEPM_ADD_MOVE(x1,y1,x2,y2) \
	( \
		m->X1=(signed char)(x1), \
		m->Y1=(signed char)(y1), \
		m->X2=(signed char)(x2), \
		m->Y2=(signed char)(y2), \
		m++ \
	)

	m=buf;
	for (p=Pieces, pend=Pieces+32; p<pend; p++) if (p->Type&Turn) {
		x1=p->X;
		y1=p->Y;
		if (p->Type&TF_Pawn) {
			if (Turn==TF_Black) {
				y2=7; if (p->N[4]) y2=p->N[4]->Y-1;
				if (y1<y2) {
					SCMEPM_ADD_MOVE(x1,y1,x1,y1+1);
				}
				if (y1==1 && y1+2<=y2) {
					SCMEPM_ADD_MOVE(x1,y1,x1,y1+2);
				}
				if ((p->N[2] && p->N[2]->X==x1+1 && (p->N[2]->Type&TF_White)) ||
				    (y1==4 && p->N[0] && p->N[0]->X==x1+1 &&
					  p->N[0]->Type==(TF_Pawn|TF_White) &&
					  MoveCount>0 && Moves[MoveCount-1].Y1==6 &&
					  Moves[MoveCount-1].Y2==4 && Moves[MoveCount-1].X2==x1+1)) {
					SCMEPM_ADD_MOVE(x1,y1,x1+1,y1+1);
				}
				if ((p->N[6] && p->N[6]->X==x1-1 && (p->N[6]->Type&TF_White)) ||
				    (y1==4 && p->N[8] && p->N[8]->X==x1-1 &&
					  p->N[8]->Type==(TF_Pawn|TF_White) &&
					  MoveCount>0 && Moves[MoveCount-1].Y1==6 &&
					  Moves[MoveCount-1].Y2==4 && Moves[MoveCount-1].X2==x1-1)) {
					SCMEPM_ADD_MOVE(x1,y1,x1-1,y1+1);
				}
			}
			else {
				y2=0; if (p->N[12]) y2=p->N[12]->Y+1;
				if (y1>y2) {
					SCMEPM_ADD_MOVE(x1,y1,x1,y1-1);
				}
				if (y1==6 && y1-2>=y2) {
					SCMEPM_ADD_MOVE(x1,y1,x1,y1-2);
				}
				if ((p->N[14] && p->N[14]->X==x1+1 && (p->N[14]->Type&TF_Black)) ||
				    (y1==3 && p->N[0] && p->N[0]->X==x1+1 &&
					  p->N[0]->Type==(TF_Pawn|TF_Black) &&
					  MoveCount>0 && Moves[MoveCount-1].Y1==1 &&
					  Moves[MoveCount-1].Y2==3 && Moves[MoveCount-1].X2==x1+1)) {
					SCMEPM_ADD_MOVE(x1,y1,x1+1,y1-1);
				}
				if ((p->N[10] && p->N[10]->X==x1-1 && (p->N[10]->Type&TF_Black)) ||
				    (y1==3 && p->N[8] && p->N[8]->X==x1-1 &&
					  p->N[8]->Type==(TF_Pawn|TF_Black) &&
					  MoveCount>0 && Moves[MoveCount-1].Y1==1 &&
					  Moves[MoveCount-1].Y2==3 && Moves[MoveCount-1].X2==x1-1)) {
					SCMEPM_ADD_MOVE(x1,y1,x1-1,y1-1);
				}
			}
		}
		else if (p->Type&(TF_Bishop|TF_Rook|TF_Queen)) {
			if (p->Type&(TF_Rook|TF_Queen)) {
				x2=7; if (p->N[ 0]) { x2=p->N[ 0]->X; if (p->N[ 0]->Type&Turn) x2--; }
				for (x=x1+1; x<=x2; x++) SCMEPM_ADD_MOVE(x1,y1,x,y1);
				y2=7; if (p->N[ 4]) { y2=p->N[ 4]->Y; if (p->N[ 4]->Type&Turn) y2--; }
				for (y=y1+1; y<=y2; y++) SCMEPM_ADD_MOVE(x1,y1,x1,y);
				x2=0; if (p->N[ 8]) { x2=p->N[ 8]->X; if (p->N[ 8]->Type&Turn) x2++; }
				for (x=x1-1; x>=x2; x--) SCMEPM_ADD_MOVE(x1,y1,x,y1);
				y2=0; if (p->N[12]) { y2=p->N[12]->Y; if (p->N[12]->Type&Turn) y2++; }
				for (y=y1-1; y>=y2; y--) SCMEPM_ADD_MOVE(x1,y1,x1,y);
			}
			if (p->Type&(TF_Bishop|TF_Queen)) {
				x2=7; if (p->N[ 2]) { x2=p->N[ 2]->X; if (p->N[ 2]->Type&Turn) x2--; }
				if (x2>x1+7-y1) x2=x1+7-y1;
				for (x=x1+1; x<=x2; x++) SCMEPM_ADD_MOVE(x1,y1,x,y1+x-x1);
				x2=0; if (p->N[ 6]) { x2=p->N[ 6]->X; if (p->N[ 6]->Type&Turn) x2++; }
				if (x2<x1+y1-7) x2=x1+y1-7;
				for (x=x1-1; x>=x2; x--) SCMEPM_ADD_MOVE(x1,y1,x,y1+x1-x);
				x2=0; if (p->N[10]) { x2=p->N[10]->X; if (p->N[10]->Type&Turn) x2++; }
				if (x2<x1-y1) x2=x1-y1;
				for (x=x1-1; x>=x2; x--) SCMEPM_ADD_MOVE(x1,y1,x,y1+x-x1);
				x2=7; if (p->N[14]) { x2=p->N[14]->X; if (p->N[14]->Type&Turn) x2--; }
				if (x2>x1+y1) x2=x1+y1;
				for (x=x1+1; x<=x2; x++) SCMEPM_ADD_MOVE(x1,y1,x,y1+x1-x);
			}
		}
		else if (p->Type&TF_Knight) {
			if (x1>0) {
				if (y1<6 && (!p->N[5] || !(p->N[5]->Type&Turn))) {
					SCMEPM_ADD_MOVE(x1,y1,x1-1,y1+2);
				}
				if (y1>1 && (!p->N[11] || !(p->N[11]->Type&Turn))) {
					SCMEPM_ADD_MOVE(x1,y1,x1-1,y1-2);
				}
				if (x1>1) {
					if (y1<7 && (!p->N[7] || !(p->N[7]->Type&Turn))) {
						SCMEPM_ADD_MOVE(x1,y1,x1-2,y1+1);
					}
					if (y1>0 && (!p->N[9] || !(p->N[9]->Type&Turn))) {
						SCMEPM_ADD_MOVE(x1,y1,x1-2,y1-1);
					}
				}
			}
			if (x1<7) {
				if (y1<6 && (!p->N[3] || !(p->N[3]->Type&Turn))) {
					SCMEPM_ADD_MOVE(x1,y1,x1+1,y1+2);
				}
				if (y1>1 && (!p->N[13] || !(p->N[13]->Type&Turn))) {
					SCMEPM_ADD_MOVE(x1,y1,x1+1,y1-2);
				}
				if (x1<6) {
					if (y1<7 && (!p->N[1] || !(p->N[1]->Type&Turn))) {
						SCMEPM_ADD_MOVE(x1,y1,x1+2,y1+1);
					}
					if (y1>0 && (!p->N[15] || !(p->N[15]->Type&Turn))) {
						SCMEPM_ADD_MOVE(x1,y1,x1+2,y1-1);
					}
				}
			}
		}
		else if (p->Type&TF_King) {
			if (x1<7) {
				if (!p->N[0] || p->N[0]->X>x1+1 || !(p->N[0]->Type&Turn)) {
					SCMEPM_ADD_MOVE(x1,y1,x1+1,y1);
				}
				if (y1<7 && (!p->N[2] || p->N[2]->X>x1+1 || !(p->N[2]->Type&Turn))) {
					SCMEPM_ADD_MOVE(x1,y1,x1+1,y1+1);
				}
			}
			if (y1<7) {
				if (!p->N[4] || p->N[4]->Y>y1+1 || !(p->N[4]->Type&Turn)) {
					SCMEPM_ADD_MOVE(x1,y1,x1,y1+1);
				}
				if (x1>0 && (!p->N[6] || p->N[6]->Y>y1+1 || !(p->N[6]->Type&Turn))) {
					SCMEPM_ADD_MOVE(x1,y1,x1-1,y1+1);
				}
			}
			if (x1>0) {
				if (!p->N[8] || p->N[8]->X<x1-1 || !(p->N[8]->Type&Turn)) {
					SCMEPM_ADD_MOVE(x1,y1,x1-1,y1);
				}
				if (y1>0 && (!p->N[10] || p->N[10]->X<x1-1 || !(p->N[10]->Type&Turn))) {
					SCMEPM_ADD_MOVE(x1,y1,x1-1,y1-1);
				}
			}
			if (y1>0) {
				if (!p->N[12] || p->N[12]->Y<y1-1 || !(p->N[12]->Type&Turn)) {
					SCMEPM_ADD_MOVE(x1,y1,x1,y1-1);
				}
				if (x1<7 && (!p->N[14] || p->N[14]->Y<y1-1 || !(p->N[14]->Type&Turn))) {
					SCMEPM_ADD_MOVE(x1,y1,x1+1,y1-1);
				}
			}
			if ((p->State&SF_CanCastle) && x1==4) {
				if (p->N[0] && (p->N[0]->State&SF_CanCastle) &&
				    !IsThreatened(x1,y1,Turn^(TF_White|TF_Black)) &&
				    !IsThreatened(x1+1,y1,Turn^(TF_White|TF_Black))) {
					SCMEPM_ADD_MOVE(x1,y1,x1+2,y1);
				}
				if (p->N[8] && (p->N[8]->State&SF_CanCastle) &&
				    !IsThreatened(x1,y1,Turn^(TF_White|TF_Black)) &&
				    !IsThreatened(x1-1,y1,Turn^(TF_White|TF_Black))) {
					SCMEPM_ADD_MOVE(x1,y1,x1-2,y1);
				}
			}
		}
	}
	return m-buf;
}


bool SilChessMachine::IsThreatened(int x, int y, int tside) const
{
	Piece * nbuf[16];
	Piece * const * n;
	const Piece * p;

	p=Board[y*8+x];
	if (p) n=p->N; else { CalcNeighbours(x,y,nbuf); n=nbuf; }
	p=n[1];
	if (p && p->Type==(TF_Knight|tside)) goto L_Threatened;
	p=n[3];
	if (p && p->Type==(TF_Knight|tside)) goto L_Threatened;
	p=n[5];
	if (p && p->Type==(TF_Knight|tside)) goto L_Threatened;
	p=n[7];
	if (p && p->Type==(TF_Knight|tside)) goto L_Threatened;
	p=n[9];
	if (p && p->Type==(TF_Knight|tside)) goto L_Threatened;
	p=n[11];
	if (p && p->Type==(TF_Knight|tside)) goto L_Threatened;
	p=n[13];
	if (p && p->Type==(TF_Knight|tside)) goto L_Threatened;
	p=n[15];
	if (p && p->Type==(TF_Knight|tside)) goto L_Threatened;
	p=n[0];
	if (p && (p->Type&tside)) {
		if (p->Type&(TF_Queen|TF_Rook)) goto L_Threatened;
		if ((p->Type&TF_King) && p->X==x+1) goto L_Threatened;
	}
	p=n[4];
	if (p && (p->Type&tside)) {
		if (p->Type&(TF_Queen|TF_Rook)) goto L_Threatened;
		if ((p->Type&TF_King) && p->Y==y+1) goto L_Threatened;
	}
	p=n[8];
	if (p && (p->Type&tside)) {
		if (p->Type&(TF_Queen|TF_Rook)) goto L_Threatened;
		if ((p->Type&TF_King) && p->X==x-1) goto L_Threatened;
	}
	p=n[12];
	if (p && (p->Type&tside)) {
		if (p->Type&(TF_Queen|TF_Rook)) goto L_Threatened;
		if ((p->Type&TF_King) && p->Y==y-1) goto L_Threatened;
	}
	p=n[2];
	if (p && (p->Type&tside)) {
		if (p->Type&(TF_Queen|TF_Bishop)) goto L_Threatened;
		if (p->X==x+1) {
			if (p->Type&TF_King) goto L_Threatened;
			if (p->Type==(TF_Pawn|TF_White)) goto L_Threatened;
		}
	}
	p=n[6];
	if (p && (p->Type&tside)) {
		if (p->Type&(TF_Queen|TF_Bishop)) goto L_Threatened;
		if (p->X==x-1) {
			if (p->Type&TF_King) goto L_Threatened;
			if (p->Type==(TF_Pawn|TF_White)) goto L_Threatened;
		}
	}
	p=n[10];
	if (p && (p->Type&tside)) {
		if (p->Type&(TF_Queen|TF_Bishop)) goto L_Threatened;
		if (p->X==x-1) {
			if (p->Type&TF_King) goto L_Threatened;
			if (p->Type==(TF_Pawn|TF_Black)) goto L_Threatened;
		}
	}
	p=n[14];
	if (p && (p->Type&tside)) {
		if (p->Type&(TF_Queen|TF_Bishop)) goto L_Threatened;
		if (p->X==x+1) {
			if (p->Type&TF_King) goto L_Threatened;
			if (p->Type==(TF_Pawn|TF_Black)) goto L_Threatened;
		}
	}
	return false;
L_Threatened:
	return true;
}


void SilChessMachine::TBDoMove(const Move & m)
{
	Piece * s, * t;
	int x1,y1,x2,y2;

	CachedInfoValid=false;

	Moves[MoveCount]=m;
	TBSetInt(MoveCount,MoveCount+1);
	TBSetInt(Turn,Turn^(TF_White|TF_Black));

	x1=m.X1;
	y1=m.Y1;
	x2=m.X2;
	y2=m.Y2;

	s=Board[y1*8+x1];
	t=Board[y2*8+x2];

	TBUnlinkPiece(*s);
	if (x1!=x2) TBSetInt(s->X,x2);
	if (y1!=y2) TBSetInt(s->Y,y2);
	if (s->State&SF_CanCastle) TBSetInt(s->State,s->State&~SF_CanCastle);
	if (s->Type&TF_Pawn) {
		if (y2==0 || y2==7) {
			TBSetInt(s->Type,s->Type^(TF_Queen|TF_Pawn));
			TBSetInt(s->Value,QueenValue);
		}
		if (x1!=x2 && !t) t=Board[y1*8+x2];
	}
	else if (s->Type&TF_King) {
		if (x2-x1>1 || x2-x1<-1) {
			if (x2>x1) t=Board[y1*8+7]; else t=Board[y1*8+0];
			TBUnlinkPiece(*t);
			TBSetInt(t->X,(x2+x1)/2);
			TBSetInt(t->State,t->State&~SF_CanCastle);
			TBLinkPiece(*t);
			t=NULL;
		}
	}
	if (t) {
		TBUnlinkPiece(*t);
		TBSetInt(t->Type,0);
	}
	TBLinkPiece(*s);
}


void SilChessMachine::TBLinkPiece(Piece & p)
{
	Piece * n[16];
	Piece * q;
	int i;

	CalcNeighbours(p.X,p.Y,n);
	TBSetPtr(Board[p.Y*8+p.X],&p);
	for (i=0; i<16; i++) {
		q=n[i];
		if (q) TBSetPtr(q->N[(i+8)&15],&p);
		if (p.N[i]!=q) TBSetPtr(p.N[i],q);
	}
}


void SilChessMachine::TBUnlinkPiece(Piece & p)
{
	TBSetPtr(Board[p.Y*8+p.X],NULL);
	if (p.N[ 0]) TBSetPtr(p.N[ 0]->N[ 8],p.N[ 8]);
	if (p.N[ 1]) TBSetPtr(p.N[ 1]->N[ 9],NULL);
	if (p.N[ 2]) TBSetPtr(p.N[ 2]->N[10],p.N[10]);
	if (p.N[ 3]) TBSetPtr(p.N[ 3]->N[11],NULL);
	if (p.N[ 4]) TBSetPtr(p.N[ 4]->N[12],p.N[12]);
	if (p.N[ 5]) TBSetPtr(p.N[ 5]->N[13],NULL);
	if (p.N[ 6]) TBSetPtr(p.N[ 6]->N[14],p.N[14]);
	if (p.N[ 7]) TBSetPtr(p.N[ 7]->N[15],NULL);
	if (p.N[ 8]) TBSetPtr(p.N[ 8]->N[ 0],p.N[ 0]);
	if (p.N[ 9]) TBSetPtr(p.N[ 9]->N[ 1],NULL);
	if (p.N[10]) TBSetPtr(p.N[10]->N[ 2],p.N[ 2]);
	if (p.N[11]) TBSetPtr(p.N[11]->N[ 3],NULL);
	if (p.N[12]) TBSetPtr(p.N[12]->N[ 4],p.N[ 4]);
	if (p.N[13]) TBSetPtr(p.N[13]->N[ 5],NULL);
	if (p.N[14]) TBSetPtr(p.N[14]->N[ 6],p.N[ 6]);
	if (p.N[15]) TBSetPtr(p.N[15]->N[ 7],NULL);
}


void SilChessMachine::TakeBack()
{
	TBIntEntry * it;
	TBPtrEntry * pt;

	CachedInfoValid=false;

	for (it=TBIntTop-1; it->Ptr; it--) *(it->Ptr)=it->Val;
	TBIntTop=it;

	for (pt=TBPtrTop-1; pt->Ptr; pt--) *(pt->Ptr)=pt->Val;
	TBPtrTop=pt;
}


void SilChessMachine::CalcNeighbours(int x, int y, Piece * * n) const
{
	Piece **p0,**p,**pe;

	for (p=n, pe=n+16; p<pe; p++) *p=NULL;
	p0=(Piece**)(Board+y*8+x);
	for (p=p0+1, pe=p0-x+7; p<=pe; p++) {
		if (*p) { n[0]=*p; break; }
	}
	for (p=p0+9, pe=p0+9*(7-(x>y?x:y)); p<=pe; p+=9) {
		if (*p) { n[2]=*p; break; }
	}
	for (p=p0+8, pe=p0-y*8+7*8; p<=pe; p+=8) {
		if (*p) { n[4]=*p; break; }
	}
	for (p=p0+7, pe=p0+7*(x<(7-y)?x:(7-y)); p<=pe; p+=7) {
		if (*p) { n[6]=*p; break; }
	}
	for (p=p0-1, pe=p0-x  ; p>=pe; p--) {
		if (*p) { n[8]=*p; break; }
	}
	for (p=p0-9, pe=p0-9*(x<y?x:y); p>=pe; p-=9) {
		if (*p) { n[10]=*p; break; }
	}
	for (p=p0-8, pe=p0-y*8;     p>=pe; p-=8) {
		if (*p) { n[12]=*p; break; }
	}
	for (p=p0-7, pe=p0-7*((7-x)<y?(7-x):y); p>=pe; p-=7) {
		if (*p) { n[14]=*p; break; }
	}
	if (x>0) {
		if (y<6) n[5]=p0[2*8-1];
		if (y>1) n[11]=p0[-2*8-1];
		if (x>1) {
			if (y<7) n[7]=p0[1*8-2];
			if (y>0) n[9]=p0[-1*8-2];
		}
	}
	if (x<7) {
		if (y<6) n[3]=p0[2*8+1];
		if (y>1) n[13]=p0[-2*8+1];
		if (x<6) {
			if (y<7) n[1]=p0[1*8+2];
			if (y>0) n[15]=p0[-1*8+2];
		}
	}
}


int SilChessMachine::Random(int min, int max)
{
	static unsigned int seed=0;
	static bool initialized=false;
	unsigned int r,n;

	if (min>=max) return min;
	if (!initialized) {
		seed=(unsigned int)time(NULL);
		initialized=true;
	}
	seed=seed*1664525+1013904223;
	r=seed;
	n=max+1-min;
	if (n<65536) r>>=16;
	return r%n+min;
}


void SilChessMachine::PrintASCII(bool flipped, const char * lastMove) const
{
	const char * icons[13]={
		"....."
		"....."
		".....",
		"....."
		".._.."
		"..O..",
		"....."
		"../>."
		"./O\\.",
		"....."
		"..|.."
		"./O\\.",
		"....."
		".|-|."
		".|O|.",
		"....."
		".\\\"/."
		".]O[.",
		"....."
		".\"\"\"."
		".(O).",
		"....."
		".._.."
		"..#..",
		"....."
		"../>."
		"./#\\.",
		"....."
		"..|.."
		"./#\\.",
		"....."
		".|-|."
		".|#|.",
		"....."
		".\\\"/."
		".]#[.",
		"....."
		".\"\"\"."
		".(#)."
	};
	int x,y,xi,yi,i;

	for (y=0; y<8; y++) {
		for (yi=0; yi<3; yi++) {
			printf("\n");
			if (yi==1) printf("%d",flipped ? y+1 : 8-y);
			else printf(" ");
			for (x=0; x<8; x++) {
				for (xi=0; xi<5; xi++) {
					if (flipped) i=icons[GetField(7-x,7-y)][yi*5+xi];
					else i=icons[GetField(x,y)][yi*5+xi];
					if (!((x+y)&1) && i=='.') i=' ';
					printf("%c",i);
				}
			}
		}
	}
	printf(" %s\n ",lastMove);
	for (x=0; x<8; x++) {
		for (xi=0; xi<5; xi++) {
			if (xi==2) printf("%c", flipped ? 'H'-x : 'A'+x);
			else printf(" ");
		}
	}
}


void SilChessMachine::PrintASCII2(bool flipped, const char * lastMove) const
{
	const char * icons[7]={
		"       "
		"       "
		"       ",
		"       "
		"  (#)  "
		"  )#(  ",
		"       "
		"  /##> "
		" /##\\  ",
		"   O   "
		" (###) "
		"  /#\\  ",
		"       "
		" [###] "
		" |###| ",
		" \\ | / "
		"  >#<  "
		" /###\\ ",
		" | | | "
		" (###) "
		" /###\\ "
	};
	int x,y,xi,yi,i,s;

	for (y=0; y<8; y++) {
		for (yi=0; yi<3; yi++) {
			printf("\n");
			if (yi==1) printf("%d",flipped ? y+1 : 8-y);
			else printf(" ");
			for (x=0; x<8; x++) {
				for (xi=0; xi<7; xi++) {
					if (flipped) s=GetField(7-x,7-y); else s=GetField(x,y);
					i=icons[s>6?s-6:s][yi*7+xi];
					if (i==' ') {
						if (((x+y)&1)) i=':'; else i=' ';
					}
					else if (i=='#') {
						if (s>6) i=' '; else i='#';
					}
					printf("%c",i);
				}
			}
		}
	}
	printf(" %s\n ",lastMove);
	for (x=0; x<8; x++) {
		for (xi=0; xi<7; xi++) {
			if (xi==3) printf("%c",flipped ? 'H'-x : 'A'+x);
			else printf(" ");
		}
	}
}


void SilChessMachine::PrintANSI(bool flipped, const char * lastMove) const
{
	const char * icons[7]={
		"       "
		"       "
		"       ",
		"   _   "
		"  (\")  "
		"  |#|  ",
		"  /o\\  "
		" /#\\#> "
		" \\##\\  ",
		"   O   "
		"  (#)  "
		"  /#\\  ",
		" [###] "
		"  |#|  "
		" /###\\ ",
		" \\\\|// "
		"  )#(  "
		" /###\\ ",
		" |%%%| "
		" (###) "
		" /###\\ "
	};
	int x,y,i,xi,yi,s;

	printf("\n\033[31m\033[43m");
	printf("  ");
	for (x=0; x<8; x++) {
		for (xi=0; xi<7; xi++) {
			if (xi==3) printf("%c",flipped ? 'h'-x : 'a'+x);
			else printf(" ");
		}
	}
	printf("  ");
	printf("\033[m");
	for (y=0; y<8; y++) {
		for (yi=0; yi<3; yi++) {
			printf("\n\033[31m\033[43m");
			if (yi==1) printf("%d ",flipped ? y+1 : 8-y);
			else printf("  ");
			printf("\033[1m");
			for (x=0; x<8; x++) {
				if (flipped) s=GetField(7-x,7-y); else s=GetField(x,y);
				if (((x+y)&1))    printf("\033[42m");
				else              printf("\033[46m");
				if (s>6)          printf("\033[30m");
				else              printf("\033[37m");
				for (xi=0; xi<7; xi++) {
					i=icons[s>6?s-6:s][yi*7+xi];
					printf("%c",i);
				}
			}
			printf("\033[m\033[31m\033[43m");
			if (yi==1) printf(" %d",flipped ? y+1 : 8-y);
			else printf("  ");
			printf("\033[m");
		}
	}
	printf(" %s\n\033[31m\033[43m  ",lastMove);
	for (x=0; x<8; x++) {
		for (xi=0; xi<7; xi++) {
			if (xi==3) printf("%c",flipped ? 'h'-x : 'a'+x);
			else printf(" ");
		}
	}
	printf("  ");
	printf("\033[m");
}


void SilChessMachine::PrintDOS(bool flipped, const char * lastMove) const
{
	const char * icons[7]={
		"       "
		"       "
		"       ",
		"       "
		"  (#)  "
		"  )#(  ",
		"       "
		"  /##> "
		" /##\\  ",
		"   O   "
		" (###) "
		"  /#\\  ",
		"       "
		" [###] "
		" |###| ",
		" \\ | / "
		"  >#<  "
		" /###\\ ",
		" | | | "
		" (###) "
		" /###\\ "
	};
	int x,y,xi,yi,i,s;

	for (y=0; y<8; y++) {
		for (yi=0; yi<3; yi++) {
			printf("\n");
			if (yi==1) printf("%d",flipped ? y+1 : 8-y);
			else printf(" ");
			for (x=0; x<8; x++) {
				for (xi=0; xi<7; xi++) {
					if (flipped) s=GetField(7-x,7-y); else s=GetField(x,y);
					i=icons[s>6?s-6:s][yi*7+xi];
					if (i==' ') {
						if (((x+y)&1)) i=177; else i=176;
					}
					else if (i=='#') {
						if (s>6) i=' '; else i='#';
					}
					printf("%c",i);
				}
			}
		}
	}
	printf(" %s\n ",lastMove);
	for (x=0; x<8; x++) {
		for (xi=0; xi<7; xi++) {
			if (xi==3) printf("%c",flipped ? 'H'-x : 'A'+x);
			else printf(" ");
		}
	}
}


void SilChessMachine::PrintMINI(bool flipped, const char * lastMove) const
{
	const char * icons=".pnbrqkPNBRQK";
	int x,y,i;

	for (y=0; y<8; y++) {
		printf("\n");
		printf("%d",flipped ? y+1 :8-y);
		for (x=0; x<8; x++) {
			if (flipped) i=icons[GetField(7-x,7-y)];
			else i=icons[GetField(x,y)];
			if (!((x+y)&1)) {
				if (i=='.') i=' ';
				printf(" %c",i);
			}
			else {
				printf(".%c",i);
			}
		}
	}
	printf(" %s\n ",lastMove);
	for (x=0; x<8; x++) {
		printf("%c ",flipped ? 'h'-x : 'a'+x);
	}
}
