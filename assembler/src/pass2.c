#include <pass2.h>

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include <symbol_table.h>
#include <pass1.h>

static symbol_t * find_exported_symbol_definition(symbol_t *e);
static uint32_t *find_symbol_for_instruction_assemble(char *label, void *section);

void pass2(void){

    if(register_callback_search_for_symbol(&find_symbol_for_instruction_assemble) != 1){
        fprintf(stderr, "Error in isa library! Errno: %d\n", get_isalib_errno());
        exit(EXIT_FAILURE);
    }

    for(pass1_section_t *s = pass1_list_first; s != NULL; s = s->next){
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
            t->address = ((pass1_section_t *)(t->section))->last_location_counter++;
        }

    }

    //go for all section and all instructions(blobs too) in it
    for(pass1_section_t *s = pass1_list_first; s != NULL; s = s->next){
        for(pass1_item_t *item = s->first_element; item != NULL; item = item->next){

            if(item->type == TYPE_INSTRUCTION){
                if(assemble_instruction(item->payload.i, (void *)s) != 1){
                    fprintf(stderr, "Error in isa library! Errno: %d\n", get_isalib_errno());
                    exit(EXIT_FAILURE);
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
            (strcmp(((pass1_section_t *)(exported_symbol->section))->section_name, ((pass1_section_t *)(i->section))->section_name) == 0) &&
            ((i->stype & STYPE_EXPORT) != STYPE_EXPORT) &&
            ((i->stype & STYPE_IMPORT) != STYPE_IMPORT)
        ) return i;
    }

    return NULL;
}

static uint32_t *find_symbol_for_instruction_assemble(char *label, void *section){
    //TODO: implementovat
    return NULL;
}
