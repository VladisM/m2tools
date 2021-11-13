#include "preprocessor_symbol_table.h"

#include "common.h"

#include <stdbool.h>
#include <string.h>
#include <stdlib.h>

#include <utillib/core.h>
#include <utillib/utils.h>

typedef struct{
    token_t *symbol;
    token_t *value;
} item_t;

static item_t *_new_item(token_t *symbol, token_t *value);
static void _destroy_item(item_t *item);
static item_t *_find_token(pst_t *table, token_t *tok);

void pst_init(pst_t **table){
    CHECK_NULL_ARGUMENT(table);
    CHECK_NOT_NULL_ARGUMENT(*table);

    *table = (pst_t *)dynmem_calloc(1, sizeof(pst_t));
    list_init(&((*table)->symbols), sizeof(item_t *));
}

void pst_destroy(pst_t *table){
    CHECK_NULL_ARGUMENT(table);

    if(table->symbols != NULL){
        while(list_count(table->symbols) > 0){
            item_t *item;
            list_pop(table->symbols, (void *)&item);
            _destroy_item(item);
        }

        list_destroy(table->symbols);
    }

    dynmem_free(table);
}

bool pst_define_constant(pst_t *table, token_t *name, token_t *value){
    CHECK_NULL_ARGUMENT(table);
    CHECK_NULL_ARGUMENT(name);

    if(pst_is_defined(table, name) == true){
        item_t *prev = _find_token(table, name);

        ERROR_WRITE("Double definition of symbol %s!", name->token);
        ERROR_WRITE("Previous definition is here: %s+%ld!", prev->symbol->filename, prev->symbol->line_number);

        return false;
    }

    item_t *tmp = _new_item(name, value);
    list_append(table->symbols, (void *)&tmp);

    return true;
}

bool pst_get_constant_value(pst_t *table, token_t *name, token_t **value){
    CHECK_NULL_ARGUMENT(table);
    CHECK_NULL_ARGUMENT(name);
    CHECK_NULL_ARGUMENT(value);
    CHECK_NOT_NULL_ARGUMENT(*value);

    item_t *item = _find_token(table, name);

    if(item == NULL){
        ERROR_WRITE("Refering to value of symbol that isn't exist!");
        ERROR_WRITE("Referenced at %s+%ld.", name->filename, name->line_number);

        return false;
    }

    if(item->value == NULL){
        ERROR_WRITE("Refering to value of symbol without value assigned!");
        ERROR_WRITE("Symbol defined at %s+%ld.", item->symbol->filename, item->symbol->line_number);
        ERROR_WRITE("Referenced at %s+%ld.", name->filename, name->line_number);

        return false;
    }

    *value = item->value;
    return true;
}

bool pst_is_defined(pst_t *table, token_t *name){
    CHECK_NULL_ARGUMENT(table);
    CHECK_NULL_ARGUMENT(name);

    item_t *item = _find_token(table, name);

    return (item == NULL) ? false : true;
}

static item_t *_new_item(token_t *symbol, token_t *value){
    CHECK_NULL_ARGUMENT(symbol);

    item_t *tmp = (item_t *)dynmem_calloc(1, sizeof(item_t));
    tmp->symbol = symbol;
    tmp->value = value;
    return tmp;
}

static void _destroy_item(item_t *item){
    CHECK_NULL_ARGUMENT(item);
    dynmem_free(item);
}

static item_t *_find_token(pst_t *table, token_t *tok){
    CHECK_NULL_ARGUMENT(tok);
    CHECK_NULL_ARGUMENT(table);

    item_t *retVal = NULL;

    for(unsigned i = 0; i < list_count(table->symbols); i++){
        item_t *tmp;
        list_at(table->symbols, i, (void *)&tmp);

        if(strcmp(tmp->symbol->token, tok->token) == 0){
            retVal = tmp;
            break;
        }
    }

    return retVal;
}
