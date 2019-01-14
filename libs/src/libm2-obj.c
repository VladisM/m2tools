#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <inttypes.h>

#include "obj.h"

#define SET_ERROR(n) if(objlib_errno == 0) objlib_errno = n

typedef enum{
    OBJECT = 0,
    OBJECT_NAME,
    SECTION,
    SECTION_NAME,
    SPEC,
    SPEC_SYMBOL,
    DATA_SYMBOL
}load_decoder_state_t;

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

            if(tmp_data->type == DATA_IS_BLOB) free(tmp_data->payload.blob->payload);
            else if(tmp_data->type == DATA_IS_INST) free_istruction_struct(tmp_data->payload.inst);
            else exit(EXIT_FAILURE);

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
    free(o);
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

                if(strcmp(line, ".object") == 0){
                    load_decoder_state = OBJECT_NAME;
                }
                else{
                    SET_ERROR(OBJRET_BROKEN_FILE);
                    return -1;
                }

                break;
            case OBJECT_NAME:

                if(new_obj(line, &my_new_obj)){
                    return -1;
                }

                load_decoder_state = SECTION;

                break;
            case SECTION:

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

                break;
            case SECTION_NAME:

                if(new_section(line, &my_new_section)){
                    return -1;
                }

                load_decoder_state = SPEC;

                break;
            case SPEC:

                if(strcmp(line, ".spec") == 0){
                    load_decoder_state = SPEC_SYMBOL;
                }
                else{
                    SET_ERROR(OBJRET_BROKEN_FILE);
                    return -1;
                }

                break;
            case SPEC_SYMBOL:

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

                            datablob_t *blob = (datablob_t *)malloc(sizeof(datablob_t));

                            if(blob == NULL){
                                SET_ERROR(OBJRET_MALLOC_FAIL);
                                return -1;
                            }

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

                            blob->lenght = num_buff_index;
                            blob->payload = (uint8_t *)malloc(sizeof(uint8_t) * num_buff_index);

                            if(blob->payload == NULL){
                                SET_ERROR(OBJRET_MALLOC_FAIL);
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
                            uint32_t address;
                            char buff[80];
                            data_symbol_t *new_data = NULL;

                            tInstruction *inst = new_instru();

                            if(inst == NULL){
                                SET_ERROR(OBJRET_MALLOC_FAIL);
                                return -1;
                            }

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

                            int base = (int)(strlen(buff)) + 2;

                            for(int i = base; line[i] != '\0'; i++){
                                if(i == 80){
                                    SET_ERROR(OBJRET_INTERNAL_ERR);
                                    return -1;
                                }

                                buff[i - base] = line[i];
                                buff[i - base + 1] = '\0';
                            }

                            if(import_from_object_file_line(inst, buff) != 1){
                                SET_ERROR(OBJRET_INTERNAL_ERR);
                                return -1;
                            }

                            if(new_data_symbol(address, DATA_IS_INST, (void *)inst, &new_data)){
                                return -1;
                            }

                            if(append_data_symbol_to_section(my_new_section, new_data)){
                                return -1;
                            }

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

int obj_write(char *filename, obj_file_t *o){

    if(filename == NULL || o == NULL){
        SET_ERROR(OBJRET_NULL_PTR);
        return -1;
    }

    FILE * fp = fopen(filename, "w");

    if(fp == NULL){
        SET_ERROR(OBJRET_FOPEN_ERROR);
        return -1;
    }

    fprintf(fp, ".object\n%s\n", o->object_file_name);

    //go for all sections

    section_t *head_section = o->first_section;

    while(head_section != NULL){
        fprintf(fp, ".section\n%s\n", head_section->section_name);

        //print symbol table of that section
        fprintf(fp, ".spec\n");

        spec_symbol_t * head_spec_symbol = head_section->spec_symbol_first;

        while(head_spec_symbol != NULL){

            fprintf(fp, "%s:0x%" PRIx32 ":", head_spec_symbol->name, head_spec_symbol->value);

            if(head_spec_symbol->type == SYMBOL_EXPORT){
                fprintf(fp, "export\n");
            }
            else if(head_spec_symbol->type == SYMBOL_IMPORT){
                fprintf(fp, "import\n");
            }
            else{
                SET_ERROR(OBJRET_INTERNAL_ERR);
                return -1;
            }

            head_spec_symbol = head_spec_symbol->next;

        }

        fprintf(fp, ".data\n");

        data_symbol_t *head_data = head_section->data_first;

        while(head_data != NULL){
            if(head_data->type == DATA_IS_BLOB){

                fprintf(fp, "B:");

                if(head_data->payload.blob->lenght > 0){

                    fprintf(fp, "0x%" PRIx32 ":0x%" PRIx8, head_data->address, head_data->payload.blob->payload[0]);
                    for(unsigned int i = 1; i < head_data->payload.blob->lenght; i++){
                        fprintf(fp, ":0x%" PRIx8, head_data->payload.blob->payload[i]);
                    }
                    fprintf(fp, "\n");

                }
                else{
                    SET_ERROR(OBJRET_BROKEN_OBJ);
                    return  -1;
                }

            }
            else if(head_data->type == DATA_IS_INST){

                fprintf(fp, "I:");

                char *line = (char *)malloc(sizeof(char) * 64);

                if(line == NULL){
                    SET_ERROR(OBJRET_MALLOC_FAIL);
                    return -1;
                }

                if(export_into_object_file_line(head_data->payload.inst, line) < 0){
                    SET_ERROR(OBJRET_INTERNAL_ERR);
                    return -1;
                }

                fprintf(fp, "0x%"PRIx32":%s\n", head_data->address, line);

                free(line);
            }
            else{
                SET_ERROR(OBJRET_INTERNAL_ERR);
                return -1;
            }

            head_data = head_data->next;
        }

        head_section = head_section->next;
    }

    fprintf(fp, ".end\n");

    fclose(fp);

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
    char * line = malloc(sizeof(char) * (strlen(object_file_name) + 1));

    if(*o == NULL || line == NULL){
        SET_ERROR(OBJRET_MALLOC_FAIL);
        return -1;
    }

    strcpy(line, object_file_name);

    (*o)->first_section = NULL;
    (*o)->last_section = NULL;
    (*o)->object_file_name = line;

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
