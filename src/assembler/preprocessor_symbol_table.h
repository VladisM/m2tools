#ifndef PREPROCESSOR_SYMBOL_TABLE_H_included
#define PREPROCESSOR_SYMBOL_TABLE_H_included

#include <stdbool.h>
#include <utillib/utils.h>

typedef struct{
    list_t *symbols;
} pst_t;

void pst_init(pst_t **table);
void pst_destroy(pst_t *table);

bool pst_define_constant(pst_t *table, token_t *name, token_t *value);
bool pst_get_constant_value(pst_t *table, token_t *name, token_t **value);
bool pst_is_defined(pst_t *table, token_t *name);

#endif
