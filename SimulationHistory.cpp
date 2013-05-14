#include <string.h>

#include "SimulationHistory.h"
#include "Player.h"
#include "BoardPrint.h"
#include "utils.h"

//extern Cell PLAYER;

void getBoardFromHist(Tl* h, int turn, Board board){
	memcpy(board,h->origin,sizeof(Board));
	int row, col;
	for(int t=0; t<=turn; ++t){
		// apply cell changes
		for(int i=0; i < h->changes_num[t]; ++i){
			row = h->changes_coords[t][i][0];
			col = h->changes_coords[t][i][1];
			board[row][col] = h->changes[t][i];
		}	
	}
}

void getNeighFromHist(Tl* h, int turn, int neigh[BOARD_FULL_HEIGHT][BOARD_FULL_WIDTH][PLAYERS_NUM]){
	memcpy(neigh,h->neigh_origin,sizeof(Board));
	int row, col;
	int player;
	int cnt;
	for(int t=0; t<=turn; ++t){
		// apply neigh count changes
		for(int i=0; i < h->neigh_changes_num[t]; ++i){
			row = h->neigh_changes_coords[t][i][0];
			col = h->neigh_changes_coords[t][i][1];
			player = h->neigh_changes[t][i][0];
			cnt = h->neigh_changes[t][i][1];
			neigh[row][col][player] = cnt;
		}	
	}
}

void printHistory(Tl* h, FILE* stream, Cell player, bool debug){
	Board board;
	int neigh_count[BOARD_FULL_HEIGHT][BOARD_FULL_WIDTH][2];
	char* s;

	// get origins
	memcpy(board,h->origin,sizeof(Board));
	memcpy(neigh_count,h->neigh_origin,sizeof(neigh_count));
	
	// print origins
	clearScreen();
	printBrdAndNeighCnt(board,neigh_count,player,stream);
	scanf("%s",&s);
	
	// print rest of the history
	int row,col;
	for(int turn=0; turn<TURNS_NUM; turn++){
		if(debug){
			scanf("%s",&s);
		}
		else{
			sleep(40);
			clearScreen();
		}
		getBoardFromHist(h,turn,board);
		getNeighFromHist(h,turn,neigh_count);
		fprintf(stream, "turn: %d   score: %d", turn, count_player_cells(board,player));
		//printBrdAndNeighCnt(board,neigh_count,player,stream);
		printBoard(board,stream);
	}
}