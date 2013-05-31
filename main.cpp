#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

#include "main.h"
#include "Board.h"
#include "BoardPrint.h"
#include "Simulation.h"
#include "SimulationHistory.h"
#include "Player.h"
#include "utils.h"

#pragma comment(linker, "/STACK:16777216")  // enlarge your stack size!

//#define DEBUG_MODE
#define TESTING_MODE

long simulations_done = 0;
long max_simulations = 100000;


bool neighDiff(int grid1[BOARD_FULL_HEIGHT][BOARD_FULL_WIDTH][PLAYERS_NUM], 
			   int grid2[BOARD_FULL_HEIGHT][BOARD_FULL_WIDTH][PLAYERS_NUM])
{
	for(int i=1; i<BOARD_HEIGHT+1; ++i){
		for(int k=1; k<BOARD_WIDTH+1; ++k){
			if(grid1[i][k][0] != grid2[i][k][0] || grid1[i][k][1] != grid2[i][k][1]){
				return true;
			}
		}
	}
	return false;
}


int generate_moves(Board board, Cell player, Coords moves){
	return 0;
}

// rename this function to minimax and make a new nextMove that only calls minimax and prints best move
int nextMove(Cell player, Board* board, int prev_row, int prev_col, Tl* h,int ply, int max_ply){
	int max_score = -10000;
	int prev_score = 0;
	int cur_score = 0;
	int score = 0;
	int i,k;
	int best_i = prev_row;
	int best_k = prev_col;
	bool stop_search = false;

	Coords alive_cells;
	int alive_cells_num = getAliveCells(*board,alive_cells);

	// TODO: make issueOrder function
	// if playing white than first move goes to center to prevent black from mirroring your moves
	if(alive_cells_num == 0){
		best_i = BOARD_FULL_HEIGHT / 2;
		best_k = BOARD_FULL_WIDTH / 2;
#ifdef TESTING_MODE
		(*board)[best_i][best_k] = player;
#else
		printf("%d %d",best_i-1,best_k-1); // -1 because my board starts at 1
#endif
		return score;
	}

	Board board_copy;
	memcpy(&board_copy,board,sizeof(Board));

	Tl* prev_h;
	if(h == NULL){
		//prev_h = (Tl*) malloc(sizeof(Tl)*(max_ply+1));
		prev_h = (Tl*) malloc(sizeof(Tl)*(max_ply+2));
	}
	else{
		prev_h = h+1;
	}

	prev_h->turns = 0;
	for(int i=0; i<TURNS_NUM; ++i){
		prev_h->changes_num[i] = 0;
	}

	// simulate current state and fill history
	if(ply == 0){
	//if(true){
		prev_score = Simulate(&board_copy,player,TURNS_NUM,prev_h);
		//Simulate(&board_copy,player,TURNS_NUM,prev_h);
	}
	else{
		//Simulate_old(&board_copy,player,TURNS_NUM,false);
		prev_score = Simulate(&board_copy,prev_row,prev_col,player,TURNS_NUM,h,prev_h);
		// max_score = Simulate(&board_copy,prev_row,prev_col,player,TURNS_NUM,h,prev_h);
		
		/*Tl* h2 = h+2;
		h2->turns = 0;
		for(int i=0; i<TURNS_NUM; ++i){
			h2->changes_num[i] = 0;
		}
		Board brd;
		memcpy(&brd,board,sizeof(Board));
		int prev_score2 = Simulate(&brd,player,TURNS_NUM,h2);
		
		Board h_brd1; 
		Board h_brd2;
		int h_neigh1[BOARD_FULL_HEIGHT][BOARD_FULL_WIDTH][PLAYERS_NUM];
		int h_neigh2[BOARD_FULL_HEIGHT][BOARD_FULL_WIDTH][PLAYERS_NUM];
		
		memcpy(h_brd1,prev_h->origin,sizeof(Board));
		memcpy(h_brd2,h2->origin,sizeof(Board));
		memcpy(h_neigh1,prev_h->neigh_origin,sizeof(h_neigh1));
		memcpy(h_neigh2,h2->neigh_origin,sizeof(h_neigh2));

		if(boardDiff(h_brd1,h_brd2) || neighDiff(h_neigh1,h_neigh2)){
			printf("\n\n Shit happened (origins):\n\n");
			printBrdAndNeighCnt(h_brd1,h_neigh1,player,stdout);
			printBrdAndNeighCnt(h_brd2,h_neigh2,player,stdout);
			char* s;
			scanf("%s",&s);
		}
		
		for(int t=0; t<TURNS_NUM; ++t){
			getBoardFromHist(prev_h,t,h_brd1);
			getBoardFromHist(h2,t,h_brd2);
			getNeighFromHist(prev_h,t,h_neigh1);
			getNeighFromHist(h2,t,h_neigh2);

			if(boardDiff(h_brd1,h_brd2) || neighDiff(h_neigh1,h_neigh2)){
				clearScreen();
				printf("\n\n Shit happened (turn %d)",t);
				printf("\n\n Origin:");
				printBrdAndNeighCnt(prev_h->origin,prev_h->neigh_origin,player,stdout);
				printf("\n\n Fast simulation:");
				printBrdAndNeighCnt(h_brd1,h_neigh1,player,stdout);
				printf("\n\n Normal simulation:");
				printBrdAndNeighCnt(h_brd2,h_neigh2,player,stdout);
				printf("\n\n Board difference: \n\n");
				printBoardDiff(h_brd1,h_brd2,stdout);
				printf("\n\n Neigh difference: \n\n");
				printNeighDiff(h_neigh1,h_neigh2,stdout);
				printf("\n\n boardToStr: \n\n");
				char* s_brd;
				boardToStr(prev_h->origin,&s_brd);
				printf("%s",s_brd);
				printf("\n\n strToBoard: \n\n");
				Board bb;
				strToBoard(s_brd,bb);
				printBoard(bb,stdout);
				fprintf(stlog,"%s",s_brd);
				printf("Board string saved to the log: %s", log_file);
				char* s;
				scanf("%s",&s);
			}
		}*/
		//free(h2);
	}
	++simulations_done;

	// search for a move
	i = prev_row;
	for(i; i<=BOARD_HEIGHT; i++){
		k = i>prev_row? prev_col : prev_col; // this row start from the prev_col, other rows from col=1
		for(k; k<=BOARD_WIDTH; k++){
			if((abs(i - prev_row > 1) || (abs(k - prev_col > 1))) && ply > 0)
			{
				break;
			}
			if(simulations_done >= max_simulations){
				stop_search = true;
				break;
			}
			if ((*board)[i][k] == dead){
				// make move
				memcpy(&board_copy,board,sizeof(Board));
				board_copy[i][k] = player;

				// process move
				if(ply < max_ply){
					score = nextMove(player,&board_copy,i,k,prev_h,ply+1,max_ply);
				}
				else{	
					score = Simulate(&board_copy,i,k,player,TURNS_NUM,prev_h,NULL);
					++simulations_done;
					/*memcpy(&board_copy,board,sizeof(Board));
					board_copy[i][k] = player;
					int score2 = Simulate_old(&board_copy,player,TURNS_NUM,false);
					if(score != score2){
						printf("\nbug detected: new_sim score: %d    old_sim score: %d", score, score2);
						char* s;
						scanf("%s",&s);
						printf("simulation_old:");
						scanf("%s",&s);
						memcpy(&board_copy,board,sizeof(Board));
						board_copy[i][k] = player;
						Simulate_old(&board_copy,player,TURNS_NUM,true);
						scanf("%s",&s);
						printf("simulation new:");
						scanf("%s",&s);
						memcpy(&board_copy,board,sizeof(Board));
						board_copy[i][k] = player;
						Simulate(&board_copy,i,k,player,TURNS_NUM,prev_h,new_h, true);
						scanf("%s",&s);

					}*/
				}
				//clearScreen();

				// unmake move
				//board[i][k] = dead;

				if(score > max_score){
                	max_score = score;
                    best_i = i;
                    best_k = k;
					/*if(score >= prev_score && ply != 0){
						stop_search = true;
						break;
					}*/
                }
			}
			if(stop_search){
				break;
			}
		}
	}
#ifdef TESTING_MODE
	if(ply == 0){
		(*board)[best_i][best_k] = player;
	}
#else
	if(ply == 0){
		printf("%d %d",best_i-1,best_k-1); // -1 because my board starts at 1
	}
#endif
	if(ply == 0){
		free(prev_h);
	}
	return max_score;
}

int main(){
    int i,k;
    char c;
    Board board;

	//----PRODUCTION CODE-----
#ifndef DEBUG_MODE 
#ifndef TESTING_MODE
    scanf("%c\n", &c);
	player = cell_of_char(c);

	//initBoard(empty_board);
	initBoard(board);

    for(i=1; i<=BOARD_HEIGHT; i++){
		for (k=1; k<=BOARD_WIDTH; k++){
			scanf("%c", &c);
			board[i][k] = cell_of_char(c);
		}
		scanf("\n");
	}

	nextMove(player,&board,1,1,NULL,0,40-count_player_cells(board,player)-1 < 1? 0 : 1);
#endif
#endif

#ifdef DEBUG_MODE
	char* s;
	//s = "-----------------------------wwww-------------------------wwww---------------------------------------------------------------------------------------wwww---------------------------wwwww------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------bbbb---------------------------bbbb---------------------------bbbb---------------------------bbbbb------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------";
    for(i=0; i<BOARD_HEIGHT; i++){
		for (k=0; k<BOARD_WIDTH; k++){
			board[i][k] = cell_of_char(s[i*BOARD_WIDTH + k]);
		}
	}

	printBoard(board);
	scanf("%s",&s);
	printf("\n\n");

	Simulate(board,player_1,500);
	
	scanf("%s",&s);
#endif

#ifdef TESTING_MODE
	const char* log_file = "log.txt";
	FILE* stlog = fopen(log_file,"w");
	setbuf(stlog,NULL); // write to file without delay
	char s[20]; // temp var

	BoardInit();
	EmptyBoard(board);
	printBoard(board,stdout);

	// temp vars
	Tl* h = (Tl*) malloc(sizeof(Tl));
	Board brd;
	
	Cell player = player_1;
	// getting moves
	for(i=0; i<CELLS_TO_PLACE; i++){
		simulations_done = 0;
		nextMove(player_1,&board,1,1,NULL,0,CELLS_TO_PLACE-count_player_cells(board,player) < 2? 0 : 1);
		memcpy(brd, board, sizeof(Board));
		int score = Simulate(&brd,player,TURNS_NUM,h);
		clearScreen();
		printBoard(board,stdout);
		printf("Simulations done: %d   Score: %d", simulations_done, score);
		//scanf("%s",&s);
	}
	
	scanf("%s",s);
	printf("\n\n");

	int score = Simulate(&board,player,TURNS_NUM,h);
	
	if (strcmp(s,"p") == 0){
		// print history
		printHistory(h, stdout, player);
	}
	else{
		// print last history frame
		clearScreen();
		getBoardFromHist(h, 499, board);
		printBoard(board, stdout);
	}
	printf("\n\nscore: %d\n\n", score);
	
	scanf("%s",s);
	fclose(stlog);
	free(h);
#endif

	return 0;
}