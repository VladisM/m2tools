#include "sl.h"

#include <stdlib.h>
#include <string.h>

#include <obj.h>
#include <microtar.h>

#define SET_ERROR(n) if(sllib_errno == 0) sllib_errno = n

//TODO: přidat dokumentaci
//TODO: přidat example použití
//TODO: zkoušet úniky dyn. paměti
//TODO: přidat ošetření na uvolnění dyn. paměti při chybě

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

int sl_load(char *filename, static_library_t **lib){

    if(filename == NULL || lib == NULL){
        SET_ERROR(SLRET_NULL_PTR);
        return -1;
    }

    if(*lib != NULL){
        SET_ERROR(SLRET_LIB_EXIST_ALREADY);
        return -1;
    }

    mtar_t my_tarfile;
    mtar_header_t file_header;

    if(mtar_open(&my_tarfile, filename, "r") != MTAR_ESUCCESS){
        SET_ERROR(SLRET_CANT_OPEN_SL);
        return -1;
    }

    if(mtar_find(&my_tarfile, "target_arch_name", &file_header) != MTAR_ESUCCESS){
        SET_ERROR(SLRET_BROKEN_SL);
        return -1;
    }

    char *target_arch_name = calloc(1, file_header.size + 1);

    if(target_arch_name == NULL){
        SET_ERROR(SLRET_MALLOC_FAIL);
        return -1;
    }

    if(mtar_read_data(&my_tarfile, target_arch_name, file_header.size) != MTAR_ESUCCESS){
        SET_ERROR(SLRET_INTERN_ERR);
        return -1;
    }

    if(strcmp(target_arch_name, TARGET_ARCH_NAME) != 0){
        SET_ERROR(SLRET_WRONG_ARCH);
        return -1;
    }

    if(mtar_find(&my_tarfile, "library_name", &file_header) != MTAR_ESUCCESS){
        SET_ERROR(SLRET_BROKEN_SL);
        return -1;
    }

    char *library_name = calloc(1, file_header.size + 1);

    if(library_name == NULL){
        SET_ERROR(SLRET_MALLOC_FAIL);
        return -1;
    }

    if(mtar_read_data(&my_tarfile, library_name, file_header.size) != MTAR_ESUCCESS){
        SET_ERROR(SLRET_INTERN_ERR);
        return -1;
    }

    if(new_sl(library_name, lib) != 0){
        SET_ERROR(SLRET_INTERN_ERR);
        return -1;
    }

    free(target_arch_name);
    free(library_name);

    if(mtar_rewind(&my_tarfile) != MTAR_ESUCCESS){
        SET_ERROR(SLRET_INTERN_ERR);
        return -1;
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
                return -1;
            }

            if(mtar_read_data(&my_tarfile, data, file_header.size) != MTAR_ESUCCESS){
                SET_ERROR(SLRET_INTERN_ERR);
                return -1;
            }

            char *tmp_filename = tmpnam(NULL);

            if(tmp_filename == NULL){
                SET_ERROR(SLRET_INTERN_ERR);
                return -1;
            }

            FILE * obj_fp = fopen(tmp_filename, "w");

            for(unsigned i = 0; data[i] != '\0'; i++) fputc(data[i], obj_fp);

            fclose(obj_fp);
            free(data);

            obj_file_t *tmp_obj = NULL;

            if(obj_load(tmp_filename, &tmp_obj) != 0){
                SET_ERROR(SLRET_INTERN_ERR);
                return -1;
            }

            remove(tmp_filename);

            if(append_objfile_to_sl(tmp_obj, *lib) != 0){
                SET_ERROR(SLRET_INTERN_ERR);
                return -1;
            }

            mtar_next(&my_tarfile);
        }
    }

    mtar_close(&my_tarfile);
    return 0;

}

int sl_write(char *filename, static_library_t *lib){

    if(filename == NULL || lib == NULL){
        SET_ERROR(SLRET_NULL_PTR);
        return -1;
    }

    mtar_t my_tarfile;
    char line_buff[128];
    obj_file_t *head = lib->first_obj;

    if(mtar_open(&my_tarfile, filename, "w") != MTAR_ESUCCESS){
        SET_ERROR(SLRET_CANT_CREATE_SL);
        return -1;
    }

    if(mtar_write_file_header(&my_tarfile, "library_name", strlen(lib->library_name)) != MTAR_ESUCCESS) goto head_write_fail;
    if(mtar_write_data(&my_tarfile, lib->library_name, strlen(lib->library_name)) != MTAR_ESUCCESS) goto head_write_fail;
    if(mtar_write_file_header(&my_tarfile, "target_arch_name", strlen(lib->target_arch_name)) != MTAR_ESUCCESS) goto head_write_fail;
    if(mtar_write_data(&my_tarfile, lib->target_arch_name, strlen(lib->target_arch_name)) != MTAR_ESUCCESS) goto head_write_fail;

    goto head_write_succes;

head_write_fail:

    SET_ERROR(SLRET_CANT_CREATE_SL);
    return -1;

head_write_succes:

    while(head != NULL){

        char *tmp_filename = tmpnam(NULL);

        if(tmp_filename == NULL){
            SET_ERROR(SLRET_INTERN_ERR);
            return -1;
        }

        if(obj_write(tmp_filename, head) != OBJRET_OK){
            SET_ERROR(SLRET_TMP_WRITE_ERR);
            return -1;
        }

        FILE * obj_fp = fopen(tmp_filename, "r");

        if(obj_fp == NULL){
            SET_ERROR(SLRET_INTERN_ERR);
            return -1;
        }

        unsigned size = 0;
        while((fgetc(obj_fp)) != EOF){
            size++;
        }

        if(mtar_write_file_header(&my_tarfile, head->object_file_name, size) != MTAR_ESUCCESS){
            SET_ERROR(SLRET_CANT_CREATE_SL);
            return -1;
        }

        rewind(obj_fp);

        while(fgets(line_buff, 128, obj_fp) != NULL){
            if(mtar_write_data(&my_tarfile, line_buff, strlen(line_buff)) != MTAR_ESUCCESS){
                SET_ERROR(SLRET_CANT_CREATE_SL);
                return -1;
            }
        }

        fclose(obj_fp);
        remove(tmp_filename);

        head = head->next;
    }

    if(mtar_finalize(&my_tarfile) != MTAR_ESUCCESS){
        SET_ERROR(SLRET_CANT_CREATE_SL);
        return -1;
    }

    mtar_close(&my_tarfile);

    return 0;
}

int new_sl(char *lib_name, static_library_t **lib){

    if(lib_name == NULL || lib == NULL){
        SET_ERROR(SLRET_NULL_PTR);
        return -1;
    }

    *lib = (static_library_t *)malloc(sizeof(static_library_t));
    char * line_1 = malloc(sizeof(char) * (strlen(lib_name) + 1));
    char * line_2 = malloc(sizeof(char) * (strlen(TARGET_ARCH_NAME) + 1));

    if(*lib == NULL || line_1 == NULL || line_2 == NULL){
        SET_ERROR(SLRET_MALLOC_FAIL);
        return -1;
    }

    strcpy(line_1, lib_name);
    strcpy(line_2, TARGET_ARCH_NAME);

    (*lib)->first_obj = NULL;
    (*lib)->last_obj = NULL;
    (*lib)->library_name = line_1;
    (*lib)->target_arch_name = line_2;

    return 0;

}

int append_objfile_to_sl(obj_file_t *o, static_library_t *lib){

    if(o == NULL || lib == NULL){
        SET_ERROR(SLRET_NULL_PTR);
        return -1;
    }

    if(lib->first_obj == NULL){
        if(lib->last_obj != NULL){
            SET_ERROR(SLRET_BROKEN_SL);
            return -1;
        }

        lib->first_obj = o;
        lib->last_obj = o;
    }
    else{
        if(lib->last_obj == NULL){
            SET_ERROR(SLRET_BROKEN_SL);
            return -1;
        }

        obj_file_t *head = lib->first_obj;
        while(head != NULL){
            if(strcmp(o->object_file_name, head->object_file_name) == 0){
                SET_ERROR(SLRET_OBJ_EXIST_ALREADY);
                return -1;
            }
            head = head->next;
        }

        lib->last_obj->next = o;
        o->prev = lib->last_obj;
        lib->last_obj = o;
    }
    return 0;
}

