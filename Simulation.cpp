#include <string.h>

#include "Board.h"
#include "BoardPrint.h"
#include "SimulationHistory.h"
#include "Player.h"
#include "utils.h"

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
	for(int turn=0; turn<turns; turn++){
		changed_cells_num = 0;

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
		cells_to_process_num = addNeighboursCoords(&cur_alive_cells, alive_cells_num, &cells_to_process);
		memcpy(cell_added,cell_added_init,sizeof(bool)*BOARD_FULL_SIZE);
		for(int i=0; i<cells_to_process_num; ++i){
			row = cells_to_process[i][0];
			col = cells_to_process[i][1];
			cell_added[row][col] = true;
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
			 Tl* prev_h, Tl* new_h, bool print_board)
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

	Coords alive_cells;
	bool cell_alive[BOARD_FULL_HEIGHT][BOARD_FULL_WIDTH];
	int alive_cells_num;
	
	Coords changed_cells;
	Cell changes[BOARD_FULL_SIZE*2][2]; // [0] - was; [1] - became
	int changed_cells_num;
	
	// before simulation starts we have only one changed cell - players move
	changed_cells[0][0] = move_row;
	changed_cells[0][1] = move_col;
	changes[0][0] = dead;
	changes[0][1] = player;
	changed_cells_num = 1;
	
	// Cells that can change their state on this turn (changed cells + their neighbours)
	int cells_to_process_num = 0;
	Coords cells_to_process;
	
	// [][][0] - 1st player's neighbours; [][][1] - 2nd player's
	int neighbours_count[BOARD_FULL_HEIGHT][BOARD_FULL_WIDTH][PLAYERS_NUM];
	int neigh_count_hist[BOARD_FULL_HEIGHT][BOARD_FULL_WIDTH][PLAYERS_NUM];

	int neighbours[8][2] = {{-1,-1},{-1,0},{-1,1},{0,-1},{0,1},{1,-1},{1,0},{1,1}};

	// simulation
	for(int turn=0; turn<turns; turn++){
		// pick cells that need processing
		cells_to_process_num = addNeighboursCoords(&changed_cells, changed_cells_num, &cells_to_process);
		memcpy(cell_added,cell_added_init,sizeof(bool)*BOARD_FULL_SIZE);
		for(int i=0; i<cells_to_process_num; ++i){
			row = cells_to_process[i][0];
			col = cells_to_process[i][1];
			cell_added[row][col] = true;
		}
		// level 2 neighbours
		int added_num = 0;
		for(int i=0; i<cells_to_process_num; ++i){
			row = cells_to_process[i][0];
			col = cells_to_process[i][1];
			for(int n=0; n<8; ++n){
				n_row = row + neighbours[n][0];
				n_col = col + neighbours[n][1];
				if(!cell_added[n_row][n_col]){
					cells_to_process[cells_to_process_num+added_num][0] = n_row;
					cells_to_process[cells_to_process_num+added_num][1] = n_col;
					++added_num;
					cell_added[n_row][n_col] = true;
				}
			}
		}
		cells_to_process_num += added_num;

		alive_cells_num = 0;
		for(int i=0; i<cells_to_process_num; ++i){
			row = cells_to_process[i][0];
			col = cells_to_process[i][1];
			Cell cell = (*board)[row][col];
			if(cell != dead){
				alive_cells[alive_cells_num][0] = row;
				alive_cells[alive_cells_num][1] = col;
				++alive_cells_num;
			}
		}

		// retrieving cell changes from history
		for(int i=0; i < prev_h->changes_num[turn]; ++i){
			row = prev_h->changes_coords[turn][i][0];
			col = prev_h->changes_coords[turn][i][1];
			board_hist[row][col] = prev_h->changes[turn][i];
		}

		// count neighbours of cells
		memset(neighbours_count,0,sizeof(neighbours_count));
		for(int i=0; i<alive_cells_num; i++){
			row = alive_cells[i][0];
			col = alive_cells[i][1];
			Cell cell = (*board)[row][col];
			if(cell != dead){
				// if cell's not dead update neigh cnt around it
				for(int n=0; n<8; ++n){
					n_row = row + neighbours[n][0];
					n_col = col + neighbours[n][1];
					++neighbours_count[n_row][n_col][cell]; 
				}
			}
		}
		// clean up. removing 2-lvl neighs from processing queue
		for(int i = cells_to_process_num-1; i >= cells_to_process_num-added_num; --i){
			row = cells_to_process[i][0];
			col = cells_to_process[i][1];
			cell_added[row][col] = false;
		}
		cells_to_process_num -= added_num;

		// update cells
		changed_cells_num = 0;
		int total_changes_num = 0; // for new history
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
			}
			// fill changes into new history
			if(new_h != NULL && cell != (*board)[row][col]){
				new_h->changes_coords[turn][total_changes_num][0] = row;
				new_h->changes_coords[turn][total_changes_num][1] = col;
				new_h->changes[turn][total_changes_num] = cell;
				++total_changes_num;
			}
			(*board)[row][col] = cell;
		}

		 //applying history info to the board
		for(int i=0; i < prev_h->changes_num[turn]; ++i){
			row = prev_h->changes_coords[turn][i][0];
			col = prev_h->changes_coords[turn][i][1];
			if(!cell_added[row][col]){ // take from history only unaffected cells
				(*board)[row][col] = prev_h->changes[turn][i];
			}
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

		if(print_board){
			clearScreen();
			printBoard(*board,stdout);
		}

		// we came to the same position in history
		if(changed_cells_num == 0){
			// copy rest of the old history to the new history
			if(new_h != NULL){
				for(int t=turn+1; t<TURNS_NUM; ++t){
					// cells
					memcpy(new_h->changes[t],prev_h->changes[t],sizeof(Cell)*2*prev_h->changes_num[t]);
					memcpy(new_h->changes_coords[t],prev_h->changes_coords[t],sizeof(int)*2*prev_h->changes_num[t]);
					
				}
				memcpy(&(new_h->changes_num[turn+1]),&(prev_h->changes_num[turn+1]),sizeof(int)*(TURNS_NUM-turn-1));
				new_h->turns = prev_h->turns;

			}
			// get last board from history and break simulation
			for(int t=turn+1; t<TURNS_NUM; ++t){
				for(int i=0; i<prev_h->changes_num[t]; ++i){
					row = prev_h->changes_coords[t][i][0];
					col = prev_h->changes_coords[t][i][1];
					(*board)[row][col] = prev_h->changes[t][i];
				}
			}
			break;
		}
	}
	// simulation ended, return score
	// TODO: optimize. instead of calling function make counters above.
	int score = count_player_cells(*board,player) - count_player_cells(*board,enemy(player));
	return score;
}