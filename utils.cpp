#ifdef _WIN32 
	#include <windows.h>
#endif

// sleep function
void sleep(unsigned milliseconds){
	#ifdef _WIN32 
		Sleep(milliseconds);
	#endif
}

void clearScreen(){
	system("cls");
}