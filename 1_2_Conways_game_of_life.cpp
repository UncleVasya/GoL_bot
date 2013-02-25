#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>

#pragma comment(linker, "/STACK:16777216")  // enlarge your stack size!

// sleep function
#ifdef _WIN32
    #include <windows.h>
    void sleep(unsigned milliseconds)
    {
        Sleep(milliseconds);
    }
#else
    #include <unistd.h>
    void sleep(unsigned milliseconds)
    {
        usleep(milliseconds * 1000); // takes microseconds
    }
#endif

//#define DEBUG_MODE
#define TESTING_MODE

// log file
const char* log_file = "log.txt";
FILE* stlog;

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
typedef int Coords[BOARD_FULL_SIZE * PLAYERS_NUM][2]; // array of coordinates of cells; delete * PLAYERS_NUM after debug

typedef struct{
	int turns;
	
	Board origin;
	int changes_num[TURNS_NUM];
	Coords changes_coords[TURNS_NUM];
	Cell changes[TURNS_NUM][BOARD_FULL_SIZE];

	int neigh_origin[BOARD_FULL_HEIGHT][BOARD_FULL_WIDTH][PLAYERS_NUM];
	int neigh_changes_num[TURNS_NUM+5]; // delete +5 after debug
	// *PLAYERS_NUM is because I separate changes for p1 and p2: 
	// if one cell have both its p1 and p2 neigh count changed there are two changes, not one
	int neigh_changes_coords[TURNS_NUM][BOARD_FULL_SIZE*PLAYERS_NUM][2];
	int neigh_changes[TURNS_NUM][BOARD_FULL_SIZE*PLAYERS_NUM][2]; // 0 - player; 1 - neigh_count

} Tl;

Board empty_board; // must be slow to generate a new one every time we need it
Cell player;

long simulations_done = 0;
long max_simulations = 10000;

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

bool neighDiff(int grid1[BOARD_FULL_HEIGHT][BOARD_FULL_WIDTH][PLAYERS_NUM], 
			   int grid2[BOARD_FULL_HEIGHT][BOARD_FULL_WIDTH][PLAYERS_NUM])
{
	for(int i=1; i<BOARD_HEIGHT+1; ++i){
		for(int k=1; k<BOARD_WIDTH+1; ++k){
			if(grid1[i][k][0] != grid2[i][k][0] || grid1[i][k][1] != grid2[i][k][1]){
				return true;
			}
		}
	}
	return false;
}

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
	for(int col=1; col<=BOARD_WIDTH; ++col){
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
	for(int row=1; row<=BOARD_HEIGHT; ++row){
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

void printHistory(Tl* h, FILE* stream, bool debug = false){
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
		printBrdAndNeighCnt(board,neigh_count,player,stream);
	}
}

//--- OLD VERSIONS OF SIMULATION CODE IS HERE TO TEST AGAINST NEW ONES----
int Simulate_old(Board* board, Cell player, int turns, bool print_board){
	int row,col, n_row, n_col; // for temp vals
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

	Coords alive_cells;
	// alive cells for this and previous steps
	int alive_cells_num = getAliveCells(*board,alive_cells);
	Coords prev_alive_cells = {{-1,-1}};
	Coords cur_alive_cells;
	memcpy(cur_alive_cells,alive_cells,sizeof(int)*BOARD_FULL_SIZE*2);

	// alive cells num for each player
	int player_alive_cells_num[PLAYERS_NUM] = {0,0};

	if(print_board){
			clearScreen();
			printBoard(*board,stdout);
			//char* s;
			//scanf("%s",&s);
		}

	int neighbours[8][2] = {{-1,-1},{-1,0},{-1,1},{0,-1},{0,1},{1,-1},{1,0},{1,1}};
	
	// simulation
	for(int turn=0; turn < turns; turn++){
		cells_to_process_num = 0;
		memset(neighbours_count,0,sizeof(int)*BOARD_FULL_SIZE*PLAYERS_NUM);
		memcpy(cell_added,cell_added_init,sizeof(bool)*BOARD_FULL_SIZE);

		// updating neighbours info
		for(int i=0; i<alive_cells_num; i++){
			row = cur_alive_cells[i][0];
			col = cur_alive_cells[i][1];
			owner = (*board)[row][col];
			
			if(!cell_added[row][col]){
				cells_to_process[cells_to_process_num][0] = row;
				cells_to_process[cells_to_process_num][1] = col;
				cells_to_process_num++;
				cell_added[row][col] = true;
			}

			for(int n=0; n<8; n++){
				n_row = row + neighbours[n][0];
				n_col = col + neighbours[n][1];
				neighbours_count[n_row][n_col][owner]++;
				if(!cell_added[n_row][n_col]){
					cells_to_process[cells_to_process_num][0] = n_row;
					cells_to_process[cells_to_process_num][1] = n_col;
					cells_to_process_num++;
					cell_added[n_row][n_col] = true;
				}
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
			if((*board)[row][col] != dead){
				if(neighbours_count[row][col][player_1] + neighbours_count[row][col][player_2] < 2 
					|| neighbours_count[row][col][player_1] + neighbours_count[row][col][player_2] > 3){
						(*board)[row][col] = dead;
				}
				else{ 
					// cell stays alive
					cur_alive_cells[alive_cells_num][0] = row;
					cur_alive_cells[alive_cells_num][1] = col;
					owner = (*board)[row][col];
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
					(*board)[row][col] = owner;
					player_alive_cells_num[owner]++;
					cur_alive_cells[alive_cells_num][0] = row;
					cur_alive_cells[alive_cells_num][1] = col;
					alive_cells_num++;
				}
			}
		}

		if(print_board){
			clearScreen();
			printBoard(*board,stdout);
			//char* s;
			//scanf("%s",&s);
		}

		// player lost
		if(player_alive_cells_num[0] == 0 || player_alive_cells_num[1] == 1){ 
			break;
		}

		// board stabilized, there will be no changes
		if(memcmp(prev_alive_cells,cur_alive_cells,sizeof(int)*2*BOARD_FULL_SIZE) == 0){
			break;
		}
		else{
			memcpy(prev_alive_cells,cur_alive_cells,sizeof(int)*2*alive_cells_num);
		}
	}
	// simulation ended, return score
	// TODO: optimize. instead of calling function make counters above.
	return count_player_cells((*board),player) - count_player_cells((*board), enemy(player));
}


//--- CURRENT VERSION CODE----

// this function does full simulation and fills history
int Simulate(Board* board, Cell player, int turns, Tl* h){
	int row,col;
	Cell owner;

	// TODO: you can transform neighbours_count into 1d-array like cells_to_process
	// because you need this info only for cells you will process.
	// OR YOU CAN  combine all the info into one array cells_to_process that will 
	// contain row, col, neighbours count etc.
	
	// [][][0] - 1st player's neighbours; [][][1] - 2nd player's
	int neighbours_count[BOARD_FULL_HEIGHT][BOARD_FULL_WIDTH][PLAYERS_NUM];
	int prev_neigh_count[BOARD_FULL_HEIGHT][BOARD_FULL_WIDTH][PLAYERS_NUM];
	
	// tracks if this cell was already added to the queue
	bool cell_added[BOARD_FULL_HEIGHT][BOARD_FULL_WIDTH];
	
	// initial cell_added array
	bool cell_added_init[BOARD_FULL_HEIGHT][BOARD_FULL_WIDTH];
	
	int alive_cells_num;
	Coords alive_cells;
	alive_cells_num = getAliveCells(*board,alive_cells); // put this outside the function for speed

	// cells out of board must be true so I don't add them to the queue
	memset(cell_added_init,true,sizeof(bool)*BOARD_FULL_SIZE);
	for(int i=1; i<=BOARD_HEIGHT; i++){
		for(int k=1; k<=BOARD_WIDTH;k++){
			cell_added_init[i][k] = false;
		}
	}

	// Cells that can change their state on this turn (alive cells + neighbours)
	int cells_to_process_num = 0;
	Coords cells_to_process;

	// needed for history filling
	int changed_cells_num = 0;
	Coords changed_cells;
	Cell changes[BOARD_FULL_SIZE];

	Coords prev_alive_cells = {{-1,-1}};
	Coords cur_alive_cells;
	memcpy(cur_alive_cells,alive_cells,sizeof(int)*BOARD_FULL_SIZE*2);

	// alive cells num for each player
	int player_alive_cells_num[PLAYERS_NUM] = {0,0};

	h->turns = 0;
	memcpy(h->origin,board,sizeof(Board));

	int neighbours[8][2] = {{-1,-1},{-1,0},{-1,1},{0,-1},{0,1},{1,-1},{1,0},{1,1}};

	// simulation
	++simulations_done;
	for(int turn=0; turn<turns; turn++){
		cells_to_process_num = 0;
		changed_cells_num = 0;
		memcpy(cell_added,cell_added_init,sizeof(bool)*BOARD_FULL_SIZE);

		if(turn == 0){
			// calculate initial neighbours count
			memset(neighbours_count,0,sizeof(int)*BOARD_FULL_SIZE*PLAYERS_NUM);
			for(int i=0; i<alive_cells_num; i++){
				row = cur_alive_cells[i][0];
				col = cur_alive_cells[i][1];
				owner = (*board)[row][col];
				int n_row, n_col;
				for(int n=0; n<8; ++n){
					n_row = row + neighbours[n][0]; 
					n_col = col + neighbours[n][1];;
					neighbours_count[n_row][n_col][owner]++;
				}
			}
			memcpy(prev_neigh_count,neighbours_count,sizeof(int)*BOARD_FULL_SIZE*PLAYERS_NUM);
			// copy to the history
			memcpy(h->neigh_origin,neighbours_count,sizeof(int)*BOARD_FULL_SIZE*PLAYERS_NUM);
		}
		
		// add alive cells and their neighbours to the processing queue
		for(int i=0; i<alive_cells_num; i++){
			// cell
			row = cur_alive_cells[i][0];
			col = cur_alive_cells[i][1];
			owner = (*board)[row][col];
			if(!cell_added[row][col]){
				cells_to_process[cells_to_process_num][0] = row;
				cells_to_process[cells_to_process_num][1] = col;
				cells_to_process_num++;
				cell_added[row][col] = true;
			}
			// neighbours
			int n_row, n_col;
			for(int n=0; n<8; ++n){
				n_row = row + neighbours[n][0]; 
				n_col = col + neighbours[n][1];;
				if(!cell_added[n_row][n_col]){
					cells_to_process[cells_to_process_num][0] = n_row;
					cells_to_process[cells_to_process_num][1] = n_col;
					cells_to_process_num++;
					cell_added[n_row][n_col] = true;
				}
			}
		}

		// updating cells
		alive_cells_num = 0;
		player_alive_cells_num[0] = 0;
		player_alive_cells_num[1] = 0;
		for(int i=0; i<cells_to_process_num; i++){
			row = cells_to_process[i][0];
			col = cells_to_process[i][1];
			Cell cell = (*board)[row][col];
			// killing alive cells
			if(cell != dead){
				if(neighbours_count[row][col][player_1] + neighbours_count[row][col][player_2] < 2 
					|| neighbours_count[row][col][player_1] + neighbours_count[row][col][player_2] > 3)
				{
					cell = dead;				
				}
			}
			// bringing dead cells to life
			else{ 
				if(neighbours_count[row][col][player_1] + neighbours_count[row][col][player_2] == 3){
					if (neighbours_count[row][col][player_1] > neighbours_count[row][col][player_2])
					{
						cell = player_1;
					}
					else{
						cell = player_2;
					}
				}
			}
			// keep track of alive cells
			if(cell != dead){
				player_alive_cells_num[cell]++;
				cur_alive_cells[alive_cells_num][0] = row;
				cur_alive_cells[alive_cells_num][1] = col;
				alive_cells_num++;
			}
			// keep track of changed cells
			if(cell != (*board)[row][col]){
				changed_cells[changed_cells_num][0] = row;
				changed_cells[changed_cells_num][1] = col;
				changes[changed_cells_num] = cell;
				changed_cells_num++;
			}
		}

		// update neighbours count around changed cells
		for(int i=0; i<changed_cells_num; i++){
			row = changed_cells[i][0];
			col = changed_cells[i][1];
			Cell owner = changes[i];
			Cell prev_owner = (*board)[row][col];
			int n_row, n_col;
			for(int n=0; n<8; ++n){
				n_row = row + neighbours[n][0]; 
				n_col = col + neighbours[n][1];
				if(owner != dead){ // new cell was born
					++neighbours_count[n_row][n_col][owner];
				}
				else{ // cell died
					--neighbours_count[n_row][n_col][prev_owner];
				}
				// add neighs of changed cells to the processing queue (for neigh history)
				if(!cell_added[n_row][n_col]){
					cells_to_process[cells_to_process_num][0] = n_row;
					cells_to_process[cells_to_process_num][1] = n_col;
					cells_to_process_num++;
					cell_added[n_row][n_col] = true;
				}
			}
			(*board)[row][col] = owner;
		}

		// fill cell changes history
		h->changes_num[turn] = changed_cells_num;
		memcpy(h->changes_coords[turn],changed_cells,sizeof(int)*2*changed_cells_num);
		memcpy(h->changes[turn],changes,sizeof(Cell)*changed_cells_num);
		
		//fill neighbours count history
		int neigh_changes_num = 0;
		for(int i=0; i<cells_to_process_num; ++i){
			row = cells_to_process[i][0];
			col = cells_to_process[i][1];		
			int p1_neigh_cnt = neighbours_count[row][col][0];
			int p2_neigh_cnt = neighbours_count[row][col][1];
				
			if(p1_neigh_cnt != prev_neigh_count[row][col][0]){
				h->neigh_changes_coords[turn][neigh_changes_num][0] = row;
				h->neigh_changes_coords[turn][neigh_changes_num][1] = col;
				h->neigh_changes[turn][neigh_changes_num][0] = player_1;
				h->neigh_changes[turn][neigh_changes_num][1] = p1_neigh_cnt;
				++neigh_changes_num;
			}
			if(p2_neigh_cnt != prev_neigh_count[row][col][1]){
				h->neigh_changes_coords[turn][neigh_changes_num][0] = row;
				h->neigh_changes_coords[turn][neigh_changes_num][1] = col;
				h->neigh_changes[turn][neigh_changes_num][0] = player_2;
				h->neigh_changes[turn][neigh_changes_num][1] = p2_neigh_cnt;
				++neigh_changes_num;
			}
		}
		h->neigh_changes_num[turn] = neigh_changes_num;
		h->turns++;

		// player lost
		if(player_alive_cells_num[0] == 0 
			//|| player_alive_cells_num[1] == 0
			)
		{
			// zero changes for future turns
			for(int t=turn+1; t<TURNS_NUM; ++t){
				h->changes_num[t] = 0;
				h->neigh_changes_num[t] = 0;
				++(h->turns); // delete later
			}
			break;
		}

		// board stabilized, there will be no changes
		if(memcmp(prev_alive_cells,cur_alive_cells,sizeof(int)*2*alive_cells_num) == 0){
			for(int t=turn+1; t<TURNS_NUM; t++){
				h->changes_num[t] = 0;
				h->neigh_changes_num[t] = 0;
				++(h->turns); // delete later
			}
			break;
		}
		else{
			// prepare for the next simulation
			memcpy(prev_alive_cells,cur_alive_cells,sizeof(int)*2*alive_cells_num);
			memcpy(prev_neigh_count,neighbours_count,sizeof(int)*BOARD_FULL_SIZE*PLAYERS_NUM);
		}
	}

	//// simulation ended. 
	//// test for correctness: final board we get from history must be the same with last board in simulation
	//int neigh_hist[BOARD_FULL_HEIGHT][BOARD_FULL_WIDTH][PLAYERS_NUM];
	//memcpy(neigh_hist,h->neigh_origin, sizeof(neigh_hist));
	//for(int turn=0; turn<h->turns; ++turn){
	//	for(int i=0; i<h->neigh_changes_num[turn-1]; ++i){
	//			row = h->neigh_changes_coords[turn-1][i][0];
	//			col = h->neigh_changes_coords[turn-1][i][1];
	//			int player_num = h->neigh_changes[turn-1][i][0];
	//			int neigh_cnt = h->neigh_changes[turn-1][i][1];
	//			neigh_hist[row][col][player_num] = neigh_cnt;
	//		}
	//}
	//int last_neigh_grid[BOARD_FULL_HEIGHT][BOARD_FULL_WIDTH][PLAYERS_NUM];
	//memset(last_neigh_grid,0,sizeof(last_neigh_grid));
	//	for(int i=0; i<cells_to_process_num; i++){
	//		row = cells_to_process[i][0];
	//		col = cells_to_process[i][1];
	//		Cell cell;

	//		for(int n=0; n<8; ++n){
	//			int n_row = row + neighbours[n][0];
	//			int n_col = col + neighbours[n][1];
	//			cell = (*board)[n_row][n_col];
	//			if(cell == player_1){
	//				last_neigh_grid[row][col][0]++; 
	//			}
	//			else if(cell == player_2){
	//				last_neigh_grid[row][col][1]++;
	//			}
	//		}
	//	}

	if(h->origin[13][15] == player_1 && h->origin[14][14] == player_1 
	   && h->origin[14][16] == player_1 && h->origin[15][15] == player_1){
		//printHistory(h,true);
	}

	// simulation ended, return score
	// TODO: optimize. instead of calling function make counters above.
	return count_player_cells(*board,player) - count_player_cells(*board, enemy(player));
}


// this function does only needed simulation and take rest from history
int Simulate(Board* brd, int move_row, int move_col, Cell player, int turns, 
			 Tl* prev_h, Tl* new_h, bool print_board = false)
{
	// temporary vars
	int row,col; 
	int n_row, n_col;
	Cell owner;

	Board* board = brd;
	
	Board board_hist;
	memcpy(&board_hist,prev_h->origin,sizeof(Board));
	//board_hist[move_row][move_col] = dead; // board without last move;
	if(new_h != NULL){
		memcpy(new_h->origin,board,sizeof(Board));
		//new_h->neigh_changes_num[0] = 0;
	}
	
	// tracks if this cell was already added to the processing queue
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

	// track if this cell was changed
	bool cell_changed[BOARD_FULL_HEIGHT][BOARD_FULL_WIDTH];
	Coords ch_cells;
	int ch_cells_num;

	// cells that was different from history on the prev. turn
	bool cell_was_changed[BOARD_FULL_HEIGHT][BOARD_FULL_WIDTH];
	Coords was_changed_cells;
	int was_changed_cells_num;

	// tracks if cell differs from the history at the current turn
	bool cell_diffs_hist[BOARD_FULL_HEIGHT][BOARD_FULL_WIDTH];
	memset(cell_diffs_hist,false,sizeof(cell_diffs_hist));

	Coords changed_cells;
	Cell changes[BOARD_FULL_SIZE*2][2]; // [0] - was; [1] - became
	int changed_cells_num;
	
	// before simulation starts we have only one changed cell - players move
	changed_cells[0][0] = move_row;
	changed_cells[0][1] = move_col;
	changes[0][0] = dead;
	changes[0][1] = player;
	changed_cells_num = 1;
	cell_diffs_hist[move_row][move_col] = true;
	
	// Cells that can change their state on this turn (changed cells + their neighbours)
	int cells_to_process_num = 0;
	Coords cells_to_process;
	
	// [][][0] - 1st player's neighbours; [][][1] - 2nd player's
	int neighbours_count[BOARD_FULL_HEIGHT][BOARD_FULL_WIDTH][PLAYERS_NUM];
	int neigh_count_hist[BOARD_FULL_HEIGHT][BOARD_FULL_WIDTH][PLAYERS_NUM];

	int neighbours[8][2] = {{-1,-1},{-1,0},{-1,1},{0,-1},{0,1},{1,-1},{1,0},{1,1}};

	//-------DELETE AFTER DEBUG-----------------
	char* brd_str = "--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------w---------------------------w-w---------------------------ww-------------------------w--w-------------------------w----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------";
	bool print_details = false;
	Board brd_from_log;
	strToBoard(brd_str,brd_from_log);
	if(false){
	//if(new_h != NULL && !boardDiff(*board,brd_from_log)){
		print_details = true;
		//fprintf(stlog,"\n\nboard from log: \n\n");
		//printBoard(brd_from_log,stlog);
	}
	//------------------------------------------

	// simulation
	++simulations_done;
	for(int turn=0; turn<turns; turn++){
		cells_to_process_num = 0;
		memcpy(cell_added,cell_added_init,sizeof(bool)*BOARD_FULL_SIZE);

		// pick cells that need processing
		for(int i=0; i<changed_cells_num; i++){
			row = changed_cells[i][0];
			col = changed_cells[i][1];

			if(!cell_added[row][col]){
				cells_to_process[cells_to_process_num][0] = row;
				cells_to_process[cells_to_process_num][1] = col;
				cells_to_process_num++;
				cell_added[row][col] = true;
			}
			
			for(int n=0; n<8; ++n){
				n_row = row + neighbours[n][0];
				n_col = col + neighbours[n][1];
				if(!cell_added[n_row][n_col]){
					cells_to_process[cells_to_process_num][0] = n_row;
					cells_to_process[cells_to_process_num][1] = n_col;
					cells_to_process_num++;
					cell_added[n_row][n_col] = true;
				}
			}
		}

		//if(turn == 0){
		//	memcpy(neigh_count_hist,prev_h->neigh_origin,sizeof(neigh_count_hist));
		//	memcpy(neighbours_count,prev_h->neigh_origin,sizeof(neighbours_count));
		//	// calculate initial neighbours count
		//	for(int i=0; i<changed_cells_num; ++i){
		//		row = changed_cells[i][0];
		//		col = changed_cells[i][1];
		//		Cell cell = changes[i][1];
		//		int n_row, n_col;
		//		for(int n=0; n<8; ++n){
		//			n_row = row + neighbours[n][0];
		//			n_col = col + neighbours[n][1];
		//			if(cell == dead){
		//				Cell prev_owner = changes[i][0];
		//				--neighbours_count[n_row][n_col][prev_owner];
		//			}
		//			else{
		//				++neighbours_count[n_row][n_col][cell];
		//			}
		//		}
		//	}
		//	if(new_h != NULL){
		//		memcpy(new_h->neigh_origin,neighbours_count,sizeof(neighbours_count));
		//	}
		//}

		if(print_details){
			fprintf(stlog,"\n\n--------------------------------------------");
			fprintf(stlog,"\n\n----------------TURN: %d-----------------------",turn);
			fprintf(stlog,"\n\n--------------------------------------------");
			fprintf(stlog,"\n\nHistory:");
			printBrdAndNeighCnt(board_hist,neigh_count_hist,player,stlog);
			fprintf(stlog,"\n\nCurrent:");
			printBrdAndNeighCnt(*board,neighbours_count,player,stlog);
			fprintf(stlog,"1. changed_cells_num: %d \n\n",changed_cells_num);
			printCoords(changed_cells,changed_cells_num,stlog);
			fprintf(stlog,"1. cells_to_process_num: %d \n\n",cells_to_process_num);
			printCoords(cells_to_process,cells_to_process_num,stlog);
		}

		// retrieving cell changes from history
		for(int i=0; i < prev_h->changes_num[turn]; ++i){
			row = prev_h->changes_coords[turn][i][0];
			col = prev_h->changes_coords[turn][i][1];
			board_hist[row][col] = prev_h->changes[turn][i];
		}

		if(print_details){
			fprintf(stlog,"1. prev_h->changes_num: %d \n\n",prev_h->changes_num[turn]);
			printCoords(prev_h->changes_coords[turn],prev_h->changes_num[turn],stlog);
			fprintf(stlog,"\n\nHistory:\n\n");
			printBoard(board_hist,stlog);
		}

		// count neighbours of cells
		int tmp_neigh_cnt[BOARD_FULL_HEIGHT][BOARD_FULL_WIDTH][PLAYERS_NUM];
		memset(tmp_neigh_cnt,0,sizeof(tmp_neigh_cnt));
		for(int i=0; i<cells_to_process_num; i++){
			row = cells_to_process[i][0];
			col = cells_to_process[i][1];
			Cell cell;

			for(int n=0; n<8; ++n){
				n_row = row + neighbours[n][0];
				n_col = col + neighbours[n][1];
				cell = (*board)[n_row][n_col];
				if(cell == player_1){
					tmp_neigh_cnt[row][col][0]++; 
				}
				else if(cell == player_2){
					tmp_neigh_cnt[row][col][1]++;
				}
			}
		}
		memcpy(neighbours_count,tmp_neigh_cnt,sizeof(neighbours_count));

		// update cells
		changed_cells_num = 0;
		int total_changes_num = 0; // for new history
		memcpy(cell_changed,cell_added_init,sizeof(cell_changed));
		memcpy(cell_was_changed,cell_added_init,sizeof(cell_changed));
		ch_cells_num = 0;
		//was_changed_cells_num = 0;
		for(int i=0; i<cells_to_process_num;i++){
			row = cells_to_process[i][0];
			col = cells_to_process[i][1];
			Cell cell = (*board)[row][col];
			// killing alive cells
			if(cell != dead){
				if(neighbours_count[row][col][player_1] + neighbours_count[row][col][player_2] < 2 
					|| neighbours_count[row][col][player_1] + neighbours_count[row][col][player_2] > 3)
				{
					cell = dead;
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
					cell = owner;
				}
			}
			// checking if new cell state differs from history
			if(cell != board_hist[row][col]){
				changed_cells[changed_cells_num][0] = row;
				changed_cells[changed_cells_num][1] = col;
				changes[changed_cells_num][0] = board_hist[row][col];
				changes[changed_cells_num][1] = cell;
				changed_cells_num++;
				//cell_diffs_hist[row][col] = true;
			}
			// fill changes into new history
			if(new_h != NULL && cell != (*board)[row][col]){
				new_h->changes_coords[turn][total_changes_num][0] = row;
				new_h->changes_coords[turn][total_changes_num][1] = col;
				new_h->changes[turn][total_changes_num] = cell;
				++total_changes_num;

				/*ch_cells[ch_cells_num][0] = row;
				ch_cells[ch_cells_num][1] = col;
				cell_changed[row][col] = true;
				++ch_cells_num;*/
			}
			(*board)[row][col] = cell;
		}

		if(print_details){
			fprintf(stlog,"\n\n2. changed_cells_num: %d \n\n",changed_cells_num);
			printCoords(changed_cells,changed_cells_num,stlog);
			fprintf(stlog,"\n\n2. ch_cells_num: %d \n\n",ch_cells_num);
			printCoords(ch_cells,ch_cells_num,stlog);
			fprintf(stlog,"\n\nBoard after update:\n\n");
			printBoard(*board,stlog);
		}

		 //applying history info to the board
		for(int i=0; i < prev_h->changes_num[turn]; ++i){
			row = prev_h->changes_coords[turn][i][0];
			col = prev_h->changes_coords[turn][i][1];
			if(!cell_added[row][col]){ // take from history only unaffected cells
				(*board)[row][col] = prev_h->changes[turn][i];
			}
		}

		if(print_details){
			fprintf(stlog,"\n\nBoard after history applying:\n\n");
			printBoard(*board,stlog);
		}

		// add to the new history changes from old history
		if(new_h != NULL){
			for(int i=0; i<prev_h->changes_num[turn]; ++i){
				row = prev_h->changes_coords[turn][i][0];
				col = prev_h->changes_coords[turn][i][1];
				if(!cell_added[row][col]){ // take from history only unaffected cells
					new_h->changes_coords[turn][total_changes_num][0] = row;
					new_h->changes_coords[turn][total_changes_num][1] = col;
					new_h->changes[turn][total_changes_num] = prev_h->changes[turn][i];
					++total_changes_num;
				}
			}
			new_h->changes_num[turn] = total_changes_num;
			++(new_h->turns);
		}

		if(print_details){
			fprintf(stlog,"\n\nNeigh count before applying history:\n\n");
			printBrdAndNeighCnt(*board,neighbours_count,player,stlog);
		}

		//// retrieve neigh changes from history
		//for(int i=0; i<prev_h->neigh_changes_num[turn]; ++i){
		//	row = prev_h->neigh_changes_coords[turn][i][0];
		//	col = prev_h->neigh_changes_coords[turn][i][1];
		//	int player_num = prev_h->neigh_changes[turn][i][0];
		//	int neigh_cnt = prev_h->neigh_changes[turn][i][1];
		//	neigh_count_hist[row][col][player_num] = neigh_cnt;
		//}

		if(print_details){
			fprintf(stlog,"2. prev_h->neigh_changes_num: %d \n\n",prev_h->neigh_changes_num[turn]);
			printCoords(prev_h->neigh_changes_coords[turn],prev_h->neigh_changes_num[turn],stlog);
			fprintf(stlog,"\n\nHistory:\n\n");
			printNeighCnt(neigh_count_hist,player,stlog);
			fprintf(stlog,"\n\nNeigh count after applying history:\n\n");
			printBrdAndNeighCnt(*board,neighbours_count,player,stlog);
		}

		// copy neigh count for needed cells from history
		//memcpy(neighbours_count,neigh_count_hist,sizeof(neigh_count_hist));
		/*for(int i=0; i<cells_to_process_num; ++i){
			row = cells_to_process[i][0];
			col = cells_to_process[i][1];
			neighbours_count[row][col][0] = neigh_count_hist[row][col][0];
			neighbours_count[row][col][1] = neigh_count_hist[row][col][1];
		}*/

		//// calculate neigh changes
		//for(int i=0; i<changed_cells_num; ++i){
		//	row = changed_cells[i][0];
		//	col = changed_cells[i][1];
		//	Cell cell = changes[i][1];
		//	int n_row, n_col;
		//	for(int n=0; n<8; ++n){
		//		n_row = row + neighbours[n][0];
		//		n_col = col + neighbours[n][1];
		//		if(cell == dead){
		//			Cell prev_owner = changes[i][0];
		//			--neighbours_count[n_row][n_col][prev_owner];
		//		}
		//		else{
		//			++neighbours_count[n_row][n_col][cell];
		//		}
		//	}
		//}

		if(print_details){
			fprintf(stlog,"\n\nNeigh count after calculating:\n\n");
			printBrdAndNeighCnt(*board,neighbours_count,player,stlog);
		}

		//int prev_ctp_num = cells_to_process_num;
		//Coords prev_ctp;
		//memcpy(prev_ctp,cells_to_process,sizeof(prev_ctp));

		//// pick cells that need processing
		//cells_to_process_num = 0;
		//memcpy(cell_added,cell_added_init,sizeof(bool)*BOARD_FULL_SIZE);
		//for(int i=0; i<changed_cells_num; i++){
		//	row = changed_cells[i][0];
		//	col = changed_cells[i][1];

		//	if(!cell_added[row][col]){
		//		cells_to_process[cells_to_process_num][0] = row;
		//		cells_to_process[cells_to_process_num][1] = col;
		//		cells_to_process_num++;
		//		cell_added[row][col] = true;
		//	}
		//	
		//	for(int n=0; n<8; ++n){
		//		n_row = row + neighbours[n][0];
		//		n_col = col + neighbours[n][1];
		//		if(!cell_added[n_row][n_col]){
		//			cells_to_process[cells_to_process_num][0] = n_row;
		//			cells_to_process[cells_to_process_num][1] = n_col;
		//			cells_to_process_num++;
		//			cell_added[n_row][n_col] = true;
		//		}
		//	}
		//}

		//// fill neigh changes into new history
		//if(new_h != NULL){
		//	int neigh_changes_num = 0;
		//	int added_num = 0;
		//	// add neighs of ch_cells to the ch_cells queue
		//	for(int i=0; i<ch_cells_num; ++i){
		//		row = ch_cells[i][0];
		//		col = ch_cells[i][1];
		//		for(int n=0; n<8; ++n){
		//			n_row = row + neighbours[n][0];
		//			n_col = col + neighbours[n][1];
		//			if(!cell_changed[n_row][n_col] && !cell_added[n_row][n_col]){
		//				ch_cells[ch_cells_num+added_num][0] = n_row;
		//				ch_cells[ch_cells_num+added_num][1] = n_col;
		//				cell_changed[n_row][n_col] = true;
		//				++added_num;
		//			}
		//		}
		//	}
		//	ch_cells_num += added_num;
		//	// process full ch_cells queue
		//	for(int i=0; i<ch_cells_num; ++i){
		//		row = ch_cells[i][0];
		//		col = ch_cells[i][1];
		//			
		//		int p1_neigh_cnt = neighbours_count[row][col][0];
		//		new_h->neigh_changes_coords[turn][neigh_changes_num][0] = row;
		//		new_h->neigh_changes_coords[turn][neigh_changes_num][1] = col;
		//		new_h->neigh_changes[turn][neigh_changes_num][0] = player_1;
		//		new_h->neigh_changes[turn][neigh_changes_num][1] = p1_neigh_cnt;
		//		++neigh_changes_num;
		//			
		//		int p2_neigh_cnt = neighbours_count[row][col][1];
		//		new_h->neigh_changes_coords[turn][neigh_changes_num][0] = row;
		//		new_h->neigh_changes_coords[turn][neigh_changes_num][1] = col;
		//		new_h->neigh_changes[turn][neigh_changes_num][0] = player_2;
		//		new_h->neigh_changes[turn][neigh_changes_num][1] = p2_neigh_cnt;
		//		++neigh_changes_num;
		//	}

		//	if(print_details){
		//		fprintf(stlog,"\n\n3. neigh_changes_num: %d \n\n",neigh_changes_num);
		//		printCoords(new_h->neigh_changes_coords[turn],neigh_changes_num,stlog);
		//	}

		//	// copy from history changes of unaffected cells
		//	for(int i=0; i<prev_h->neigh_changes_num[turn]; ++i){
		//		row = prev_h->neigh_changes_coords[turn][i][0];
		//		col = prev_h->neigh_changes_coords[turn][i][1];
		//		if(!cell_added[row][col] && !cell_changed[row][col]){
		//			new_h->neigh_changes_coords[turn][neigh_changes_num][0] = row;
		//			new_h->neigh_changes_coords[turn][neigh_changes_num][1] = col;
		//			new_h->neigh_changes[turn][neigh_changes_num][0] = prev_h->neigh_changes[turn][i][0];
		//			new_h->neigh_changes[turn][neigh_changes_num][1] = prev_h->neigh_changes[turn][i][1];
		//			++neigh_changes_num;
		//		}
		//	}

		//	if(print_details){
		//		fprintf(stlog,"\n\n4. neigh_changes_num: %d \n\n",neigh_changes_num);
		//		printCoords(new_h->neigh_changes_coords[turn],neigh_changes_num,stlog);
		//	}

		//	for(int i=0; i<cells_to_process_num; ++i){
		//		row = cells_to_process[i][0];
		//		col = cells_to_process[i][1];

		//		if(cell_changed[row][col]){
		//			continue;
		//		}
		//		
		//		int p1_neigh_cnt = neighbours_count[row][col][0];
		//		new_h->neigh_changes_coords[turn][neigh_changes_num][0] = row;
		//		new_h->neigh_changes_coords[turn][neigh_changes_num][1] = col;
		//		new_h->neigh_changes[turn][neigh_changes_num][0] = player_1;
		//		new_h->neigh_changes[turn][neigh_changes_num][1] = p1_neigh_cnt;
		//		++neigh_changes_num;
		//			
		//		int p2_neigh_cnt = neighbours_count[row][col][1];
		//		new_h->neigh_changes_coords[turn][neigh_changes_num][0] = row;
		//		new_h->neigh_changes_coords[turn][neigh_changes_num][1] = col;
		//		new_h->neigh_changes[turn][neigh_changes_num][0] = player_2;
		//		new_h->neigh_changes[turn][neigh_changes_num][1] = p2_neigh_cnt;
		//		++neigh_changes_num;
		//	}

		//	if(print_details){
		//		fprintf(stlog,"\n\n5. neigh_changes_num: %d \n\n",neigh_changes_num);
		//		printCoords(new_h->neigh_changes_coords[turn],neigh_changes_num,stlog);
		//	}

		//	for(int i=0; i<prev_ctp_num; ++i){
		//		row = prev_ctp[i][0];
		//		col = prev_ctp[i][1];
		//		if(!cell_added[row][col] && !cell_changed[row][col]){
		//			int p1_neigh_cnt = neighbours_count[row][col][0];
		//			new_h->neigh_changes_coords[turn][neigh_changes_num][0] = row;
		//			new_h->neigh_changes_coords[turn][neigh_changes_num][1] = col;
		//			new_h->neigh_changes[turn][neigh_changes_num][0] = player_1;
		//			new_h->neigh_changes[turn][neigh_changes_num][1] = p1_neigh_cnt;
		//			++neigh_changes_num;
		//			
		//			int p2_neigh_cnt = neighbours_count[row][col][1];
		//			new_h->neigh_changes_coords[turn][neigh_changes_num][0] = row;
		//			new_h->neigh_changes_coords[turn][neigh_changes_num][1] = col;
		//			new_h->neigh_changes[turn][neigh_changes_num][0] = player_2;
		//			new_h->neigh_changes[turn][neigh_changes_num][1] = p2_neigh_cnt;
		//			++neigh_changes_num;
		//		}
		//	}
		//	if(print_details){
		//		fprintf(stlog,"\n\n6. neigh_changes_num: %d \n\n",neigh_changes_num);
		//		printCoords(new_h->neigh_changes_coords[turn],neigh_changes_num,stlog);
		//	}
		//	new_h->neigh_changes_num[turn] = neigh_changes_num;
		//}

		if(print_board){
			clearScreen();
			printBoard(*board,stdout);
		}

		// we came to the same position in history
		if(changed_cells_num == 0){
			// get last board from history and break simulation
			// also copy old history data to the new history
			// HINT: use memcpy here
			for(int t=turn+1; t<TURNS_NUM; ++t){
				// cells
				for(int i=0; i<prev_h->changes_num[t]; ++i){
					row = prev_h->changes_coords[t][i][0];
					col = prev_h->changes_coords[t][i][1];
					(*board)[row][col] = prev_h->changes[t][i];

					if(new_h != NULL){
						new_h->changes[t][i] = prev_h->changes[t][i];
						new_h->changes_coords[t][i][0] = row;
						new_h->changes_coords[t][i][1] = col;
					}
				}
				if(new_h != NULL){
					new_h->changes_num[t] = prev_h->changes_num[t];
				}
				// neigh count
				/*if(new_h != NULL){
					for(int i=0; i<prev_h->neigh_changes_num[t]; ++i){
						new_h->neigh_changes_coords[t][i][0] = prev_h->neigh_changes_coords[t][i][0];
						new_h->neigh_changes_coords[t][i][1] = prev_h->neigh_changes_coords[t][i][1];
						new_h->neigh_changes[t][i][0] = prev_h->neigh_changes[t][i][0];
						new_h->neigh_changes[t][i][1] = prev_h->neigh_changes[t][i][1];
					}
					new_h->neigh_changes_num[t] = prev_h->neigh_changes_num[t];
				}*/
			}
			if(new_h != NULL){
				new_h->turns = prev_h->turns;
			}
			break;
		}
	}

	if(new_h != NULL && count_player_cells(new_h->origin,player) > 10){
		//printHistory(new_h,true);
	}

	// simulation ended, return score
	// TODO: optimize. instead of calling function make counters above.
	int score = count_player_cells(*board,player) - count_player_cells(*board,enemy(player));
	return score;
}

int generate_moves(Board board, Cell player, Coords moves){
	return 0;
}

// rename this function to minimax and make a new nextMove that only calls minimax and prints best move
int nextMove(Cell player, Board* board, int prev_row, int prev_col, Tl* h,int ply, int max_ply){
	int max_score = -10000;
	int prev_score = 0;
	int cur_score = 0;
	int score = 0;
	int i,k;
	int best_i = prev_row;
	int best_k = prev_col;
	bool stop_search = false;

	Coords alive_cells;
	int alive_cells_num = getAliveCells(*board,alive_cells);

	// TODO: make issueOrder function
	// if playing white than first move goes to center to prevent black from mirroring your moves
	if(alive_cells_num == 0){
		best_i = BOARD_FULL_HEIGHT / 2;
		best_k = BOARD_FULL_WIDTH / 2;
#ifdef TESTING_MODE
		(*board)[best_i][best_k] = player;
#else
		printf("%d %d",best_i-1,best_k-1); // -1 because my board starts at 1
#endif
		return score;
	}

	Board board_copy;
	memcpy(&board_copy,board,sizeof(Board));

	Tl* prev_h;
	if(h == NULL){
		//prev_h = (Tl*) malloc(sizeof(Tl)*(max_ply+1));
		prev_h = (Tl*) malloc(sizeof(Tl)*(max_ply+2));
	}
	else{
		prev_h = h+1;
	}

	prev_h->turns = 0;
	for(int i=0; i<TURNS_NUM; ++i){
		prev_h->changes_num[i] = 0;
	}

	// simulate current state and fill history
	if(ply == 0){
	//if(true){
		prev_score = Simulate(&board_copy,player,TURNS_NUM,prev_h);
		//Simulate(&board_copy,player,TURNS_NUM,prev_h);
	}
	else{
		//Simulate_old(&board_copy,player,TURNS_NUM,false);
		prev_score = Simulate(&board_copy,prev_row,prev_col,player,TURNS_NUM,h,prev_h);
		// max_score = Simulate(&board_copy,prev_row,prev_col,player,TURNS_NUM,h,prev_h);
		
		/*Tl* h2 = h+2;
		h2->turns = 0;
		for(int i=0; i<TURNS_NUM; ++i){
			h2->changes_num[i] = 0;
		}
		Board brd;
		memcpy(&brd,board,sizeof(Board));
		int prev_score2 = Simulate(&brd,player,TURNS_NUM,h2);
		
		Board h_brd1; 
		Board h_brd2;
		int h_neigh1[BOARD_FULL_HEIGHT][BOARD_FULL_WIDTH][PLAYERS_NUM];
		int h_neigh2[BOARD_FULL_HEIGHT][BOARD_FULL_WIDTH][PLAYERS_NUM];
		
		memcpy(h_brd1,prev_h->origin,sizeof(Board));
		memcpy(h_brd2,h2->origin,sizeof(Board));
		memcpy(h_neigh1,prev_h->neigh_origin,sizeof(h_neigh1));
		memcpy(h_neigh2,h2->neigh_origin,sizeof(h_neigh2));

		if(boardDiff(h_brd1,h_brd2) || neighDiff(h_neigh1,h_neigh2)){
			printf("\n\n Shit happened (origins):\n\n");
			printBrdAndNeighCnt(h_brd1,h_neigh1,player,stdout);
			printBrdAndNeighCnt(h_brd2,h_neigh2,player,stdout);
			char* s;
			scanf("%s",&s);
		}
		
		for(int t=0; t<TURNS_NUM; ++t){
			getBoardFromHist(prev_h,t,h_brd1);
			getBoardFromHist(h2,t,h_brd2);
			getNeighFromHist(prev_h,t,h_neigh1);
			getNeighFromHist(h2,t,h_neigh2);

			if(boardDiff(h_brd1,h_brd2) || neighDiff(h_neigh1,h_neigh2)){
				clearScreen();
				printf("\n\n Shit happened (turn %d)",t);
				printf("\n\n Origin:");
				printBrdAndNeighCnt(prev_h->origin,prev_h->neigh_origin,player,stdout);
				printf("\n\n Fast simulation:");
				printBrdAndNeighCnt(h_brd1,h_neigh1,player,stdout);
				printf("\n\n Normal simulation:");
				printBrdAndNeighCnt(h_brd2,h_neigh2,player,stdout);
				printf("\n\n Board difference: \n\n");
				printBoardDiff(h_brd1,h_brd2,stdout);
				printf("\n\n Neigh difference: \n\n");
				printNeighDiff(h_neigh1,h_neigh2,stdout);
				printf("\n\n boardToStr: \n\n");
				char* s_brd;
				boardToStr(prev_h->origin,&s_brd);
				printf("%s",s_brd);
				printf("\n\n strToBoard: \n\n");
				Board bb;
				strToBoard(s_brd,bb);
				printBoard(bb,stdout);
				fprintf(stlog,"%s",s_brd);
				printf("Board string saved to the log: %s", log_file);
				char* s;
				scanf("%s",&s);
			}
		}*/
		//free(h2);
	}

	// search for a move
	i = prev_row;
	for(i; i<=BOARD_HEIGHT; i++){
		k = i>prev_row? prev_col : prev_col; // this row start from the prev_col, other rows from col=1
		for(k; k<=BOARD_WIDTH; k++){
			if((abs(i - prev_row > 1) || (abs(k - prev_col > 1))) && ply > 0)
			{
				break;
			}
			if(simulations_done >= max_simulations){
				stop_search = true;
				break;
			}
			if ((*board)[i][k] == dead){
				// make move
				memcpy(&board_copy,board,sizeof(Board));
				board_copy[i][k] = player;

				// process move
				if(ply < max_ply){
					score = nextMove(player,&board_copy,i,k,prev_h,ply+1,max_ply);
				}
				else{	
					score = Simulate(&board_copy,i,k,player,TURNS_NUM,prev_h,NULL);
					
					/*memcpy(&board_copy,board,sizeof(Board));
					board_copy[i][k] = player;
					int score2 = Simulate_old(&board_copy,player,TURNS_NUM,false);
					if(score != score2){
						printf("\nbug detected: new_sim score: %d    old_sim score: %d", score, score2);
						char* s;
						scanf("%s",&s);
						printf("simulation_old:");
						scanf("%s",&s);
						memcpy(&board_copy,board,sizeof(Board));
						board_copy[i][k] = player;
						Simulate_old(&board_copy,player,TURNS_NUM,true);
						scanf("%s",&s);
						printf("simulation new:");
						scanf("%s",&s);
						memcpy(&board_copy,board,sizeof(Board));
						board_copy[i][k] = player;
						Simulate(&board_copy,i,k,player,TURNS_NUM,prev_h,new_h, true);
						scanf("%s",&s);

					}*/
				}
				//clearScreen();

				// unmake move
				//board[i][k] = dead;

				if(score > max_score){
                	max_score = score;
                    best_i = i;
                    best_k = k;
					/*if(score >= prev_score && ply != 0){
						stop_search = true;
						break;
					}*/
                }
			}
			if(stop_search){
				break;
			}
		}
	}
#ifdef TESTING_MODE
	if(ply == 0){
		(*board)[best_i][best_k] = player;
	}
#else
	if(ply == 0){
		printf("%d %d",best_i-1,best_k-1); // -1 because my board starts at 1
	}
#endif
	if(ply == 0){
		free(prev_h);
	}
	return max_score;
}

int main(){
    int i,k;
    char c;
    Board board;

	//----PRODUCTION CODE-----
#ifndef DEBUG_MODE 
#ifndef TESTING_MODE
    scanf("%c\n", &c);
	player = cell_of_char(c);

	//initBoard(empty_board);
	initBoard(board);

    for(i=1; i<=BOARD_HEIGHT; i++){
		for (k=1; k<=BOARD_WIDTH; k++){
			scanf("%c", &c);
			board[i][k] = cell_of_char(c);
		}
		scanf("\n");
	}

	nextMove(player,&board,1,1,NULL,0,40-count_player_cells(board,player)-1 < 1? 0 : 1);
#endif
#endif

#ifdef DEBUG_MODE
	char* s;
	//s = "-----------------------------wwww-------------------------wwww---------------------------------------------------------------------------------------wwww---------------------------wwwww------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------bbbb---------------------------bbbb---------------------------bbbb---------------------------bbbbb------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------";
    for(i=0; i<BOARD_HEIGHT; i++){
		for (k=0; k<BOARD_WIDTH; k++){
			board[i][k] = cell_of_char(s[i*BOARD_WIDTH + k]);
		}
	}

	printBoard(board);
	scanf("%s",&s);
	printf("\n\n");

	Simulate(board,player_1,500);
	
	scanf("%s",&s);
#endif

#ifdef TESTING_MODE
	stlog = fopen(log_file,"w");
	setbuf(stlog,NULL);
	char* s;

	//initBoard(empty_board);
	initBoard(board);
	printBoard(board,stdout);
	
	// getting moves
	for(i=0; i<CELLS_TO_PLACE; i++){
		simulations_done = 0;
		nextMove(player_1,&board,1,1,NULL,0,CELLS_TO_PLACE-count_player_cells(board,player_1) < 2? 0 : 1);
		clearScreen();
		printBoard(board,stdout);
		printf("Simulations done: %d", simulations_done);
		//scanf("%s",&s);
	}
	
	scanf("%s",&s);
	printf("\n\n");
	
	Board board_copy;
	memcpy(&board_copy,board,sizeof(Board));

	Coords alive_cells;
	Tl* h = (Tl*) malloc(sizeof(Tl));
	int alive_cells_num = getAliveCells(board,alive_cells);
	int score = Simulate(&board,player,TURNS_NUM,h);
	//int score = Simulate_old(&board_copy,player,TURNS_NUM,true);
	
	// print history
	printHistory(h,stdout);
	printf("\n\nscore: %d\n\n", score);
	
	scanf("%s",&s);
	fclose(stlog);
#endif

	return 0;
}