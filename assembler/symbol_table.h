/**
 * @file symbol_table.h
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
 *  - util.c
 *  - util.h
 */

#ifndef PASS1_SYMBOL_TABLE_H_included
#define PASS1_SYMBOL_TABLE_H_included

#include <stdint.h>
#include "common_defs.h"

void new_symbol(char *label, uint32_t address, uint8_t stype, tok_t *parent, void *section);
void symbol_table_cleanup(void);

#ifndef NDEBUG
void print_symboltable(void);
#endif

#endif
