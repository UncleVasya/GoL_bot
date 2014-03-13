#include "main.h"
#include "Board.h"

// max size of board representation in ListLife format
const int MAX_LL_SIZE = BOARD_SIZE+1; // +1 for ending 0

// interface between LL and bot logic
void testListLife();
int SimulateLL(Board *board, Cell player, int turns);

// debug functions for bitmaps; can be useful not only for LL.
// think about moving them out
void binToStr(unsigned n, char* str, int strSize);
void print_3x3(char* str);

// inner functions. don't use them outside of ListLife files
void life_1_7(int *this_gen, int *new_gen);
void llToBoard(int* ll, Board board);
void boardToLL(Board board, int* ll);