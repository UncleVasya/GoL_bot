#include "Board.h"
#include "SimulationHistory.h"

int Simulate(Board* board, Cell player, int turns, Tl* h);
int Simulate(Board* brd, int move_row, int move_col, Cell player, int turns, 
			 Tl* prev_h, Tl* new_h, bool print_board = false);