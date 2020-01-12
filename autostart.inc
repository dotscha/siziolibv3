.print "plus/4 autostart"
//----------------------------------------------
// Autostarter by Siz
// (c) 2007.07.31
//----------------------------------------------
	
// Set autostartable program load address
//loadaddress	= $1001
// Set autostartable program start address
//startaddress	= $100d
	
//----------------------------------------------
	
	.label	ptr	= $02
	.label	ptr2	= $04
	.label	loadptr	= $9d
		
	.label	stopvec	= $0326
	
#include	"kernal-table.inc"
	
//----------------------------------------------
	
	* = $0201
	
	.pseudopc	$1001
	
	.word basend
	.word 2020
	.byte $9e // BASIC SYS token
	.text "4111"
	.byte 0
basend:	.word 0
	
start:	.word startaddress
	
	sei
	ldx #0
!:	lda $1000,x
	sta $0200,x
	dex
	bne !-
	
	.pseudopc

	jmp !+
	
!:	lda offs+data
	ldx offs+data+1
	sta ptr
	stx ptr+1
	lda #<offs+data+2
	ldx #>offs+data+2
	sta ptr2
	stx ptr2+1
	ldy #0
!:	lda (ptr2),y
	sta (ptr),y
	inc ptr
	bne *+4
	inc ptr+1
	inc ptr2
	bne *+4
	inc ptr2+1
	lda ptr2+1
	cmp loadptr+1
	bne !-
	lda ptr2
	cmp loadptr		
	bne !-
	jmp (start)
	
// Pad data
	
	.byte 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0				
	.byte 0, 0				
	
//----------------------------------------------
	
newstop:
	lda #1
	rts
autostart:
	lda #<newstop
	sta stopvec
	pla
	pla
	pla
	pla
	jsr $ec8b
	sta loadptr
	jsr $ec8b
	sta loadptr+1
	jsr $f09f
	lda #$65
	ldx #$f2
	sta stopvec
	stx stopvec+1
	jmp (start)					
	
// Pad data
// $0288
	.byte 0, 0, 0, 0, 0, 0, 0, 0				
// $0290
	.byte 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0				
// $02a0
	.byte 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0				
// $02b0
	.byte 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0				
// $02c0
	.byte 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0				
// $02d0
	.byte 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0				
// $02e0
	.byte 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0				
// $02f0
	.byte 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0				
	
// Kernal vectortable
	.word $8686, $8712, $8956, $8b6e
	.word $8bd6, $9417, $896a, $8b88
	.word $8c8b, $ce42, $ce0e, $f44c
	.word $ef53, $ee5d, $ed18, $ed60
	.word $ef0c, $ebe8, $ec4b, autostart
	
//----------------------------------------------
data	
// Append autostartable code here
//----------------------------------------------
	
	.word loadaddress
	.pseudopc loadaddress
