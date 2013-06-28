#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>

#pragma comment(linker, "/STACK:16777216") 

// real board
const int BOARD_HEIGHT = 29;
const int BOARD_WIDTH = 29;
const int BOARD_SIZE = BOARD_HEIGHT * BOARD_WIDTH;

// board representation with additional sides
const int BOARD_FULL_HEIGHT = 1 + BOARD_HEIGHT + 1;
const int BOARD_FULL_WIDTH = 1 + BOARD_WIDTH  + 1;
const int BOARD_FULL_SIZE = BOARD_FULL_HEIGHT * BOARD_FULL_WIDTH;

const int CELLS_TO_PLACE = 40;
const int TURNS_NUM = 500;
const int PLAYERS_NUM = 2; // violates the idea of Cell enum :(

enum Cell {player_1, player_2, dead, incorrect};
typedef Cell Board[BOARD_FULL_HEIGHT][BOARD_FULL_WIDTH];
typedef int Coords[BOARD_FULL_SIZE][2]; // array of coordinates of cells

Board empty_board; // must be slow to generate a new one every time we need it
Cell player;

void clearScreen(){
	system("cls");
}

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

void initBoard(Board board){
	int i,k;
	// fill full board with 'incorrect'
	//memset(board,char_of_cell(incorrect),sizeof(Cell)*BOARD_FULL_SIZE);
	for(i=0; i<BOARD_FULL_HEIGHT; i++){
		for(k=0; k<BOARD_FULL_WIDTH; k++){
			board[i][k] = incorrect;
		}
	}
	// fill inner real board part with 'dead'
	for(i=1; i<=BOARD_HEIGHT; i++){
		for(k=1; k<=BOARD_WIDTH; k++){
			board[i][k] = dead;
		}
	}
}

void printBoard(Board board){
	int i,k;
	for(i=0; i<BOARD_FULL_HEIGHT; i++){
		for(k=0; k<BOARD_FULL_WIDTH; k++){
			printf("%c", char_of_cell(board[i][k]));
		}
		printf("\n");
	}
}

Cell enemy(Cell player){
	switch(player){
		case player_1: return player_2;
		case player_2: return player_1;
	}
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

int Simulate(Board board, Coords alive_cells, Cell player, int turns, bool print_board){
	int row,col;
	Cell owner;

	// TODO: you can transform neighbours_count into 1d-array like cells_to_process
	// because you need this info only for cells you will process.
	// OR YOU CAN  combine all the info into one array cells_to_process that will 
	// contain row, col, neighbours count etc.
	
	// [][][0] - 1st player's neighbours; [][][1] - 2nd player's
	int neighbours_count[BOARD_FULL_HEIGHT][BOARD_FULL_WIDTH][PLAYERS_NUM];
	
	// tracks if this cell was already added to the queue
	bool cell_added[BOARD_FULL_HEIGHT][BOARD_FULL_WIDTH];
	
	// init empty cell_added array
	bool cell_added_init[BOARD_FULL_HEIGHT][BOARD_FULL_WIDTH];
	
	// cells out of board must be true so I don't add them to the queue
	memset(cell_added_init,true,sizeof(bool)*BOARD_FULL_SIZE);
	for(int i=1; i<=BOARD_HEIGHT; i++){
		for(int k=1; k<=BOARD_WIDTH;k++){
			cell_added_init[i][k] = false;
		}
	}

	// Cells that can change their state on this turn (alive cells + neighbours)
	int cells_to_process_num;
	Coords cells_to_process;

	// alive cells for this and previous steps
	int alive_cells_num = getAliveCells(board,alive_cells);
	Coords prev_alive_cells;
	Coords cur_alive_cells;
	memcpy(cur_alive_cells,alive_cells,sizeof(int)*BOARD_FULL_SIZE*2);

	// alive cells num for each player
	int player_alive_cells_num[PLAYERS_NUM] = {0,0};

	if(print_board){
			clearScreen();
			printBoard(board);
			//char* s;
			//scanf("%s",&s);
		}

	// simulation
	for(int turn=0; turn < turns; turn++){
		cells_to_process_num = 0;
		memset(neighbours_count,0,sizeof(int)*BOARD_FULL_SIZE*PLAYERS_NUM);
		memcpy(cell_added,cell_added_init,sizeof(bool)*BOARD_FULL_SIZE);

		// updating neighbours info
		for(int i=0; i<alive_cells_num; i++){
			row = cur_alive_cells[i][0];
			col = cur_alive_cells[i][1];
			owner = board[row][col];
			
			// TODO: make an array [(-1,-1), (-1,0), (-1,1)...] and update in loop
			// UPD: failed, too slow (+30%). Next idea: keep constant array of neighbours
			// coordinates for every cell. Calculate it on the startup.
			if(!cell_added[row][col]){
				cells_to_process[cells_to_process_num][0] = row;
				cells_to_process[cells_to_process_num][1] = col;
				cells_to_process_num++;
				cell_added[row][col] = true;
			}

			neighbours_count[row-1][col-1][owner]++;
			if(!cell_added[row-1][col-1]){
				cells_to_process[cells_to_process_num][0] = row-1;
				cells_to_process[cells_to_process_num][1] = col-1;
				cells_to_process_num++;
				cell_added[row-1][col-1] = true;
			}

			neighbours_count[row-1][col][owner]++;
			if(!cell_added[row-1][col]){
				cells_to_process[cells_to_process_num][0] = row-1;
				cells_to_process[cells_to_process_num][1] = col;
				cells_to_process_num++;
				cell_added[row-1][col] = true;
			}

			neighbours_count[row-1][col+1][owner]++;
			if(!cell_added[row-1][col+1]){
				cells_to_process[cells_to_process_num][0] = row-1;
				cells_to_process[cells_to_process_num][1] = col+1;
				cells_to_process_num++;
				cell_added[row-1][col+1] = true;
			}

			neighbours_count[row][col-1]	[owner]++;
			if(!cell_added[row][col-1]){
				cells_to_process[cells_to_process_num][0] = row;
				cells_to_process[cells_to_process_num][1] = col-1;
				cells_to_process_num++;
				cell_added[row][col-1] = true;
			}

			neighbours_count[row][col+1][owner]++;
			if(!cell_added[row][col+1]){
				cells_to_process[cells_to_process_num][0] = row;
				cells_to_process[cells_to_process_num][1] = col+1;
				cells_to_process_num++;
				cell_added[row][col+1] = true;
			}

			neighbours_count[row+1][col-1][owner]++;
			if(!cell_added[row+1][col-1]){
				cells_to_process[cells_to_process_num][0] = row+1;
				cells_to_process[cells_to_process_num][1] = col-1;
				cells_to_process_num++;
				cell_added[row+1][col-1] = true;
			}

			neighbours_count[row+1][col][owner]++;
			if(!cell_added[row+1][col]){
				cells_to_process[cells_to_process_num][0] = row+1;
				cells_to_process[cells_to_process_num][1] = col;
				cells_to_process_num++;
				cell_added[row+1][col] = true;
			}

			neighbours_count[row+1][col+1][owner]++;
			if(!cell_added[row+1][col+1]){
				cells_to_process[cells_to_process_num][0] = row+1;
				cells_to_process[cells_to_process_num][1] = col+1;
				cells_to_process_num++;
				cell_added[row+1][col+1] = true;
			}
		}

		// updating cells of the new board
		alive_cells_num = 0;
		player_alive_cells_num[0] = 0;
		player_alive_cells_num[1] = 0;
		for(int i=0; i<cells_to_process_num;i++){
			row = cells_to_process[i][0];
			col = cells_to_process[i][1];
			// killing alive cells
			if(board[row][col] != dead){
				if(neighbours_count[row][col][player_1] + neighbours_count[row][col][player_2] < 2 
					|| neighbours_count[row][col][player_1] + neighbours_count[row][col][player_2] > 3){
						board[row][col] = dead;
				}
				else{ 
					// cell stays alive
					cur_alive_cells[alive_cells_num][0] = row;
					cur_alive_cells[alive_cells_num][1] = col;
					owner = board[row][col];
					player_alive_cells_num[owner]++;
					alive_cells_num++;
				}
			}
			// bringing dead cells to life
			else{ 
				if(neighbours_count[row][col][player_1] + neighbours_count[row][col][player_2] == 3){
					if (neighbours_count[row][col][player_1] > neighbours_count[row][col][player_2]){
						owner = player_1;
					}
					else{
						owner = player_2;
					}
					board[row][col] = owner;
					player_alive_cells_num[owner]++;
					cur_alive_cells[alive_cells_num][0] = row;
					cur_alive_cells[alive_cells_num][1] = col;
					alive_cells_num++;
				}
			}
		}

		if(print_board){
			clearScreen();
			printBoard(board);
		}

		// player lost
		if(player_alive_cells_num[0] == 0 || player_alive_cells_num[1] == 1){ 
			break;
		}

		// board stabilized, there will be no changes
		if(memcmp(prev_alive_cells,cur_alive_cells,sizeof(int)*2*alive_cells_num) == 0){
			break;
		}
		else{
			memcpy(prev_alive_cells,cur_alive_cells,sizeof(int)*2*alive_cells_num);
		}
	}
	// simulation ended, return score
	// TODO: optimize. instead of calling function make counters above.
	return count_player_cells(board,player) - count_player_cells(board, enemy(player));
}

// rename this function to minimax and make a new nextMove that only calls minimax and prints best move
int nextMove(Cell player, Board board, int prev_row, int prev_col, int ply, int max_ply){
	const int max_simulations = BOARD_WIDTH * BOARD_HEIGHT; 
	int simulations_done = 0;
	int max_score = -10000;
	int score = 0;
	int i,k;
	int best_i = prev_row;
	int best_k = prev_col;

	Coords alive_cells;
	int alive_cells_num = getAliveCells(board,alive_cells);

	// TODO: make issueOrder function
	// if playing white than first move goes to center to prevent black from mirroring your moves
	if(alive_cells_num == 0){
		best_i = BOARD_FULL_HEIGHT / 2;
		best_k = BOARD_FULL_WIDTH / 2;
#ifdef TESTING_MODE
		board[best_i][best_k] = player;
#else
		printf("%d %d",best_i-1,best_k-1); // -1 because my board starts at 1
#endif
		return score;
	}

	i = prev_row;
	for(i; i<=BOARD_HEIGHT; i++){
		k = i>prev_row? 1 : prev_col; // this row start from the prev_col, other rows from col=1
		for(k; k<=BOARD_WIDTH; k++){
			if (board[i][k] == dead){
				// make move
				Board board_copy;
				memcpy(board_copy,board,sizeof(Cell)*BOARD_FULL_SIZE);
				board_copy[i][k] = player;
				alive_cells[alive_cells_num][0] = i;
				alive_cells[alive_cells_num][1] = k;
				
				// process move
				if(ply > 0){
					score = nextMove(player,board_copy,i,k,ply+1,max_ply);
				}
				else{				
					score = Simulate(board_copy,alive_cells,player,TURNS_NUM,false);
					simulations_done++;
				}

				// unmake move
				alive_cells[alive_cells_num][0] = -1;
				alive_cells[alive_cells_num][1] = -1;

				if(score > max_score){
                	max_score = score;
                    best_i = i;
                    best_k = k;
                }
			}
		}
		if(simulations_done >= max_simulations){
			break;
		}
	}

	if(ply == 0){
		printf("%d %d",best_i-1,best_k-1); // -1 because my board starts at 1
	}
	return score;
}

int main() {
    int i,k;
    char c;
    Board board;

    scanf("%c\n", &c);
	player = cell_of_char(c);

	initBoard(empty_board);
	initBoard(board);

    for(i=1; i<=BOARD_HEIGHT; i++){
		for (k=1; k<=BOARD_WIDTH; k++){
			scanf("%c", &c);
			board[i][k] = cell_of_char(c);
		}
		scanf("\n");
	}

    nextMove(player,board,1,1,0,0);

	return 0;
}