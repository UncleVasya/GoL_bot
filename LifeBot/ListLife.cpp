#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ListLife.h"
#include "Board.h"
#include "BoardPrint.h"
#include "Player.h"

FILE* stream = stdout;

void binToStr(unsigned n, char* str, int strSize = 32){
	itoa (n, str, 2);
	int len = strlen(str);
	memcpy(&str[strSize - strlen(str)] ,str, len+1);
	memset(str, '0', strSize - len);
}

void llToBoard(int* ll, Board board){
	EmptyBoard(board);
	int row, col;
	while (*ll != 0){
		if (*ll > 0) // row value
			row = *ll++;
		if (*ll < 0){ // col value
			col = -(*ll++);
			board[row][col] = player_1;
		}
	}
}

void boardToLL(Board board, int* ll){
	for (int row=BOARD_HEIGHT; row>0; --row){
        *ll = row;
        for (int col=BOARD_WIDTH; col>0; --col){
            if (board[row][col] == player_1){
                *(++ll) = -col;
            }
        }
        if (*ll < 0) ++ll;
    }
    *ll = 0;
}

void printLL(int* ll){
    do{
        printf("%3d ", *ll);
    } while (*ll++);
}

bool llDiff(int* ll_1, int* ll_2){
    do{
        if (*ll_1++ != *ll_2++) return true;
    } while (*ll_1 || *ll_2);
    return false;
}

void print_3x3(char* str){
	fprintf(stream, "\n"
					"%c%c%c \n"
					"%c%c%c \n"
					"%c%c%c \n", 
					str[0], str[3], str[6],
					str[1], str[4], str[7],
					str[2], str[5], str[8]);
}

void testListLife(){
	Board board;
	char* s;

    int test_boards[][MAX_LL_SIZE] = {
                                        {2,-2, 1,-3,-2,-1, 0},
                                        {4,-5, 3,-4,-3, 0},
                                        {4,-4, 3,-5,-4,-3, 0},
		                                {6,-4, 5,-4,-3, 0},
                                        {29,-29,-28,-27, 28,-28, 0}
                                    };
    for (int i=0; i<4; ++i){
        int* ll_1 = test_boards[i];
        llToBoard(ll_1, board);
        int ll_2[MAX_LL_SIZE];
        boardToLL(board, ll_2);
        
        printf("\n\nTest %d:", i);
        printf("\nll_1: "); printLL(ll_1);
        printf("\nll_2: "); printLL(ll_2);
        
        if (llDiff(ll_1, ll_2)){
            printf("\n\nShit happened!");
            scanf("%s", &s);
        }
    }
    scanf("%s", &s);

    int ll_boards[2][MAX_LL_SIZE] = //{2,-2, 1,-3,-2,-1, 0};
                                    //{4,-5, 3,-4,-3, 0};
                                    //{4,-4, 3,-5,-4,-3, 0};
		                            //{6,-4, 5,-4,-3, 0};
                                    {29,-29,-28,-27, 28,-28, 0};
    int *prev = ll_boards[0];
    int *next = ll_boards[1];
    
    llToBoard(prev, board);
	printBoard(board, stdout);
	printf("\n\n");
	scanf("%s", &s);

    for(int turn=0; turn<TURNS_NUM; ++turn){
        life_1_7(prev, next);
        
        llToBoard(next, board);
	    printBoard(board, stdout);
	    scanf("%s", &s);
        
        // swap boards
        int *tmp = prev;
        prev = next;
        next = tmp;
    }
	
    llToBoard(next, board);
	printBoard(board, stdout);
	scanf("%s", &s);
}