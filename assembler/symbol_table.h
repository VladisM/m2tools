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
