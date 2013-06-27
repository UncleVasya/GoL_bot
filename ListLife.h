#include "main.h"
#include "Board.h"

// max size of board representation in ListLife format
const int MAX_LL_SIZE = BOARD_SIZE+1; // +1 for ending 0

void testListLife();
void life_1_7(int *this_gen, int *new_gen);

void binToStr(unsigned n, char* str, int strSize);
void llToBoard(int* ll, Board board);
void print_3x3(char* str);