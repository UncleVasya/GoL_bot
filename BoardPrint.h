#include <stdio.h>

#include "main.h"

void printBoardDiff(Board board1, Board board2, FILE* stream, bool detailed = false);
void printNeighDiff(int grid1[BOARD_FULL_HEIGHT][BOARD_FULL_WIDTH][PLAYERS_NUM], 
					int grid2[BOARD_FULL_HEIGHT][BOARD_FULL_WIDTH][PLAYERS_NUM],
					FILE* stream,
					bool detailed = false);
void printCoords(Coords coords, size_t len, FILE* stream = stdout);
void printLegendRow(FILE* stream);
void printBoardRow(Board board, int row, FILE* stream);
void printNeighRow(int neigh[BOARD_FULL_HEIGHT][BOARD_FULL_WIDTH][PLAYERS_NUM], 
				   int row, int player, FILE* stream);
void printBoard(Board board, FILE* stream);
void printNeighCnt(int neigh_count[BOARD_FULL_HEIGHT][BOARD_FULL_WIDTH][2], int player, FILE* stream);
void printBrdAndNeighCnt(Board board, int neigh_count[BOARD_FULL_HEIGHT][BOARD_FULL_WIDTH][2], 
						 Cell player, FILE* stream);