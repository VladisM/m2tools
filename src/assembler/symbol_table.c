#include "symbol_table.h"

#include "common.h"

#include <utillib/core.h>
#include <utillib/utils.h>

#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#define CHECK_IF_INITIALIZED() {if(symbol_table == NULL){ error("Symbol table is not initialized!"); }}
#define CHECK_IF_NOT_INITIALIZED() {if(symbol_table != NULL){ error("Symbol table is already initialized!"); }}

static symbol_t *new_symbol(char *name, isa_address_t value, symbol_type_t type, preprocessed_token_t *parent, section_t *section);
static void symbol_destroy(symbol_t *symbol);
static bool compare_symbols(symbol_t *A, symbol_t *B);

static list_t *symbol_table = NULL;

void symbol_table_init(void){
    CHECK_IF_NOT_INITIALIZED();
    list_init(&symbol_table, sizeof(symbol_t *));
}

void symbol_table_deinit(void){
    CHECK_IF_INITIALIZED();

    for(unsigned int i = 0; i < list_count(symbol_table); i++){
        symbol_t *symbol = NULL;
        list_at(symbol_table, i, (void *)&symbol);

        symbol_destroy(symbol);
    }

    list_destroy(symbol_table);
    symbol_table = NULL;
}

list_t *symbol_table_get_all(void){
    CHECK_IF_INITIALIZED();
    return symbol_table;
}

bool symbol_table_append(char *name,
                         isa_address_t value,
                         symbol_type_t type,
                         preprocessed_token_t *parent,
                         section_t *section)
{
    CHECK_NULL_ARGUMENT(name);
    CHECK_NULL_ARGUMENT(parent);
    CHECK_NULL_ARGUMENT(section);
    CHECK_IF_INITIALIZED();

    symbol_t *new = new_symbol(name, value, type, parent, section);

    for(unsigned int i = 0; i < list_count(symbol_table); i++){
        symbol_t *head = NULL;
        list_at(symbol_table, i, (void *)&head);

        if(compare_symbols(new, head) == true){
            ERROR_WRITE("Multiple symbol definition! Symbol %s at %s+%ld!", new->name, new->parent->origin.filename, new->parent->origin.line_number);
            error_buffer_append_if_defined(new->parent);

            ERROR_WRITE("Previously seen as %s at %s+%ld!", head->name, head->parent->origin.filename, head->parent->origin.line_number);
            error_buffer_append_if_defined(head->parent);

            symbol_destroy(new);
            return false;
        }
    }

    list_append(symbol_table, (void *)&new);
    list_append(section->symbols, (void *)&new);

    return true;
}

static symbol_t *new_symbol(char *name,
                            isa_address_t value,
                            symbol_type_t type,
                            preprocessed_token_t *parent,
                            section_t *section)
{
    CHECK_NULL_ARGUMENT(name);
    CHECK_NULL_ARGUMENT(parent);
    CHECK_NULL_ARGUMENT(section);

    symbol_t *tmp = (symbol_t *)dynmem_calloc(1, sizeof(symbol_t));

    tmp->name = name;
    tmp->value = value;
    tmp->type = type;
    tmp->parent = parent;
    tmp->section = section;

    return tmp;
}

static void symbol_destroy(symbol_t *symbol){
    CHECK_NULL_ARGUMENT(symbol);

    dynmem_free(symbol);
}

static bool compare_symbols(symbol_t *A, symbol_t *B){
    if(
        (strcmp(A->name, B->name) == 0) &&
        (strcmp(A->section->section_name, B->section->section_name) == 0) &&
        (A->type == B->type)
    ){
        return true;
    }
    else{
        return false;
    }
}
