; Source modified for forward only decompression and 64Tass syntax by Siz.

; modified by Krill/Plush to implement forward decrunching and integrate into loader

;
; Copyright (c) 2002 - 2018 Magnus Lind.
;
; This software is provided 'as-is', without any express or implied warranty.
; In no event will the authors be held liable for any damages arising from
; the use of this software.
;
; Permission is granted to anyone to use this software for any purpose,
; including commercial applications, and to alter it and redistribute it
; freely, subject to the following restrictions:
;
;   1. The origin of this software must not be misrepresented; you must not
;   claim that you wrote the original software. If you use this software in a
;   product, an acknowledgment in the product documentation would be
;   appreciated but is not required.
;
;   2. Altered source versions must be plainly marked as such, and must not
;   be misrepresented as being the original software.
;
;   3. This notice may not be removed or altered from any distribution.
;
;   4. The names of this software and/or it's copyright holders may not be
;   used to endorse or promote products derived from this software without
;   specific prior written permission.
;
; -------------------------------------------------------------------
; The decruncher jsr:s to the get_crunched_byte address when it wants to
; read a crunched byte into A. This subroutine has to preserve X and Y
; register and must not modify the state of the carry nor the overflow flag.
; -------------------------------------------------------------------
;.import get_crunched_byte
; -------------------------------------------------------------------
; this function is the heart of the decruncher.
; It initializes the decruncher zeropage locations and precalculates the
; decrunch tables and decrunches the data
; This function will not change the interrupt status bit and it will not
; modify the memory configuration.
; -------------------------------------------------------------------
;.export decrunch

; -------------------------------------------------------------------
; Controls if the shared get_bits routines should be inlined or not.
;INLINE_GET_BITS=1
INLINE_GET_BITS=0
; -------------------------------------------------------------------
; if literal sequences is not used (the data was crunched with the -c
; flag) then the following line can be uncommented for shorter and.
; slightly faster code.
;LITERAL_SEQUENCES_NOT_USED = 1
LITERAL_SEQUENCES_NOT_USED = 0
; -------------------------------------------------------------------
; if the sequence length is limited to 256 (the data was crunched with
; the -M256 flag) then the following line can be uncommented for
; shorter and slightly faster code.
;MAX_SEQUENCE_LENGTH_256 = 1
MAX_SEQUENCE_LENGTH_256 = 0
; -------------------------------------------------------------------
; zero page addresses used
; -------------------------------------------------------------------

MEM_DECOMP_TO_API=0
LOADCOMPD_TO=0

zp_len_lo = $a7
zp_len_hi = $a8

zp_src_lo  = $ae
zp_src_hi  = zp_src_lo + 1

zp_bits_hi = $fc

zp_bitbuf  = $fd
zp_dest_lo = zp_bitbuf + 1      ; dest addr lo
zp_dest_hi = zp_bitbuf + 2      ; dest addr hi

tabl_bi = decrunch_table
tabl_lo = decrunch_table + 52
tabl_hi = decrunch_table + 104

SETDECOMPGETBYTE .macro
        sta decompgetbyte + 1
        sty decompgetbyte + 2
.endm

FORWARD_DECRUNCHING = 1

decompress = decrunch

        ;; refill bits is always inlined
mac_refill_bits .MACRO
        pha
        jsr get_crunched_byte
        rol
        sta zp_bitbuf
        pla
.ENDM

.IF 1=INLINE_GET_BITS
mac_get_bits .MACRO
.BLOCK
        adc #$80                ; needs c=0, affects v
        asl
        bpl gb_skip
gb_next:
        asl zp_bitbuf
        bne gb_ok
        mac_refill_bits
gb_ok:
        rol
        bmi gb_next
gb_skip:
        bvc skip
gb_get_hi:
        sec
        sta zp_bits_hi
        jsr get_crunched_byte
skip:
.BEND
.ENDM
.ELSE
mac_get_bits .MACRO
        jsr get_bits
.ENDM
get_bits:
        adc #$80                ; needs c=0, affects v
        asl
        bpl gb_skip
gb_next:
        asl zp_bitbuf
        bne gb_ok
        mac_refill_bits
gb_ok:
        rol
        bmi gb_next
gb_skip:
        bvs gb_get_hi
        rts
gb_get_hi:
        sec
        sta zp_bits_hi
get_crunched_byte:
        php
decompgetbyte:
        jsr getcmem
        plp
        rts
.ENDIF
; -------------------------------------------------------------------
; no code below this comment has to be modified in order to generate
; a working decruncher of this source file.
; However, you may want to relocate the tables last in the file to a
; more suitable address.
; -------------------------------------------------------------------

; -------------------------------------------------------------------
; jsr this label to decrunch, it will in turn init the tables and
; call the decruncher
; no constraints on register content, however the
; decimal flag has to be #0 (it almost always is, otherwise do a cld)
decrunch:
; -------------------------------------------------------------------
; init zeropage, x and y regs. (12 bytes)
;
        ldy #0
.if 1=MEM_DECOMP_TO_API
        jsr get_crunched_byte
storedadrh:
        sta zp_dest_hi
        jsr get_crunched_byte
storedadrl:
        sta zp_dest_lo
        jsr get_crunched_byte
        sta zp_bitbuf
.else
        ldx #3
init_zp:
        jsr get_crunched_byte
        sta zp_bitbuf - 1,x
        dex
        bne init_zp
.endif

.if 1=LOADCOMPD_TO
        clc
        lda loadaddroffslo
        adc zp_dest_lo
        sta zp_dest_lo
        lda loadaddroffshi
        adc zp_dest_hi
        sta zp_dest_hi
.endif

; -------------------------------------------------------------------
; calculate tables (62 bytes) + get_bits macro
; x and y must be #0 when entering
;
        clc
table_gen:
        tax
        tya
        and #$0f
        sta tabl_lo,y
        beq shortcut            ; start a new sequence
; -------------------------------------------------------------------
        txa
        adc tabl_lo - 1,y
        sta tabl_lo,y
        lda zp_len_hi
        adc tabl_hi - 1,y
shortcut:
        sta tabl_hi,y
; -------------------------------------------------------------------
        lda #$01
        sta <zp_len_hi
        lda #$78                ; %01111000
        mac_get_bits
; -------------------------------------------------------------------
        lsr
        tax
        beq rolled
        php
rolle:
        asl zp_len_hi
        sec
        ror
        dex
        bne rolle
        plp
rolled:
        ror
        sta tabl_bi,y
        bmi no_fixup_lohi
        lda zp_len_hi
        stx zp_len_hi
        .BYTE $24
no_fixup_lohi:
        txa
; -------------------------------------------------------------------
        iny
        cpy #52
        bne table_gen
; -------------------------------------------------------------------
; prepare for main decruncher
        ldy zp_dest_lo
        stx zp_dest_lo
        stx zp_bits_hi
; -------------------------------------------------------------------
; copy one literal byte to destination (11 bytes)
;
literal_start1:
.IF 0=FORWARD_DECRUNCHING
        tya
        bne no_hi_decr
        dec zp_dest_hi
no_hi_decr:
        dey
        jsr get_crunched_byte
        sta (zp_dest_lo),y
.ELSE
        jsr get_crunched_byte
        sta (zp_dest_lo),y
        iny
        bne no_hi_incr
        inc zp_dest_hi
no_hi_incr:
.ENDIF
; -------------------------------------------------------------------
; fetch sequence length index (15 bytes)
; x must be #0 when entering and contains the length index + 1
; when exiting or 0 for literal byte
next_round:
        dex
        lda zp_bitbuf
no_literal1:
        asl
        bne nofetch8
        jsr get_crunched_byte
        rol
nofetch8:
        inx
        bcc no_literal1
        sta zp_bitbuf
; -------------------------------------------------------------------
; check for literal byte (2 bytes)
;
        beq literal_start1
; -------------------------------------------------------------------
; check for decrunch done and literal sequences (4 bytes)
;
        cpx #$11
.IF 1=INLINE_GET_BITS
        bcc skip_jmp
        jmp exit_or_lit_seq
skip_jmp:
.ELSE
        bcs exit_or_lit_seq
.ENDIF
; -------------------------------------------------------------------
; calulate length of sequence (zp_len) (18(11) bytes) + get_bits macro
;
        lda tabl_bi - 1,x
        mac_get_bits
        adc tabl_lo - 1,x       ; we have now calculated zp_len_lo
        sta zp_len_lo
.IF 0=MAX_SEQUENCE_LENGTH_256
        lda zp_bits_hi
        adc tabl_hi - 1,x       ; c = 0 after this.
        sta zp_len_hi
; -------------------------------------------------------------------
; here we decide what offset table to use (27(26) bytes) + get_bits_nc macro
; z-flag reflects zp_len_hi here
;
        ldx zp_len_lo
.ELSE
        tax
.ENDIF
        lda #$e1
        cpx #$03
        bcs gbnc2_next
        lda tabl_bit,x
gbnc2_next:
        asl zp_bitbuf
        bne gbnc2_ok
        tax
        jsr get_crunched_byte
        rol
        sta zp_bitbuf
        txa
gbnc2_ok:
        rol
        bcs gbnc2_next
        tax
; -------------------------------------------------------------------
; calulate absolute offset (zp_src) (21 bytes) + get_bits macro
;
.IF 0=MAX_SEQUENCE_LENGTH_256
        lda #0
        sta zp_bits_hi
.ENDIF
.IF 0=FORWARD_DECRUNCHING
        lda tabl_bi,x
        mac_get_bits
        adc tabl_lo,x
        sta zp_src_lo
        lda zp_bits_hi
        adc tabl_hi,x
        adc zp_dest_hi
        sta zp_src_hi
.ELSE
        lda tabl_bi,x
        mac_get_bits
        clc
        adc tabl_lo,x
        eor #$ff
        sta zp_src_lo
        lda zp_bits_hi
        adc tabl_hi,x
        eor #$ff
        adc zp_dest_hi
        sta zp_src_hi
.ENDIF
; -------------------------------------------------------------------
; prepare for copy loop (2 bytes)
;
        ldx zp_len_lo
; -------------------------------------------------------------------
; main copy loop (30 bytes)
;
copy_next:
.IF 0=FORWARD_DECRUNCHING
        tya
        bne copy_skip_hi
        dec zp_dest_hi
        dec zp_src_hi
copy_skip_hi:
        dey
.IF 0=LITERAL_SEQUENCES_NOT_USED
        bcs get_literal_byte
.ENDIF
        lda (zp_src_lo),y
literal_byte_gotten:
        sta (zp_dest_lo),y
.ELSE
.IF 0=LITERAL_SEQUENCES_NOT_USED
        bcc get_literal_byte
.ENDIF
        lda (zp_src_lo),y
literal_byte_gotten:
        sta (zp_dest_lo),y
        iny
        bne copy_skip_hi
        inc zp_dest_hi
        inc zp_src_hi
copy_skip_hi:
.ENDIF
        dex
        bne copy_next
.IF 0=MAX_SEQUENCE_LENGTH_256
        lda zp_len_hi
.IF 1=INLINE_GET_BITS
        bne copy_next_hi
.ENDIF
.ENDIF
begin_stx:
        stx zp_bits_hi
.IF 0=INLINE_GET_BITS
        beq next_round
.ELSE
        jmp next_round
.ENDIF
.IF 0=MAX_SEQUENCE_LENGTH_256
copy_next_hi:
        dec zp_len_hi
        jmp copy_next
.ENDIF
.IF 0=LITERAL_SEQUENCES_NOT_USED
get_literal_byte:
        jsr get_crunched_byte
.IF 0=FORWARD_DECRUNCHING
        bcs literal_byte_gotten
.ELSE
        bcc literal_byte_gotten
.ENDIF
.ENDIF
; -------------------------------------------------------------------
; exit or literal sequence handling (16(12) bytes)
;
exit_or_lit_seq:
.IF 0=LITERAL_SEQUENCES_NOT_USED
        beq decr_exit
        jsr get_crunched_byte
.IF 0=MAX_SEQUENCE_LENGTH_256
        sta zp_len_hi
.ENDIF
        jsr get_crunched_byte
        tax
.IF 0=FORWARD_DECRUNCHING
        bcs copy_next
.ELSE
        clc
        bcc copy_next
.ENDIF
decr_exit:
.ENDIF
        rts
; -------------------------------------------------------------------
; the static stable used for bits+offset for lengths 3, 1 and 2 (3 bytes)
; bits 4, 2, 4 and offsets 16, 48, 32
tabl_bit:
        .BYTE %11100001, %10001100, %11100010
; -------------------------------------------------------------------
; end of decruncher
; -------------------------------------------------------------------

; -------------------------------------------------------------------
; this 156 byte table area may be relocated. It may also be clobbered
; by other data between decrunches.
; -------------------------------------------------------------------
decrunch_table = $ff40
;        .byte 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
;        .byte 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
;        .byte 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
;        .byte 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
;        .byte 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
;        .byte 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
;        .byte 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
;        .byte 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
;        .byte 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
;        .byte 0,0,0,0,0,0,0,0,0,0,0,0
; -------------------------------------------------------------------
; end of decruncher
; -------------------------------------------------------------------
