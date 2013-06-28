#include "Board.h"
#include "Player.h"

Cell enemy(Cell player){
	switch(player){
		case player_1: return player_2;
		case player_2: return player_1;
	}
}