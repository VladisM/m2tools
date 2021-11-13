#include "_filelib.h"

bool sl_load(char *filename, sl_file_t **f){
    return _load_file(filename, (void **)f, &check_structure_sl, &loading_loop_sl);
}

bool sl_write(sl_file_t *f, char *filename){
    return _write_file(filename, (void *)f, &check_structure_sl, &writing_loop_sl);
}

void sl_file_new(sl_file_t **f){
    CHECK_NULL_ARGUMENT(f);
    CHECK_NOT_NULL_ARGUMENT(*f);

    *f = (sl_file_t *)dynmem_malloc(sizeof(ldm_file_t));

    (*f)->target_arch_name = NULL;
    (*f)->objects = NULL;

    _set_arch_name(&(*f)->target_arch_name);
    list_init(&(*f)->objects, sizeof(sl_holder_t *));
}

void sl_file_destroy(sl_file_t *f){
    CHECK_NULL_ARGUMENT(f);

    if(f->objects != NULL){
        while(list_count(f->objects) > 0){
            sl_holder_t *tmp = NULL;
            list_windraw(f->objects, (void *)&tmp);
            sl_holder_destroy(tmp);
        }
        list_destroy(f->objects);
    }

    if(f->target_arch_name != NULL){
        dynmem_free(f->target_arch_name);
    }

    dynmem_free(f);
}

void sl_holder_new(sl_holder_t **holder, char *object_name, obj_file_t *object){
    CHECK_NULL_ARGUMENT(holder);
    CHECK_NOT_NULL_ARGUMENT(*holder);
    CHECK_NULL_ARGUMENT(object_name);
    CHECK_NULL_ARGUMENT(object);

    *holder = (sl_holder_t *)dynmem_malloc(sizeof(sl_holder_t));
    (*holder)->object = NULL;
    (*holder)->object_name = NULL;

    const char *obj_basename = NULL;
    size_t obj_basename_len = 0;

    cwk_path_get_basename(object_name, &obj_basename, &obj_basename_len);

    char *tmp = (char *)dynmem_calloc(obj_basename_len + 1, sizeof(char));
    strncpy(tmp, obj_basename, obj_basename_len);

    (*holder)->object = object;
    (*holder)->object_name = tmp;
}

void sl_holder_destroy(sl_holder_t *holder){
    CHECK_NULL_ARGUMENT(holder);

    obj_file_destroy(holder->object);
    dynmem_free(holder->object_name);
    dynmem_free(holder);
}

void sl_holder_into_file(sl_file_t *f, sl_holder_t *holder){
    CHECK_NULL_ARGUMENT(holder);
    CHECK_NULL_ARGUMENT(f);

    list_append(f->objects, (void *)&holder);
}
