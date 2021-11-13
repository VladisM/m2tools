#include "_filelib.h"

bool obj_load(char *filename, obj_file_t **f){
    return _load_file(filename, (void **)f, &check_structure_obj, &loading_loop_obj);
}

bool obj_load_string(string_t *input, obj_file_t **f, char *filename){
    return _load_string(input, filename, (void **)f, &check_structure_obj, &loading_loop_obj);
}

bool obj_write(obj_file_t *f, char *filename){
    return _write_file(filename, (void *)f, &check_structure_obj, &writing_loop_obj);
}

void obj_write_string(obj_file_t *f, string_t **output){
    _write_string(output, (void *)f, &check_structure_obj, &writing_loop_obj);
}

void obj_file_new(obj_file_t **f){
    CHECK_NULL_ARGUMENT(f);
    CHECK_NOT_NULL_ARGUMENT(*f);

    *f = (obj_file_t *)dynmem_malloc(sizeof(obj_file_t));

    (*f)->section_list = NULL;
    (*f)->target_arch_name = NULL;

    _set_arch_name(&(*f)->target_arch_name);
    list_init(&(*f)->section_list, sizeof(obj_section_t *));
}

void obj_file_destroy(obj_file_t *f){
    CHECK_NULL_ARGUMENT(f);

    if(f->target_arch_name != NULL)
        dynmem_free(f->target_arch_name);

    if(f->section_list != NULL){
        while(list_count(f->section_list) > 0){
            obj_section_t *tmp = NULL;
            list_windraw(f->section_list, (void *)&tmp);
            obj_section_destroy(tmp);
        }
        list_destroy(f->section_list);
    }

    dynmem_free(f);
}

void obj_section_new(char *section_name, obj_section_t **section){
    CHECK_NULL_ARGUMENT(section_name);
    CHECK_NULL_ARGUMENT(section);
    CHECK_NOT_NULL_ARGUMENT(*section);

    *section = (obj_section_t *)dynmem_malloc(sizeof(obj_section_t));

    (*section)->data_symbol_list = NULL;
    (*section)->exported_symbol_list = NULL;
    (*section)->imported_symbol_list = NULL;
    (*section)->section_name = NULL;

    list_init(&(*section)->data_symbol_list, sizeof(obj_data_t *));
    list_init(&(*section)->exported_symbol_list, sizeof(obj_symbol_t *));
    list_init(&(*section)->imported_symbol_list, sizeof(obj_symbol_t *));

    (*section)->section_name = dynmem_strdup(section_name);
}

void obj_section_destroy(obj_section_t *section){
    CHECK_NULL_ARGUMENT(section);

    if(section->section_name != NULL){
        dynmem_free(section->section_name);
    }

    if(section->exported_symbol_list != NULL){
        while(list_count(section->exported_symbol_list) > 0){
            obj_symbol_t *tmp = NULL;
            list_windraw(section->exported_symbol_list, (void *)&tmp);
            obj_symbol_destroy(tmp);
        }
        list_destroy(section->exported_symbol_list);
    }

    if(section->imported_symbol_list != NULL){
        while(list_count(section->imported_symbol_list) > 0){
            obj_symbol_t *tmp = NULL;
            list_windraw(section->imported_symbol_list, (void *)&tmp);
            obj_symbol_destroy(tmp);
        }
        list_destroy(section->imported_symbol_list);
    }

    if(section->data_symbol_list != NULL){
        while(list_count(section->data_symbol_list) > 0){
            obj_data_t *tmp = NULL;
            list_windraw(section->data_symbol_list, (void *)&tmp);
            obj_data_destroy(tmp);
        }
        list_destroy(section->data_symbol_list);
    }

    dynmem_free(section);
}

void obj_section_into_file(obj_file_t *file, obj_section_t *section){
    CHECK_NULL_ARGUMENT(file);
    CHECK_NULL_ARGUMENT(section);

    list_append(file->section_list, (void*)&section);
}

void obj_data_new(obj_data_t **symbol, isa_address_t address, isa_instruction_word_t value, bool relocation, bool special, isa_address_t special_value){
    CHECK_NULL_ARGUMENT(symbol);
    CHECK_NOT_NULL_ARGUMENT(*symbol);

    *symbol = (obj_data_t *)dynmem_malloc(sizeof(obj_data_t));

    (*symbol)->address = address;
    (*symbol)->payload.data_value = value;
    (*symbol)->relocation = relocation;
    (*symbol)->special = special;
    (*symbol)->blob = false;
    (*symbol)->special_value = special_value;
}

void obj_data_destroy(obj_data_t *symbol){
    CHECK_NULL_ARGUMENT(symbol);

    dynmem_free(symbol);
}

void obj_data_into_section(obj_section_t *section, obj_data_t *symbol){
    CHECK_NULL_ARGUMENT(section);
    CHECK_NULL_ARGUMENT(symbol);

    list_append(section->data_symbol_list, (void *)&symbol);
}

void obj_blob_new(obj_data_t **symbol, isa_address_t address, isa_memory_element_t value){
    CHECK_NULL_ARGUMENT(symbol);
    CHECK_NOT_NULL_ARGUMENT(*symbol);

    *symbol = (obj_data_t *)dynmem_malloc(sizeof(obj_data_t));

    (*symbol)->address = address;
    (*symbol)->payload.blob_value = value;
    (*symbol)->relocation = false;
    (*symbol)->special = false;
    (*symbol)->blob = true;
    (*symbol)->special_value = 0;
}

void obj_blob_destroy(obj_data_t *symbol){
    obj_data_destroy(symbol);
}

void obj_blob_into_section(obj_section_t *section, obj_data_t *symbol){
    obj_data_into_section(section, symbol);
}

void obj_symbol_new(obj_symbol_t **symbol, char *name, isa_address_t value){
    CHECK_NULL_ARGUMENT(name);
    CHECK_NULL_ARGUMENT(symbol);
    CHECK_NOT_NULL_ARGUMENT(*symbol);

    *symbol = (obj_symbol_t *)dynmem_malloc(sizeof(obj_symbol_t));

    (*symbol)->name = NULL;
    (*symbol)->value = value;
    (*symbol)->name = dynmem_strdup(name);
}

void obj_symbol_destroy(obj_symbol_t *symbol){
    CHECK_NULL_ARGUMENT(symbol);

    if(symbol->name != NULL)
        dynmem_free(symbol->name);

    dynmem_free(symbol);
}

void obj_exported_symbol_into_section(obj_section_t *section, obj_symbol_t *symbol){
    CHECK_NULL_ARGUMENT(section);
    CHECK_NULL_ARGUMENT(symbol);

    list_append(section->exported_symbol_list, (void *)&symbol);
}

void obj_imported_symbol_into_section(obj_section_t *section, obj_symbol_t *symbol){
    CHECK_NULL_ARGUMENT(section);
    CHECK_NULL_ARGUMENT(symbol);

    list_append(section->imported_symbol_list, (void *)&symbol);
}
