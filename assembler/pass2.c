/**
 * @file pass2.c
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
 *  - asm_util.c
 *  - asm_util.h
 */

#include "pass2.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "common_defs.h"
#include "asm_util.h"

#include <isa.h>

static void _pass2(void);
static symbol_t *find_exported_symbol_definition(symbol_t *e);
static isa_address_t *find_symbol_for_instruction_assemble(char *label, void *section);

static symbol_t * last_found_symbol = NULL;

void pass2(void){
    _pass2();
}

static void _pass2(void){

    if(!register_callback_search_for_symbol(&find_symbol_for_instruction_assemble)){
        fprintf(stderr, "Error in isa library! Errno: %d\n", get_isalib_errno());
        exit(EXIT_FAILURE);
    }

    if(!register_callback_convert_to_int(&convert_to_int)){
        fprintf(stderr, "Error in isa library! Errno: %d\n", get_isalib_errno());
        exit(EXIT_FAILURE);
    }

    if(!register_callback_is_number(&is_number)){
        fprintf(stderr, "Error in isa library! Errno: %d\n", get_isalib_errno());
        exit(EXIT_FAILURE);
    }

    for(pass_section_t *s = pass_list_first; s != NULL; s = s->next){
        s->last_location_counter = 0;
    }

    for(symbol_t *t = symbol_first; t != NULL; t = t->next){

        if((t->stype & STYPE_EXPORT) == STYPE_EXPORT){
            symbol_t *e = find_exported_symbol_definition(t);

            if(e == NULL){
                fprintf(stderr, "Error! Exporting symbol '%s' from file '%s@%d' that is not defined!\n", t->label, t->parent->fileInfo->name, t->parent->lineNumber);
                exit(EXIT_FAILURE);
            }

            t->address = e->address;
        }
        else if((t->stype & STYPE_IMPORT) == STYPE_IMPORT){
            t->address = ((pass_section_t *)(t->section))->last_location_counter++;
        }

    }

    //go for all section and all instructions(blobs too) in it
    for(pass_section_t *s = pass_list_first; s != NULL; s = s->next){
        for(pass_item_t *item = s->first_element; item != NULL; item = item->next){

            item->relocation = 0;
            item->special = 0;

            if(item->type == TYPE_INSTRUCTION){

                last_found_symbol = NULL;

                if(!assemble_instruction(item->payload.i, (void *)s)){
                    fprintf(stderr, "Error in isa library! Errno: %d\n", get_isalib_errno());
                    exit(EXIT_FAILURE);
                }

                if(last_found_symbol != NULL){
                    if( ((last_found_symbol->stype & STYPE_ABSOLUTE) != STYPE_ABSOLUTE) && ((last_found_symbol->stype & STYPE_RELOCATION) == STYPE_RELOCATION) ){
                        item->relocation = 1;
                    }

                    if((last_found_symbol->stype & STYPE_IMPORT) == STYPE_IMPORT){
                        item->special = 1;
                    }
                }
            }
            else if(item->type == TYPE_BLOB){
                //Nothing to do here :)
            }
            else{
                fprintf(stderr, "Internal error in pass2!\n");
                exit(EXIT_FAILURE);
            }

        }
    }

}

void pass2_cleanup(void){
    return;
}

static symbol_t * find_exported_symbol_definition(symbol_t *exported_symbol){

    for(symbol_t *i = symbol_first; i != NULL; i = i->next){
        if(
            (strcmp(exported_symbol->label, i->label) == 0) &&
            (strcmp(((pass_section_t *)(exported_symbol->section))->section_name, ((pass_section_t *)(i->section))->section_name) == 0) &&
            ((i->stype & STYPE_EXPORT) != STYPE_EXPORT) &&
            ((i->stype & STYPE_IMPORT) != STYPE_IMPORT)
        ) return i;
    }

    return NULL;
}

static isa_address_t *find_symbol_for_instruction_assemble(char *label, void *section){

    for(symbol_t *i = symbol_first; i != NULL; i = i->next){
        if(
            (strcmp(label, i->label) == 0) &&
            (strcmp( ((pass_section_t*)section)->section_name, ((pass_section_t *)(i->section))->section_name) == 0) &&
            ((i->stype & STYPE_EXPORT) != STYPE_EXPORT)
        ){
            last_found_symbol = i;
            return &(i->address);
        }
    }

    return NULL;
}

/* ---------------------------------------------------------------------
 * Debuging functions.
 */

#ifndef NDEBUG
void print_pass2_buffer(void){
    printf("\npass buffer: \n");

    if(pass_list_first == NULL){
        printf("  - Section list is empty\n");
    }
    else{
        for(pass_section_t *s = pass_list_first; s != NULL; s = s->next){

            printf("  - Section '%s':\n", s->section_name);
            if(s->first_element == NULL){
                printf("      - List is empty\n");
            }
            else{
                for(pass_item_t *t = s->first_element; t != NULL; t = t->next){
                    if(t->type == TYPE_INSTRUCTION){
                        printf("      - from %s @ %d \t Addr: "PRIisa_addr" \t INST \t Rel: %d \t Spec: %d \t '%s' \n", t->token->fileInfo->name, t->token->lineNumber, t->location, t->relocation, t->special, t->payload.i->line);
                    }
                    else if(t->type == TYPE_BLOB){
                        printf("      - from %s @ %d \t Addr: "PRIisa_addr" \t BLOB \t Len: %d\n", t->token->fileInfo->name, t->token->lineNumber, t->location, t->payload.b->blob_len);
                    }
                    else{
                        fprintf(stderr, "Internal error in pass1, unknown pass1_item type!\n");
                        exit(EXIT_FAILURE);
                    }
                }
            }
        }
    }
}
#endif
