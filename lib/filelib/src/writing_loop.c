#include "_filelib.h"

bool writing_loop_ldm(void *input, string_t *output){
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
    return true;
}

bool writing_loop_obj(void *input, string_t *output){
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
    return true;
}

bool writing_loop_sl(void *input, string_t *output){
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
        if(!writing_loop_obj(head->object, tmp)){
            FILELIB_ERROR_WRITE("Error writing out object file from library.");
            return false;
        }

        string_appendf(output, ".file %s\r\n", head->object_name);
        string_concatenate(output, tmp);

        string_destroy(tmp);
    }

    string_appendf(output, "%s\r\n", ".end");
    return true;
}

/**
 * @brief Recursive function to print in binary
 * @warning This is sketchy shit!
 * @param output At first call, this have to be NULL, subsequent calls have this set to string_t object.
 * @param input number to deal with it
 * @return char* first iteration will return char string with converted number ... hopefully ...
 */
static char *__printf_mif_number_bin(string_t *output, unsigned long long input){
    if(output == NULL){
        string_t *tmp_obj = NULL;
        string_init(&tmp_obj);
        __printf_mif_number_bin(tmp_obj, input);
        char *tmp_char = dynmem_strdup(string_get(tmp_obj));
        string_destroy(tmp_obj);
        return tmp_char;
    }
    else{
        if(input > 1) __printf_mif_number_bin(output, input / 2);
        string_append(output, (input % 2 == 1) ? "1" : "0");
        return NULL;
    }
}

static char *_printf_mif_number_bin(unsigned long long input){
    return __printf_mif_number_bin(NULL, input);
}

static char *_printf_mif_number(char *fmt, long long input){
    CHECK_NULL_ARGUMENT(fmt);

    int len_needed = 0;
    char *tmp = NULL;

    len_needed = snprintf(NULL, 0, fmt, input);
    tmp = (char *)dynmem_calloc(len_needed + 1, sizeof(char));
    sprintf(tmp, fmt, input);
    return tmp;
}

static char *printf_mif_number(mif_radix_t number_format, long long input){
    char *result = NULL;

    switch(number_format){
        case RADIX_HEX:
            result = _printf_mif_number("%llX", input);
            break;
        case RADIX_OCT:
            result = _printf_mif_number("%llo", input);
            break;
        case RADIX_DEC:
            result = _printf_mif_number("%lld", input);
            break;
        case RADIX_UNS:
            result = _printf_mif_number("%llu", input);
            break;
        case RADIX_BIN:
            result = _printf_mif_number_bin(input);
            break;
        default:
            error("Wanted to print unsupported format, missing check???");
            break;
    }

    return result;
}

static bool check_if_radix_supported(mif_radix_t radix){
    switch(radix){
        case RADIX_HEX:
        case RADIX_OCT:
        case RADIX_DEC:
        case RADIX_UNS:
        case RADIX_BIN:
            return true;
        default:
            FILELIB_ERROR_WRITE("Unsupported radix to write out MIF file!");
            return false;
    }
}

static char *get_radix_string(mif_radix_t radix){
    switch(radix){
        case RADIX_UNK: return "UNK";
        case RADIX_HEX: return "HEX";
        case RADIX_BIN: return "BIN";
        case RADIX_OCT: return "OCT";
        case RADIX_DEC: return "DEC";
        case RADIX_UNS: return "UNS";
        default: return NULL;
    }
}

bool writing_loop_mif(void *input, string_t *output){
    CHECK_NULL_ARGUMENT(input);
    CHECK_NULL_ARGUMENT(output);

    mif_file_t *_data = (mif_file_t *)input;

    if(!check_if_radix_supported(_data->settings.address_radix))
        return false;

    if(!check_if_radix_supported(_data->settings.data_radix))
        return false;

    char *address_radix = get_radix_string(_data->settings.address_radix);
    char *data_radix = get_radix_string(_data->settings.data_radix);

    string_appendf(output, "-- Intended for %s architecture.\r\n", TARGET_ARCH_NAME);

    string_appendf(output, "DEPTH = %d;\r\n", _data->settings.depth);
    string_appendf(output, "WIDTH = %d;\r\n", _data->settings.data_width);
    string_appendf(output, "ADDRESS_RADIX = %s;\r\n", address_radix);
    string_appendf(output, "DATA_RADIX = %s;\r\n", data_radix);

    string_append(output, "CONTENT\r\nBEGIN\r\n");

    isa_address_t top_address = 0;

    for(unsigned i = 0; i < list_count(_data->content); i++){
        mif_item_t *head = NULL;

        list_at(_data->content, i, (void *)&head);

        char *addr_string = printf_mif_number(_data->settings.address_radix, head->address);
        char *data_string = printf_mif_number(_data->settings.data_radix, head->data);

        string_appendf(output, "%s : %s\r\n", addr_string, data_string);

        dynmem_free(addr_string);
        dynmem_free(data_string);

        if(head->address > top_address)
            top_address = head->address;
    }

    if(top_address > _data->settings.depth){
        FILELIB_ERROR_WRITE("Error! MIF have insufficient depth to hold all required data!");
        return false;
    }

    if(top_address < _data->settings.depth){
        char *addr_1 = printf_mif_number(_data->settings.address_radix, top_address + 1);
        char *addr_2 = printf_mif_number(_data->settings.address_radix, _data->settings.depth - 1);

        string_appendf(output, "[%s..%s] : 0\r\n", addr_1, addr_2);
    }

    string_append(output, "END;\r\n");

    return true;
}
