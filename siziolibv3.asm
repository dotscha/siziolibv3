	.encoding "petscii_mixed"

	.segment main []
	#import "kernal-table.inc"
	#import "basic_header.inc"

	jsr	primm
	.text	"IOLibV3 starter by Siz (c) 2020.01.09"
	.byte	14, 13, 0
	jsr	siziolib.detect
	sei
	sta	ted_ramen
	lda	siziolib.drivetyp
	sta	ted_romen
	cli
	bne	!+
	rts
!:	jsr	siziolib.init
	rts

	#define io_prtstatus
	#define io_needvideodetect
	#define io_needmemdetect
	#define io_needsounddetect
	#define io_needloader
	#import "iolib.inc"

	.disk [filename="IOLibV3.d64", name="IOLIBV3", id=" ACE "]
	{
		[name="IOLIBV3TEST", segments="main"]
	}
