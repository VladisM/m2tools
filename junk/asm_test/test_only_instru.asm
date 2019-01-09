.SECTION text
MVIA     R1 3
ADD      R1 R13 R2
LDI         R2                  R1
ST             R1 304
MVIA     R1             3
ADD      R1 R13 R2
LDI      R2 R1
.SECTION bss
.ORG 256
.DAT_W 10 20 30 40
