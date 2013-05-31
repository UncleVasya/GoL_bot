#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "main.h"

//#define PRINT_GENERATED

void binToStr(unsigned n, char* str, int strSize = 32){
	itoa (n, str, 2);
	int len = strlen(str);
	memcpy(&str[strSize - strlen(str)] ,str, len+1);
	memset(str, '0', strSize - len);
}

void SimListLife(){
	unsigned bitmap;
	int popcount[1 << 4] = { 0, 1, 1, 2,  1, 2, 2, 3,  1, 2, 2, 3,  2, 3, 3, 4 };
	
	for(int bitmap=0; bitmap < 1<<9; ++bitmap){
		int bitmapAnd15 = bitmap & 15;
		int bitmapRShift5 = bitmap >> 5;
		int neighbours = popcount[bitmapAnd15] + popcount[bitmapRShift5];
#ifdef PRINT_GENERATED
		char str[33];
		binToStr(bitmap, str, 9);
		fprintf(stream,"     bitmap: %3u    %s\n", bitmap, str);
		binToStr(bitmapAnd15, str, 9);
		fprintf(stream,"bitmap & 15: %3u    %s\n", bitmapAnd15, str);
		binToStr(bitmapRShift5, str, 9);
		fprintf(stream,"bitmap >> 5: %3u    %s\n", bitmapAnd15, str);
		fprintf(stream,"\n neighbours: popcount[%u] + popcount[%u] = %u + %u = %u\n\n\n", 
				bitmapAnd15, bitmapRShift5, popcount[bitmapAnd15], 
				popcount[bitmapRShift5], neighbours);
#endif
	}
}