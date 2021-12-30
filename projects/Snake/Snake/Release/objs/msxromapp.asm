;--------------------------------------------------------
; File Created by SDCC : free open source ANSI-C Compiler
; Version 4.1.0 #12072 (MINGW64)
;--------------------------------------------------------
	.module msxromapp
	.optsdcc -mz80
	
;--------------------------------------------------------
; Public variables in this module
;--------------------------------------------------------
	.globl _main
	.globl _gameOver
	.globl _game
	.globl _title
	.globl _print
	.globl __print
	.globl _Width
	.globl _Vpeek
	.globl _SetColors
	.globl _JoystickRead
	.globl _Screen
	.globl _Cls
	.globl _Locate
	.globl _InputChar
	.globl _lastJiffy
	.globl _content
	.globl _EoG
	.globl _joy
	.globl _y
	.globl _x
;--------------------------------------------------------
; special function registers
;--------------------------------------------------------
;--------------------------------------------------------
; ram data
;--------------------------------------------------------
	.area _DATA
_x::
	.ds 1
_y::
	.ds 1
_joy::
	.ds 1
_EoG::
	.ds 1
_content::
	.ds 1
_lastJiffy::
	.ds 2
;--------------------------------------------------------
; ram data
;--------------------------------------------------------
	.area _INITIALIZED
_gameScreen:
	.ds 769
_titleScreen:
	.ds 769
_gameOverMsg:
	.ds 97
;--------------------------------------------------------
; absolute external ram data
;--------------------------------------------------------
	.area _DABS (ABS)
;--------------------------------------------------------
; global & static initialisations
;--------------------------------------------------------
	.area _HOME
	.area _GSINIT
	.area _GSFINAL
	.area _GSINIT
;--------------------------------------------------------
; Home
;--------------------------------------------------------
	.area _HOME
	.area _HOME
;--------------------------------------------------------
; code
;--------------------------------------------------------
	.area _CODE
;msxromapp.c:28: void _print(char* msg) {
;	---------------------------------
; Function _print
; ---------------------------------
__print::
;msxromapp.c:52: __endasm;
	ld	hl, #2; retrieve address from stack
	add	hl, sp
	ld	b, (hl)
	inc	hl
	ld	h, (hl)
	ld	l, b
	_printMSG_loop	:
	ld	a, (hl) ; print
	or	a
	ret	z
	push	hl
	push	ix
	ld	iy, (0xfcc0)
	ld	ix, #0x00a2
	call	0x001c
	ei	; in some MSXs (i.e. F1XV) CALSLT returns with di.
	pop	ix
	pop	hl
	inc	hl
	jr	_printMSG_loop
;msxromapp.c:54: return;
;msxromapp.c:55: }
	ret
_Done_Version:
	.ascii "Made with FUSION-C 1.2 (ebsoft)"
	.db 0x00
;msxromapp.c:61: void print(char* msg) {
;	---------------------------------
; Function print
; ---------------------------------
_print::
;msxromapp.c:65: _print(msg);
	pop	bc
	pop	hl
	push	hl
	push	bc
	push	hl
	call	__print
	pop	af
;msxromapp.c:66: return;
;msxromapp.c:67: }
	ret
;msxromapp.c:69: void title() {
;	---------------------------------
; Function title
; ---------------------------------
_title::
;msxromapp.c:70: Cls();
	call	_Cls
;msxromapp.c:71: print(titleScreen);
	ld	hl, #_titleScreen
	push	hl
	call	_print
	pop	af
;msxromapp.c:72: InputChar();
;msxromapp.c:73: }
	jp	_InputChar
;msxromapp.c:75: void game() {
;	---------------------------------
; Function game
; ---------------------------------
_game::
;msxromapp.c:76: Cls();
	call	_Cls
;msxromapp.c:77: print(gameScreen);
	ld	hl, #_gameScreen
	push	hl
	call	_print
	pop	af
;msxromapp.c:80: x = 10;
	ld	hl, #_x
	ld	(hl), #0x0a
;msxromapp.c:81: y = 10;
	ld	hl, #_y
	ld	(hl), #0x0a
;msxromapp.c:82: EoG = false;
	ld	hl, #_EoG
	ld	(hl), #0x00
;msxromapp.c:83: Pokew(BIOS_JIFFY, 0);
	ld	hl, #0x0000
	ld	(0xfc9e), hl
;msxromapp.c:86: while (!EoG) {
00111$:
	ld	iy, #_EoG
	bit	0, 0 (iy)
	jp	NZ,_InputChar
;msxromapp.c:89: while (lastJiffy == Peekw(BIOS_JIFFY)) {}
00101$:
	ld	hl, #0xfc9e
	ld	c, (hl)
	inc	hl
	ld	b, (hl)
	ld	hl, (_lastJiffy)
	cp	a, a
	sbc	hl, bc
	jr	Z, 00101$
;msxromapp.c:92: if (Peekw(BIOS_JIFFY) == 15)
	ld	hl, #0xfc9e
	ld	c, (hl)
	inc	hl
	ld	b, (hl)
	ld	a, c
	sub	a, #0x0f
	or	a, b
	jp	NZ,00110$
;msxromapp.c:95: joy = JoystickRead(CURSORKEYS); //read the joystick value
	xor	a, a
	push	af
	inc	sp
	call	_JoystickRead
	ld	a, l
	inc	sp
	ld	(_joy+0), a
;msxromapp.c:97: switch (joy) {
	ld	iy, #_joy
	ld	a, 0 (iy)
	dec	a
	jr	Z, 00104$
	ld	a, 0 (iy)
	sub	a, #0x03
	jr	Z, 00105$
	ld	a, 0 (iy)
	sub	a, #0x05
	jr	Z, 00106$
	ld	a, 0 (iy)
	sub	a, #0x07
	jr	Z, 00107$
	jr	00108$
;msxromapp.c:98: case UP:
00104$:
;msxromapp.c:99: y--;
	ld	hl, #_y
	dec	(hl)
;msxromapp.c:100: break;
	jr	00108$
;msxromapp.c:101: case RIGHT:
00105$:
;msxromapp.c:102: x++;
	ld	hl, #_x
	inc	(hl)
;msxromapp.c:103: break;
	jr	00108$
;msxromapp.c:104: case DOWN:
00106$:
;msxromapp.c:105: y++;
	ld	hl, #_y
	inc	(hl)
;msxromapp.c:106: break;
	jr	00108$
;msxromapp.c:107: case LEFT:
00107$:
;msxromapp.c:108: x--;
	ld	hl, #_x
	dec	(hl)
;msxromapp.c:110: }
00108$:
;msxromapp.c:112: content = Vpeek(NAMETABLE + y * 32 + x);
	ld	a, (#_y + 0)
	ld	l, a
	ld	h, #0x00
	add	hl, hl
	add	hl, hl
	add	hl, hl
	add	hl, hl
	add	hl, hl
	ld	c, l
	ld	a, h
	add	a, #0x18
	ld	b, a
	ld	a, (#_x + 0)
	ld	l, a
	ld	h, #0x00
	add	hl, bc
	push	hl
	call	_Vpeek
	pop	af
	ld	a, l
	ld	(_content+0), a
;msxromapp.c:113: EoG = ((joy > 0) && (content != ' '));
	ld	a,(#_joy + 0)
	or	a, a
	jr	Z, 00116$
	ld	a,(#_content + 0)
	sub	a, #0x20
	jr	NZ, 00117$
00116$:
	xor	a, a
	jr	00118$
00117$:
	ld	a, #0x01
00118$:
	ld	(_EoG+0), a
;msxromapp.c:114: Locate(x, y);
	ld	a,(#_y + 0)
	push	af
	inc	sp
	ld	a,(#_x + 0)
	push	af
	inc	sp
	call	_Locate
;msxromapp.c:115: print("*");
	ld	hl, #___str_1
	ex	(sp),hl
	call	_print
	pop	af
;msxromapp.c:117: Pokew(BIOS_JIFFY, 0);
	ld	hl, #0x0000
	ld	(0xfc9e), hl
00110$:
;msxromapp.c:123: lastJiffy = Peekw(BIOS_JIFFY);
	ld	hl, #0xfc9e
	ld	a, (hl)
	ld	(_lastJiffy+0), a
	inc	hl
	ld	a, (hl)
	ld	(_lastJiffy+1), a
;msxromapp.c:126: InputChar();
;msxromapp.c:127: }
	jp	00111$
___str_1:
	.ascii "*"
	.db 0x00
;msxromapp.c:129: void gameOver() {
;	---------------------------------
; Function gameOver
; ---------------------------------
_gameOver::
;msxromapp.c:130: Locate(0, 10);
	ld	a, #0x0a
	push	af
	inc	sp
	xor	a, a
	push	af
	inc	sp
	call	_Locate
;msxromapp.c:131: print(gameOverMsg);
	ld	hl, #_gameOverMsg
	ex	(sp),hl
	call	_print
	pop	af
;msxromapp.c:132: InputChar();
;msxromapp.c:133: }
	jp	_InputChar
;msxromapp.c:135: void main(void) {
;	---------------------------------
; Function main
; ---------------------------------
_main::
;msxromapp.c:136: Screen(1);
	ld	a, #0x01
	push	af
	inc	sp
	call	_Screen
	inc	sp
;msxromapp.c:137: Width(32);
	ld	a, #0x20
	push	af
	inc	sp
	call	_Width
	inc	sp
;msxromapp.c:138: SetColors(255, 0, 0);
	xor	a, a
	push	af
	inc	sp
	xor	a, a
	ld	d,a
	ld	e,#0xff
	push	de
	call	_SetColors
	pop	af
	inc	sp
;msxromapp.c:141: while (true) {
00102$:
;msxromapp.c:142: title();
	call	_title
;msxromapp.c:143: game();
	call	_game
;msxromapp.c:144: gameOver();
	call	_gameOver
;msxromapp.c:146: }
	jr	00102$
	.area _CODE
	.area _INITIALIZER
__xinit__gameScreen:
	.ascii "+------------------------------+|                           "
	.ascii "   ||                              ||                       "
	.ascii "       ||                              ||                   "
	.ascii "           ||                              ||               "
	.ascii "               ||                              ||           "
	.ascii "                   ||                              ||       "
	.ascii "                       ||                              ||   "
	.ascii "                           ||                              |"
	.ascii "|                              ||                           "
	.ascii "   ||                              ||                       "
	.ascii "       ||                              ||                   "
	.ascii "           ||                              |+---------------"
	.ascii "---------------+ Score 0     High 0    Level 01"
	.db 0x00
	.db 0x00
__xinit__titleScreen:
	.ascii "+------------------------------+|                           "
	.ascii "   ||                              ||                       "
	.ascii "       ||                              ||                   "
	.ascii "           ||                              ||               "
	.ascii "               ||                              ||           "
	.ascii "                   ||        S  N  A  K  E         ||       "
	.ascii "                       ||                              ||   "
	.ascii "                           ||                              |"
	.ascii "|                              ||                           "
	.ascii "   ||                              ||                       "
	.ascii "       ||                              ||                   "
	.ascii "           ||                              |+---------------"
	.ascii "---------------+              2021             "
	.db 0x00
	.db 0x00
__xinit__gameOverMsg:
	.ascii "+------------------------------+|      G A M E     O V E R  "
	.ascii "   |+------------------------------+"
	.db 0x00
	.area _CABS (ABS)
