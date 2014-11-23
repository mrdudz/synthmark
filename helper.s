
        .export _loadzpage, _savezpage
_loadzpage:
        ldx #2
@lp:
        lda buffer,x
        sta $0000,x
        inx
        bne @lp
        rts
_savezpage:
        ldx #2
@lp:
        lda $0000,x
        sta buffer,x
        inx
        bne @lp
        rts

buffer:
        .res $100

        .export _zpcode
_zpcode:
        .repeat 32, num
        lda zpdata + num
        sta $0002 + num
        .endrepeat
        jmp $0002

zpdata:
        ldy #$14
@lp2:
        ldx #$14
@lp1:
        lda $02
        iny
        adc $03
        iny
        adc $04
        dey
        adc $05
        dey

        dex
        bne @lp1
        dey
        bne @lp2
        rts
zpdataend:

        .export _dtvturboon
_dtvturboon:
        lda #$01  ;enable extended features
        sta $d03f
        .byte $32,$99 ;sac $99 enable burstmode and skipcycles
        lda #%00000011
        .byte $32,$00 ;sac $00
        ; skip badlines
        lda #$20
        sta $d03c
        rts

        .export _dtvturbooff
_dtvturbooff:
        .byte $32,$00 ;sac $00
        lda #%00000000
        .byte $32,$ee ;sac $ee
        rts

ptr     = $fc

        .P02
        .import _vic_pal
        .export _set_vic_pal
_set_vic_pal:
        sei
p1:
        lda $d012
p2:
        cmp $d012
        beq p2
        bmi p1

        ldx #1	; PAL
        cmp #55
        beq pal
        dex		; NTSC
pal:
        stx _vic_pal
        cli
        rts

        .P816
        .import _ram_banks
        .export _set_ram_banks
_set_ram_banks:
        ; test all banks
        stz ptr + 0
        lda #$04
        sta ptr + 1
        stz ptr + 2

bank:
        ; write data
        ldy #0
l0:
        tya
        sta [ptr],y
        iny
        bne l0

        ; compare
        ldy #0
l1:
        tya
        cmp [ptr],y
        bne error
        iny
        bne l1

        ; ok, next bank
        inc ptr + 2
        bne bank
error:
        lda ptr + 2
        sta _ram_banks
        rts

        .P02
