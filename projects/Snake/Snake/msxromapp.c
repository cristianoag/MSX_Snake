//snake game for msx

#include <stdbool.h>
#include <stdlib.h>
#include "targetconfig.h" 
#include "MSX/BIOS/msxbios.h" //bios definitions
#include "msx_fusion.h" //fusion c
#include "screens.h" //game screens

#define NAMETABLE	0x1800  //default initial address for the nametable (6144 in decimal)

//defines for the tiles that will be used for the game
#define TILE_GRASS		' '
#define TILE_SNAKETAIL	'o'
#define TILE_SNAKEHEAD	'*'
#define TILE_APPLE		'#'

unsigned int snakeHeadPos; //snake head position
unsigned char direction, lastDirection; //snake direction and lastknown direction
unsigned char joy; //joystick value
bool EoG; //end of game control
unsigned char content; //used to get the current position of the snake
unsigned int lastJiffy; //stores the last bios JIFFY

unsigned int snake[512]; //array to store the snake body
unsigned int *snakeHead, *snakeTail; //pointers to the snake head and tail
unsigned int applePos; //position of the random apple
unsigned char growth; //controls the snake growth

unsigned char bonus;
unsigned int score;
unsigned int highscore;


//asm function to print a char in the screen
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
	_print(titleScreen); //print the title screen

	while (Inkey() > 0) {}
	InputChar(); //wait for a key
}

//drop an apple to the a garden free space
void dropApple()
{
	do {
		applePos = NAMETABLE + 32 + rand() % (32 * 21);

	} while (Vpeek(applePos) != TILE_GRASS);

	Vpoke(applePos, TILE_APPLE);
	
}

//main game routine
void game() {
	srand(Peekw(BIOS_JIFFY));

	Cls(); //clear the screen
	_print(gameScreen); //print the game screen

	//initialize game variables
	score = 0;
    
	//print the highscore
	Locate(18, 23);
	PrintNumber(highscore);

	snakeHeadPos = NAMETABLE + 10 * 32 + 11;
	direction = RIGHT; //starting direction is right
	lastDirection = 0; //initially none
	EoG = false; //the game is starting
	Pokew(BIOS_JIFFY, 65); //initialize JIFFY to 65

	//initialize the snake
	snakeTail = snake; //snakeTail will point to the first snake element
	snakeHead = snake + 1; //snakeHead points to the next element
	snake[0] = snakeHeadPos - 1;
	snake[1] = snakeHeadPos;
	Vpoke(snakeHeadPos - 1, TILE_SNAKETAIL);
	Vpoke(snakeHeadPos, TILE_SNAKEHEAD);

	//drop the first apple
	dropApple();

	// Game's main loop
	while (!EoG) {

		//wait until BIOS changes the value of Jiffy
		while (lastJiffy == Peekw(BIOS_JIFFY)) {	
			joy = JoystickRead(CURSORKEYS); //read the keyboard value
			
			//logic to avoid the issue with pushing opposite direction
			if ((((lastDirection == UP) || (lastDirection == DOWN)) && ((joy == RIGHT) || (joy == LEFT))) ||
				(((lastDirection == RIGHT) || (lastDirection == LEFT)) && ((joy == UP) || (joy == DOWN))))
				direction = joy;
		}

		//from this point, 1 pass each 15 frames
		if (Peekw(BIOS_JIFFY) >= 15)
		{
			//check the direction being pushed
			switch (direction) {
			case UP:
				snakeHeadPos -= 32;
				break;
			case RIGHT:
				snakeHeadPos++;
				break;
			case DOWN:
				snakeHeadPos += 32;
				break;
			case LEFT:
				snakeHeadPos--;
				break;
			}

			//get the current position of the snake
			content = Vpeek(snakeHeadPos);
			//check if the content is an apple
			if (content == TILE_APPLE) {
				//the snake eats the apple
				
				dropApple();
				bonus = (rand() & 5) + 1;
				growth += bonus;
				score += bonus;
				Locate(7, 23);
				PrintNumber(score);
				
				if (score > highscore) {
					highscore = score;
					Locate(18, 23);
					PrintNumber(highscore);
				}
			}
			else {
				//check if the snake is being moved by the user and if it is not colliding
				EoG = (content != TILE_GRASS);
			}
			if (growth == 0) {
				Vpoke(*snakeTail, TILE_GRASS); //erases the last tail segment
				snakeTail++; //new position for the tail 
				if (snakeTail > &snake[511]) snakeTail = snake;
			}
			else
			{
				growth--;
			}
						
			Vpoke(*snakeHead, TILE_SNAKETAIL); //replaces the old head with a tail segment
			Vpoke(snakeHeadPos, TILE_SNAKEHEAD); //draws the head in the new position
			snakeHead++;
			if (snakeHead > &snake[511]) snakeHead = snake;
			*snakeHead = snakeHeadPos;
			//updates the last direction executed
			lastDirection = direction;
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
	KeySound(0); //turn off the msx beep when pushing keys
	Screen(1); //sets screen mode 1
	Width(32); //ensures 32 columns
	//SetColors(255, 0, 0); //set the color for the screen 
	highscore = 0;

	//game infinite loop
	while (true) {
		title(); //title
		game(); //game
		gameOver(); //game over
	}
}

