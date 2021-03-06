/**
 * @file obj.c
 *
 * @brief Implementation of library that deal with tasks related with object files.
 *
 * For more informations please see file obj.h.
 *
 * @author Bc. Vladislav Mlejnecký <v.mlejnecky@seznam.cz>
 * @date 15.06.2019
 *
 * @note This file is part of m2tools project.
 */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <inttypes.h>
#include <stdarg.h>
#include <stdbool.h>

#include "obj.h"

#include <isa.h>

#define SET_ERROR(n) if(objlib_errno == 0) objlib_errno = n

typedef enum{
    OBJECT = 0,
    OBJECT_NAME,
    ARCH,
    ARCH_NAME,
    SECTION,
    SECTION_NAME,
    SPEC,
    SPEC_SYMBOL,
    DATA_SYMBOL
}load_decoder_state_t;

typedef struct strbuf_s{
    char *str_ptr;
    long unsigned int actual_lenght;
    long unsigned int total_lenght;
}strbuf_t;

/**
 * @brief Print to string buffer
 *
 * Automatically reallocate buffer if needed.
 *
 * @return -1 if fail; 0 if OK
 */
static bool my_sprintf(strbuf_t *sbuffer, const char *fmt, ...);

/**
 * @brief Create string buffer structure and initialize it
 *
 * @param sbuffer Pointer that will point to new structure, have to be NULL.
 *
 * @return false if fail; true if OK
 */
static bool new_strbuf(strbuf_t **sbuffer);

/**
 * @brief Free and dealoc structure given with new_strbuf
 *
 * @param sbuffer pointer to struct to clear
 */
static void free_strbuf(strbuf_t *sbuffer);

/**
 * @brief Internal function that write out content of object file into strbuf_t
 *
 * @param o Pointer to struct with object file stored.
 *
 * @return pointer to your strbuf_t struct, you have to free it before terminating
 * program, use the free_strbuf function.
 */
static strbuf_t *obj_write_to_strbuf(obj_file_t *o);

/**
 * @brief Internal function that load content of strbuf_t into obj_file_t struct
 *
 * @param strbuf Pointer to strbuf structure with stored content of object file.
 *
 * @return Pointer to your new obj_file_t struct filed with information from strbuf struct.
 * At the end of the program returning pointer should be cleared by free_object_file().
 */
static obj_file_t *obj_load_from_strbuf(strbuf_t *strbuf);

static load_decoder_state_t load_decoder_state;

static obj_file_err_t objlib_errno;

void clear_objlib_errno(void){
    objlib_errno = 0;
}

obj_file_err_t get_objlib_errno(void){
    return objlib_errno;
}

void free_obj_blob_payload(datablob_t *b){
    if(b != NULL){
        free(b->payload);
        free(b);
    }
}

void free_obj_data_symbol(data_symbol_t *symbol){
    if(symbol->type == DATA_IS_BLOB){
        free_obj_blob_payload(symbol->payload.blob);
    }
    else if(symbol->type == DATA_IS_INST){
        free_istruction_struct(symbol->payload.inst);
    }
    else{
        exit(EXIT_FAILURE);
    }

    free(symbol);
}

void free_obj_spec_symbol(spec_symbol_t *symbol){
    free(symbol->name);
    free(symbol);
}

void free_obj_section(section_t *section){
    data_symbol_t *tmp_data, *head_data;
    spec_symbol_t *tmp_spec, *head_spec;

    head_data = section->data_first;

    while(head_data != NULL){
        tmp_data = head_data;
        head_data = head_data->next;

        free_obj_data_symbol(tmp_data);
    }

    head_spec = section->spec_symbol_first;

    while(head_spec != NULL){
        tmp_spec = head_spec;
        head_spec = head_spec->next;

        free_obj_spec_symbol(tmp_spec);
    }

    free(section->section_name);
    free(section);
}

void free_object_file(obj_file_t *o){

    section_t *tmp_section, *head_section;

    if(o == NULL){
        return;
    }

    head_section = o->first_section;

    while(head_section != NULL){
        tmp_section = head_section;
        head_section = head_section->next;

        free_obj_section(tmp_section);
    }

    free(o->object_file_name);
    free(o->target_arch_name);
    free(o);

    return;
}

bool obj_load_from_file(char *filename, obj_file_t **o){

    if(filename == NULL || o == NULL){
        SET_ERROR(OBJRET_NULL_PTR);
        return false;
    }

    if(*o != NULL){
        SET_ERROR(OBJRET_WRONG_ARG);
        return false;
    }

    FILE *fp = fopen(filename, "r");

    if(fp == NULL){
        SET_ERROR(OBJRET_FOPEN_ERROR);
        return false;
    }

    strbuf_t *strbuf = NULL;

    if(!new_strbuf(&strbuf)){
        SET_ERROR(OBJRET_INTERNAL_ERR);
        fclose(fp);
        return false;
    }

    int c;
    while((c = fgetc(fp)) != EOF){
        if(!my_sprintf(strbuf, "%c", (char)c)){
            SET_ERROR(OBJRET_INTERNAL_ERR);
            fclose(fp);
            return false;
        }
    }

    fclose(fp);

    *o = obj_load_from_strbuf(strbuf);

    free_strbuf(strbuf);

    if(*o == NULL){
        SET_ERROR(OBJRET_INTERNAL_ERR);
        return false;
    }

    return true;

}

bool obj_load_from_string(char *s, obj_file_t **o){
    if(s == NULL || o == NULL){
        SET_ERROR(OBJRET_NULL_PTR);
        return false;
    }

    if(*o != NULL){
        SET_ERROR(OBJRET_WRONG_ARG);
        return false;
    }

    char *s_dup = strdup(s);

    strbuf_t strptr;
    strptr.str_ptr = s_dup;
    strptr.actual_lenght = strlen(s_dup) + 1;
    strptr.total_lenght = strptr.actual_lenght;

    *o = obj_load_from_strbuf(&strptr);

    free(s_dup);

    if(*o == NULL){
        SET_ERROR(OBJRET_INTERNAL_ERR);
        return false;
    }

    return true;

}

bool obj_write_to_string(char **s, obj_file_t *o){

    if(o == NULL || s == NULL){
        SET_ERROR(OBJRET_NULL_PTR);
        return false;
    }

    if(*s != NULL){
        SET_ERROR(OBJRET_WRONG_ARG);
        return false;
    }

    //get strbuf for object file
    strbuf_t *strbuf = obj_write_to_strbuf(o);

    //check for nested errors
    if(strbuf == NULL) return -1;

    char *new_string = strdup(strbuf->str_ptr);

    if(new_string == NULL){
        SET_ERROR(OBJRET_INTERNAL_ERR);
        return false;
    }

    *s = new_string;

    return true;
}

bool obj_write_to_file(char *filename, obj_file_t *o){

    if(o == NULL || filename == NULL){
        SET_ERROR(OBJRET_NULL_PTR);
        return false;
    }

    //get strbuf for object file
    strbuf_t *strbuf = obj_write_to_strbuf(o);

    //check for nested errors
    if(strbuf == NULL) return false;

    //write out data into file
    FILE * fp = fopen(filename, "w");

    if(fp == NULL){
        SET_ERROR(OBJRET_FOPEN_ERROR);
        return false;
    }

    fputs(strbuf->str_ptr, fp);
    fflush(fp);

    //close file and clear strbuf
    fclose(fp);
    free_strbuf(strbuf);

    return true;
}

bool new_spec_symbol(char *name, isa_address_t value, symbol_type_t type, spec_symbol_t **s){

    if(name == NULL || s == NULL){
        SET_ERROR(OBJRET_NULL_PTR);
        return false;
    }

    if(*s != NULL){
        SET_ERROR(OBJRET_WRONG_ARG);
        return false;
    }

    spec_symbol_t *tmp_s = (spec_symbol_t *)malloc(sizeof(spec_symbol_t));
    char *line = malloc(sizeof(char) * (strlen(name) + 1));

    if(tmp_s == NULL || line == NULL){
        if(*s != NULL) free(*s);
        if(line != NULL) free(line);
        SET_ERROR(OBJRET_MALLOC_FAIL);
        return false;
    }

    strcpy(line, name);

    tmp_s->name = line;
    tmp_s->value = value;
    tmp_s->prev = NULL;
    tmp_s->next = NULL;
    tmp_s->type = type;

    *s = tmp_s;

    return true;
}

bool new_data_symbol(isa_address_t address, data_symbol_type_t type, void *payload_ptr, data_symbol_t **d){

    if(d == NULL){
        SET_ERROR(OBJRET_NULL_PTR);
        return false;
    }
    if(*d != NULL){
        SET_ERROR(OBJRET_WRONG_ARG);
        return false;
    }

    data_symbol_t *tmp_d = (data_symbol_t *)malloc(sizeof(data_symbol_t));

    if(tmp_d == NULL){
        SET_ERROR(OBJRET_MALLOC_FAIL);
        return false;
    }

    tmp_d->prev = NULL;
    tmp_d->next = NULL;
    tmp_d->address = address;
    tmp_d->type = type;
    tmp_d->relocation = false;
    tmp_d->special = false;

    if(type == DATA_IS_BLOB){
        tmp_d->payload.blob = (datablob_t *)payload_ptr;
    }
    else if(type == DATA_IS_INST){
        tmp_d->payload.inst = (tInstruction *)payload_ptr;
    }
    else{
        SET_ERROR(OBJRET_INTERNAL_ERR);
        free(tmp_d);
        return false;
    }

    *d = tmp_d;

    return true;

}

bool new_obj(char * object_file_name, obj_file_t **o){

    if(object_file_name == NULL || o == NULL){
        SET_ERROR(OBJRET_NULL_PTR);
        return false;
    }

    *o = (obj_file_t *)malloc(sizeof(obj_file_t));
    char * line_1 = malloc(sizeof(char) * (strlen(object_file_name) + 1));
    char * line_2 = malloc(sizeof(char) * (strlen(TARGET_ARCH_NAME) + 1));

    if(*o == NULL || line_1 == NULL || line_2 == NULL){
        SET_ERROR(OBJRET_MALLOC_FAIL);
        return false;
    }

    strcpy(line_1, object_file_name);
    strcpy(line_2, TARGET_ARCH_NAME);

    (*o)->first_section = NULL;
    (*o)->last_section = NULL;
    (*o)->object_file_name = line_1;
    (*o)->target_arch_name = line_2;
    (*o)->next = NULL;
    (*o)->prev = NULL;

    return true;

}

bool new_section(char *section_name, section_t **s){

    if(section_name == NULL || s == NULL){
        SET_ERROR(OBJRET_NULL_PTR);
        return false;
    }

    *s = (section_t *)malloc(sizeof(section_t));
    char * line = malloc(sizeof(char) * (strlen(section_name) + 1));

    if(*s == NULL || line == NULL){
        SET_ERROR(OBJRET_MALLOC_FAIL);
        return false;
    }

    strcpy(line, section_name);

    (*s)->spec_symbol_first = NULL;
    (*s)->spec_symbol_last = NULL;
    (*s)->data_first = NULL;
    (*s)->data_last = NULL;
    (*s)->prev = NULL;
    (*s)->next = NULL;
    (*s)->section_name = line;

    return true;
}

bool new_blob(unsigned int lenght, datablob_t **b){
    if(b == NULL){
        SET_ERROR(OBJRET_NULL_PTR);
        return false;
    }

    *b = (datablob_t *)malloc(sizeof(datablob_t));
    uint8_t *payload = (uint8_t *) malloc(sizeof(uint8_t) * lenght);

    if(*b == NULL || payload == NULL){
        SET_ERROR(OBJRET_MALLOC_FAIL);
        return false;
    }

    (*b)->payload = payload;
    (*b)->lenght = lenght;

    for(unsigned int i = 0; i < lenght; i++) (*b)->payload[i] = 0;

    return true;
}

bool append_section_to_obj(obj_file_t *o, section_t *s){

    if(o == NULL || s == NULL){
        SET_ERROR(OBJRET_NULL_PTR);
        return false;
    }

    if(o->first_section == NULL){

        if(o->last_section != NULL){
            SET_ERROR(OBJRET_BROKEN_OBJ);
            return false;
        }

        o->first_section = s;
        o->last_section = s;

    }
    else{

        if(o->last_section == NULL){
            SET_ERROR(OBJRET_BROKEN_OBJ);
            return false;
        }

        section_t *head = o->first_section;

        while(head != NULL){

            if(strcmp(s->section_name, head->section_name) == 0){
                SET_ERROR(OBJRET_SECTION_EXIST_ALREADY);
                return false;
            }

            head = head->next;
        }


        o->last_section->next = s;
        s->prev = o->last_section;
        o->last_section = s;
    }

    return true;

}

bool append_spec_symbol_to_section(section_t *section, spec_symbol_t *symbol){

    if(section == NULL || symbol == NULL){
        SET_ERROR(OBJRET_NULL_PTR);
        return false;
    }

    if(section->spec_symbol_first == NULL){

        if(section->spec_symbol_last != NULL){
            SET_ERROR(OBJRET_BROKEN_SECTION);
            return false;
        }

        section->spec_symbol_first = symbol;
        section->spec_symbol_last = symbol;
    }
    else{

        if(section->spec_symbol_last == NULL){
            SET_ERROR(OBJRET_BROKEN_SECTION);
            return false;
        }

        section->spec_symbol_last->next = symbol;
        symbol->prev = section->spec_symbol_last;
        section->spec_symbol_last = symbol;

    }

    return true;

}

bool append_data_symbol_to_section(section_t *section, data_symbol_t *data){

    if(section == NULL || data == NULL){
        SET_ERROR(OBJRET_NULL_PTR);
        return false;
    }

    if(section->data_first == NULL){

        if(section->data_last != NULL){
            SET_ERROR(OBJRET_BROKEN_SECTION);
            return false;
        }

        section->data_first = data;
        section->data_last = data;
    }
    else{

        if(section->data_last == NULL){
            SET_ERROR(OBJRET_BROKEN_SECTION);
            return false;
        }

        section->data_last->next = data;
        data->prev = section->data_last;
        section->data_last = data;
    }

    return true;

}

static bool new_strbuf(strbuf_t **sbuffer){
    if(*sbuffer != NULL){
        SET_ERROR(OBJRET_WRONG_ARG);
        return false;
    }

    *sbuffer = (strbuf_t *)malloc(sizeof(strbuf_t));

    if(*sbuffer == NULL){
        SET_ERROR(OBJRET_MALLOC_FAIL);
        return false;
    }

    (*sbuffer)->actual_lenght = 0;
    (*sbuffer)->total_lenght = 64;
    (*sbuffer)->str_ptr = (char *)malloc(sizeof(char) * (*sbuffer)->total_lenght);

    if((*sbuffer)->str_ptr == NULL){
        SET_ERROR(OBJRET_MALLOC_FAIL);
        return false;
    }

    (*sbuffer)->str_ptr[0] = '\0';

    return true;
}

static bool my_sprintf(strbuf_t *sbuffer, const char *fmt, ...){

    va_list argptr;
    va_list argptr_2;
    char *tmp_str = NULL;

    if(sbuffer == NULL){
        SET_ERROR(OBJRET_NULL_PTR);
        return false;
    }

    va_start(argptr, fmt);
    va_copy(argptr_2, argptr);

    int vsnprintf_ret = vsnprintf(NULL, 0, fmt, argptr);

    if(vsnprintf_ret < 0){
        SET_ERROR(OBJRET_INTERNAL_ERR);
        return false;
    }

    long unsigned int char_count_to_print = (long unsigned int)vsnprintf_ret;

    if((sbuffer->actual_lenght + char_count_to_print + 1) >= sbuffer->total_lenght){
        void *ptr = realloc(sbuffer->str_ptr, sbuffer->total_lenght * 2);

        if(ptr == NULL){
            SET_ERROR(OBJRET_MALLOC_FAIL);
            return false;
        }

        sbuffer->str_ptr = (char *)ptr;
        sbuffer->total_lenght *= 2;
    }

    tmp_str = (char *)malloc(sizeof(char) * (char_count_to_print + 1));

    if(tmp_str == NULL){
        SET_ERROR(OBJRET_MALLOC_FAIL);
        return false;
    }

    memset((void *)tmp_str, 0, sizeof(char) * (char_count_to_print + 1));

    vsprintf(tmp_str, fmt, argptr_2);
    va_end(argptr_2);

    strcat(sbuffer->str_ptr, tmp_str);
    sbuffer->actual_lenght += char_count_to_print;
    free(tmp_str);

    return true;
}

static strbuf_t *obj_write_to_strbuf(obj_file_t *o){

    strbuf_t* strbuf = NULL;

    if(!new_strbuf(&strbuf)){
        return NULL;
    }

    my_sprintf(strbuf, ".object\n%s\n", o->object_file_name);
    my_sprintf(strbuf, ".arch\n%s\n", o->target_arch_name);

    //go for all sections

    section_t *head_section = o->first_section;

    while(head_section != NULL){
        my_sprintf(strbuf, ".section\n%s\n", head_section->section_name);

        //print symbol table of that section
        my_sprintf(strbuf, ".spec\n");

        spec_symbol_t * head_spec_symbol = head_section->spec_symbol_first;

        while(head_spec_symbol != NULL){

            my_sprintf(strbuf, "%s:" PRIisa_addr ":", head_spec_symbol->name, head_spec_symbol->value);

            if(head_spec_symbol->type == SYMBOL_EXPORT){
                my_sprintf(strbuf, "export\n");
            }
            else if(head_spec_symbol->type == SYMBOL_IMPORT){
                my_sprintf(strbuf, "import\n");
            }
            else{
                SET_ERROR(OBJRET_INTERNAL_ERR);
                free_strbuf(strbuf);
                return NULL;
            }

            head_spec_symbol = head_spec_symbol->next;

        }

        my_sprintf(strbuf, ".data\n");

        data_symbol_t *head_data = head_section->data_first;

        while(head_data != NULL){
            if(head_data->type == DATA_IS_BLOB){

                my_sprintf(strbuf, "B:");

                if(head_data->payload.blob->lenght > 0){

                    my_sprintf(strbuf, PRIisa_addr ":0x%" PRIx8, head_data->address, head_data->payload.blob->payload[0]);
                    for(unsigned int i = 1; i < head_data->payload.blob->lenght; i++){
                        my_sprintf(strbuf, ":0x%" PRIx8, head_data->payload.blob->payload[i]);
                    }
                    my_sprintf(strbuf, "\n");

                }
                else{
                    SET_ERROR(OBJRET_BROKEN_OBJ);
                    free_strbuf(strbuf);
                    return NULL;
                }

            }
            else if(head_data->type == DATA_IS_INST){

                my_sprintf(strbuf, "I:");

                char *line = (char *)malloc(sizeof(char) * 64);

                if(line == NULL){
                    SET_ERROR(OBJRET_MALLOC_FAIL);
                    free_strbuf(strbuf);
                    return NULL;
                }

                if(!export_into_object_file_line(head_data->payload.inst, line)){
                    SET_ERROR(OBJRET_INTERNAL_ERR);
                    free(line);
                    free_strbuf(strbuf);
                    return NULL;
                }

                uint8_t rel = 0;
                uint8_t spec = 0;

                if(head_data->relocation == true) rel = 1;
                if(head_data->special == true) spec = 1;

                my_sprintf(strbuf, PRIisa_addr":%"PRIx8":%"PRIx8":%s\n", head_data->address, rel, spec, line);

                free(line);
            }
            else{
                SET_ERROR(OBJRET_INTERNAL_ERR);
                free_strbuf(strbuf);
                return NULL;
            }

            head_data = head_data->next;
        }

        head_section = head_section->next;
    }

    my_sprintf(strbuf, ".end\n");

    return strbuf;

}

static void free_strbuf(strbuf_t *sbuffer){
    if(sbuffer == NULL) return;

    free(sbuffer->str_ptr);
    free(sbuffer);
}

static obj_file_t *obj_load_from_strbuf(strbuf_t *strbuf){

    if(strbuf == NULL){
        SET_ERROR(OBJRET_NULL_PTR);
        return NULL;
    }

    load_decoder_state = OBJECT;
    char line[128];

    obj_file_t *my_new_obj = NULL;
    section_t *my_new_section = NULL;

    int end_of_file = 0;
    unsigned int strbufpos = 0;

    while(end_of_file == 0){

        memset((void *)line, '\0', sizeof(line));

        {
            unsigned int i = 0;
            while(
                (strbuf->str_ptr[strbufpos] != '\0') &&
                (strbuf->str_ptr[strbufpos] != '\n') &&
                (strbuf->str_ptr[strbufpos] != '\r') &&
                (i < (sizeof(line) - 1))
            ){
                line[i++] = strbuf->str_ptr[strbufpos++];
            }
        }

        strbufpos++;

        switch(load_decoder_state){
            case OBJECT:
                {
                    if(strcmp(line, ".object") == 0){
                        load_decoder_state = OBJECT_NAME;
                    }
                    else{
                        SET_ERROR(OBJRET_BROKEN_FILE);
                        return NULL;
                    }
                }
                break;
            case OBJECT_NAME:
                {
                    if(!new_obj(line, &my_new_obj)){
                        return NULL;
                    }

                    load_decoder_state = ARCH;
                }
                break;
            case ARCH:
                {
                    if(strcmp(line, ".arch") == 0){
                        load_decoder_state = ARCH_NAME;
                    }
                    else{
                        SET_ERROR(OBJRET_BROKEN_FILE);
                        return NULL;
                    }
                }
                break;
            case ARCH_NAME:
                {
                    if(strcmp(line, TARGET_ARCH_NAME) == 0){
                        load_decoder_state = SECTION;
                    }
                    else{
                        SET_ERROR(OBJRET_WRONG_ARCH);
                        return NULL;
                    }
                }
                break;
            case SECTION:
                {
                    if(strcmp(line, ".section") == 0){
                        load_decoder_state = SECTION_NAME;
                    }
                    else{
                        SET_ERROR(OBJRET_BROKEN_FILE);
                        return NULL;
                    }
section_care:
                    if(my_new_section != NULL){
                        if(!append_section_to_obj(my_new_obj, my_new_section)){
                            return NULL;
                        }
                        my_new_section = NULL;
                    }
                }
                break;
            case SECTION_NAME:
                {
                    if(!new_section(line, &my_new_section)){
                        return NULL;
                    }

                    load_decoder_state = SPEC;
                }
                break;
            case SPEC:
                {
                    if(strcmp(line, ".spec") == 0){
                        load_decoder_state = SPEC_SYMBOL;
                    }
                    else{
                        SET_ERROR(OBJRET_BROKEN_FILE);
                        return NULL;
                    }
                }
                break;
            case SPEC_SYMBOL:
                {
                    if(strcmp(line, ".data") == 0){
                        load_decoder_state = DATA_SYMBOL;
                    }
                    else{
                        spec_symbol_t *new_symbol = NULL;

                        isa_address_t value;
                        char *type_s = (char *)malloc(sizeof(char) * 128);
                        char *name = (char *)malloc(sizeof(char) * 128);

                        for(int i = 0; line[i] != '\0'; i++) if(line[i] == ':') line[i] = ' ';

                        if(sscanf(line, "%s "SCNisa_addr" %s", name, &value, type_s) != 3){
                            SET_ERROR(OBJRET_BROKEN_FILE);
                            return NULL;
                        }

                        symbol_type_t type;
                        if(strcmp(type_s, "export") == 0){
                            type = SYMBOL_EXPORT;
                        }
                        else if(strcmp(type_s, "import") == 0){
                            type = SYMBOL_IMPORT;
                        }
                        else{
                            SET_ERROR(OBJRET_BROKEN_FILE);
                            return NULL;
                        }

                        if(!new_spec_symbol(name, value, type, &new_symbol)){
                            return NULL;
                        }

                        if(!append_spec_symbol_to_section(my_new_section, new_symbol)){
                            return NULL;
                        }

                        free(type_s);
                        free(name);
                    }
                }
                break;
            case DATA_SYMBOL:
                {
                    if(strcmp(line, ".section") == 0){
                        load_decoder_state = SECTION_NAME;
                        goto section_care;
                    }
                    else if(strcmp(line, ".end") == 0){
                        end_of_file = 1;
                        goto section_care;
                    }
                    else{

                        if(line[0] == 'B'){

                            isa_address_t address;
                            char buff[80];
                            data_symbol_t *new_data = NULL;
                            int base = 2;

                            datablob_t *blob = NULL;

                            for(int i = 2; line[i] != ':'; i++){
                                if(i == 80){
                                    SET_ERROR(OBJRET_INTERNAL_ERR);
                                    return NULL;
                                }

                                buff[i - 2] = line[i];
                                buff[i - 1] = '\0';
                            }

                            if(sscanf(buff, SCNisa_addr , &address) != 1){
                                SET_ERROR(OBJRET_BROKEN_FILE);
                                return NULL;
                            }

                            uint8_t num_buff[80];
                            unsigned int num_buff_index = 0;

                            while(1){
                                base += (int)strlen(buff) + 1;

                                for(int i = base; line[i] != ':'; i++){

                                    if(line[i] == '\0') break;

                                    if(i == 80){
                                        SET_ERROR(OBJRET_INTERNAL_ERR);
                                        return NULL;
                                    }

                                    buff[i - base] = line[i];
                                    buff[i - base + 1] = '\0';
                                }

                                uint8_t num = 0;

                                if(sscanf(buff, "0x%" SCNx8, &num) != 1){
                                    SET_ERROR(OBJRET_BROKEN_FILE);
                                    return NULL;
                                }

                                num_buff[num_buff_index++] = num;

                                if(line[base + (int)strlen(buff) + 1] == '\0'){
                                    break;
                                }

                            }

                            if(!new_blob(num_buff_index, &blob)){
                                SET_ERROR(OBJRET_INTERNAL_ERR);
                                return NULL;
                            }

                            for(unsigned int i = 0; i < num_buff_index; i++){
                                blob->payload[i] = num_buff[i];
                            }

                            if(!new_data_symbol(address, DATA_IS_BLOB, (void *)blob, &new_data)){
                                return NULL;
                            }

                            if(!append_data_symbol_to_section(my_new_section, new_data)){
                                return NULL;
                            }

                        }
                        else if(line[0] == 'I'){
                            isa_address_t address = 0;
                            uint8_t relocation = 0;
                            uint8_t special = 0;

                            char *line_for_isalib = NULL;
                            char *ptr_for_isalib = NULL;
                            char *linedup = NULL;
                            char *ptr_for_objlib = NULL;
                            data_symbol_t *new_data = NULL;
                            tInstruction *inst = new_instru();

                            line_for_isalib = strdup(line);
                            linedup = strdup(line);

                            if(inst == NULL || line_for_isalib == NULL || linedup == NULL){
                                SET_ERROR(OBJRET_MALLOC_FAIL);
                                return NULL;
                            }

                            for(int i = (int)strlen(line_for_isalib); i >= 0; i--){
                                if(i == 0){
                                    SET_ERROR(OBJRET_BROKEN_FILE);
                                    return NULL;
                                }
                                if(line_for_isalib[i] == ':'){
                                    line_for_isalib[i] = '\0';
                                    linedup[i] = '\0';
                                    break;
                                }
                            }

                            ptr_for_isalib = line_for_isalib + strlen(line_for_isalib) + 1;
                            ptr_for_objlib = linedup + 2;


                            if(sscanf(ptr_for_objlib, SCNisa_addr":%"SCNx8":%"SCNx8, &address, &relocation, &special) != 3){
                                SET_ERROR(OBJRET_BROKEN_FILE);
                                return NULL;
                            }

                            if(!import_from_object_file_line(inst, ptr_for_isalib)){
                                SET_ERROR(OBJRET_INTERNAL_ERR);
                                return NULL;
                            }

                            if(!new_data_symbol(address, DATA_IS_INST, (void *)inst, &new_data)){
                                return NULL;
                            }

                            if(relocation == 1) new_data->relocation = true;
                            if(special == 1) new_data->special = true;

                            if(!append_data_symbol_to_section(my_new_section, new_data)){
                                return NULL;
                            }

                            free(line_for_isalib);
                            free(linedup);

                        }
                        else{
                            SET_ERROR(OBJRET_BROKEN_FILE);
                            return NULL;
                        }
                    }
                }

                break;
            default:
                SET_ERROR(OBJRET_INTERNAL_ERR);
                return NULL;
        }
    }

    return my_new_obj;
}
