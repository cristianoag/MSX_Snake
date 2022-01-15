//snake game for msx

#include <stdbool.h>
#include <stdlib.h>
#include "targetconfig.h" 
#include "MSX/BIOS/msxbios.h" //bios definitions
#include "msx_fusion.h" //fusion c
#include "screens.h" //game screens
#include "tiles.h" //tiles for the game
#include "sounds.h" //sounds for the game

#define NAMETABLE		0x1800 //default initial address for the nametable (6144 in decimal)
#define PATTERNTABLE	0x0000 //default address for the pattern table in screen 1
#define COLORTABLE		0X2000 //default address for the color table in screen 1

#define MOVES_NEW_LEVEL 100 //amount of moves to change the game level

unsigned int snakeHeadPos; //snake head position
unsigned char direction, lastDirection; //snake direction and lastknown direction
unsigned char joy; //joystick value

bool EoG; //end of game control
bool collision; //saves the collision state
bool levelUP; //change level state control
bool appleEat; //control if when an apple is eaten

unsigned char levelUpFrame; //control 1 second (60 frameis in 60hz)
unsigned char collisionTile; //collision animation frame
unsigned char content; //used to get the current position of the snake
unsigned int lastJiffy; //stores the last bios JIFFY

unsigned int snake[512]; //array to store the snake body
unsigned int *snakeHead, *snakeTail; //pointers to the snake head and tail
unsigned int applePos; //position of the random apple
unsigned char growth; //controls the snake growth
unsigned char waitFrames, waitMoves, currentLevel; 
unsigned char appleEatFrame; //count the frames after eating an apple
unsigned char levelUPSound;

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

	//blocktoVRAM(COLORTABLE, tileColors_title, sizeof(tileColors_title));
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

void buildFont()
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
	//set colors
	blocktoVRAM(COLORTABLE, tileColors_game, sizeof(tileColors_game));
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
		
	} while (Vpeek(applePos) != TILE_GRASS_EMPTY);

	Vpoke(applePos, TILE_APPLE);
}

//main game routine
void game() {
	//set colors
	blocktoVRAM(COLORTABLE, tileColors_game, sizeof(tileColors_game));
	//seed for the random function
	srand(Peekw(BIOS_JIFFY));
	
	Cls(); //clear the screen
	_print(gameScreen); //print the game screen

	//initialize game variables
	score = 0;
	growth = 0;
	appleEat = false;
    
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
	collision = false;
	collisionTile = TILE_HEADXPLOD;
	levelUP = false;
	Pokew(BIOS_JIFFY, 65); //initialize JIFFY to 65

	//initialize PSG (sound)
	for (int i = 0; i < sizeof(gameSound); i++) {
		PSGwrite(i, gameSound[i]);
	}

	//initialize the snake
	snakeTail = snake; //snakeTail will point to the first snake element
	snakeHead = snake + 1; //snakeHead points to the next element
	snake[0] = snakeHeadPos - 1;
	snake[1] = snakeHeadPos;
	Vpoke(snakeHeadPos - 1, TILE_SNAKETAIL);
	Vpoke(snakeHeadPos, TILE_SNAKEHEAD + 1);

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
		if ((Peekw(BIOS_JIFFY) >= waitFrames) && (! collision))
		{
			//controls level changes
			if (!(--waitMoves)) {
				//next level
				levelUP = true;
				levelUpFrame = 0;
				levelUPSound = 60;
				PSGwrite(10, 15);

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
			collision = (content != TILE_GRASS_EMPTY) && (content != TILE_APPLE);

			if (collision) {
				//collision start
				
				//if we kill the snake colliding with its tail, use the tile grass background color
				if (content < TILE_VINE) {
					Vpoke(COLORTABLE + TILE_HEADXPLOD / 8,
						((tileColors_game[TILE_HEADXPLOD / 8] & 0xf0) | (tileColors_game[TILE_GRASS / 8] & 0x0f)));
				}

				//animation
				Vpoke(snakeHeadPos, TILE_HEADXPLOD);
				Beep();

			}
			else {
				//check if the content is an apple
				if (content == TILE_APPLE) {
					//the snake eats the apple
					dropApple();
					
					appleEat = true;
					appleEatFrame = 16;
					
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

				//draws the head in the new position
				Vpoke(snakeHeadPos, TILE_SNAKEHEAD + (direction - 1) / 2); 
				//moving sound
				//PSGwrite(13, 4);

			}

			//erases the last tail segment
			if (growth == 0) {
				Vpoke(*snakeTail, TILE_GRASS_EMPTY); 
				snakeTail++; //new position for the tail 
				if (snakeTail > &snake[511]) 
					snakeTail = snake;
			}
			else
			{
				growth--;
			}
						
			Vpoke(*snakeHead, TILE_SNAKETAIL); //replaces the old head with a tail segment
						
			//update the buffer
			snakeHead++;
			if (snakeHead > &snake[511]) snakeHead = snake;
			*snakeHead = snakeHeadPos;

			//updates the last direction executed
			lastDirection = direction;
			Pokew(BIOS_JIFFY, 0); //reset the JIFFY to 0
		}

		//sound effects and everything that needs to run with the 
		{
			//sound effect when the sneake eats an apple
			if (appleEat) {
				PSGwrite(9, --appleEatFrame);
				appleEat = appleEatFrame > 0;
			}

			//color and sound effects when level changes
			if (levelUP) {
				//test if we are in the odd frame
				if (++levelUpFrame & 1) {
					//random color & sound
					Vpoke(COLORTABLE + TILE_SNAKETAIL / 8, 
						(rand() & 0x00f0) + 3);
					PSGwrite(4, rand()); //sound effect - channel C
				}
				else {
				//	//next color
				//	Vpoke(COLORTABLE + TILE_SNAKETAIL / 8, 
				//		tailColors[(currentLevel - 1) % sizeof(tailColors)]);
					PSGwrite(4, levelUPSound--);
				}
				if (!(levelUP = levelUpFrame < 60))
				{
					PSGwrite(10, 0);
				}
			}
			else  //back to the original color for the snake body
			{
				Vpoke(COLORTABLE + TILE_SNAKETAIL / 8,	0x23);
			}
		
			//collision animation
			if (collision && (Peekw(BIOS_JIFFY) >= 6))  {
				
				//paint the next explosion tile in the head
				Vpoke(snakeHeadPos, ++collisionTile);
				Pokew(BIOS_JIFFY, 0); //reset the jiffy

				//set the end of game if the last explosion tile is reached
				EoG = (collisionTile == TILE_HEADXPLOD + 7);

				for (int i = 0; i < sizeof(xplodSound); i++) {
					PSGwrite(i, xplodSound[i]);
				}
			}
		
		}

		//get the current JIFFY and put it on lastJiffy
		lastJiffy = Peekw(BIOS_JIFFY);
	}
		
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
	blocktoVRAM(COLORTABLE, tileColors_title, sizeof(tileColors_title));
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

