.SECTION text
.IMPORT LABEL_A
.IMPORT LABEL_C
.IMPORT LABEL_D
.EXPORT some_fun

some_fun:   OR R0 R0 R0
            CALL LABEL_C
            CALL LABEL_D
            RET
