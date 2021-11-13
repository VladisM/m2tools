#include "filegen.h"

#include "symbol_table.h"
#include "section_table.h"
#include "common.h"
#include "pass_item.h"

#include <utillib/core.h>
#include <utillib/utils.h>
#include <filelib.h>

#include <stdbool.h>
#include <stdlib.h>

bool generate_file(char *output_filename){
    CHECK_NULL_ARGUMENT(output_filename);

    obj_file_t *obj_file = NULL;
    obj_file_new(&obj_file);

    list_t *section_list = section_table_get_all();

    for(unsigned int section_index = 0; section_index < list_count(section_list); section_index++){
        section_t *head_section = NULL;
        list_at(section_list, section_index, (void *)&head_section);

        obj_section_t *obj_section = NULL;
        obj_section_new(head_section->section_name, &obj_section);

        list_t *head_section_assigned_symbols = head_section->symbols;
        list_t *head_section_assigned_items = head_section->items;

        for(unsigned int symbols_index = 0; symbols_index < list_count(head_section_assigned_symbols); symbols_index++){
            symbol_t *head_symbol = NULL;
            list_at(head_section_assigned_symbols, symbols_index, (void *)&head_symbol);

            if(head_symbol->type == SYMBOL_TYPE_ABSOLUTE || head_symbol->type == SYMBOL_TYPE_RELOCATION){
                continue;
            }

            obj_symbol_t *obj_symbol = NULL;
            obj_symbol_new(&obj_symbol, head_symbol->name, head_symbol->value);

            if(head_symbol->type == SYMBOL_TYPE_EXPORT){
                obj_exported_symbol_into_section(obj_section, obj_symbol);
            }
            else if(head_symbol->type == SYMBOL_TYPE_IMPORT){
                obj_imported_symbol_into_section(obj_section, obj_symbol);
            }
            else{
                error("Symbol from assembler do have type that is not supported in obj file!");
            }
        }

        for(unsigned int items_index = 0; items_index < list_count(head_section_assigned_items); items_index++){
            pass_item_t *head_item = NULL;
            list_at(head_section_assigned_items, items_index, (void *)&head_item);

            if(head_item->type == ITEM_BLOB){
                obj_data_t *obj_blob = NULL;
                obj_blob_new(&obj_blob, head_item->address, head_item->value.blob);

                obj_blob_into_section(obj_section, obj_blob);
            }
            else if(head_item->type == ITEM_INST){
                obj_data_t *obj_data = NULL;
                obj_data_new(&obj_data, head_item->address, head_item->value.instr, head_item->relocation, head_item->special, head_item->special_value);

                obj_data_into_section(obj_section, obj_data);
            }
            else{
                error("Item from assembler do have type that is not supported in obj file!");
            }
        }

        obj_section_into_file(obj_file, obj_section);
    }

    if(!obj_write(obj_file, output_filename)){
        ERROR_WRITE(filelib_error());
        obj_file_destroy(obj_file);
        return false;
    }
    else{
        obj_file_destroy(obj_file);
        return true;
    }
}
