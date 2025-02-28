	.include	"plus4_io_map.inc"
	.include	"iolib_def.inc"

ptr	= $9e

startload	lda #2
	jsr sendfnam
	jsr readbyte
	sta ptr
	jsr readbyte
	sta ptr+1
	cmp #$fe
	bne +
	jsr readbyte
	jsr readbyte
	jmp loaderror
+	ldy #0
loadloop	jsr readbyte
	cmp #$ac
	bne storebyte
	jsr readbyte
	cmp #$ac
	beq storebyte
	cmp #$ff
	beq eof
	cmp #$f7
	beq loaderror
	jsr sync
	jmp loadloop
storebyte	sta (ptr),Y
	iny
	bne +
	inc ptr+1
+	jmp loadloop
eof	clc
loaderror	ldx filename
	ldy filename+1
	lda #0
	rts
sendfnam	pha
	stx filename
	sty filename+1
	jsr hardsync
	pla
	jsr writebyte
	lda filename
	jsr writebyte
	lda filename+1
	jsr writebyte
	jmp sync

filename	.text "f0"
