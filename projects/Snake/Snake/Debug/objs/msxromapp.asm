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
	.globl _SetColors
	.globl _Screen
	.globl _Cls
	.globl _Locate
	.globl _InputChar
	.globl _EoG
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
_EoG::
	.ds 1
;--------------------------------------------------------
; ram data
;--------------------------------------------------------
	.area _INITIALIZED
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
;msxromapp.c:22: void _print(char* msg) {
;	---------------------------------
; Function _print
; ---------------------------------
__print::
;msxromapp.c:45: __endasm;
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
;msxromapp.c:47: return;
;msxromapp.c:48: }
	ret
_Done_Version:
	.ascii "Made with FUSION-C 1.2 (ebsoft)"
	.db 0x00
_gameScreen:
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
_titleScreen:
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
_gameOverMsg:
	.ascii "+------------------------------+|      G A M E     O V E R  "
	.ascii "   |+------------------------------+"
	.db 0x00
;msxromapp.c:54: void print(char* msg) {
;	---------------------------------
; Function print
; ---------------------------------
_print::
;msxromapp.c:56: _print("[DEBUG]");
	ld	hl, #___str_4
	push	hl
	call	__print
	pop	af
;msxromapp.c:58: _print(msg);
	pop	bc
	pop	hl
	push	hl
	push	bc
	push	hl
	call	__print
	pop	af
;msxromapp.c:59: return;
;msxromapp.c:60: }
	ret
___str_4:
	.ascii "[DEBUG]"
	.db 0x00
;msxromapp.c:62: void title() {
;	---------------------------------
; Function title
; ---------------------------------
_title::
;msxromapp.c:63: Cls();
	call	_Cls
;msxromapp.c:64: print(titleScreen);
	ld	hl, #_titleScreen
	push	hl
	call	_print
	pop	af
;msxromapp.c:65: InputChar();
;msxromapp.c:66: }
	jp	_InputChar
;msxromapp.c:68: void game() {
;	---------------------------------
; Function game
; ---------------------------------
_game::
;msxromapp.c:69: Cls();
	call	_Cls
;msxromapp.c:70: print(gameScreen);
	ld	hl, #_gameScreen
	push	hl
	call	_print
;msxromapp.c:73: x = 10;
;msxromapp.c:74: y = 10;
;msxromapp.c:75: EoG = false;
	ld	a,#0x0a
	ld	(#_x),a
	ld	(#_y), a
	pop	af
	ld	iy,#_EoG
	ld	0 (iy), #0x00
;msxromapp.c:78: while (!EoG) {
00101$:
	ld	iy, #_EoG
	bit	0, 0 (iy)
	jp	NZ,_InputChar
;msxromapp.c:79: Locate(x, y);
	ld	a,(#_y + 0)
	push	af
	inc	sp
	ld	a,(#_x + 0)
	push	af
	inc	sp
	call	_Locate
;msxromapp.c:80: print("*");
	ld	hl, #___str_5
	ex	(sp),hl
	call	_print
	pop	af
;msxromapp.c:83: InputChar();
;msxromapp.c:84: }
	jr	00101$
___str_5:
	.ascii "*"
	.db 0x00
;msxromapp.c:86: void gameOver() {
;	---------------------------------
; Function gameOver
; ---------------------------------
_gameOver::
;msxromapp.c:87: Locate(0, 10);
	ld	a, #0x0a
	push	af
	inc	sp
	xor	a, a
	push	af
	inc	sp
	call	_Locate
;msxromapp.c:88: print(gameOverMsg);
	ld	hl, #_gameOverMsg
	ex	(sp),hl
	call	_print
	pop	af
;msxromapp.c:89: InputChar();
;msxromapp.c:90: }
	jp	_InputChar
;msxromapp.c:92: void main(void) {
;	---------------------------------
; Function main
; ---------------------------------
_main::
;msxromapp.c:93: Screen(1);
	ld	a, #0x01
	push	af
	inc	sp
	call	_Screen
	inc	sp
;msxromapp.c:94: Width(32);
	ld	a, #0x20
	push	af
	inc	sp
	call	_Width
	inc	sp
;msxromapp.c:95: SetColors(255, 0, 0);
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
;msxromapp.c:98: while (1) {
00102$:
;msxromapp.c:99: title();
	call	_title
;msxromapp.c:100: game();
	call	_game
;msxromapp.c:101: gameOver();
	call	_gameOver
;msxromapp.c:103: }
	jr	00102$
	.area _CODE
	.area _INITIALIZER
	.area _CABS (ABS)
