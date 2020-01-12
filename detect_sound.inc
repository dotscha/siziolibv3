.print "IOLibV3 sound source detection"

.namespace siziolib {
det_sound:

.namespace soundinternal {
.label	sidptr	=	2
	lda	sidptr
	pha
	lda	sidptr+1
	pha
	lda	#0
	sta	siziolib.soundtype
#if	plus4
	.print ". compiling for 264 series"
	sei
	sta	ted_ramen
	ldx	#<sid_solder
	ldy	#>sid_solder
	jsr	io_detsidaddr
	cmp	#$ff
	beq	!+
	ldx	#<sid_csory
	ldy	#>sid_csory
	jsr	io_detsidaddr
#else
	ldx	#<$d400
	ldy	#>$d400
#endif
	cmp	#$ff
	bne	nosid
!:	inc	siziolib.soundtype
	jsr	detsidtp
#if	plus4
	jsr	detnae
#endif
	ldx	sidptr
	ldy	sidptr+1
	stx	siziolib.sidbase
	sty	siziolib.sidbase+1
nosid:
#if	plus4
	sta	ted_romen
	cli
#endif
	#if	io_prtstatus
	jsr	primm
	.text	" Sound: "
	.byte	0
#if	plus4
	sei
	sta	ted_ramen
#endif
	lda	siziolib.soundtype
#if	plus4
	sta	ted_romen
	cli
	bmi	naefound
#endif
	bne	!+
	jsr	primm
	.text	"TED"
	.byte	0
	jmp	finishedprinttype
!:	pha
	jsr	primm
	.text	"SIDCard"
	.byte	0
#if	plus4
	jmp	sidwith
naefound:
	pha
	jsr	primm
	.text	"NAE v"
	.byte	0
	lda	nae_control+15
	pha
	lsr
	lsr
	lsr
	lsr
	ora	#'0'
	jsr	chrout
	lda	#'.'
	jsr	chrout
	pla
	and	#%00001111
	ora	#'0'
	jsr	chrout
#endif
sidwith:
	jsr	primm
	.text	" with "
	.byte	0
	pla
	asl
	asl
	tax
	ldy	#4
!:	lda	io_sidtypes-4,x
	jsr	chrout
	inx
	dey
	bne	!-
finishedprinttype:
	jsr	siziolib.prtcr
#endif
	pla
	sta	sidptr+1
	pla
	sta	sidptr
	rts

// Detect SID address
//=======================================
// In: XR: SID address low
// YR: SID address high
// Out: AC: 0: not found $ff: found
//=======================================

io_detsidaddr:
	lda	#0
	sta	siziolib.sidbase
	sta	siziolib.sidbase+1
	
	stx	sidptr
	sty	sidptr+1
	
	ldy	#24
!:	sta	(sidptr),y
	dey
	bpl !-
	
	ldy	#25
	lda	(sidptr),y
	iny
	and	(sidptr),y
	rts
	
//=======================================
// Detect NST SIDCard
//=======================================

#if	plus4
detnae:
	lda	#%11100000
	sta	nae_control+13
	ldx	#0
!:	lda	nae_control+15
	cmp	nae_control+15
	bne	nonae
	dex
	bne	!-
	and	#%11100000
	cmp	#%11100000
	beq	!+		// Legacy SID-Card
	lda	nae_control+15
	beq	nonae		// VICE :)
	lda	siziolib.soundtype
	ora	#%10000000
!:	sta	siziolib.soundtype
nonae:	rts
#endif
	
// Detect SID chip type
// original version from Reflex Math demo
//=======================================
	
detsidtp:
	lda	#1
	sta	siziolib.soundtype
	
//Set a low frequency for Oscillator
// (Voice 3) #$0200 which is somewhere
// between B-0 and C-1, according to demo
	
	ldy	#15
	lda	#$02
	sta	(sidptr),y
	
//Set Sawtooth and Triangle waveforms for
//voice 3 and start release of ADSR
	
	ldy	#18
	lda	#%00110000
	sta	(sidptr),y
	
//Check if output of voice 3 ever goes
//higher than $80. It's only possible on
//new SID chip (8580) and never on the
//old one (6581)
	
	ldy	#0
	ldx	#0
	sty	ystore
loop2:
	ldy	#27
	lda	(sidptr),y
	bmi	newsid
	dex
	bne	loop2
	ldy	ystore
	dey
	sty	ystore
	bne	loop2
	beq	oldsid
newsid:
	inc	siziolib.soundtype
oldsid:
	rts

ystore:
	.byte	0

	#if	io_prtstatus
io_sidtypes:
	.text	"6581"
	.text	"8580"
	#endif
}
#if	plus4
setsidc64:
	#if	io_prtstatus
	jsr	primm
	.text	" Switching NAE to C64 clock"
	.byte	13, 0
	#endif
	lda	#$d1
	sta	nae_control+13
	rts
#endif
}