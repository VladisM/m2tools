#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <inttypes.h>
#include <stdarg.h>

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
    unsigned int actual_lenght;
    unsigned int total_lenght;
}strbuf_t;

/**
 * @brief Print to string buffer
 *
 * Automatically reallocate buffer if needed.
 *
 * @return -1 if fail; 0 if OK
 */
static int my_sprintf(strbuf_t *sbuffer, char *fmt, ...);

/**
 * @brief Create string buffer structure and initialize it
 *
 * @param sbuffer Pointer that will point to new structure, have to be NULL.
 *
 * @return -1 if fail; 0 if OK
 */
static int new_strbuf(strbuf_t **sbuffer);

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

static obj_file_t *obj_load_from_strbuf(strbuf_t *strbuf);

load_decoder_state_t load_decoder_state;

static obj_file_err_t objlib_errno;

void clear_objlib_errno(void){
    objlib_errno = 0;
}

obj_file_err_t get_objlib_errno(void){
    return objlib_errno;
}

void free_object_file(obj_file_t *o){

    section_t *tmp_section, *head_section;
    data_symbol_t *tmp_data, *head_data;
    spec_symbol_t *tmp_spec, *head_spec;

    if(o == NULL){
        return;
    }

    head_section = o->first_section;

    while(head_section != NULL){
        tmp_section = head_section;
        head_section = head_section->next;

        head_data = tmp_section->data_first;

        while(head_data != NULL){
            tmp_data = head_data;
            head_data = head_data->next;

            if(tmp_data->type == DATA_IS_BLOB){
                free(tmp_data->payload.blob->payload);
                free(tmp_data->payload.blob);
            }
            else if(tmp_data->type == DATA_IS_INST){
                free_istruction_struct(tmp_data->payload.inst);
            }
            else{
                exit(EXIT_FAILURE);
            }

            free(tmp_data);
        }

        head_spec = tmp_section->spec_symbol_first;

        while(head_spec != NULL){
            tmp_spec = head_spec;
            head_spec = head_spec->next;

            free(tmp_spec->name);
            free(tmp_spec);
        }

        free(tmp_section->section_name);
        free(tmp_section);
    }

    free(o->object_file_name);
    free(o->target_arch_name);
    free(o);
}

int obj_load_from_file(char *filename, obj_file_t **o){

}

int obj_load_from_string(char *s, obj_file_t **o){
    if(s == NULL || o == NULL){
        SET_ERROR(OBJRET_NULL_PTR);
        return -1;
    }

    if(*o != NULL){
        SET_ERROR(OBJRET_WRONG_ARG);
        return -1;
    }

    char *s_dup = strdup(s);

    strbuf_t strptr;
    strptr.str_ptr = s_dup;
    strptr.actual_lenght = strlen(s_dup) + 1;
    strptr.total_lenght = strptr.actual_lenght;

    *o = obj_load_from_strbuf(&strptr);

    free(s_dup);

    if(*o == NULL){
        return -1;
    }
    else{
        return 0;
    }
}

int obj_load(char *filename, obj_file_t **o){

    if(filename == NULL || o == NULL){
        SET_ERROR(OBJRET_NULL_PTR);
        return -1;
    }

    if(*o != NULL){
        SET_ERROR(OBJRET_OBJ_EXIST_ALREADY);
        return -1;
    }

    FILE * fp = fopen(filename, "r");

    if(fp == NULL){
        SET_ERROR(OBJRET_FOPEN_ERROR);
        return -1;
    }

    load_decoder_state = OBJECT;
    char * line = (char *)malloc(sizeof(char) * 128);

    if(line == NULL){
        SET_ERROR(OBJRET_MALLOC_FAIL);
        return -1;
    }

    obj_file_t *my_new_obj = NULL;
    section_t *my_new_section = NULL;

    int end_of_file = 0;

    while(end_of_file == 0){

        char * ret = fgets(line, 128, fp);

        if(ret == NULL){
            SET_ERROR(OBJRET_BROKEN_FILE);
            return -1;
        }

        for(int i = 0; line[i] != '\0'; i++){
            if(line[i] == '\n' || line[i] == '\r'){
                line[i] = '\0';
            }
        }

        switch(load_decoder_state){
            case OBJECT:
                {
                    if(strcmp(line, ".object") == 0){
                        load_decoder_state = OBJECT_NAME;
                    }
                    else{
                        SET_ERROR(OBJRET_BROKEN_FILE);
                        return -1;
                    }
                }
                break;
            case OBJECT_NAME:
                {
                    if(new_obj(line, &my_new_obj)){
                        return -1;
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
                        return -1;
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
                        return -1;
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
                        return -1;
                    }
section_care:
                    if(my_new_section != NULL){
                        if(append_section_to_obj(my_new_obj, my_new_section)){
                            return -1;
                        }
                        my_new_section = NULL;
                    }
                }
                break;
            case SECTION_NAME:
                {
                    if(new_section(line, &my_new_section)){
                        return -1;
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
                        return -1;
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

                        uint32_t value;
                        char *type_s = (char *)malloc(sizeof(char) * 128);
                        char *name = (char *)malloc(sizeof(char) * 128);

                        for(int i = 0; line[i] != '\0'; i++) if(line[i] == ':') line[i] = ' ';

                        if(sscanf(line, "%s 0x%X %s", name, &value, type_s) != 3){
                            SET_ERROR(OBJRET_BROKEN_FILE);
                            return -1;
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
                            return -1;
                        }

                        if(new_spec_symbol(name, value, type, &new_symbol)){
                            return -1;
                        }

                        if(append_spec_symbol_to_section(my_new_section, new_symbol)){
                            return -1;
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

                            uint32_t address;
                            char buff[80];
                            data_symbol_t *new_data = NULL;
                            int base = 2;

                            datablob_t *blob = NULL;

                            for(int i = 2; line[i] != ':'; i++){
                                if(i == 80){
                                    SET_ERROR(OBJRET_INTERNAL_ERR);
                                    return -1;
                                }

                                buff[i - 2] = line[i];
                                buff[i - 1] = '\0';
                            }

                            if(sscanf(buff, "0x%"SCNx32 , &address) != 1){
                                SET_ERROR(OBJRET_BROKEN_FILE);
                                return -1;
                            }

                            uint8_t num_buff[80];
                            unsigned int num_buff_index = 0;

                            while(1){
                                base += (int)strlen(buff) + 1;

                                for(int i = base; line[i] != ':'; i++){

                                    if(line[i] == '\0') break;

                                    if(i == 80){
                                        SET_ERROR(OBJRET_INTERNAL_ERR);
                                        return -1;
                                    }

                                    buff[i - base] = line[i];
                                    buff[i - base + 1] = '\0';
                                }

                                uint8_t num = 0;

                                if(sscanf(buff, "0x%" SCNx8, &num) != 1){
                                    SET_ERROR(OBJRET_BROKEN_FILE);
                                    return -1;
                                }

                                num_buff[num_buff_index++] = num;

                                if(line[base + (int)strlen(buff) + 1] == '\0'){
                                    break;
                                }

                            }

                            if(new_blob(num_buff_index, &blob) != 0){
                                SET_ERROR(OBJRET_INTERNAL_ERR);
                                return -1;
                            }

                            for(unsigned int i = 0; i < num_buff_index; i++){
                                blob->payload[i] = num_buff[i];
                            }

                            if(new_data_symbol(address, DATA_IS_BLOB, (void *)blob, &new_data)){
                                return -1;
                            }

                            if(append_data_symbol_to_section(my_new_section, new_data)){
                                return -1;
                            }

                        }
                        else if(line[0] == 'I'){
                            uint32_t address = 0;
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
                                return -1;
                            }

                            for(int i = (int)strlen(line_for_isalib); i >= 0; i--){
                                if(i == 0){
                                    SET_ERROR(OBJRET_BROKEN_FILE);
                                    return -1;
                                }
                                if(line_for_isalib[i] == ':'){
                                    line_for_isalib[i] = '\0';
                                    linedup[i] = '\0';
                                    break;
                                }
                            }

                            ptr_for_isalib = line_for_isalib + strlen(line_for_isalib) + 1;
                            ptr_for_objlib = linedup + 2;


                            if(sscanf(ptr_for_objlib, "0x%"SCNx32":%"SCNx8":%"SCNx8, &address, &relocation, &special) != 3){
                                SET_ERROR(OBJRET_BROKEN_FILE);
                                return -1;
                            }

                            if(import_from_object_file_line(inst, ptr_for_isalib) != 1){
                                SET_ERROR(OBJRET_INTERNAL_ERR);
                                return -1;
                            }

                            if(new_data_symbol(address, DATA_IS_INST, (void *)inst, &new_data)){
                                return -1;
                            }

                            new_data->relocation = relocation;
                            new_data->special = special;

                            if(append_data_symbol_to_section(my_new_section, new_data)){
                                return -1;
                            }

                            free(line_for_isalib);
                            free(linedup);

                        }
                        else{
                            SET_ERROR(OBJRET_BROKEN_FILE);
                            return -1;
                        }
                    }
                }

                break;
            default:
                SET_ERROR(OBJRET_INTERNAL_ERR);
                return -1;
        }
    }

    free(line);
    fclose(fp);

    *o = my_new_obj;

    return 0;

}

int obj_write_to_string(char **s, obj_file_t *o){

    if(o == NULL){
        SET_ERROR(OBJRET_NULL_PTR);
        return -1;
    }

    if(s != NULL){
        SET_ERROR(OBJRET_WRONG_ARG);
        return -1;
    }

    //get strbuf for object file
    strbuf_t *strbuf = obj_write_to_strbuf(o);

    //check for nested errors
    if(strbuf == NULL) return -1;

    char *new_string = strdup(strbuf->str_ptr);

    if(new_string == NULL){
        SET_ERROR(OBJRET_INTERNAL_ERR);
        return -1;
    }

    *s = new_string;
}

int obj_write_to_file(char *filename, obj_file_t *o){

    if(o == NULL || filename == NULL){
        SET_ERROR(OBJRET_NULL_PTR);
        return -1;
    }

    //get strbuf for object file
    strbuf_t *strbuf = obj_write_to_strbuf(o);

    //check for nested errors
    if(strbuf == NULL) return -1;

    //write out data into file
    FILE * fp = fopen(filename, "w");

    if(fp == NULL){
        SET_ERROR(OBJRET_FOPEN_ERROR);
        return -1;
    }

    fputs(strbuf->str_ptr, fp);
    fflush(fp);

    //close file and clear strbuf
    fclose(fp);
    free_strbuf(strbuf);

    return 0;
}

int new_spec_symbol(char *name, uint32_t value, symbol_type_t type, spec_symbol_t **s){

    if(name == NULL || s == NULL){
        SET_ERROR(OBJRET_NULL_PTR);
        return -1;
    }

    *s = (spec_symbol_t *)malloc(sizeof(spec_symbol_t));
    char *line = malloc(sizeof(char) * (strlen(name) + 1));

    if(*s == NULL || line == NULL){
        SET_ERROR(OBJRET_MALLOC_FAIL);
        return -1;
    }

    strcpy(line, name);

    (*s)->name = line;
    (*s)->value = value;
    (*s)->prev = NULL;
    (*s)->next = NULL;
    (*s)->type = type;

    return 0;
}

int new_data_symbol(uint32_t address, data_symbol_type_t type, void *payload_ptr, data_symbol_t **d){

    if(d == NULL){
        SET_ERROR(OBJRET_NULL_PTR);
        return -1;
    }

    *d = (data_symbol_t *)malloc(sizeof(data_symbol_t));

    if(*d == NULL){
        SET_ERROR(OBJRET_MALLOC_FAIL);
        return -1;
    }

    (*d)->prev = NULL;
    (*d)->next = NULL;
    (*d)->address = address;
    (*d)->type = type;
    (*d)->relocation = 0;
    (*d)->special = 0;

    if(type == DATA_IS_BLOB){
        (*d)->payload.blob = (datablob_t *)payload_ptr;
    }
    else if(type == DATA_IS_INST){
        (*d)->payload.inst = (tInstruction *)payload_ptr;
    }
    else{
        SET_ERROR(OBJRET_INTERNAL_ERR);
        return -1;
    }

    return 0;

}

int new_obj(char * object_file_name, obj_file_t **o){

    if(object_file_name == NULL || o == NULL){
        SET_ERROR(OBJRET_NULL_PTR);
        return -1;
    }

    *o = (obj_file_t *)malloc(sizeof(obj_file_t));
    char * line_1 = malloc(sizeof(char) * (strlen(object_file_name) + 1));
    char * line_2 = malloc(sizeof(char) * (strlen(TARGET_ARCH_NAME) + 1));

    if(*o == NULL || line_1 == NULL || line_2 == NULL){
        SET_ERROR(OBJRET_MALLOC_FAIL);
        return -1;
    }

    strcpy(line_1, object_file_name);
    strcpy(line_2, TARGET_ARCH_NAME);

    (*o)->first_section = NULL;
    (*o)->last_section = NULL;
    (*o)->object_file_name = line_1;
    (*o)->target_arch_name = line_2;
    (*o)->next = NULL;
    (*o)->prev = NULL;

    return 0;

}

int new_section(char *section_name, section_t **s){

    if(section_name == NULL || s == NULL){
        SET_ERROR(OBJRET_NULL_PTR);
        return -1;
    }

    *s = (section_t *)malloc(sizeof(section_t));
    char * line = malloc(sizeof(char) * (strlen(section_name) + 1));

    if(*s == NULL || line == NULL){
        SET_ERROR(OBJRET_MALLOC_FAIL);
        return -1;
    }

    strcpy(line, section_name);

    (*s)->spec_symbol_first = NULL;
    (*s)->spec_symbol_last = NULL;
    (*s)->data_first = NULL;
    (*s)->data_last = NULL;
    (*s)->prev = NULL;
    (*s)->next = NULL;
    (*s)->section_name = line;

    return 0;
}

int new_blob(unsigned int lenght, datablob_t **b){
    if(b == NULL){
        SET_ERROR(OBJRET_NULL_PTR);
        return -1;
    }

    *b = (datablob_t *)malloc(sizeof(datablob_t));
    uint8_t *payload = (uint8_t *) malloc(sizeof(uint8_t) * lenght);

    if(*b == NULL || payload == NULL){
        SET_ERROR(OBJRET_MALLOC_FAIL);
        return -1;
    }

    (*b)->payload = payload;
    (*b)->lenght = lenght;

    for(unsigned int i = 0; i < lenght; i++) (*b)->payload[i] = 0;


    return 0;
}

int append_section_to_obj(obj_file_t *o, section_t *s){

    if(o == NULL || s == NULL){
        SET_ERROR(OBJRET_NULL_PTR);
        return -1;
    }

    if(o->first_section == NULL){

        if(o->last_section != NULL){
            SET_ERROR(OBJRET_BROKEN_OBJ);
            return -1;
        }

        o->first_section = s;
        o->last_section = s;

    }
    else{

        if(o->last_section == NULL){
            SET_ERROR(OBJRET_BROKEN_OBJ);
            return -1;
        }

        section_t *head = o->first_section;

        while(head != NULL){

            if(strcmp(s->section_name, head->section_name) == 0){
                SET_ERROR(OBJRET_SECTION_EXIST_ALREADY);
                return -1;
            }

            head = head->next;
        }


        o->last_section->next = s;
        s->prev = o->last_section;
        o->last_section = s;
    }

    return 0;

}

int append_spec_symbol_to_section(section_t *section, spec_symbol_t *symbol){

    if(section == NULL || symbol == NULL){
        SET_ERROR(OBJRET_NULL_PTR);
        return -1;
    }

    if(section->spec_symbol_first == NULL){

        if(section->spec_symbol_last != NULL){
            SET_ERROR(OBJRET_BROKEN_SECTION);
            return -1;
        }

        section->spec_symbol_first = symbol;
        section->spec_symbol_last = symbol;
    }
    else{

        if(section->spec_symbol_last == NULL){
            SET_ERROR(OBJRET_BROKEN_SECTION);
            return -1;
        }

        section->spec_symbol_last->next = symbol;
        symbol->prev = section->spec_symbol_last;
        section->spec_symbol_last = symbol;

    }

    return 0;

}

int append_data_symbol_to_section(section_t *section, data_symbol_t *data){

    if(section == NULL || data == NULL){
        SET_ERROR(OBJRET_NULL_PTR);
        return -1;
    }

    if(section->data_first == NULL){

        if(section->data_last != NULL){
            SET_ERROR(OBJRET_BROKEN_SECTION);
            return -1;
        }

        section->data_first = data;
        section->data_last = data;
    }
    else{

        if(section->data_last == NULL){
            SET_ERROR(OBJRET_BROKEN_SECTION);
            return -1;
        }

        section->data_last->next = data;
        data->prev = section->data_last;
        section->data_last = data;
    }

    return 0;

}

static int new_strbuf(strbuf_t **sbuffer){
    if(*sbuffer != NULL){
        SET_ERROR(OBJRET_WRONG_ARG);
        return -1;
    }

    *sbuffer = (strbuf_t *)malloc(sizeof(strbuf_t));

    if(*sbuffer == NULL){
        SET_ERROR(OBJRET_MALLOC_FAIL);
        return -1;
    }

    (*sbuffer)->actual_lenght = 0;
    (*sbuffer)->total_lenght = 64;
    (*sbuffer)->str_ptr = (char *)malloc(sizeof(char) * (*sbuffer)->total_lenght);

    if((*sbuffer)->str_ptr == NULL){
        SET_ERROR(OBJRET_MALLOC_FAIL);
        return -1;
    }

    (*sbuffer)->str_ptr[0] = '\0';
}

static int my_sprintf(strbuf_t *sbuffer, char *fmt, ...){

    va_list argptr;
    char *tmp_str = NULL;

    if(sbuffer == NULL){
        SET_ERROR(OBJRET_NULL_PTR);
        return -1;
    }

    va_start(argptr, fmt);
    int char_count_to_print = vsnprintf(NULL, 0, fmt, argptr);

    if(sbuffer->actual_lenght + char_count_to_print + 1 > sbuffer->total_lenght){
        void *ptr = realloc(sbuffer->str_ptr, sbuffer->total_lenght * 2);

        if(ptr == NULL){
            SET_ERROR(OBJRET_MALLOC_FAIL);
            return -1;
        }

        sbuffer->str_ptr = (char *)ptr;
    }

    tmp_str = (char *)malloc(sizeof(char) * (char_count_to_print + 1));

    if(tmp_str == NULL){
        SET_ERROR(OBJRET_MALLOC_FAIL);
        return -1;
    }

    vsprintf(tmp_str, fmt, argptr);
    va_end(argptr);

    strcat(sbuffer->str_ptr, tmp_str);
    free(tmp_str);
}

static strbuf_t *obj_write_to_strbuf(obj_file_t *o){

    strbuf_t* strbuf = NULL;

    if(new_strbuf(&strbuf) == -1){
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

            my_sprintf(strbuf, "%s:0x%" PRIx32 ":", head_spec_symbol->name, head_spec_symbol->value);

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

                    my_sprintf(strbuf, "0x%" PRIx32 ":0x%" PRIx8, head_data->address, head_data->payload.blob->payload[0]);
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

                if(export_into_object_file_line(head_data->payload.inst, line) < 0){
                    SET_ERROR(OBJRET_INTERNAL_ERR);
                    free(line);
                    free_strbuf(strbuf);
                    return NULL;
                }

                my_sprintf(strbuf, "0x%"PRIx32":%"PRIx8":%"PRIx8":%s\n", head_data->address, head_data->relocation, head_data->special, line);

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

        int i = 0;
        while((strbuf->str_ptr[strbufpos] != '\0') && (i < (sizeof(line) - 1))){
            line[i] = strbuf->str_ptr[strbufpos];

            if(strbuf->str_ptr[strbufpos] == '\0'){
                end_of_file = 1;
            }

            if(line[i] == '\n' || line[i] == '\r'){
                line[i] = '\0';
                break;
            }
            else{
                strbufpos++;
                i++;
            }

        }



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
                    if(new_obj(line, &my_new_obj)){
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
                        if(append_section_to_obj(my_new_obj, my_new_section)){
                            return NULL;
                        }
                        my_new_section = NULL;
                    }
                }
                break;
            case SECTION_NAME:
                {
                    if(new_section(line, &my_new_section)){
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

                        uint32_t value;
                        char *type_s = (char *)malloc(sizeof(char) * 128);
                        char *name = (char *)malloc(sizeof(char) * 128);

                        for(int i = 0; line[i] != '\0'; i++) if(line[i] == ':') line[i] = ' ';

                        if(sscanf(line, "%s 0x%X %s", name, &value, type_s) != 3){
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

                        if(new_spec_symbol(name, value, type, &new_symbol)){
                            return NULL;
                        }

                        if(append_spec_symbol_to_section(my_new_section, new_symbol)){
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

                            uint32_t address;
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

                            if(sscanf(buff, "0x%"SCNx32 , &address) != 1){
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

                            if(new_blob(num_buff_index, &blob) != 0){
                                SET_ERROR(OBJRET_INTERNAL_ERR);
                                return NULL;
                            }

                            for(unsigned int i = 0; i < num_buff_index; i++){
                                blob->payload[i] = num_buff[i];
                            }

                            if(new_data_symbol(address, DATA_IS_BLOB, (void *)blob, &new_data)){
                                return NULL;
                            }

                            if(append_data_symbol_to_section(my_new_section, new_data)){
                                return NULL;
                            }

                        }
                        else if(line[0] == 'I'){
                            uint32_t address = 0;
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


                            if(sscanf(ptr_for_objlib, "0x%"SCNx32":%"SCNx8":%"SCNx8, &address, &relocation, &special) != 3){
                                SET_ERROR(OBJRET_BROKEN_FILE);
                                return NULL;
                            }

                            if(import_from_object_file_line(inst, ptr_for_isalib) != 1){
                                SET_ERROR(OBJRET_INTERNAL_ERR);
                                return NULL;
                            }

                            if(new_data_symbol(address, DATA_IS_INST, (void *)inst, &new_data)){
                                return NULL;
                            }

                            new_data->relocation = relocation;
                            new_data->special = special;

                            if(append_data_symbol_to_section(my_new_section, new_data)){
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
