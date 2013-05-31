#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "Board.h"

Board board_init;
bool cell_added_init[BOARD_FULL_HEIGHT][BOARD_FULL_WIDTH];

char char_of_cell(Cell cell){
	switch(cell){
		case player_1: return 'w';
		case player_2: return 'b';
		case dead: return '-';
		case incorrect: return '0';
		default: return '#';
	}	
}

Cell cell_of_char(char c){
	switch(c){
		case 'w': return player_1;
		case 'b': return player_2;
		case '-': return dead;
		case '0': return incorrect;
		default: return incorrect;
	}
}

void strToBoard(char* s, Board board){
	int row, col;
	for(int i=0; i<BOARD_SIZE; ++i){
		row = i / BOARD_WIDTH + 1;
		col = i % BOARD_WIDTH + 1;
		board[row][col] = cell_of_char(s[i]);
	}
}

void boardToStr(Board board, char** str){
	int row, col;
	char* s = (char*) malloc(sizeof(char)*(BOARD_SIZE+1)); // +1 is for NULL-terminant
	for(int i=0; i<BOARD_SIZE; ++i){
		row = i / BOARD_WIDTH + 1;
		col = i % BOARD_WIDTH + 1;
		s[i] = char_of_cell(board[row][col]);
	}
	s[BOARD_SIZE] = NULL;
	*str = s;
}

void initBoard(){
	int i,k;
	// fill full board with 'incorrect'
	for(i=0; i<BOARD_FULL_HEIGHT; i++){
		for(k=0; k<BOARD_FULL_WIDTH; k++){
			board_init[i][k] = incorrect;
		}
	}
	// fill inner real board part with 'dead'
	for(i=1; i<=BOARD_HEIGHT; i++){
		for(k=1; k<=BOARD_WIDTH; k++){
			board_init[i][k] = dead;
		}
	}
}

void initCellAdded(){
	// cells out of board must be true to ensure they will not be processed
	memset(cell_added_init,true,sizeof(bool)*BOARD_FULL_SIZE);
	for(int i=1; i<=BOARD_HEIGHT; i++){
		for(int k=1; k<=BOARD_WIDTH;k++){
			cell_added_init[i][k] = false;
		}
	}
}

void BoardInit(){
	initBoard();
	initCellAdded();
}

void EmptyBoard(Board board){
	memcpy(board, board_init, sizeof(Cell)*BOARD_FULL_SIZE);
}

void EmptyCellAdded(bool cell_added[BOARD_FULL_HEIGHT][BOARD_FULL_WIDTH]){
	memcpy(cell_added, cell_added_init, sizeof(bool)*BOARD_FULL_SIZE);
}

bool boardDiff(Board board1, Board board2)
{
	for(int row=1; row<=BOARD_HEIGHT; ++row){
		for(int col=1; col<=BOARD_WIDTH; ++col){
			if(board1[row][col] != board2[row][col]){
				return true;
			}
		}
	}
	return false;
}

bool coordsHas(Coords list, int len, int row, int col){
	for(int n=0; n<len; ++n){
		int n_row = list[n][0];
		int n_col = list[n][1];
		if( n_row == row && n_col == col){
			return true;
		}
	}
	return false;
}

bool coordsDiff(Coords list1, Coords list2, int len){
	for(int n=0; n<len; ++n){
		int row = list1[n][0];
		int col = list1[n][1];
		if(!coordsHas(list2, len, row, col)){
			return true;
		}
	}
	return false;
}

int count_player_cells(Board board, Cell player){
	int i,k;
	int count = 0;
	for(i=1; i<=BOARD_HEIGHT; i++){
		for(k=1; k<=BOARD_WIDTH; k++){
			if(board[i][k] == player){
				count++;
			}
		}
	}
	return count;
}

int getAliveCells(Board board, Coords alive_cells){
	int i,k;
	int n = 0;
	memset(alive_cells,-1,sizeof(int)*BOARD_FULL_SIZE*2);
	for(i=1; i<=BOARD_HEIGHT; i++){
		for(k=1; k<=BOARD_WIDTH; k++){
			if(board[i][k] != dead){
				alive_cells[n][0] = i;
				alive_cells[n][1] = k;
				n++;
			}
		}
	}
	return n;
}

int addNeighboursCoords(Coords *cells, int cells_num, int level, int (*new_list)[2], 
					    bool cell_added[BOARD_FULL_HEIGHT][BOARD_FULL_WIDTH], 
						bool update_cells_added)
{	
	const int neighbours[24][2] =
			{
				// level 1: [0..7]
				         {-1,-1}, {-1,0}, {-1,1},
				         {0,-1},          {0,1}, 
				         {1,-1},  {1,0},  {1,1},
				// level 2: [8..23]
				{-2,-2}, {-2,-1}, {-2,0}, {-2,1}, {-2,2},
				{-1,-2},                          {-1,2},
				{0,-2},                           {0,2},
				{1,-2},                           {1,2},
				{2,-2},  {2,-1},  {2,0},  {2,1},  {2,2},
			};
	
	int neigh_first, neigh_last;
	switch (level){
		case 1: neigh_first = 0; neigh_last = 7;  break;
		case 2: neigh_first = 8; neigh_last = 23; break;
	}

	int new_list_size = 0;
	for(int i=0; i<cells_num; i++){
		int row = (*cells)[i][0];
		int col = (*cells)[i][1];
		for(int n=neigh_first; n <= neigh_last; ++n){
			int n_row = row + neighbours[n][0];
			int n_col = col + neighbours[n][1];
			if(n_row > 0 && n_row <= BOARD_HEIGHT   // check if row and col are in range
				&& n_col > 0 && n_col <= BOARD_WIDTH)
			{
				if (!cell_added[n_row][n_col]){
					new_list[new_list_size][0] = n_row;
					new_list[new_list_size][1] = n_col;
					cell_added[n_row][n_col] = true;
					++new_list_size;
				}
			}
		}
	}
	if (!update_cells_added){
		// revert cell_added changes
		for (int i = new_list_size-1; i>=0; --i){
			int row = new_list[i][0];
			int col = new_list[i][1];
			cell_added[row][col] = false;
		}
	}
	return new_list_size;
}