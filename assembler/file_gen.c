#include "file_gen.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <libgen.h>

#include "common_defs.h"
#include "util.h"

#include <obj.h>
#include <isa.h>

static obj_file_t *my_obj_file = NULL;

static inline void obj_lib_error_exit(void);

void filegen_create_object_file(char * abs_filename){

    char *abs_filename_dup = strdup(abs_filename);

    if(abs_filename_dup == NULL){
        fprintf(stderr, "Error! Internall error!\n");
        exit(EXIT_FAILURE);
    }

    char *filename = basename(abs_filename_dup);

    //create new object file
    if(new_obj(filename, &my_obj_file) != OBJRET_OK){
        obj_lib_error_exit();
    }

    //go for all section
    for(pass_section_t *s = pass_list_first; s != NULL; s = s->next){

        section_t *my_section = NULL;

        //create new section and append it int list
        if(new_section(s->section_name, &my_section) != OBJRET_OK){
            obj_lib_error_exit();
        }

        if(append_section_to_obj(my_obj_file, my_section) != OBJRET_OK){
            obj_lib_error_exit();
        }

        //go for all instructions/blobs in section
        for(pass_item_t *item = s->first_element; item != NULL; item = item->next){
            if(item->type == TYPE_INSTRUCTION){
                data_symbol_t *my_data = NULL;

                if(new_data_symbol(item->location, DATA_IS_INST, (void *)(item->payload.i), &my_data) != OBJRET_OK){
                    obj_lib_error_exit();
                }
                
                my_data->relocation = item->relocation;
                my_data->special = item->special;
                
                if(append_data_symbol_to_section(my_section, my_data) != OBJRET_OK){
                    obj_lib_error_exit();
                }
            }
            else if(item->type == TYPE_BLOB){
                data_symbol_t *my_data = NULL;
                datablob_t *my_blob = NULL;

                if(new_blob(item->payload.b->blob_len, &my_blob) != OBJRET_OK){
                    obj_lib_error_exit();
                }

                for(unsigned int i = 0; i < item->payload.b->blob_len; i++){
                    my_blob->payload[i] = item->payload.b->blob_data[i];
                }

                if(new_data_symbol(item->location, DATA_IS_INST, (void *)(my_blob), &my_data) != OBJRET_OK){
                    obj_lib_error_exit();
                }

                if(append_data_symbol_to_section(my_section, my_data) != OBJRET_OK){
                    obj_lib_error_exit();
                }

            }
            else{
                fprintf(stderr, "Internal error in file gen!\n");
                exit(EXIT_FAILURE);
            }
        }

        //go through all symbol
        for(symbol_t *t = symbol_first; t != NULL; t = t->next){

            //but work only at symbols from actual section
            if( strcmp(((pass_section_t *)(t->section))->section_name, s->section_name) == 0 ){

                spec_symbol_t* my_spec_symbol = NULL;

                //check if symbol is special, if so, create it and add it into section

                if((t->stype & STYPE_EXPORT) == STYPE_EXPORT){
                    if(new_spec_symbol(t->label, t->address, SYMBOL_EXPORT, &my_spec_symbol) != OBJRET_OK){
                        obj_lib_error_exit();
                    }
                }

                if((t->stype & STYPE_IMPORT) == STYPE_IMPORT){
                    if(new_spec_symbol(t->label, t->address, SYMBOL_IMPORT, &my_spec_symbol) != OBJRET_OK){
                        obj_lib_error_exit();
                    }
                }

                if(my_spec_symbol != NULL){
                    if(append_spec_symbol_to_section(my_section, my_spec_symbol) != OBJRET_OK){
                        obj_lib_error_exit();
                    }
                }

            }

        }

    }

    if(obj_write(abs_filename, my_obj_file) != OBJRET_OK){
        obj_lib_error_exit();
    }
}

void filegen_cleanup(void){
    free_object_file(my_obj_file);
    return;
}

static inline void obj_lib_error_exit(void){
    fprintf(stderr, "Error, failed in obj lib. Errno %d.\n", get_objlib_errno());
    exit(EXIT_FAILURE);
}
