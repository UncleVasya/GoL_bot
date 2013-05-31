#include <stdio.h>

#pragma once

// real board
const int BOARD_HEIGHT = 29;
const int BOARD_WIDTH = 29;
const int BOARD_SIZE = BOARD_HEIGHT * BOARD_WIDTH;

// board representation with additional sides
const int BOARD_FULL_HEIGHT = 1 + BOARD_HEIGHT + 1;
const int BOARD_FULL_WIDTH = 1 + BOARD_WIDTH  + 1;
const int BOARD_FULL_SIZE = BOARD_FULL_HEIGHT * BOARD_FULL_WIDTH;

enum Cell {player_1, player_2, dead, incorrect};
typedef Cell Board[BOARD_FULL_HEIGHT][BOARD_FULL_WIDTH];
typedef int Coords[BOARD_FULL_SIZE][2]; // array of coordinates of cells; delete * PLAYERS_NUM after debug

void BoardInit();
void EmptyBoard(Board board);
void EmptyCellAdded(bool cell_added[BOARD_FULL_HEIGHT][BOARD_FULL_WIDTH]);

bool boardDiff(Board board1, Board board2);
bool coordsHas(Coords list, int len, int row, int col);
bool coordsDiff(Coords list1, Coords list2, int len);

int count_player_cells(Board board, Cell player);
int getAliveCells(Board board, Coords alive_cells);
int addNeighboursCoords(Coords *cells, int cells_num, int level, int (*new_list)[2], bool cell_added[BOARD_FULL_HEIGHT][BOARD_FULL_WIDTH], bool update_cells_added);

char char_of_cell(Cell cell);
Cell cell_of_char(char c);

void strToBoard(char* s, Board board);
void boardToStr(Board board, char** str);
