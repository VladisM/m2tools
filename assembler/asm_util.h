/**
 * @file asm_util.h
 *
 * @brief Two pass assembler with simple preprocessor.
 *
 * @author Bc. Vladislav Mlejnecký <v.mlejnecky@seznam.cz>
 * @date 15.06.2019
 *
 * @note This file is part of m2tools project.
 *
 * Assembler is splitted into multiple files:
 *  - assembler.c
 *  - common_defs.h
 *  - file_gen.c
 *  - file_gen.h
 *  - pass1.c
 *  - pass1.h
 *  - pass2.c
 *  - pass2.h
 *  - symbol_table.c
 *  - symbol_table.h
 *  - tokenizer.c
 *  - tokenizer.h
 *  - ams_util.c
 *  - ams_util.h
 */

#ifndef ASM_UTIL_H_included
#define ASM_UTIL_H_included

#include <stdint.h>

typedef union{
    uint8_t byte;
    uint16_t hword;
    uint32_t word;
}val_t;

typedef enum{
    BYTE,
    HWORD,
    WORD
}val_types_t;

long int convert_to_int(char *l);
int is_number(char *s);
int format_integer(val_types_t size, val_t *out_val, long int val);
char *basename(char *path);

#ifndef NDEBUG
void print_start(int x);
void print_end(int x);
#endif

#endif
