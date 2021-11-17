#include "_filelib.h"

bool ldm_load(char *filename, ldm_file_t **f){
    return _load_file(filename, (void **)f, &check_structure_ldm, &loading_loop_ldm);
}

bool ldm_write(ldm_file_t *f, char *filename){
    return _write_file(filename, (void *)f, &check_structure_ldm, &writing_loop_ldm);
}

void ldm_file_new(ldm_file_t **f){
    CHECK_NULL_ARGUMENT(f);
    CHECK_NOT_NULL_ARGUMENT(*f);

    *f = (ldm_file_t *)dynmem_malloc(sizeof(ldm_file_t));

    (*f)->target_arch_name = NULL;
    (*f)->entry_point = 0;
    (*f)->memories = NULL;

    _set_arch_name(&(*f)->target_arch_name);
    list_init(&(*f)->memories, sizeof(ldm_memory_t *));
}

void ldm_file_destroy(ldm_file_t *f){
    CHECK_NULL_ARGUMENT(f);

    if(f->target_arch_name != NULL){
        dynmem_free(f->target_arch_name);
        f->target_arch_name = NULL;
    }

    if(f->memories != NULL){
        while(list_count(f->memories) > 0){
            ldm_memory_t *tmp = NULL;
            list_windraw(f->memories, (void *)&tmp);
            ldm_mem_destroy(tmp);
        }
        list_destroy(f->memories);
    }

    dynmem_free(f);
}

void ldm_file_set_entry(ldm_file_t *f, isa_address_t entry_point){
    CHECK_NULL_ARGUMENT(f);

    f->entry_point = entry_point;
}

void ldm_mem_new(char *memory_name, ldm_memory_t **mem, isa_address_t size, isa_address_t begin_addr){
    CHECK_NULL_ARGUMENT(memory_name);
    CHECK_NULL_ARGUMENT(mem);
    CHECK_NOT_NULL_ARGUMENT(*mem);

    *mem = dynmem_malloc(sizeof(ldm_memory_t));

    (*mem)->memory_name = NULL;
    (*mem)->begin_addr = begin_addr;
    (*mem)->items = NULL;
    (*mem)->size = size;

    list_init(&((*mem)->items), sizeof(ldm_item_t *));

    (*mem)->memory_name = dynmem_strdup(memory_name);
}

void ldm_mem_into_file(ldm_file_t *f, ldm_memory_t *mem){
    CHECK_NULL_ARGUMENT(f);
    CHECK_NULL_ARGUMENT(mem);

    list_append(f->memories, (void *)&mem);
}

void ldm_mem_destroy(ldm_memory_t *mem){
    CHECK_NULL_ARGUMENT(mem);

    if(mem->memory_name != NULL){
        dynmem_free(mem->memory_name);
        mem->memory_name = NULL;
    }

    if(mem->items != NULL){
        while(list_count(mem->items) > 0){
            ldm_item_t *tmp = NULL;
            list_windraw(mem->items, (void *)&tmp);
            ldm_item_destroy(tmp);
        }
        list_destroy(mem->items);
    }

    dynmem_free(mem);
}

void ldm_item_new(isa_address_t address, isa_memory_element_t word, ldm_item_t **item){
    CHECK_NULL_ARGUMENT(item);
    CHECK_NOT_NULL_ARGUMENT(*item);

    ldm_item_t *tmp = (ldm_item_t *)dynmem_malloc(sizeof(ldm_item_t));

    tmp->address = address;
    tmp->word = word;

    *item = tmp;
}

void ldm_item_into_mem(ldm_memory_t *mem, ldm_item_t *item){
    CHECK_NULL_ARGUMENT(mem);
    CHECK_NULL_ARGUMENT(item);

    list_append(mem->items, (void *)&item);
}

void ldm_item_destroy(ldm_item_t *i){
    CHECK_NULL_ARGUMENT(i);

    dynmem_free(i);
}
