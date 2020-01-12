.print "IOLibV3 definitions"
.namespace siziolib {

// IOLibV3 definitions to be included in programs where you use it
// ---------------------------------------------------------------

		// Base address of IOLibV3 functions
	.label	base		= $fc00
		// Data definitions for IOLibV3
	.label	data		= base-5

// Function entry points
// ---------------------
		// Decrunch file
		// Compress data to be loaded with "exomizer mem -f" command
		// parameters: X: file name first byte, Y: file name second byte
	.label	decrunch	= base - $180

		// Load uncompressed file
		// parameters: X: file name first byte, Y: file name second byte
	.label	load		= base
		// Read a byte from drive
	.label	readbyte	= base + 3
		// Write a byte to drive
	.label	writebyte	= base + 6
		// Do a hard synchronization between byte transfers (slower)
	.label	hardsync	= base + 9
		// Do a soft synchronization between byte transfers (faster)
	.label	sync		= base + 12
		// Init loader (to be called only once)
	.label	initloader	= base + 15

// Data definitions
// ----------------
		// Detected RAM size
		// -----------------
		// bit 0-5
		//  0: 16k
		//  1: 32k
		//  2: 64k
		//  3: 256k
		//  4: 512k
		//  5: 1024k
		//  6: 2048k
		//  7: 4096k
		// bit 6
		//  Hannes expansion present
		// bit 7
		// Csory expansion present
	.label	ramsize		= data

		// Type of sound source detected 
		// -----------------------------
		// $00: TED only
		// $01: Legacy SIDCard SID 6581
		// $02: Legacy SIDCard SID 8580
		// $81: NAE SID 6581
		// $82: NAE SID 8580
	.label	soundtype	= ramsize + 1

		// Base address of detected SID
		// ----------------------------
	.label	sidbase	= soundtype + 1

		// Register offset for TCBM paddle
	.label	tcbmoffs	= sidbase + 2
}
