.SECTION text
;test asm file
.EXPORT      send_char
send_char:
    PUSH     R13
    OR       R0 SP R13
    PUSH     R6

#include test2.asm

L_5:
    MVIA     R1 3
    ADD      R1 R13 R2
    LDI      R2 R1
    ST       R1 304
    MVIA     R1 3
    ADD      R1 R13 R2
    LDI      R2 R1
    PUSH     R1
    CALL     vgaio_putc
    INC      SP SP
.IMPORT      vgaio_init
.IMPORT      ps_init
.IMPORT      input_buffer_count
