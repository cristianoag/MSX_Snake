; Lib to create the eden

;-
;buildEden (int VRAMAddr, char* RAMAddr, unsigned int blockLength)
;-
_buildEden:: 
	ld		hl, #2
	add		hl, sp

	ld		a, (hl)		;load less significative byte
	out		(#0x99), a	;write to the VDP

	inc		hl			;point to the most significative byte
	ld		a, (hl)		;load the most significative byte
	or		#0x40
	out		(#0x99), a  ;write to the VDP

	inc		hl
	ld		e, (hl)
	inc		hl
	ld		d, (hl)

	inc		hl
	inc		hl

	ld		b, (hl)
	push	bc

	dec		hl
	ld		b, (hl)
	ld		l, e
	ld		h, d

_newChar:
	ld		a, (hl)
	inc		hl
	cp		#0xaf
	jr		nz, outToVRAM
	ld		a, r
	rlca
	ld		r, a
	and		#0x0f ;16 tiles
	add		#0xa8 ;TILE_GRASS

	
outToVRAM:
	out		(#0x98), a 
	djnz	_newChar

	pop		bc
	xor		a
	cp		b
	ret		z 

	dec		b
	push	bc
	ld		b, #255
	jr		_newChar
