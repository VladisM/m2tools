.SECTION text
L_3:
#constant PPP 302
#constant AAA AND
L_4:
    LD       PPP R1
    MVIA     R2 4032
    AAA      R1 R2 R6
#include test3.asm
