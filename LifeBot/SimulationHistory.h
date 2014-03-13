#include <stdio.h>

#include "main.h"
#include "Board.h"

#pragma once

typedef struct{
	int turns;
	
	Board origin;
	int changes_num[TURNS_NUM];
	Coords changes_coords[TURNS_NUM];
	Cell changes[TURNS_NUM][BOARD_FULL_SIZE];

	int neigh_origin[BOARD_FULL_HEIGHT][BOARD_FULL_WIDTH][PLAYERS_NUM];
	int neigh_changes_num[TURNS_NUM];
	// *PLAYERS_NUM is because I separate changes for p1 and p2: 
	// if one cell have both its p1 and p2 neigh count changed there are two changes, not one
	int neigh_changes_coords[TURNS_NUM][BOARD_FULL_SIZE*PLAYERS_NUM][2];
	int neigh_changes[TURNS_NUM][BOARD_FULL_SIZE*PLAYERS_NUM][2]; // 0 - player; 1 - neigh_count

} Tl;

void getBoardFromHist(Tl* h, int turn, Board board);
void getNeighFromHist(Tl* h, int turn, int neigh[BOARD_FULL_HEIGHT][BOARD_FULL_WIDTH][PLAYERS_NUM]);
void printHistory(Tl* h, FILE* stream, Cell player, bool debug = false);