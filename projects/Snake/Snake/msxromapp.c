// ----------------------------------------------------------
//		msxromapp.c - by Danilo Angelo, 2020
//
//		ROM program(cartridge) for MSX example
//		C version
// ----------------------------------------------------------

#include <stdbool.h>
#include "targetconfig.h"
#include "MSX/BIOS/msxbios.h"
#include "msx_fusion.h"
#include "screens.h"

#define NAMETABLE	0x1800

unsigned char x, y; //snake coordinates
unsigned char joy; //joystick value
bool EoG; //end of game control
unsigned char content;
unsigned int lastJiffy;

// ----------------------------------------------------------
//	This is an example of embedding asm code into C.
//	This is only for the demo app.
//	You can safely remove it for your application.
#pragma disable_warning 85	// because the var msg is not used in C context

void _print(char* msg) {
#ifndef __INTELLISENSE__
__asm
	ld      hl, #2; retrieve address from stack
	add     hl, sp
	ld		b, (hl)
	inc		hl
	ld		h, (hl)
	ld		l, b

_printMSG_loop :
	ld		a, (hl)	; print
	or a
	ret z
	push	hl
	push	ix
	ld		iy, (BIOS_ROMSLT)
	ld		ix, #BIOS_CHPUT
	call	BIOS_CALSLT
	ei				; in some MSXs (i.e. F1XV) CALSLT returns with di.
	pop		ix
	pop		hl
	inc		hl
	jr		_printMSG_loop
__endasm;
#endif
	return;
}

// ----------------------------------------------------------
//	This is an example of using debug code in C.
//	This is only for the demo app.
//	You can safely remove it for your application.
void print(char* msg) {
#ifdef DEBUG
	_print("[DEBUG]");
#endif
	_print(msg);
	return;
}

void title() {
	Cls();
	print(titleScreen);
	InputChar();
}

void game() {
	Cls();
	print(gameScreen);

	// Initialize game variables
	x = 10;
	y = 10;
	EoG = false;
	Pokew(BIOS_JIFFY, 0);

	// Game's main loop
	while (!EoG) {

		//wait until BIOS changes the value of Jiffy
		while (lastJiffy == Peekw(BIOS_JIFFY)) {}
		
		//from this point, 1 pass per frame
		if (Peekw(BIOS_JIFFY) == 15)
		{

			joy = JoystickRead(CURSORKEYS); //read the joystick value

			switch (joy) {
			case UP:
				y--;
				break;
			case RIGHT:
				x++;
				break;
			case DOWN:
				y++;
				break;
			case LEFT:
				x--;
				break;
			}

			content = Vpeek(NAMETABLE + y * 32 + x);
			EoG = ((joy > 0) && (content != ' '));
			Locate(x, y);
			print("*");

			Pokew(BIOS_JIFFY, 0);
		}

		//sound routinne (1 execution per frame)
		{}

		lastJiffy = Peekw(BIOS_JIFFY);
	}

	InputChar();
}

void gameOver() {
	Locate(0, 10);
	print(gameOverMsg);
	InputChar();
}

void main(void) {
	Screen(1);
	Width(32);
	SetColors(255, 0, 0);
	
	//game infinite loop
	while (true) {
		title();
		game();
		gameOver();
	}
}

