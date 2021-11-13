#include "_filelib.h"

void writing_loop_ldm(void *input, string_t *output){
    CHECK_NULL_ARGUMENT(input);
    CHECK_NULL_ARGUMENT(output);

    ldm_file_t *_data = (ldm_file_t *)input;

    string_appendf(output, "%s\r\n", ".ldm");
    string_appendf(output, "%s %s\r\n", ".arch", _data->target_arch_name);

    char *entry_point = platformlib_write_isa_address(_data->entry_point);
    string_appendf(output, "%s %s\r\n", ".entry", entry_point);
    dynmem_free(entry_point);
    entry_point = NULL;

    for(unsigned i = 0; i < list_count(_data->memories); i++){
        ldm_memory_t *head_mem = NULL;
        list_at(_data->memories, i, (void *)&head_mem);

        char *memory_origin = platformlib_write_isa_address(head_mem->begin_addr);
        char *memory_size = platformlib_write_isa_address(head_mem->size);

        string_appendf(output, "%s %s %s %s\r\n", ".mem", head_mem->memory_name, memory_origin, memory_size);

        dynmem_free(memory_origin);
        dynmem_free(memory_size);

        for(unsigned j = 0; j < list_count(head_mem->items); j++){
            ldm_item_t *head_item = NULL;
            list_at(head_mem->items, j, (void *)&head_item);

            char *item_address = platformlib_write_isa_address(head_item->address);
            char *item_value = platformlib_write_isa_memory_element(head_item->word);

            string_appendf(output, "%s %s %s\r\n", ".item", item_address, item_value);

            dynmem_free(item_address);
            dynmem_free(item_value);
        }
    }

    string_appendf(output, "%s\r\n", ".end");
}

void writing_loop_obj(void *input, string_t *output){
    CHECK_NULL_ARGUMENT(input);
    CHECK_NULL_ARGUMENT(output);

    obj_file_t *_data = (obj_file_t *)input;

    string_appendf(output, ".object\r\n");
    string_appendf(output, ".arch %s\r\n", _data->target_arch_name);

    for(unsigned i = 0; i < list_count(_data->section_list); i++){
        obj_section_t *section = NULL;
        list_at(_data->section_list, i, (void *)&section);

        string_appendf(output, ".section %s\r\n", section->section_name);

        for(unsigned j = 0; j < list_count(section->exported_symbol_list); j++){
            obj_symbol_t *symbol = NULL;
            list_at(section->exported_symbol_list, j, (void *)&symbol);

            char *value = platformlib_write_isa_address(symbol->value);

            string_appendf(output, ".export %s %s\r\n", symbol->name, value);

            dynmem_free(value);
        }

        for(unsigned j = 0; j < list_count(section->imported_symbol_list); j++){
            obj_symbol_t *symbol = NULL;
            list_at(section->imported_symbol_list, j, (void *)&symbol);

            char *value = platformlib_write_isa_address(symbol->value);

            string_appendf(output, ".import %s %s\r\n", symbol->name, value);

            dynmem_free(value);
        }

        for(unsigned j = 0; j < list_count(section->data_symbol_list); j++){
            obj_data_t *symbol = NULL;
            list_at(section->data_symbol_list, j, (void *)&symbol);

            char *address = platformlib_write_isa_address(symbol->address);
            char *value = NULL;

            if(symbol->blob == true){
                value = platformlib_write_isa_memory_element(symbol->payload.blob_value);

                string_appendf(output, ".blob %s %s\r\n", address, value);
            }
            else{
                value = platformlib_write_isa_instruction_word(symbol->payload.data_value);
                char *relocation = symbol->relocation ? "1" : "0";
                char *special = symbol->special ? "1" : "0";
                char *special_value = platformlib_write_isa_address(symbol->special_value);

                string_appendf(output, ".data %s %s %s %s %s\r\n", address, value, special_value, relocation, special);

                dynmem_free(special_value);
            }

            dynmem_free(address);
            dynmem_free(value);
        }
    }

    string_appendf(output, "%s\r\n", ".end");
}

void writing_loop_sl(void *input, string_t *output){
    CHECK_NULL_ARGUMENT(input);
    CHECK_NULL_ARGUMENT(output);

    sl_file_t *_data = (sl_file_t *)input;

    string_appendf(output, "%s\r\n", ".sl");
    string_appendf(output, "%s %s\r\n", ".arch", _data->target_arch_name);

    for(unsigned i = 0; i < list_count(_data->objects); i++){
        sl_holder_t *head = NULL;
        string_t *tmp = NULL;

        list_at(_data->objects, i, (void *)&head);

        string_init(&tmp);
        writing_loop_obj(head->object, tmp);

        string_appendf(output, ".file %s\r\n", head->object_name);
        string_concatenate(output, tmp);

        string_destroy(tmp);
    }

    string_appendf(output, "%s\r\n", ".end");
}
