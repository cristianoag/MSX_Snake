#pragma once

char gameSound[] = {
	0,  0, //channel A freq none; movement
	255,0, //channel B freq eat apple 
	40, 0, //channel C freq level up
	31,	   //movement (noise)
	49,	   //mixing A: noise, B: tone, C: tone
	16,0,0,//volume
	100,4, //envelope freq
};

char xplodSound[] = {
	255,	31,		//channel A freq
	100,	20,		//channel B freq
	255,	1,		//channel C freq
	31,				//noise freq
	128,			//mixing
	16,16,16,		//volume
	10,		20,		//envelope freq
	0				//envelope
};