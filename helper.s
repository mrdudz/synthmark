
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


