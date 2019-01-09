.SECTION text
#define CATA
#define CATB
#define CATC
#define INC

OR R0 R0 R1
#ifdef CATA
    OR R0 R0 R2
    #ifdef CATB
        OR R0 R0 R3
        #ifdef CATC
            OR R0 R0 R4
            #ifdef INC
                #include t.asm
            #endif
            OR R0 R0 R5
        #endif
        OR R0 R0 R6
    #endif
    OR R0 R0 R7
#endif
OR R0 R0 R8

;#define FOO

#ifdef FOO
#include t.asm
#endif
#ifndef FOO
#include t2.asm
#endif
