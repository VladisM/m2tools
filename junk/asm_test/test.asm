.SECTION text
;test asm file
#define TEST
;#define CAT
#define CAT2
#define CAT3
#define CAT4
.EXPORT      send_char
.DAT_B 10 20 40
send_char:
    PUSH     R13
    OR       R0 SP R13
    PUSH     R6
    RET
    SWI
;#define CAT
#ifndef CAT
#ifndef CAT
#include test2.asm
.CONS PRASE 10
L_5:
    MVIA     R1 3
    ADD      R1 R13 R2
    LDI      R2 R1
    ST       R1 304
    MVIA     R1 3
    ADD      R1 R13 R2
    LDI      R2 R1
#endif
    PUSH     R1
    INC      SP SP
.IMPORT      vgaio_init
#endif
.IMPORT      ps_init
.IMPORT      input_buffer_count
.DAT_B 10 10 10                           30 20
