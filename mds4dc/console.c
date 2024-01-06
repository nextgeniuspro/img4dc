#include "console.h"

// uniquement pour Windows
#ifdef WIN32

#include <windows.h>
#include <wincon.h>
	
// changer la couleur � l'emplacement du curseur
void textColor(int color) {
	int bkgnd = 0; // noir
    SetConsoleTextAttribute(GetStdHandle (STD_OUTPUT_HANDLE), color + (bkgnd << 4));
}

// aller � la position (X;Y) dans une appli Win32 Console
void gotoXY(uint32_t x, uint32_t y) {
	COORD pos;
	pos.X = x;
	pos.Y = y;
	SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), pos);
}

// donne la position X du curseur
uint32_t whereX() {
    CONSOLE_SCREEN_BUFFER_INFO info;

    GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &info);
    return (uint32_t)info.dwCursorPosition.X;
}

// donne la position Y du curseur
uint32_t whereY() {
    CONSOLE_SCREEN_BUFFER_INFO info;

    GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &info);
    return (uint32_t)info.dwCursorPosition.Y;
}

#else

// pour les autres syst�mes d'exploitation

#include "tools.h"
#include <curses.h>

void textcolor(int attr, int fg, int bg) {	
	char command[13];

	//Command is the control command to the terminal
	sprintf(command, "%c[%d;%d;%dm", 0x1B, attr, fg + 30, bg + 40);
	printf("%s", command);
}

void textColor(int color) {
	textcolor(0, color, WHITE);
}

int gotoXY(uint32_t x, uint32_t y) {
	// rien
}

uint32_t whereX() {
}

uint32_t whereY() {
}

#endif
