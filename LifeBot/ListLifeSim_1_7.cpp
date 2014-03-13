#include <stdio.h>
#include "ListLife.h"

//#define DEBUG_MODE

void life_1_7(int *this_gen, int *new_gen)
{
	char* s;
	unsigned bitmap;
	int *next, *prev;
	int x, y;
	static enum {
		DEAD, LIVE
	} state[1 << 9];

	if(state[007] == 0) {
		for(bitmap = 0; bitmap < 1<<9; bitmap++) {
			unsigned bitmap_no_center = bitmap & ~020; // don't count center as a neighbour
			for(x = y = 0; y < 9; y++)
				if(bitmap_no_center & 1<<y)
					x += 1;
			if(bitmap & 020) {
				if(x == 2 || x == 3)
					state[bitmap] = LIVE;
				else
					state[bitmap] = DEAD;
			} else {
				if(x == 3)
					state[bitmap] = LIVE;
				else
					state[bitmap] = DEAD;
			}
		}
	}

	prev = next = this_gen;
	bitmap = 0;
	*new_gen = 0;
	for(;;) {
		/* did we write an X co-ordinate? */
		if(*new_gen < 0)
			new_gen++;
		if(prev == next || y<=0) {
			/* start a new group of rows */
			if(*next == 0) {
				*new_gen = 0;
				return;
			}
			y = *next++ + 1;
		} else {
			/* move to next row and work out which ones to scan */
			if(*prev == y-- && *prev != 0)
				prev++;
			if(*this_gen == y && *this_gen != 0)
				this_gen++;
			if(*next == y-1 && *next != 0)
				next++;
		}
        *new_gen = y; // write new row co-ordinate
		for(;;) {
			/* skip to the leftmost cell */
			x = *prev;
			if(x > *this_gen)
				x = *this_gen;
			if(x > *next)
				x = *next;
			/* end of line? */
			if(x >= 0)
				break;
			for(;;) {
				#ifdef DEBUG_MODE
					fprintf(stdout,"\n\n"
								   "x        : %3d \n"
								   "y        : %3d \n"
								   "prev     : %3d \n"
								   "this_gen : %3d \n"
								   "next     : %3d \n"
								   "new_gen  : %3d \n"
								   "cell_x   : %3d \n"
								   "cell_y   : %3d \n",
								   x, y, *prev, *this_gen, 
								   *next, *new_gen, -(x-1), y);
					char str[33];
					binToStr(bitmap, str, 9);
					fprintf(stdout,"\n\nbitmap before:");
					print_3x3(str);
				#endif
				/* add a column to the bitmap */
				if(x < 0){
					if(*prev == x) {
						bitmap |= 0100;
						prev++;		
						#ifdef DEBUG_MODE
							binToStr(bitmap, str, 9);
							fprintf(stdout,"\n\n|= 0100:");
							print_3x3(str);
						#endif
					}
					if(*this_gen == x) {
						bitmap |= 0200;
						this_gen++;
						#ifdef DEBUG_MODE
							binToStr(bitmap, str, 9);
							fprintf(stdout,"\n\n= 0200:");
							print_3x3(str);
						#endif
					}
					if(*next == x) {
						bitmap |= 0400;
						next++;
						#ifdef DEBUG_MODE
							binToStr(bitmap, str, 9);
							fprintf(stdout,"\n\n|= 0400:");
							print_3x3(str);
						#endif
					}
				}
				/* what does this bitmap indicate? */
				if(state[bitmap] == LIVE && -x < BOARD_WIDTH && y <= BOARD_HEIGHT)
					*++new_gen = x - 1;
				else if(bitmap == 000)
					break;
				/* move right */
				bitmap >>= 3;
				x += 1;
				#ifdef DEBUG_MODE
					binToStr(bitmap, str, 9);
					fprintf(stdout,"\n\n>= 3");
					print_3x3(str);
					//scanf("%s",&s);
				#endif
			}
		}
	}
}
#ifdef DEBUG_MODE
	#undef DEBUG_MODE
#endif