#include "ln_symbol_list.h"

#include <stdbool.h>
#include <stdlib.h>

#include <obj.h>
#include <isa.h>

#include "ln_section_list.h"

symbol_holder_t *exported_symbols = NULL;
symbol_holder_t *imported_symbols = NULL;

static bool append_to_exported(spec_symbol_t *s, section_list_item_t *section);
static bool append_to_imported(spec_symbol_t *s, section_list_item_t *section);
static bool create_new_holder(symbol_holder_t **ptr);

bool parse_symbols(section_list_item_t *section){

    //go thru all symbols
    spec_symbol_t head = section->section->spec_symbol_first;
    while(head != NULL){

        if(head->type == SYMBOL_EXPORT){
            append_to_exported(head, section);
        }
        else if(head->type == SYMBOL_IMPORT){
            append_to_imported(head, section);
        }
        else{
            //TODO: set error
            return false;
        }

        head = head->next;
    }

    return false;
}

void clean_up_symbol_lists(void){
    return;
}

static bool append_to_exported(spec_symbol_t *symbol, section_list_item_t *section){

}

static bool append_to_imported(spec_symbol_t *symbol, section_list_item_t *section){
    symbol_holder_t *new_hld = NULL;

    if(!create_new_holder(&new_hld)){
        return false;
    }

    new_hld->section = section;
    new_hld->sym = symbol;

    symbol_holder_t *head = imported_symbols;
    while(head->next != NULL){
        head = head->next;
    }

    head->next = new_hld;

    return true;
}

static bool create_new_holder(symbol_holder_t **ptr){
    symbol_type_t *ptr_tmp = NULL;

    ptr_tmp = (symbol_holder_t *)malloc(sizeof(symbol_holder_t));

    if(ptr_tmp == NULL){
        //TODO: set error
        return false;
    }

    ptr_tmp->next = NULL;
    ptr_tmp->sym = NULL;
    ptr_tmp->section = NULL;

    *ptr = ptr_tmp;

    return true;
}
