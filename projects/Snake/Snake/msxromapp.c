//snake game for msx

#include <stdbool.h>
#include <stdlib.h>
#include "targetconfig.h" 
#include "MSX/BIOS/msxbios.h" //bios definitions
#include "msx_fusion.h" //fusion c
#include "screens.h" //game screens
#include "tiles.h" //tiles for the game

#define NAMETABLE		0x1800 //default initial address for the nametable (6144 in decimal)
#define PATTERNTABLE	0x0000 //default address for the pattern table in screen 1

#define MOVES_NEW_LEVEL 100 //amount of moves to change the game level

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
unsigned char waitFrames, waitMoves, currentLevel; 

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

//char map function for debug
#ifdef DEBUG
void charMap() {
	for (int y = 0; y < 16; y++) {
		//print the first line 0 to F
		Vpoke(NAMETABLE + 2 + y, y < 10 ? '0' + y : 'A' - 10 + y);
		//print the first column 0 to F
		Vpoke(NAMETABLE + 64 + (y * 32), y < 10 ? '0' + y : 'A' - 10 + y);
		for (int x = 0; x < 16; x++)
			//print the chars 
			Vpoke(NAMETABLE + 66 + y * 32 + x, y * 16 + x);
	}
}
#endif

//transfers a data block from RAM to VRAM
void blocktoVRAM(int VRAMAddr, char* RAMAddr, int blockLength) {
	VpokeFirst(VRAMAddr);
	while(blockLength > 0) {
		VpokeNext(*(RAMAddr++));
		blockLength--;
	}
}

//build the game tiles
void buildTiles() {
	
	blocktoVRAM(PATTERNTABLE + TILE_APPLE * 8, tiles_apple, sizeof(tiles_apple)); //apple
	blocktoVRAM(PATTERNTABLE + TILE_SNAKEHEAD * 8, tiles_snakeHead, sizeof(tiles_snakeHead)); //head
	blocktoVRAM(PATTERNTABLE + TILE_SNAKETAIL * 8, tiles_snakeTail, sizeof(tiles_snakeTail)); //tail
	blocktoVRAM(PATTERNTABLE + TILE_HEADXPLOD * 8, tiles_headXplod, sizeof(tiles_headXplod));
	blocktoVRAM(PATTERNTABLE + TILE_VINE * 8, tiles_vine, sizeof(tiles_vine)); //vine
	blocktoVRAM(PATTERNTABLE + TILE_GRASS * 8, tiles_grass, sizeof(tiles_grass)); //grass
}

char allJoysticks() {
	char result;
	if (result = JoystickRead(0)) return result;
	if (result = JoystickRead(1)) return result;
	return JoystickRead(2);
}

char allTriggers() {
	return TriggerRead(0) || TriggerRead(1) || TriggerRead(2) || TriggerRead(3) || TriggerRead(4);
}

void
buildFont
()
{
	// Italic
	unsigned char temp;
	for	(int i = 0; i < 128 ; i++) {
		for(int j =	0; j < 4; j++) {
			temp = Vpeek(PATTERNTABLE + i *	8 + j);
			Vpoke(PATTERNTABLE + i * 8 + j, temp >> 1 );
		}
	}
}

//prints the game title screen
void title() {
	Cls(); //clear the screen
	_print(titleScreen); //print the title screen

	while (allJoysticks() || allTriggers()) {}	// waits for key release
	while (!(allJoysticks() || allTriggers())) {}	// waits for key press
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
    
	//initialize difficulty and moves to change level
	waitFrames = 15;
	waitMoves = MOVES_NEW_LEVEL;
	currentLevel = 1;

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
			joy = allJoysticks(); //read the value of any joystick or keyboard
			
			//logic to avoid the issue with pushing opposite direction
			if ((((lastDirection == UP) || (lastDirection == DOWN)) && ((joy == RIGHT) || (joy == LEFT))) ||
				(((lastDirection == RIGHT) || (lastDirection == LEFT)) && ((joy == UP) || (joy == DOWN))))
				direction = joy;
		}

		//from this point, 1 pass each waitFrames(difficulty) frames
		if (Peekw(BIOS_JIFFY) >= waitFrames)
		{
			waitMoves--; //decrement the moves to change level
			//controls level changes
			if (!waitMoves) {
				Locate(29, 23); //position to print the current level number
				PrintNumber(++currentLevel); //print the level
				waitFrames--; //speed up the game by reducing the the waitFrames countes
				waitMoves = 100; //reset the level control counter
			}

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
			Vpoke(snakeHeadPos, TILE_SNAKEHEAD + (direction -1)/2); //draws the head in the new position
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
	Vpoke(snakeHeadPos, TILE_HEADXPLOD + 3);
	Pokew(BIOS_JIFFY, 0); //reset bios jiffy
	while (Peek(BIOS_JIFFY) < 40) {} //wait 40 cycles
}

//game over routine
void gameOver() {
	Locate(0, 10);
	_print(gameOverMsg); //print the game over message

	while (allJoysticks() || allTriggers()) {}	// waits for key release
	while (!(allJoysticks() || allTriggers())) {}	// waits for key press
}

//main loop for the game
void main(void) {
	KeySound(0); //turn off the msx beep when pushing keys
	Screen(1); //sets screen mode 1
	Width(32); //ensures 32 columns
	SetColors(12, 3, 1); //set the color for the screen 
	buildTiles(); //calls the function to build the game tiles
	//buildFont();
	highscore = 0;

	//if it is in debug mode, print the char map and wait for a key press
#ifdef DEBUG
	charMap();
	while (!(allJoysticks() || allTriggers())) {} // waits until key press
#endif


	//game infinite loop
	while (true) {
		title(); //title
		game(); //game
		gameOver(); //game over
	}
}

