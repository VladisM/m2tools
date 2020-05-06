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

#include <isa.h>
#include <obj.h>
#include <ldm.h>
#include <sl.h>

#include "ln_section_list.h"
#include "linker_util.h"
#include "ldparser.h"

#define SET_ERROR(n) if(symbol_list_errno == SYMBOLLIST_OK) symbol_list_errno = n

symbol_holder_t *exported_symbols = NULL;
symbol_holder_t *imported_symbols = NULL;
symbol_holder_t *absolute_linker_symbols = NULL;

static bool append_to_exported(spec_symbol_t *s, section_list_item_t *section);
static bool append_to_imported(spec_symbol_t *s, section_list_item_t *section);
static bool append_to_absolute(spec_symbol_t *symbol);
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
            fprintf(stderr, "Error! Given symbol that is not export type or import type!\nSymbol: %s\n", head->name);
            exit(EXIT_FAILURE);
        }

        head = head->next;
    }

    return true;
}

bool parse_linker_symbols(lds_t *lds){
    sym_t *head = lds->first_sym;

    while(head != NULL){
        spec_symbol_t *tmp = NULL;

        if(!new_spec_symbol(head->name, head->value, SYMBOL_EXPORT, &tmp)){
            SET_ERROR(SECTION_OBJLIB_ERROR);
            return false;
        }
        if(!append_to_absolute(tmp)){
            return false;
        }

        head = head->next;
    }

    return true;
}

bool check_imported_symbols_exist(char *entry_point){

    if(entry_point == NULL){
        SET_ERROR(SYMBOLLIST_WRONG_ARG);
        return false;
    }

    {
        bool found = false;
        for(symbol_holder_t *head_exp = exported_symbols; head_exp != NULL; head_exp = head_exp->next){
            if(strcmp(entry_point, head_exp->sym->name) == 0){
                found = true;
                head_exp->section->used = true;
                break;
            }
        }
        if(!found){
            fprintf(stderr, "Error: unresolved symbol! Please include definition of this symbol.\n");
            fprintf(stderr, "Symbol '%s' as entry point!\n", entry_point);
            SET_ERROR(SYMBOLLIST_MISSING_EXPORT);
            return false;
        }
    }

    for(symbol_holder_t *head_imp = imported_symbols; head_imp != NULL; head_imp = head_imp->next){
        bool found = false;
        symbol_holder_t *same_symbol = NULL;

        //find counterpart for actual symbol
        for(symbol_holder_t *head_abs = absolute_linker_symbols; head_abs != NULL; head_abs = head_abs->next){
            if(are_holders_same(head_imp, head_abs)){
                same_symbol = head_abs;
                found = true;
            }
        }
        if(!found){
            for(symbol_holder_t *head_exp = exported_symbols; head_exp != NULL; head_exp = head_exp->next){
                if(are_holders_same(head_imp, head_exp)){
                    same_symbol = head_exp;
                    found = true;
                }
            }
        }

        if(found){
            if(same_symbol->section != NULL){       //symbols defined in linker script have section == NULL
                same_symbol->section->used = true;
            }
        }
        else{
            //symbol didn't found that mean there is unresolved symbol as we
            //should have everything cashed(all libs and objs) at this moment

            fprintf(stderr, "Error: unresolved symbol! Please include definition of this symbol.\n");
            fprintf(stderr, "Symbol %s from '%s'\n", head_imp->sym->name, head_imp->section->section->section_name);
            SET_ERROR(SYMBOLLIST_MISSING_EXPORT);
            return false;
        }
    }

    return true;
}

bool mark_section_with_symbol_as_used(char *symbol_name){
    bool found = false;

    if(symbol_name == NULL){
        SET_ERROR(SYMBOLLIST_WRONG_ARG);
        return false;
    }

    //is useless to search abolute_liner_symbols as they are not assigned into any sections
    for(symbol_holder_t *head = exported_symbols; head != NULL; head = head->next){
        if(strcmp(symbol_name, head->sym->name) == 0){
            head->section->used = true;
            found = true;
            break;
        }
    }

    if(found == true){
        return true;
    }
    else{
        SET_ERROR(SYMBOLLIST_MISSING_EXPORT);
        fprintf(stderr, "Error! Missing definition of symbol '%s', please provide it!\n", symbol_name);
        return false;
    }
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

    if(absolute_linker_symbols != NULL){
        head = absolute_linker_symbols;
        while(head != NULL){
            tmp = head;
            head = head->next;

            free_obj_spec_symbol(tmp->sym);
            free(tmp);
        }
    }

    return;
}

static bool append_to_absolute(spec_symbol_t *symbol){
    symbol_holder_t *new_hld = NULL;

    if(!create_new_holder(&new_hld)){
        return false;
    }

    new_hld->section = NULL;
    new_hld->sym = symbol;

    if(absolute_linker_symbols == NULL){
        absolute_linker_symbols = new_hld;
    }
    else{
        symbol_holder_t *head_ex = exported_symbols;

        while(head_ex != NULL){
            if(are_holders_same(head_ex, new_hld)){
                fprintf(stderr, "Error! Multiple export symbol definition found!\n");
                fprintf(stderr, "Symbol: %s from linker script colide with same symbol in code.\n", head_ex->sym->name);
                SET_ERROR(SYMBOLLIST_MULTIPLE);
                return false;
            }
            head_ex = head_ex->next;
        }

        symbol_holder_t *head_abs = absolute_linker_symbols;

        while(1){
            if(are_holders_same(head_abs, new_hld)){
                fprintf(stderr, "Error! Multiple export symbol definition found!\n");
                fprintf(stderr, "Symbol: %s from linker script.\n", head_abs->sym->name);
                SET_ERROR(SYMBOLLIST_MULTIPLE);
                return false;
            }

            if(head_abs->next == NULL){
                head_abs->next = new_hld;
                break;
            }

            head_abs = head_abs->next;
        }
    }

    return true;
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
        symbol_holder_t *head_abs = absolute_linker_symbols;

        while(head_abs != NULL){
            if(are_holders_same(head_abs, new_hld)){
                fprintf(stderr, "Error! Multiple export symbol definition found!\n");
                fprintf(stderr, "Symbol: %s colide with symbol from liner script.\n", head_abs->sym->name);
                SET_ERROR(SYMBOLLIST_MULTIPLE);
                return false;
            }
            head_abs = head_abs->next;
        }

        symbol_holder_t *head_ex = exported_symbols;

        while(1){
            if(are_holders_same(head_ex, new_hld)){
                fprintf(stderr, "Error! Multiple export symbol definition found! Symbol: %s\n", head_ex->sym->name);
                SET_ERROR(SYMBOLLIST_MULTIPLE);
                return false;
            }

            if(head_ex->next == NULL){
                head_ex->next = new_hld;
                break;
            }

            head_ex = head_ex->next;
        }
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

    check_malloc((void *)ptr_tmp);

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

bool search_for_import_symbol_name(section_list_item_t *section, isa_address_t id, symbol_holder_t **result){

    for(symbol_holder_t *head = imported_symbols; head != NULL; head = head->next){
        if((strcmp(section->section->section_name, head->section->section->section_name) == 0) && (id == head->sym->value)){
            *result = head;
            return true;
        }
    }

    return false;
}

bool search_for_exported_symbol(symbol_holder_t *import_label, symbol_holder_t **result){

    for(symbol_holder_t *head = exported_symbols; head != NULL; head = head->next){
        if(strcmp(import_label->sym->name, head->sym->name) == 0){
            *result = head;
            return true;
        }
    }

    return false;
}

#ifndef NDEBUG
void print_symbols_lists(void){
    if(exported_symbols == NULL){
        printf("Exported symbols:\n'-(null)\n");
    }
    else{
        printf("Exported symbols:\n");
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
        printf("Imported symbols:\n");
        for(symbol_holder_t *head = imported_symbols; head != NULL; head = head->next){
            if(head->next != NULL)
                printf("|- '%s' from '%s' val: "PRIisa_addr"\n", head->sym->name, head->section->section->section_name, head->sym->value);
            else
                printf("'- '%s' from '%s' val: "PRIisa_addr"\n", head->sym->name, head->section->section->section_name, head->sym->value);
        }
    }

    if(absolute_linker_symbols == NULL){
        printf("Absolute symbols:\n'-(null)\n");
    }
    else{
        printf("Absolute symbols:\n");
        for(symbol_holder_t *head = absolute_linker_symbols; head != NULL; head = head->next){
            if(head->next != NULL)
                printf("|- '%s' val: "PRIisa_addr"\n", head->sym->name, head->sym->value);
            else
                printf("'- '%s' val: "PRIisa_addr"\n", head->sym->name, head->sym->value);
        }
    }
}
#endif
