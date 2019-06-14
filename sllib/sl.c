#include "sl.h"

#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include <obj.h>
#include <microtar.h>

#define SET_ERROR(n) if(sllib_errno == 0) sllib_errno = n

static sl_err_t sllib_errno = SLRET_OK;

void clear_sllib_errno(void){
    sllib_errno = SLRET_OK;
}

sl_err_t get_sllib_errno(void){
    return sllib_errno;
}

void free_sl(static_library_t *lib){
    if(lib != NULL){
        obj_file_t *tmp, *head;

        head = lib->first_obj;
        while(head != NULL){
            tmp = head;
            head = head->next;

            free_object_file(tmp);
        }

        free(lib->library_name);
        free(lib->target_arch_name);
        free(lib);
    }
}

bool sl_load(char *filename, static_library_t **lib){

    if(filename == NULL || lib == NULL){
        SET_ERROR(SLRET_NULL_PTR);
        return false;
    }

    if(*lib != NULL){
        SET_ERROR(SLRET_LIB_EXIST_ALREADY);
        return false;
    }

    mtar_t my_tarfile;
    mtar_header_t file_header;

    if(mtar_open(&my_tarfile, filename, "r") != MTAR_ESUCCESS){
        SET_ERROR(SLRET_CANT_OPEN_SL);
        return false;
    }

    if(mtar_find(&my_tarfile, "target_arch_name", &file_header) != MTAR_ESUCCESS){
        SET_ERROR(SLRET_BROKEN_SL);
        mtar_close(&my_tarfile);
        return false;
    }

    char *target_arch_name = calloc(1, file_header.size + 1);

    if(target_arch_name == NULL){
        SET_ERROR(SLRET_MALLOC_FAIL);
        mtar_close(&my_tarfile);
        return false;
    }

    if(mtar_read_data(&my_tarfile, target_arch_name, file_header.size) != MTAR_ESUCCESS){
        SET_ERROR(SLRET_INTERN_ERR);
        free(target_arch_name);
        mtar_close(&my_tarfile);
        return false;
    }

    if(strcmp(target_arch_name, TARGET_ARCH_NAME) != 0){
        SET_ERROR(SLRET_WRONG_ARCH);
        free(target_arch_name);
        mtar_close(&my_tarfile);
        return false;
    }

    if(mtar_find(&my_tarfile, "library_name", &file_header) != MTAR_ESUCCESS){
        SET_ERROR(SLRET_BROKEN_SL);
        free(target_arch_name);
        mtar_close(&my_tarfile);
        return false;
    }

    char *library_name = calloc(1, file_header.size + 1);

    if(library_name == NULL){
        SET_ERROR(SLRET_MALLOC_FAIL);
        free(target_arch_name);
        mtar_close(&my_tarfile);
        return false;
    }

    if(mtar_read_data(&my_tarfile, library_name, file_header.size) != MTAR_ESUCCESS){
        SET_ERROR(SLRET_INTERN_ERR);
        free(library_name);
        free(target_arch_name);
        mtar_close(&my_tarfile);
        return false;
    }

    if(~new_sl(library_name, lib)){
        SET_ERROR(SLRET_INTERN_ERR);
        free(library_name);
        free(target_arch_name);
        mtar_close(&my_tarfile);
        return false;
    }

    free(target_arch_name);
    free(library_name);

    if(mtar_rewind(&my_tarfile) != MTAR_ESUCCESS){
        SET_ERROR(SLRET_INTERN_ERR);
        mtar_close(&my_tarfile);
        free_sl(*lib);
        return false;
    }

    while(1){
        int ret = mtar_read_header(&my_tarfile, &file_header);

        if(ret == MTAR_ENULLRECORD) break;

        if(strcmp(file_header.name, "library_name") == 0){
            mtar_next(&my_tarfile);
            continue;
        }
        else if(strcmp(file_header.name, "target_arch_name") == 0){
            mtar_next(&my_tarfile);
            continue;
        }
        else{
            char *data = calloc(1, file_header.size + 1);

            if(data == NULL){
                SET_ERROR(SLRET_MALLOC_FAIL);
                free_sl(*lib);
                return false;
            }

            if(mtar_read_data(&my_tarfile, data, file_header.size) != MTAR_ESUCCESS){
                SET_ERROR(SLRET_INTERN_ERR);
                free_sl(*lib);
                free(data);
                mtar_close(&my_tarfile);
                return false;
            }

            obj_file_t *tmp_obj = NULL;

            if(obj_load_from_string(data, &tmp_obj) != 0){
                SET_ERROR(SLRET_INTERN_ERR);
                free_sl(*lib);
                free(data);
                mtar_close(&my_tarfile);
                return false;
            }

            free(data);

            if(~append_objfile_to_sl(tmp_obj, *lib)){
                SET_ERROR(SLRET_INTERN_ERR);
                free_sl(*lib);
                mtar_close(&my_tarfile);
                free_object_file(tmp_obj);
                return false;
            }

            mtar_next(&my_tarfile);
        }
    }

    mtar_close(&my_tarfile);
    return true;

}

bool sl_write(char *filename, static_library_t *lib){

    if(filename == NULL || lib == NULL){
        SET_ERROR(SLRET_NULL_PTR);
        return false;
    }

    mtar_t my_tarfile;
    obj_file_t *head = lib->first_obj;

    if(mtar_open(&my_tarfile, filename, "w") != MTAR_ESUCCESS){
        SET_ERROR(SLRET_CANT_CREATE_SL);
        return false;
    }

    if(mtar_write_file_header(&my_tarfile, "library_name", strlen(lib->library_name))         != MTAR_ESUCCESS) goto head_write_fail;
    if(mtar_write_data(&my_tarfile, lib->library_name, strlen(lib->library_name))             != MTAR_ESUCCESS) goto head_write_fail;
    if(mtar_write_file_header(&my_tarfile, "target_arch_name", strlen(lib->target_arch_name)) != MTAR_ESUCCESS) goto head_write_fail;
    if(mtar_write_data(&my_tarfile, lib->target_arch_name, strlen(lib->target_arch_name))     != MTAR_ESUCCESS) goto head_write_fail;

    goto head_write_succes;

head_write_fail:

    SET_ERROR(SLRET_CANT_CREATE_SL);
    return false;

head_write_succes:

    while(head != NULL){

        char *obj_str = NULL;

        if(obj_write_to_string(&obj_str, head) != 0){
            SET_ERROR(SLRET_TMP_WRITE_ERR);
            return false;
        }

        unsigned size = strlen(obj_str) + 1;

        if(mtar_write_file_header(&my_tarfile, head->object_file_name, size) != MTAR_ESUCCESS){
            SET_ERROR(SLRET_CANT_CREATE_SL);
            free(obj_str);
            return false;
        }

        if(mtar_write_data(&my_tarfile, obj_str, size) != MTAR_ESUCCESS){
            SET_ERROR(SLRET_CANT_CREATE_SL);
            free(obj_str);
            return false;
        }

        free(obj_str);

        head = head->next;
    }

    if(mtar_finalize(&my_tarfile) != MTAR_ESUCCESS){
        SET_ERROR(SLRET_CANT_CREATE_SL);
        return false;
    }

    mtar_close(&my_tarfile);

    return true;
}

bool new_sl(char *lib_name, static_library_t **lib){

    if(lib_name == NULL || lib == NULL){
        SET_ERROR(SLRET_NULL_PTR);
        return false;
    }

    *lib = (static_library_t *)malloc(sizeof(static_library_t));
    char * line_1 = malloc(sizeof(char) * (strlen(lib_name) + 1));
    char * line_2 = malloc(sizeof(char) * (strlen(TARGET_ARCH_NAME) + 1));

    if(*lib == NULL || line_1 == NULL || line_2 == NULL){
        SET_ERROR(SLRET_MALLOC_FAIL);

        if(*lib   != NULL) free(lib);
        if(line_1 != NULL) free(line_1);
        if(line_2 != NULL) free(line_2);

        return false;
    }

    strcpy(line_1, lib_name);
    strcpy(line_2, TARGET_ARCH_NAME);

    (*lib)->first_obj = NULL;
    (*lib)->last_obj = NULL;
    (*lib)->library_name = line_1;
    (*lib)->target_arch_name = line_2;

    return true;

}

bool append_objfile_to_sl(obj_file_t *o, static_library_t *lib){

    if(o == NULL || lib == NULL){
        SET_ERROR(SLRET_NULL_PTR);
        return false;
    }

    if(lib->first_obj == NULL){
        if(lib->last_obj != NULL){
            SET_ERROR(SLRET_BROKEN_SL);
            return false;
        }

        lib->first_obj = o;
        lib->last_obj = o;
    }
    else{
        if(lib->last_obj == NULL){
            SET_ERROR(SLRET_BROKEN_SL);
            return false;
        }

        obj_file_t *head = lib->first_obj;
        while(head != NULL){
            if(strcmp(o->object_file_name, head->object_file_name) == 0){
                SET_ERROR(SLRET_OBJ_EXIST_ALREADY);
                return false;
            }
            head = head->next;
        }

        lib->last_obj->next = o;
        o->prev = lib->last_obj;
        lib->last_obj = o;
    }
    return true;
}
