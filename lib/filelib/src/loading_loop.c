#include "_filelib.h"

bool loading_loop_ldm(queue_t *input, void **output, char *filename){
    CHECK_NULL_ARGUMENT(filename);
    CHECK_NULL_ARGUMENT(output);
    CHECK_NULL_ARGUMENT(input);
    CHECK_NOT_NULL_ARGUMENT(*output);

    ldm_file_t *ldm_file = NULL;
    ldm_memory_t *open_mem = NULL;

    bool retVal = false;
    bool check_ldm = false;
    bool check_entry = false;
    bool check_arch = false;
    bool check_end = false;
    token_t *head = NULL;

    while(queue_count(input) > 0){
        head = _token_load(input);

        if(is_token(head, ".ldm")){
            if(check_ldm == true){
                _multiple_record_error(".ldm", head->filename, head->line_number);
                break;
            }

            ldm_file_new(&ldm_file);
            check_ldm = true;
        }
        else if(is_token(head, ".arch")){
            if(check_arch == true){
                _multiple_record_error(".arch", head->filename, head->line_number);
                break;
            }

            if(queue_count(input) < 1){
                _not_enough_tokens_error(".arch", head->filename, head->line_number);
                break;
            }

            if(check_ldm == false){
                _wrong_records_order_error(".arch", ".ldm", head->filename, head->line_number);
                break;
            }

            token_t *arg = _token_load(input);

            if(strcmp(arg->token, ldm_file->target_arch_name) != 0){
                _wrong_architecture_error(arg->filename, arg->token);
                tokenizer_token_destroy(arg);
                break;
            }

            tokenizer_token_destroy(arg);
            check_arch = true;
        }
        else if(is_token(head, ".entry")){
            if(check_entry == true){
                _multiple_record_error(".entry", head->filename, head->line_number);
                break;
            }

            if(queue_count(input) < 1){
                _not_enough_tokens_error(".entry", head->filename, head->line_number);
                break;
            }

            if(check_arch == false){
                _wrong_records_order_error(".entry", ".arch", head->filename, head->line_number);
                break;
            }

            token_t *arg = _token_load(input);
            isa_address_t entry_point = 0;

            if(!platformlib_read_isa_address(arg->token, &entry_point)){
                FILELIB_ERROR_WRITE("Can't decode entry point at %s+%ld!", head->filename, head->line_number);
                tokenizer_token_destroy(arg);
                break;
            }

            ldm_file_set_entry(ldm_file, entry_point);
            tokenizer_token_destroy(arg);

            check_entry = true;
        }
        else if(is_token(head, ".mem")){
            if(queue_count(input) < 3){
                _not_enough_tokens_error(".mem", head->filename, head->line_number);
                break;
            }

            if(check_entry == false){
                _wrong_records_order_error(".mem", ".entry", head->filename, head->line_number);
                break;
            }

            if(open_mem != NULL){
                ldm_mem_into_file(ldm_file, open_mem);
                open_mem = NULL;
            }

            token_t *arg_1 = _token_load(input);
            token_t *arg_2 = _token_load(input);
            token_t *arg_3 = _token_load(input);

            isa_address_t memory_origin;
            isa_address_t memory_size;

            if(!platformlib_read_isa_address(arg_2->token, &memory_origin)){
                _cant_decode_isa_address_error(head->filename, arg_2->line_number);
                tokenizer_token_destroy(arg_1);
                tokenizer_token_destroy(arg_2);
                tokenizer_token_destroy(arg_3);
                break;
            }

            if(!platformlib_read_isa_address(arg_3->token, &memory_size)){
                _cant_decode_isa_address_error(head->filename, arg_3->line_number);
                tokenizer_token_destroy(arg_1);
                tokenizer_token_destroy(arg_2);
                tokenizer_token_destroy(arg_3);
                break;
            }

            ldm_mem_new(arg_1->token, &open_mem, memory_size, memory_origin);

            tokenizer_token_destroy(arg_1);
            tokenizer_token_destroy(arg_2);
            tokenizer_token_destroy(arg_3);
        }
        else if(is_token(head, ".item")){
            if(queue_count(input) < 2){
                _not_enough_tokens_error(".item", head->filename, head->line_number);
                break;
            }

            if(open_mem == NULL){
                FILELIB_ERROR_WRITE("Found .item record but no memory open at %s+%ld!", head->filename, head->line_number);
                break;
            }

            token_t *arg_1 = _token_load(input);
            token_t *arg_2 = _token_load(input);

            isa_address_t address = 0;
            isa_memory_element_t word = 0;
            ldm_item_t *tmp_item = NULL;

            if(!platformlib_read_isa_address(arg_1->token, &address)){
                _cant_decode_isa_address_error(head->filename, arg_1->line_number);
                tokenizer_token_destroy(arg_1);
                tokenizer_token_destroy(arg_2);
                break;
            }

            if(!platformlib_read_isa_memory_element(arg_2->token, &word)){
                _cant_decode_isa_word_error(head->filename, arg_2->line_number);
                tokenizer_token_destroy(arg_1);
                tokenizer_token_destroy(arg_2);
                break;
            }

            ldm_item_new(address, word, &tmp_item);
            ldm_item_into_mem(open_mem, tmp_item);

            tokenizer_token_destroy(arg_1);
            tokenizer_token_destroy(arg_2);
        }
        else if(is_token(head, ".end")){
            if(open_mem != NULL){
                ldm_mem_into_file(ldm_file, open_mem);
                open_mem = NULL;
            }

            retVal = true;
            check_end = true;
            break;
        }
        else{
            _unrecognized_record_error(head->filename, head->line_number);
            break;
        }

        tokenizer_token_destroy(head);
        head = NULL;
    }

    if(open_mem != NULL){
        ldm_mem_destroy(open_mem);
        open_mem = NULL;
        retVal = false;
    }

    if(head != NULL){
        tokenizer_token_destroy(head);
    }

    if(!check_ldm){
        _missing_record_error(".ldm", filename);
        retVal = false;
    }
    if(!check_arch){
        _missing_record_error(".arch", filename);
        retVal = false;
    }
    if(!check_entry){
        _missing_record_error(".entry", filename);
        retVal = false;
    }
    if(!check_end){
        _missing_record_error(".end", filename);
        retVal = false;
    }

    if(retVal == true){
        *output = (void *)ldm_file;
    }
    else{
        *output = NULL;
        ldm_file_destroy(ldm_file);
    }

    return retVal;
}

bool loading_loop_obj(queue_t *input, void **output, char *filename){
    CHECK_NULL_ARGUMENT(filename);
    CHECK_NULL_ARGUMENT(output);
    CHECK_NULL_ARGUMENT(input);
    CHECK_NOT_NULL_ARGUMENT(*output);

    obj_file_t *obj_file = NULL;
    obj_section_t *open_section = NULL;

    bool retVal = false;
    bool check_object = false;
    bool check_arch = false;
    bool check_end = false;
    token_t *head = NULL;

    while(queue_count(input) > 0){
        head = _token_load(input);

        //object files can be loaded from static libraries and don't have filename set
        char *_filename = (strcmp(head->filename, "") == 0) ? filename : head->filename;

        if(is_token(head, ".object")){
            if(check_object == true){
                _multiple_record_error(head->token, _filename, head->line_number);
                break;
            }

            obj_file_new(&obj_file);
            check_object = true;
        }
        else if(is_token(head, ".arch")){
            if(check_arch == true){
                _multiple_record_error(head->token, _filename, head->line_number);
                break;
            }

            if(queue_count(input) < 1){
                _not_enough_tokens_error(head->token, _filename, head->line_number);
                break;
            }

            if(check_object == false){
                _wrong_records_order_error(head->token, ".object", _filename, head->line_number);
                break;
            }

            token_t *arg = _token_load(input);

            if(strcmp(arg->token, obj_file->target_arch_name) != 0){
                _wrong_architecture_error(arg->filename, arg->token);
                tokenizer_token_destroy(arg);
                break;
            }

            tokenizer_token_destroy(arg);
            check_arch = true;
        }
        else if(is_token(head, ".section")){
            if(queue_count(input) < 1){
                _not_enough_tokens_error(head->token, _filename, head->line_number);
                break;
            }

            if(check_arch == false){
                _wrong_records_order_error(head->token, ".arch", _filename, head->line_number);
                break;
            }

            if(open_section != NULL){
                obj_section_into_file(obj_file, open_section);
                open_section = NULL;
            }

            token_t *arg = _token_load(input);

            obj_section_new(arg->token, &open_section);

            tokenizer_token_destroy(arg);
        }
        else if(is_token(head, ".export") || is_token(head, ".import")){
            if(open_section == NULL){
                _wrong_records_order_error(head->token, ".section", _filename, head->line_number);
                break;
            }

            if(queue_count(input) < 2){
                _not_enough_tokens_error(head->token, _filename, head->line_number);
                break;
            }

            token_t *arg_1 = _token_load(input);
            token_t *arg_2 = _token_load(input);

            isa_address_t value = 0;
            obj_symbol_t *new_symbol = NULL;

            if(!platformlib_read_isa_address(arg_2->token, &value)){
                _cant_decode_isa_address_error(head->filename, arg_2->line_number);
                tokenizer_token_destroy(arg_1);
                tokenizer_token_destroy(arg_2);
            }

            obj_symbol_new(&new_symbol, arg_1->token, value);

            if(is_token(head, ".export")){
                obj_exported_symbol_into_section(open_section, new_symbol);
            }
            else{
                obj_imported_symbol_into_section(open_section, new_symbol);
            }

            tokenizer_token_destroy(arg_1);
            tokenizer_token_destroy(arg_2);
        }
        else if(is_token(head, ".data")){
            if(open_section == NULL){
                _wrong_records_order_error(head->token, ".section", _filename, head->line_number);
                break;
            }

            if(queue_count(input) < 4){
                _not_enough_tokens_error(head->token, _filename, head->line_number);
                break;
            }

            token_t *arg_1 = _token_load(input);
            token_t *arg_2 = _token_load(input);
            token_t *arg_3 = _token_load(input);
            token_t *arg_4 = _token_load(input);
            token_t *arg_5 = _token_load(input);

            isa_address_t address = 0;
            isa_instruction_word_t value = 0;
            obj_data_t *tmp = NULL;
            isa_address_t special_value = 0;

            bool relocation = false;
            bool special = false;

            if(!platformlib_read_isa_address(arg_1->token, &address)){
                _cant_decode_isa_address_error(head->filename, arg_1->line_number);
                tokenizer_token_destroy(arg_1);
                tokenizer_token_destroy(arg_2);
                tokenizer_token_destroy(arg_3);
                tokenizer_token_destroy(arg_4);
                tokenizer_token_destroy(arg_5);
            }

            if(!platformlib_read_isa_instruction_word(arg_2->token, &value)){
                _cant_decode_isa_word_error(head->filename, arg_2->line_number);
                tokenizer_token_destroy(arg_1);
                tokenizer_token_destroy(arg_2);
                tokenizer_token_destroy(arg_3);
                tokenizer_token_destroy(arg_4);
                tokenizer_token_destroy(arg_5);
            }

            if(!platformlib_read_isa_address(arg_3->token, &special_value)){
                _cant_decode_isa_word_error(head->filename, arg_2->line_number);
                tokenizer_token_destroy(arg_1);
                tokenizer_token_destroy(arg_2);
                tokenizer_token_destroy(arg_3);
                tokenizer_token_destroy(arg_4);
                tokenizer_token_destroy(arg_5);
            }

            relocation = (strcmp(arg_4->token, "1") == 0) ? true : false;
            special = (strcmp(arg_5->token, "1") == 0) ? true : false;

            obj_data_new(&tmp, address, value, relocation, special, special_value);
            obj_data_into_section(open_section, tmp);

            tokenizer_token_destroy(arg_1);
            tokenizer_token_destroy(arg_2);
            tokenizer_token_destroy(arg_3);
            tokenizer_token_destroy(arg_4);
            tokenizer_token_destroy(arg_5);
        }
        else if(is_token(head, ".blob")){
            if(open_section == NULL){
                _wrong_records_order_error(head->token, ".section", _filename, head->line_number);
                break;
            }

            if(queue_count(input) < 2){
                _not_enough_tokens_error(head->token, _filename, head->line_number);
                break;
            }

            token_t *arg_1 = _token_load(input);
            token_t *arg_2 = _token_load(input);

            isa_address_t address = 0;
            isa_memory_element_t value = 0;
            obj_data_t *tmp = NULL;

            if(!platformlib_read_isa_address(arg_1->token, &address)){
                _cant_decode_isa_address_error(head->filename, arg_1->line_number);
                tokenizer_token_destroy(arg_1);
                tokenizer_token_destroy(arg_2);
            }

            if(!platformlib_read_isa_memory_element(arg_2->token, &value)){
                _cant_decode_isa_memory_element(head->filename, arg_2->line_number);
                tokenizer_token_destroy(arg_1);
                tokenizer_token_destroy(arg_2);
            }

            obj_blob_new(&tmp, address, value);
            obj_data_into_section(open_section, tmp);

            tokenizer_token_destroy(arg_1);
            tokenizer_token_destroy(arg_2);
        }
        else if(is_token(head, ".end")){
            if(open_section != NULL){
                obj_section_into_file(obj_file, open_section);
                open_section = NULL;
            }

            retVal = true;
            check_end = true;
            break;
        }
        else{
            _unrecognized_record_error(_filename, head->line_number);
            break;
        }

        tokenizer_token_destroy(head);
        head = NULL;
    }

    if(open_section != NULL){
        obj_section_destroy(open_section);
        open_section = NULL;
        retVal = false;
    }

    if(head != NULL){
        tokenizer_token_destroy(head);
    }

    if(!check_object){
        _missing_record_error(".ldm", filename);
        retVal = false;
    }
    if(!check_arch){
        _missing_record_error(".arch", filename);
        retVal = false;
    }
    if(!check_end){
        _missing_record_error(".end", filename);
        retVal = false;
    }

    if(retVal == true){
        *output = (void *)obj_file;
    }
    else{
        *output = NULL;
        if(obj_file != NULL) obj_file_destroy(obj_file);
    }

    return retVal;
}

bool loading_loop_sl(queue_t *input, void **output, char *filename){
    CHECK_NULL_ARGUMENT(filename);
    CHECK_NULL_ARGUMENT(output);
    CHECK_NULL_ARGUMENT(input);
    CHECK_NOT_NULL_ARGUMENT(*output);

    sl_file_t *sl_file = NULL;

    bool retVal = false;
    bool check_sl = false;
    bool check_arch = false;
    bool check_file = false;
    bool check_end = false;
    token_t *head = NULL;

    while(queue_count(input) > 0){
        head = _token_load(input);

        if(is_token(head, ".sl")){
            if(check_sl == true){
                _multiple_record_error(head->token, head->filename, head->line_number);
                break;
            }

            sl_file_new(&sl_file);
            check_sl = true;
        }
        else if(is_token(head, ".arch")){
            if(check_arch == true){
                _multiple_record_error(head->filename, head->filename, head->line_number);
                break;
            }

            if(queue_count(input) < 1){
                _not_enough_tokens_error(head->filename, head->filename, head->line_number);
                break;
            }

            if(check_sl == false){
                _wrong_records_order_error(head->filename, ".sl", head->filename, head->line_number);
                break;
            }

            token_t *arg = _token_load(input);

            if(strcmp(arg->token, sl_file->target_arch_name) != 0){
                _wrong_architecture_error(arg->filename, arg->token);
                tokenizer_token_destroy(arg);
                break;
            }

            tokenizer_token_destroy(arg);
            check_arch = true;
        }
        else if(is_token(head, ".file")){
            if(queue_count(input) < 1){
                _not_enough_tokens_error(head->filename, head->filename, head->line_number);
                break;
            }

            if(check_arch == false){
                _wrong_records_order_error(head->filename, ".arch", head->filename, head->line_number);
            }

            token_t *arg = _token_load(input);
            obj_file_t *obj_file = NULL;

            if(!loading_loop_obj(input, (void **)&obj_file, filename)){
                FILELIB_ERROR_WRITE("Error parsing file %s!", filename);
                tokenizer_token_destroy(arg);
                retVal = false;
                break;
            }

            sl_holder_t *holder = NULL;
            sl_holder_new(&holder, arg->token, obj_file);
            sl_holder_into_file(sl_file, holder);

            tokenizer_token_destroy(arg);
            check_file = true;
        }
        else if(is_token(head, ".end")){
            if(check_file == false){
                _wrong_records_order_error(head->filename, ".file", head->filename, head->line_number);
            }

            retVal = true;
            check_end = true;
            break;
        }
        else{
            _unrecognized_record_error(head->filename, head->line_number);
            break;
        }

        tokenizer_token_destroy(head);
    }

    if(head != NULL){
        tokenizer_token_destroy(head);
    }

    if(!check_sl){
        _missing_record_error(".sl", filename);
        retVal = false;
    }
    if(!check_arch){
        _missing_record_error(".arch", filename);
        retVal = false;
    }
    if(!check_file){
        _missing_record_error(".file", filename);
        retVal = false;
    }
    if(!check_end){
        _missing_record_error(".end", filename);
        retVal = false;
    }

    if(retVal == true){
        *output = (void *)sl_file;
    }
    else{
        *output = NULL;
        sl_file_destroy(sl_file);
    }

    return retVal;
}
