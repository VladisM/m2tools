/**
 * @file ln_symbol_list.c
 *
 * @brief Portable linker from m2tools.
 *
 * @author Bc. Vladislav Mlejnecký <v.mlejnecky@seznam.cz>
 * @date 07.02.2020
 *
 * @note This file is part of m2tools project.
 *
 * Linker is splitted into multiple files:
 *  - ldparser.c
 *  - ldparser.h
 *  - linker.c
 *  - linker_util.c
 *  - linker_util.h
 *  - ln_section_list.c
 *  - ln_section_lish.h
 *  - ln_symbol_list.c
 *  - ln_symbol_list.h
 */

#include "ln_symbol_list.h"

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include <obj.h>
#include <isa.h>

#include "ln_section_list.h"

#define SET_ERROR(n) if(symbol_list_errno == SYMBOLLIST_OK) symbol_list_errno = n

symbol_holder_t *exported_symbols = NULL;
symbol_holder_t *imported_symbols = NULL;

static bool append_to_exported(spec_symbol_t *s, section_list_item_t *section);
static bool append_to_imported(spec_symbol_t *s, section_list_item_t *section);
static bool create_new_holder(symbol_holder_t **ptr);
static bool are_holders_same(symbol_holder_t *A, symbol_holder_t *B);

static ln_symbol_list_errno_t symbol_list_errno = SYMBOLLIST_OK;

bool parse_symbols(section_list_item_t *section){
    spec_symbol_t *head = section->section->spec_symbol_first;

    while(head != NULL){

        if(head->type == SYMBOL_EXPORT){
            if(!append_to_exported(head, section)){
                return false;
            }
        }
        else if(head->type == SYMBOL_IMPORT){
            if(!append_to_imported(head, section)){
                return false;
            }
        }
        else{
            SET_ERROR(SYMBOLLIST_WTF);
            return false;
        }

        head = head->next;
    }

    return true;
}

void clean_up_symbol_lists(void){
    symbol_holder_t *head = NULL;
    symbol_holder_t *tmp = NULL;

    if(exported_symbols != NULL){
        head = exported_symbols;
        while(head != NULL){
            tmp = head;
            head = head->next;

            free(tmp);
        }
    }

    if(imported_symbols != NULL){
        head = imported_symbols;
        while(head != NULL){
            tmp = head;
            head = head->next;

            free(tmp);
        }
    }

    return;
}

static bool append_to_exported(spec_symbol_t *symbol, section_list_item_t *section){
    symbol_holder_t *new_hld = NULL;

    if(!create_new_holder(&new_hld)){
        return false;
    }

    new_hld->section = section;
    new_hld->sym = symbol;

    if(exported_symbols == NULL){
        exported_symbols = new_hld;
    }
    else{
        symbol_holder_t *head = exported_symbols;
        while(head->next != NULL){
            if(are_holders_same(head, new_hld)){
                SET_ERROR(SYMBOLLIST_MULTIPLE);
                return false;
            }

            head = head->next;
        }
        head->next = new_hld;
    }

    return true;
}

static bool append_to_imported(spec_symbol_t *symbol, section_list_item_t *section){
    symbol_holder_t *new_hld = NULL;

    if(!create_new_holder(&new_hld)){
        return false;
    }

    new_hld->section = section;
    new_hld->sym = symbol;

    if(imported_symbols == NULL){
        imported_symbols = new_hld;
    }
    else{
        symbol_holder_t *head = imported_symbols;
        while(head->next != NULL){
            head = head->next;
        }
        head->next = new_hld;
    }

    return true;
}

static bool are_holders_same(symbol_holder_t *A, symbol_holder_t *B){
    if(strcmp(A->sym->name, B->sym->name) == 0) return true;
    else return false;
}

static bool create_new_holder(symbol_holder_t **ptr){
    symbol_holder_t *ptr_tmp = NULL;

    ptr_tmp = (symbol_holder_t *)malloc(sizeof(symbol_holder_t));

    if(ptr_tmp == NULL){
        SET_ERROR(SYMBOLLIST_MALLOC);
        return false;
    }

    ptr_tmp->next = NULL;
    ptr_tmp->sym = NULL;
    ptr_tmp->section = NULL;

    *ptr = ptr_tmp;

    return true;
}

void clear_symbol_list_errno(void){
    symbol_list_errno = SYMBOLLIST_OK;
}

ln_symbol_list_errno_t get_symbol_list_errno(void){
    return symbol_list_errno;
}

#ifndef NDEBUG
void print_symbols_lists(void){
    if(exported_symbols == NULL){
        printf("Exported symbols:\n'-(null)\n");
    }
    else{
        printf("Exported symbol:\n");
        for(symbol_holder_t *head = exported_symbols; head != NULL; head = head->next){
            if(head->next != NULL)
                printf("|- '%s' from '%s' val: "PRIisa_addr"\n", head->sym->name, head->section->section->section_name, head->sym->value);
            else
                printf("'- '%s' from '%s' val: "PRIisa_addr"\n", head->sym->name, head->section->section->section_name, head->sym->value);
        }
    }

    if(imported_symbols == NULL){
        printf("Imported symbols:\n'-(null)\n");
    }
    else{
        printf("Imported symbol:\n");
        for(symbol_holder_t *head = imported_symbols; head != NULL; head = head->next){
            if(head->next != NULL)
                printf("|- '%s' from '%s' val: "PRIisa_addr"\n", head->sym->name, head->section->section->section_name, head->sym->value);
            else
                printf("'- '%s' from '%s' val: "PRIisa_addr"\n", head->sym->name, head->section->section->section_name, head->sym->value);
        }
    }
}
#endif
