#ifndef SYMBOL_TABLE_H_included
#define SYMBOL_TABLE_H_included

#include "section_table.h"
#include "preprocessor.h"

#include <platformlib.h>

typedef enum{
    SYMBOL_TYPE_ABSOLUTE = 0,
    SYMBOL_TYPE_RELOCATION,
    SYMBOL_TYPE_IMPORT,
    SYMBOL_TYPE_EXPORT
} symbol_type_t;

typedef struct{
    char *name;
    isa_address_t value;
    symbol_type_t type;
    preprocessed_token_t *parent;
    section_t *section;
} symbol_t;

void symbol_table_init(void);
void symbol_table_deinit(void);

bool symbol_table_append(char *name,
                         isa_address_t value,
                         symbol_type_t type,
                         preprocessed_token_t *parent,
                         section_t *section);

list_t *symbol_table_get_all(void);

#endif
