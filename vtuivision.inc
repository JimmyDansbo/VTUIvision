.ifndef vtuivision_inc
vtuivision_inc = 1

VERA_ADDR_L	= $9F20
VERA_ADDR_M	= $9F21
VERA_ADDR_H	= $9F22
VERA_DATA0	= $9F23
VERA_DATA1	= $9F24
VERA_CTRL	= $9F25

r0	= $02
r0l	= r0
r0h	= r0+1
r1	= $04
r1l	= r1
r1h	= r1+1
r2	= $06
r2l	= r2
r2h	= r2+1
r3	= $08
r3l	= r3
r3h	= r3+1
r4	= $0A
r4l	= r4
r4h	= r4+1
r5	= $0C
r5l	= r5
r5h	= r5+1
r6	= $0E
r6l	= r6
r6h	= r6+1
r7	= $10
r7l	= r7
r7h	= r7+1
r8	= $12
r8l	= r8
r8h	= r8+1
r9	= $14
r9l	= r9
r9h	= r9+1
r10	= $16
r10l	= r10
r10h	= r10+1
r11	= $18
r11l	= r11
r11h	= r11+1
r12	= $1A
r12l	= r12
r12h	= r12+1

        jmp     vtuivision_end

; *****************************************************************************
; Use KERNAL API to set screen mode or swap between them.
; *****************************************************************************
; INPUT:		.A = Screenmode ($00-$0B & $80 or $FF)
; USES:			.A, .X & ,Y
; RETURNS:		.C = 1 in case of error.
; Supported screen modes:
; $00: 80x60 text
; $01: 80x30 text
; $02: 40x60 text
; $03: 40x30 text
; $04: 40x15 text
; $05: 20x30 text
; $06: 20x15 text
; $07: 22x23 text
; $08: 64x50 text
; $09: 64x25 text
; $0A: 32x50 text
; $0B: 32x25 text
; $80: 320x240@256c (40x30 text)
; *****************************************************************************
vtui_screen_set:
	clc			; Clear carry to ensure screen mode is set
	jmp	$FF5F		; screen_set_mode X16 kernal API call.
vtui_screen_get:
        sec
        jmp     $FF5F

; *****************************************************************************
; Set VERA bank (High memory) without touching anything else
; *****************************************************************************
; INPUTS:	.C = Bank number, 0 or 1
; USES:		.A
; *****************************************************************************
vtui_set_bank:
	lda	VERA_ADDR_H
	ora	#$01
	bcs	@end
	and	#$FE
@end:	sta	VERA_ADDR_H
	rts

; *****************************************************************************
; Set the stride without changing other values in VERA_ADDR_H
; *****************************************************************************
; INPUT:		.A = Stride value
; *****************************************************************************
vtui_set_stride:
@tmp	= r0l
	asl			; Stride is stored in upper nibble
	asl
	asl
	asl
        phy
        ldy     @tmp
        phy
	sta	@tmp
	lda	VERA_ADDR_H	; Set stride value to 0 in VERA_ADDR_H
	and	#$0F
	ora	@tmp
	sta	VERA_ADDR_H
        ply
        sty     @tmp
        ply
	rts

; *****************************************************************************
; Set the decrement value without changing other values in VERA_ADDR_H
; *****************************************************************************
; INPUT:		.C (1 = decrement, 0 = increment)
; *****************************************************************************
vtui_set_decr:
        pha
	lda	VERA_ADDR_H
	ora	#%00001000
	bcs	@end
	and	#%11110111
@end:	sta	VERA_ADDR_H
        pla
	rts

; *****************************************************************************
; Write character and possibly color to current VERA address
; If VERA stride = 1 and decrement = 0, colorcode in X will be written as well.
; *****************************************************************************
; INPUTS:	.A = character
;		.X = bg-/fg-color
; *****************************************************************************
vtui_plot_char:
        pha
	sta	VERA_DATA0	; Store character
	lda	VERA_ADDR_H	; Isolate stride & decr value
	and	#$F8		; Ignore nibble increment, nibble address & VRAM Bank
	cmp	#$10		; If stride=1 & decr=0 we can write color
	bne	:+
	stx	VERA_DATA0	; Write color
:       pla
	rts

; *****************************************************************************
; Read character and possibly color from current VERA address
; If VERA stride = 1 and decrement = 0, colorcode will be returned in X
; *****************************************************************************
; OUTPUS:	.A = character
;		.X = bg-/fg-color
; *****************************************************************************
vtui_scan_char:
	lda	VERA_ADDR_H	; Isolate stride & decr value
	and	#$F8		; Ignore VRAM Bank
	cmp	#$10		; If stride=1 & decr=0 we can read color
	bne	:+
        lda     VERA_DATA0      ; Read character
	ldx	VERA_DATA0	; Read color
        rts
:	lda	VERA_DATA0	; Read character
	rts

; *****************************************************************************
; Create a horizontal line going from left to right.
; *****************************************************************************
; INPUTS:	.A	= Character to use for drawing the line
;		.Y	= Length of the line
;		.X	= bg- & fg-color
; USES:		.Y & 1 byte on stack
; *****************************************************************************
vtui_hline:
	sta	VERA_DATA0
	pha			; Save .A so it can be used to check stride
	lda	VERA_ADDR_H
	and	#$F8		; Ignore VRAM Bank
	cmp	#$10		; If Stride=1 & Decr=0
	bne	:+		; we can write the color
	stx	VERA_DATA0
:	pla			; Restore .A
	dey
	bne	vtui_hline
	rts
vtuivision_end:
.endif