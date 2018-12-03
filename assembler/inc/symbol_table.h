#ifndef PASS1_SYMBOL_TABLE_H_included
#define PASS1_SYMBOL_TABLE_H_included

#include <stdint.h>

#include <tokenizer.h>

#define STYPE_ABSOLUTE    0x01U
#define STYPE_RELOCATION  0x02U
#define STYPE_EXPORT      0x04U
#define STYPE_IMPORT      0x08U

typedef struct symbol_s{
    struct symbol_s *next;
    struct symbol_s *prev;
    char *label;
    uint32_t address;
    uint8_t stype;
    tok_t * parent;
}symbol_t;

void new_symbol(char *label, uint32_t address, uint8_t stype, tok_t *parent);
void symbol_table_cleanup(void);

#ifdef DEBUG
void print_symboltable(void);
#endif

#endif
