.SECTION text
.IMPORT LABEL_A
.IMPORT LABEL_B
.IMPORT LABEL_C
.IMPORT some_fun
.EXPORT main

main:   CALL LABEL_A
        CALL LABEL_B
        CALL LABEL_C
        CALL some_fun
        OR R0 R0 R0
        BZ R0 main
