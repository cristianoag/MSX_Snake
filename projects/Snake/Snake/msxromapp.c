//snake game for msx

#include <stdbool.h>
#include "targetconfig.h" 
#include "MSX/BIOS/msxbios.h" //bios definitions
#include "msx_fusion.h" //fusion c
#include "screens.h" //game screens

#define NAMETABLE	0x1800  //default initial address for the nametable (6144 in decimal)

unsigned char x, y, direction; //snake coordinates and direction
unsigned char joy; //joystick value
bool EoG; //end of game control
unsigned char content; //used to get the current position of the snake
unsigned int lastJiffy; //stores the last bios JIFFY

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

// print function
void print(char* msg) {
#ifdef DEBUG
	_print("[DEBUG]");
#endif
	_print(msg);
	return;
}

//prints the game title screen
void title() {
	Cls(); //clear the screen
	print(titleScreen); //print the title screen
	while (Inkey() > 0) {}
	InputChar(); //wait for a key
}

//main game routine
void game() {
	Cls(); //clear the screen
	print(gameScreen); //print the game screen

	// Initialize game variables
	x = 10; //initial snake position x
	y = 10; //initial snake position y
	direction = RIGHT; //starting direction is right

	EoG = false; //the game is starting
	Pokew(BIOS_JIFFY, 0); //initialize JIFFY to 0

	// Game's main loop
	while (!EoG) {

		//wait until BIOS changes the value of Jiffy
		while (lastJiffy == Peekw(BIOS_JIFFY)) {}
		
		//read the keyboard
		joy = JoystickRead(CURSORKEYS); //read the keyboard value
		//logic to avoid the issue with pushing opposite direction
		if (((joy == UP) && (direction != DOWN)) || 
			((joy == RIGHT) && (direction != LEFT)) || 
			((joy == DOWN) && (direction != UP)) || 
			((joy == LEFT) && (direction != RIGHT)))
		{
			direction = joy;
		}

		//from this point, 1 pass each 15 frames
		if (Peekw(BIOS_JIFFY) == 15)
		{
			//check the direction being pushed
			switch (direction) {
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

			//get the current position of the snake
			content = Vpeek(NAMETABLE + y * 32 + x);
			//check if the snake is being moved by the user and if it is not colliding
			EoG = (content != ' ');
			
			//move the snake and print
			Locate(x, y);
			print("*");

			//reset the JIFFY to 0
			Pokew(BIOS_JIFFY, 0);
		}

		//from this point, 1 pass per frame
		{}

		//get the current JIFFY and put it on lastJiffy
		lastJiffy = Peekw(BIOS_JIFFY);
	}

	Beep();
	Pokew(BIOS_JIFFY, 0); //reset bios jiffy
	while (Peek(BIOS_JIFFY) < 40) {} //wait 40 cycles
}

//game over routine
void gameOver() {
	Locate(0, 10);
	print(gameOverMsg); //print the game over message
	while (Inkey() > 0) {} 
	InputChar(); //wait for the user
}

//main loop for the game
void main(void) {
	Screen(1); //sets screen mode 1
	Width(32); //ensures 32 columns
	SetColors(255, 0, 0); //set the color for the screen 
	
	//game infinite loop
	while (true) {
		title(); //title
		game(); //game
		gameOver(); //game over
	}
}

