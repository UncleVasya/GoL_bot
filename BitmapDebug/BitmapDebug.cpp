#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//#define PRINT_GENERATED

const char* log_file = "log.txt";
FILE* stlog = fopen(log_file,"w");
FILE* stream;

enum CellState {DEAD, LIVE};

void binToStr(unsigned n, char* str, int strSize = 32){
	itoa (n, str, 2);
	int len = strlen(str);
	memcpy(&str[strSize - strlen(str)] ,str, len+1);
	memset(str, '0', strSize - len);
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

void gen_1_4(CellState st[1 << 9]){
	unsigned bitmap;
	int popcount[1 << 4] = { 0, 1, 1, 2,  1, 2, 2, 3,  1, 2, 2, 3,  2, 3, 3, 4 };
	enum {DEAD, LIVE} state[1 << 9];
	
	for(int bitmap=0; bitmap < 1<<9; ++bitmap){
		int bitmapAnd15 = bitmap & 15;
		int bitmapRShift5 = bitmap >> 5;
		int neighbours = popcount[bitmapAnd15] + popcount[bitmapRShift5];

#ifdef PRINT_GENERATED
		char str[33];
		binToStr(bitmap, str, 9);
		fprintf(stream,"bitmap: %3u    %s\n", bitmap, str);
		print_3x3(str);
		
		binToStr(bitmapAnd15, str, 9);
		fprintf(stream,"bitmap & 15: %3u    %s\n", bitmapAnd15, str);
		print_3x3(str);
		
		binToStr(bitmapRShift5, str, 9);
		fprintf(stream,"bitmap >> 5: %3u    %s\n", bitmapRShift5, str);
		print_3x3(str);
		
		fprintf(stream,"\n neighbours: popcount[%u] + popcount[%u] = %u + %u = %u\n\n\n", 
				bitmapAnd15, bitmapRShift5, popcount[bitmapAnd15], 
				popcount[bitmapRShift5], neighbours);

		/*char* s;
		scanf("%s",&s);*/
#endif

		if(bitmap & 020) {
			if(neighbours == 2 || neighbours == 3)
				state[bitmap] = LIVE;
			else
				state[bitmap] = DEAD;
		} 
		else {
			if(neighbours == 3)
				state[bitmap] = LIVE;
			else
				state[bitmap] = DEAD;
		}
	}
	memcpy(st,state,sizeof(state));
}

void gen_1_8(CellState st[1 << 9]){
	char* s;
	unsigned bitmap, bitmap_no_center;
	int x, y;
	enum {DEAD, LIVE} state[1 << 9];

	for(bitmap = 0; bitmap < 1<<9; bitmap++) {
		bitmap_no_center = bitmap & ~020; // don't count center as a neighbour
		for(x = y = 0; y < 9; y++)
			if(bitmap_no_center & 1<<y)
				x += 1;
			if(bitmap & 020) {
				if(x == 2 || x == 3)
					state[bitmap] = LIVE;
				else
					state[bitmap] = DEAD;
			} 
			else {
				if(x == 3)
					state[bitmap] = LIVE;
				else
					state[bitmap] = DEAD;
			}
	}
	memcpy(st,state,sizeof(state));
}

void test_LL_gens(){
	stream = stdout;
	char str[33];

	enum CellState state1[1 << 9];
	enum CellState state2[1 << 9];

	gen_1_4(state1);
	gen_1_8(state2);

	// check gens to be equal
	for(unsigned bitmap = 0; bitmap < 1<<9; ++bitmap){
		printf("\n\n\nBitmap: %u", bitmap);
		binToStr(bitmap, str, 9);
		print_3x3(str);
		printf("state1[%u]: %d     state2[%u]: %d\n", 
				bitmap, state1[bitmap], bitmap, state2[bitmap]);
		if(state1[bitmap] != state2[bitmap]){
			printf("\nShit happened!\n");
			scanf("%s", str);
		}
		//scanf("%s", str);
	}

	scanf("%s", &str);
}

void main(){
	test_LL_gens();
}