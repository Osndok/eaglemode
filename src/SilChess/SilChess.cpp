//------------------------------------------------------------------------------
// SilChess.cpp - Command line version of SilChess
//
// Copyright (C) 2000-2001,2007-2008 Oliver Hamann.
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
#include <ctype.h>
#if !defined(__DOS__)
	#include <signal.h>
#endif
#include <SilChess/SilChessMachine.h>

static void PrintUsage();
static int xboard_mode(SilChessMachine * machine);


int main(int argc, char * argv[])
{
	char tmp[256];
	char * p;
	int i,depth1,depth2;
	SilChessMachine * machine;
	SilChessMachine::Move m;
	SilChessMachine::Charset charset;

	printf("SilChess 3.1 - Copyright (C) 2000-2008 Oliver Hamann\n");

	machine=new SilChessMachine();

	if (argc>5) {
		PrintUsage();
		return 1;
	}
	for (i=1; i<argc; i++) for (p=argv[i]; *p!=0; p++) *p=(char)tolower(*p);
	#if defined(__DOS__) || defined(_WIN32)
		charset=SilChessMachine::CS_DOS;
	#else
		charset=SilChessMachine::CS_ANSI;
	#endif
	depth2=-1;
	if (argc>1) {
		if (strcmp(argv[1],"white")==0)  machine->SetHumanWhite(true);
		else if (strcmp(argv[1],"black")==0) machine->SetHumanWhite(false);
		else {
			PrintUsage();
			return 1;
		}
		if (argc>2) {
			machine->SetSearchDepth(atoi(argv[2]));
			if (argc>3) {
				if (strcmp(argv[3],"ascii")==0) charset=SilChessMachine::CS_ASCII;
				else if (strcmp(argv[3],"ascii2")==0) charset=SilChessMachine::CS_ASCII2;
				else if (strcmp(argv[3],"ansi")==0) charset=SilChessMachine::CS_ANSI;
				else if (strcmp(argv[3],"dos")==0) charset=SilChessMachine::CS_DOS;
				else if (strcmp(argv[3],"mini")==0) charset=SilChessMachine::CS_MINI;
				else {
					PrintUsage();
					return 1;
				}
				if (argc>4) depth2=atoi(argv[4]);
			}
		}
	}

	machine->StartNewGame();

	for (;;) {

		if (machine->GetMoveCount()>0) {
			machine->GetMove(machine->GetMoveCount()-1).ToString(tmp);
		}
		else tmp[0]=0;
		sprintf(tmp+strlen(tmp)," %d",machine->GetValue());
		if (machine->IsCheck()) strcat(tmp," check!");
		machine->Print(charset,tmp);

		if (machine->IsMate()) {
			printf(" MATE!\n");
			break;
		}
		else if (machine->IsDraw()) {
			printf(" DRAW!\n");
			break;
		}
		else if (machine->IsEndless()) {
			printf(" ENDLESS!\n");
			break;
		}

		if (machine->IsHumanOn() && depth2<0) {
			printf(" move: ");
			fgets(tmp,sizeof(tmp)-1,stdin);
			tmp[sizeof(tmp)-1]=0;
			for (i=strlen(tmp)-1; i>=0 && (unsigned char)tmp[i]<=32; i--);
			tmp[i+1]=0;
			if (m.FromString(tmp)) {
				if (!machine->IsLegalMove(m)) {
					printf("illegal move! <press enter to continue>");
					fgets(tmp,sizeof(tmp)-1,stdin);
				}
				else machine->DoMove(m);
			}
			else if (strcmp(tmp,"exit")==0 || strcmp(tmp,"quit")==0) {
				break;
			}
			else if (strcmp(tmp,"reset")==0) {
				machine->StartNewGame();
			}
			else if (strncmp(tmp,"load ",5)==0 && (unsigned char)tmp[5]>32) {
				if (!machine->Load(tmp+5)) {
					fprintf(stderr,"can not load %s\n",tmp+5);
					exit(1);
				}
			}
			else if (strncmp(tmp,"save ",5)==0 && (unsigned char)tmp[5]>32) {
				if (!machine->Save(tmp+5)) {
					fprintf(stderr,"can not save %s\n",tmp+5);
					exit(1);
				}
			}
			else if (strcmp(tmp,"undo")==0) {
				machine->UndoMove();
				machine->UndoMove();
			}
			else if (strncmp(tmp,"oppo ",5)==0 && m.FromString(tmp+5)) {
				machine->UndoMove();
				if (!machine->IsLegalMove(m)) {
					printf("illegal move! <press enter to continue>");
					fgets(tmp,sizeof(tmp)-1,stdin);
				}
				else machine->DoMove(m);
			}
			else if (strcmp(tmp,"flip")==0) {
				machine->SetHumanWhite(!machine->IsHumanWhite());
			}
			else if (strncmp(tmp,"hint ",5)==0 && tmp[5]>='0' && tmp[5]<='9') {
				depth1=machine->GetSearchDepth();
				machine->SetSearchDepth(atoi(tmp+5));
				printf("searching hint (%d)...",machine->GetSearchDepth()); fflush(stdout);
				machine->SearchMove(&m);
				machine->SetSearchDepth(depth1);
				m.ToString(tmp);
				printf(" %s <press enter>",tmp);
				printf("\a");
				fgets(tmp,sizeof(tmp)-1,stdin);
			}
			else if (strncmp(tmp,"depth ",6)==0 && tmp[6]>='0' && tmp[6]<='9') {
				machine->SetSearchDepth(atoi(tmp+6));
			}
			else if (strncmp(tmp,"finish ",7)==0 && tmp[7]>='0' && tmp[7]<='9') {
				depth2=atoi(tmp+7);
			}
			else if (strcmp(tmp,"list")==0) {
				for (i=0; i<machine->GetMoveCount(); i++) {
					machine->GetMove(i).ToString(tmp);
					printf("%s",tmp);
					if (i+1==machine->GetMoveCount() || ((i+1)%16)==0) printf("\n");
					else printf(" ");
				}
				printf("<press enter>");
				fgets(tmp,sizeof(tmp)-1,stdin);
			}
			else if (strcmp(tmp,"train")==0) {
				machine->GeneticTraining();
			}
			else if (strcmp(tmp,"xboard")==0) {
				return xboard_mode(machine);
			}
			else {
				printf("possible commands:\n");
				printf(" b1c3        something like this to make a move\n");
				printf(" exit        terminate program\n");
				printf(" reset       start new game\n");
				printf(" load <fn>   load file <fn>\n");
				printf(" save <fn>   save to file <fn>\n");
				printf(" undo        take back move\n");
				printf(" oppo <move> replace opponents move by <move>\n");
				printf(" flip        exchange sides\n");
				printf(" hint <d>    give a hint with depth <d>\n");
				printf(" depth <d>   set computer search depth\n");
				printf(" finish <d>  do the rest with depth <d>\n");
				printf(" list        list all moves\n");
				printf(" train       genetic training (for development)\n");
				printf(" xboard      enter xboard protocol (for xboard pipe)\n");
				printf("<press enter to continue>");
				fgets(tmp,sizeof(tmp)-1,stdin);
			}
		}
		else {
			if (machine->IsHumanOn()) {
				depth1=machine->GetSearchDepth();
				machine->SetSearchDepth(depth2);
				printf(" searching (%d)...",machine->GetSearchDepth()); fflush(stdout);
				i=machine->SearchMove(&m);
				machine->SetSearchDepth(depth1);
			}
			else {
				printf(" searching (%d)...",machine->GetSearchDepth()); fflush(stdout);
				i=machine->SearchMove(&m);
			}
			if (!i) {
				fprintf(stderr,"\nruntime error\n");
				exit(1);
			}
			machine->DoMove(m);
			printf("\a\n");
		}

	}
	return 0;
}


static void PrintUsage()
{
	printf("Usage: silchess [<side> [<depth> [<charset> [<depth2>]]]]\n");
	printf(" <side>    - black or white, the color you want to play.\n");
	printf(" <depth>   - 0, 1, 2, ..., how hard the computer is playing.\n");
	printf(" <charset> - ascii, ascii2, ansi, dos or mini - type of board display.\n");
	printf(" <depth2>  - give this to let the computer play against itself.\n");
}


static int xboard_mode(SilChessMachine * machine)
{
	char command[512],tmp[512];
	SilChessMachine::Move m;
	int i,protocol;
	bool force;

	// This is an experimental implementation of the xboard protocol.
	// Protocol description found at:
	//   http://www.tim-mann.org/xboard/engine-intf.html
	// Example for starting xboard:
	//   xboard -fcp 'bin/silchess white 5'

	#if !defined(__DOS__)
		signal(SIGINT,SIG_IGN);
	#endif
	protocol=1;
	machine->StartNewGame();
	machine->SetHumanWhite(true);
	force=false;
	printf("\n");
	for (;;) {
		if (machine->IsMate()) {
			if (machine->IsWhiteOn()) printf("0-1 {Black mates}\n");
			else                      printf("1-0 {White mates}\n");
		}
		else if (machine->IsDraw()) {
			printf("1/2-1/2 {Draw}\n");
		}
		else if (machine->IsEndless()) {
			printf("1/2-1/2 {Draw}\n");
		}
		else if (!force && machine->IsHumanOn()) {
			if (!machine->SearchMove(&m)) {
				fprintf(stderr,"run time error\n");
				exit(1);
			}
			machine->DoMove(m);
			m.ToString(tmp);
			printf("move %s\n",tmp);
		}
		fflush(stdout);
		fgets(command,sizeof(command)-1,stdin);
		command[sizeof(command)-1]=0;
		for (i=strlen(command)-1; i>=0 && (unsigned char)command[i]<=32; i--);
		command[i+1]=0;
		if (strcmp(command,"new")==0) {
			machine->StartNewGame();
			machine->SetHumanWhite(false);
			force=false;
			continue;
		}
		if (strcmp(command,"quit")==0) return 0;
		if (strcmp(command,"protover 2")==0) {
			protocol=2;
			continue;
		}
		if (strcmp(command,"random")==0) continue;
		if (strcmp(command,"force")==0) {
			force=true;
			continue;
		}
		if (strcmp(command,"go")==0) {
			force=false;
			continue;
		}
		if (strcmp(command,"white")==0) {
			machine->SetHumanWhite(true);
			continue;
		}
		if (strcmp(command,"black")==0) {
			machine->SetHumanWhite(false);
			continue;
		}
		if (strncmp(command,"level ",6)==0) continue;
		if (strncmp(command,"st ",3)==0) continue;
		if (strncmp(command,"sd ",3)==0) {
//			machine->SetSearchDepth(atoi(tmp+3));
			continue;
		}
		if (strncmp(command,"time ",5)==0) continue;
		if (strncmp(command,"otim ",5)==0) continue;
		if (strcmp(command,"draw")==0) continue;
		if (strncmp(command,"result ",7)==0) continue;
		if (strcmp(command,"hint")==0) {
			if (!machine->SearchMove(&m)) {
				fprintf(stderr,"run time error\n");
				exit(1);
			}
			m.ToString(tmp);
			printf("Hint: %s\n",tmp);
			continue;
		}
		if (strcmp(command,"hard")==0) continue;
		if (strcmp(command,"easy")==0) continue;
		if (strcmp(command,"post")==0) continue;
		if (strcmp(command,"nopose")==0) continue;
		if (strncmp(command,"name ",5)==0) continue;
		if (strncmp(command,"rating ",7)==0) continue;
		if (strcmp(command,"computer")==0) continue;
		if (strcmp(command,"?")==0) continue;

		if (strlen(command)==5 && command[4]=='q') command[4]=0;
		if (m.FromString(command)) {
			if (!machine->IsLegalMove(m)) printf("Illegal move: %s\n",tmp);
			else machine->DoMove(m);
			continue;
		}

		printf("unknown command: %s\n",command);
	}
}
