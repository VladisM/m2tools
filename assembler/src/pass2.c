#include <pass2.h>

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include <symbol_table.h>
#include <pass1.h>

void pass2(void){

    for(pass1_section_t *s = pass1_list_first; s != NULL; s = s->next){
        s->last_location_counter = 0;
    }

    for(symbol_t *t = symbol_first; t != NULL; t = t->next){

        if((t->stype & STYPE_EXPORT) == STYPE_EXPORT){
            symbol_t *e = find_exported_symbol_definition(t);

            if(e == NULL){
                fprintf(stderr, "Error! Exporting symbol '%s' from file '%s' that is not defined!\n", t->label, t->parent->fileInfo->name);
                exit(EXIT_FAILURE);
            }

            t->address = e->address;
        }
        else if((t->stype & STYPE_IMPORT) == STYPE_IMPORT){
            t->address = ((pass1_section_t *)(t->section))->last_location_counter++;
        }

    }

}

void pass2_cleanup(void){
    return;
}
