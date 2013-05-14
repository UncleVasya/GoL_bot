#include <stdio.h>
#include <stdlib.h>

#include "main.h"
#include "Board.h"



void printBoardDiff(Board board1, Board board2, FILE* stream, bool detailed = false)
{
	for(int row=1; row<=BOARD_HEIGHT; ++row){
		for(int col=1; col<=BOARD_WIDTH; ++col){
			if(board1[row][col] != board2[row][col]){
				if(!detailed){
					fprintf(stream,"(%d %d)  ",row,col);
				}
				else{
					fprintf(stream, "cell: (%d %d)    board 1: %c    board 2: %c \n", row, col,
							char_of_cell(board1[row][col]), char_of_cell(board2[row][col]));
				}
			}
		}
	}
}

void printNeighDiff(int grid1[BOARD_FULL_HEIGHT][BOARD_FULL_WIDTH][PLAYERS_NUM], 
					int grid2[BOARD_FULL_HEIGHT][BOARD_FULL_WIDTH][PLAYERS_NUM],
					FILE* stream,
					bool detailed = false)
{
	for(int row=1; row<=BOARD_HEIGHT; ++row){
		for(int col=1; col<=BOARD_WIDTH; ++col){
			if(grid1[row][col][0] != grid2[row][col][0] || grid1[row][col][1] != grid2[row][col][1]){
				if(!detailed){
					fprintf(stream,"(%d %d)  ",row,col);
				}
				else{
					fprintf(stream, "cell: (%d %d)    grid 1: %d %d    grid 2: %d %d \n", row,col,
							grid1[row][col][0], grid1[row][col][1], grid2[row][col][0], grid2[row][col][1]);
				}
			}
		}
	}
}

void printCoords(Coords coords, size_t len, FILE* stream = stdout){
	int row,col;
	for(int i=0; i<len; ++i){
		row = coords[i][0];
		col = coords[i][1];
		fprintf(stream,"(%d %d)  ",row,col);
	}
}

void printLegendRow(FILE* stream){
	for(int col=1; col<=BOARD_WIDTH; ++col){
		fprintf(stream,"%d", col % 10);
	}
}

void printBoardRow(Board board, int row, FILE* stream){
	fprintf(stream,"%d  ", row % 10);
	for(int col=0; col<=BOARD_WIDTH+1; ++col){
		fprintf(stream,"%c", char_of_cell(board[row][col]));
	}
}

void printNeighRow(int neigh[BOARD_FULL_HEIGHT][BOARD_FULL_WIDTH][PLAYERS_NUM], int row, int player,
					FILE* stream)
{
	fprintf(stream,"%d  ", row % 10);
	for(int col=1; col<=BOARD_WIDTH; ++col){
		fprintf(stream,"%d", neigh[row][col][player]);
	}
}

void printBoard(Board board, FILE* stream){
	// legend on top
	fprintf(stream,"   ");
	printLegendRow(stream);
	fprintf(stream,"\n\n");
	// board
	for(int row=0; row<=BOARD_HEIGHT+1; ++row){
		printBoardRow(board,row,stream);
		fprintf(stream,"\n");
	}
	fprintf(stream,"\n\n");
}

void printNeighCnt(int neigh_count[BOARD_FULL_HEIGHT][BOARD_FULL_WIDTH][2], int player, FILE* stream){
	// legend on top
	fprintf(stream,"   ");
	printLegendRow(stream);
	fprintf(stream,"\n\n");
	// grid
	for(int row=1; row<=BOARD_HEIGHT; row++){
		printNeighRow(neigh_count,row,player,stream);
		fprintf(stream,"\n");
	}
	fprintf(stream,"\n\n");
}

void printBrdAndNeighCnt(Board board, int neigh_count[BOARD_FULL_HEIGHT][BOARD_FULL_WIDTH][2], Cell player,
						 FILE* stream)
{
	fprintf(stream,"\n\n");
	// legends on top
	fprintf(stream,"   ");
	printLegendRow(stream);
	fprintf(stream,"      ");
	printLegendRow(stream);
	fprintf(stream,"\n\n");
	// grid
	for(int row=1; row<=BOARD_HEIGHT; ++row){
		printBoardRow(board,row,stream);
		fprintf(stream,"   ");
		printNeighRow(neigh_count,row,player,stream);
		fprintf(stream,"\n");
	}
	fprintf(stream,"\n\n");
}