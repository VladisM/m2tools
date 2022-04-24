#include "_filelib.h"

void check_structure_ldm(void *input){
    CHECK_NULL_ARGUMENT(input);

    ldm_file_t *_input = input;

    if(_input->target_arch_name == NULL){
        error("Broken ldm_file structure! target_arch_name == NULL!");
    }

    if(_input->memories == NULL){
        error("Broken ldm_file structure! memories == NULL!");
    }

    for(unsigned i = 0; i < list_count(_input->memories); i++){
        ldm_memory_t *tmp = NULL;
        list_at(_input->memories, i, (void *)&tmp);

        if(tmp->memory_name == NULL){
            error("Broken ldm_file structure! One memory have NULL name!");
        }

        if(tmp->items == NULL){
            error("Broken ldm_file structure! One memory have NULL items!");
        }
    }
}

void check_structure_obj(void *input){
    CHECK_NULL_ARGUMENT(input);

    obj_file_t *_input = (obj_file_t *)input;

    if(_input->target_arch_name == NULL){
        error("Broken obj_file structure! target_arch_name == NULL!");
    }

    if(_input->section_list == NULL){
        error("Broken obj_file structure! section_list == NULL!");
    }

    for(unsigned i = 0; i < list_count(_input->section_list); i++){
        obj_section_t *tmp = NULL;
        list_at(_input->section_list, i, (void *)&tmp);

        if(tmp->section_name == NULL){
            error("Broken obj_file structure! One section doesn't have name!");
        }

        if(tmp->data_symbol_list == NULL){
            error("Broken obj_file structure! One section have NULL data_symbol_list!");
        }

        if(tmp->exported_symbol_list == NULL){
            error("Broken obj_file structure! One section have NULL exported symbols!");
        }

        if(tmp->imported_symbol_list == NULL){
            error("Broken obj_file structure! One section have NULL imported symbols!");
        }

        for(unsigned ii = 0; ii < list_count(tmp->exported_symbol_list); ii++){
            obj_symbol_t *tmp_symbol = NULL;
            list_at(tmp->exported_symbol_list, ii, (void *)&tmp_symbol);

            if(tmp_symbol->name == NULL){
                error("Broken obj_file structure! One symbol have name == NULL!");
            }
        }

        for(unsigned ii = 0; ii < list_count(tmp->imported_symbol_list); ii++){
            obj_symbol_t *tmp_symbol = NULL;
            list_at(tmp->imported_symbol_list, ii, (void *)&tmp_symbol);

            if(tmp_symbol->name == NULL){
                error("Broken obj_file structure! One symbol have name == NULL!");
            }
        }
    }
}

void check_structure_sl(void *input){
    CHECK_NULL_ARGUMENT(input);

    sl_file_t *_input = (sl_file_t *)input;

    if(_input->target_arch_name == NULL){
        error("Broken sl_file structure! target_arch_name == NULL!");
    }

    if(_input->objects == NULL){
        error("Broken sl_file structure! objects == NULL!");
    }

    for(unsigned i = 0; i < list_count(_input->objects); i++){
        sl_holder_t *tmp = NULL;
        list_at(_input->objects, i, (void *)&tmp);

        if(tmp->object_name == NULL){
            error("Broken sl_file structure! One object file doesn't have name!");
        }

        if(tmp->object == NULL){
            error("Broken sl_file structure! One holder is empty!");
        }

        check_structure_obj((void *)tmp->object);
    }
}
