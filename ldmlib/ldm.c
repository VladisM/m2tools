/**
 * @file ldm.c
 *
 * @brief Library for manipulating LDM files.
 *
 * For more informations please see file ldm.h
 *
 * @author Bc. Vladislav Mlejnecký <v.mlejnecky@seznam.cz>
 * @date 30.09.2019
 *
 * @note This file is part of m2tools project.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include "ldm.h"

#include <isa.h>

#define SET_ERROR(n) if(ldmlib_errno == LDMERR_OK) ldmlib_errno = n
#define BROKEN_FILE_RETURN(f) SET_ERROR(LDMERR_BROKEN_FILE);fclose(f);return false

typedef enum{
    LDM_FILE = 0,
    LDM_NAME,
    ARCH,
    ARCH_NAME,
    ENTRY,
    ENTRY_VAL,
    MEM,
    MEM_NAME,
    BEGIN,
    BEGIN_ADDR,
    SIZE,
    SIZE_VAL,
    ITEM,
    ITEM_VAL
}ldm_reader_state_t;

static tLdmError ldmlib_errno = LDMERR_OK;
static ldm_reader_state_t ldmload_decoder_state = LDM_FILE;

tLdmError get_ldmlib_errno(void){
    return ldmlib_errno;
}

void clear_ldmlib_errno(void){
    ldmlib_errno = 0;
}

bool ldm_load(char *filename, ldm_file_t **f){

    if(filename == NULL || f == NULL){
        SET_ERROR(LDMERR_NULL_PTR);
        return false;
    }

    if(*f != NULL){
        SET_ERROR(LDMERR_PTR_NOT_NULL);
        return false;
    }

    FILE *fp = fopen(filename, "r");

    if(fp == NULL){
        SET_ERROR(LDMERR_FOPEN_ERROR);
        return false;
    }


    char line[128];
    unsigned int line_pos = 0;
    bool end_of_file = false;

    char *name_buf = NULL;
    isa_address_t entry_point = 0;

    while(end_of_file != true){

        memset((void *)line, '\0', sizeof(line));

        {
            int c;
            while(true){
                c = fgetc(fp);
                if((c == EOF)||(c == '\n')||(c == '\r')||(c == '\0'))
                    break;
                else if (line_pos > (sizeof(line) - 1)){
                    SET_ERROR(LDMERR_BROKEN_FILE);
                    fclose(fp);
                    return false;
                }
                else
                    line[line_pos++] = (char)c;
            }
        }

        //TODO: finish FSM
        switch(ldmload_decoder_state){
            case LDM_FILE:
                if(strcmp(line, ".ldm") == 0)
                    ldmload_decoder_state = LDM_NAME;
                else
                    BROKEN_FILE_RETURN(fp);
                break;

            case LDM_NAME:
                name_buf = (char *)malloc(sizeof(char) * (strlen(line) + 1));

                if(name_buff == NULL){
                    SET_ERROR(LDMERR_MALLOC_FAILED);
                    fclose(fp);
                    return false;
                }

                strcpy(name_buf, line);

                ldmload_decoder_state = ARCH;
                break;

            case ARCH:
                if(strcmp(line, ".arch") == 0)
                    ldmload_decoder_state = ARCH_NAME;
                else{
                    free(name_buf);
                    BROKEN_FILE_RETURN(fp);
                }
                break;

            case ARCH_NAME:
                if(strcmp(line, TARGET_ARCH_NAME) == 0)
                    ldmload_decoder_state = ENTRY;
                else{
                    SET_ERROR(LDMERR_WRONG_ARCH);
                    free(name_buf);
                    fclose(fp);
                    return false;
                }
                break;

            case ENTRY:
                if(strcmp(line, ".entry") == 0)
                    ldmload_decoder_state = ENTRY_VAL;
                else{
                    free(name_buf);
                    BROKEN_FILE_RETURN(fp);
                }
                break;

            case ENTRY_VAL:
                if(sscanf(line, SCNisa_addr, &entry_point) != 1){
                    free(name_buf);
                    BROKEN_FILE_RETURN(fp);
                }
                else{
                    if(new_ldm_file(f, name_buf, entry_point) != true){
                        free(name_buf);
                        SET_ERROR(LDMERR_INTERNAL_ERR);
                        fclose(fp);
                        return false;
                    }
                    else{
                        free(name_buf);
                        ldmload_decoder_state = MEM;
                    }
                }
                break;
            case MEM:
                break;
            case MEM_NAME:
                break;
            case BEGIN:
                break;
            case BEGIN_ADDR:
                break;
            case SIZE:
                break;
            case SIZE_VAL:
                break;
            case ITEM:
                break;
            case ITEM_VAL:
                break;
            default:
                SET_ERROR(LDMERR_INTERNAL_ERR);
                fclose(fp);
                return false;
                break;
        }
    }

    fclose(fp);

    //TODO: až to bude hotové tak vracet true!!!!!!
    return false;
}

bool ldm_write(char *filename, ldm_file_t *f){

    if(filename == NULL || f == NULL){
        SET_ERROR(LDMERR_NULL_PTR);
        return false;
    }

    FILE *fp = fopen(filename, "w");

    if(fp == NULL){
        SET_ERROR(LDMERR_FOPEN_ERROR);
        return false;
    }

    fprintf(fp, ".ldm\n%s\n", f->ldm_file_name);
    fprintf(fp, ".arch\n%s\n", f->target_arch_name);
    fprintf(fp, ".entry\n"PRIisa_addr"\n", f->entry_point);

    ldm_mem_t *head_mem = f->first_mem;

    while(head_mem != NULL){
        fprintf(fp, ".mem\n%s\n", head_mem->mem_name);
        fprintf(fp, ".begin\n"PRIisa_addr"\n", head_mem->begin_addr);
        fprintf(fp, ".size\n"PRIisa_addr"\n", head_mem->size);
        fprintf(fp, ".item\n");

        ldm_item_t *head_item = head_mem->first_item;

        while(head_item != NULL){
            fprintf(fp, PRIisa_addr";"PRIisa_iw"\n", head_item->address, head_item->word);
            head_item = head_item->next;
        }

        head_mem = head_mem->next;
    }

    fprintf(fp, ".end\n");

    fclose(fp);

    return true;
}

void free_ldm_file_struct(ldm_file_t *f){

    ldm_mem_t *head_mem, *tmp_mem;
    ldm_item_t *head_item, *tmp_item;

    if(f == NULL){
        return;
    }

    head_mem= f->first_mem;

    while(head_mem != NULL){
        tmp_mem = head_mem;
        head_mem = head_mem->next;

        head_item = tmp_mem->first_item;

        while(head_item != NULL){
            tmp_item = head_item;
            head_item = head_item->next;

            free(tmp_item);
        }

        free(tmp_mem->mem_name);
        free(tmp_mem);
    }

    free(f->target_arch_name);
    free(f->ldm_file_name);
    free(f);
}

bool new_ldm_file(ldm_file_t **f, char *filename, isa_address_t entry_point){
    if(*f != NULL){
        SET_ERROR(LDMERR_PTR_NOT_NULL);
        return false;
    }

    *f = (ldm_file_t *)malloc(sizeof(ldm_file_t));

    if(*f == NULL){
        SET_ERROR(LDMERR_MALLOC_FAILED);
        return false;
    }

    (*f)->ldm_file_name = (char *)malloc(sizeof(char) * (strlen(filename) + 1));
    (*f)->target_arch_name = (char *)malloc(sizeof(char) * (strlen(TARGET_ARCH_NAME) + 1));

    if((*f)->ldm_file_name == NULL || (*f)->target_arch_name == NULL){
        SET_ERROR(LDMERR_MALLOC_FAILED);
        return false;
    }

    strcpy((*f)->ldm_file_name, filename);
    strcpy((*f)->target_arch_name, TARGET_ARCH_NAME);

    (*f)->entry_point = entry_point;
    (*f)->first_mem = NULL;
    (*f)->last_mem = NULL;

    return true;
}

bool new_mem(ldm_mem_t **m, char *mem_name, isa_address_t begin_addr, isa_address_t size){
    if(*m != NULL){
        SET_ERROR(LDMERR_PTR_NOT_NULL);
        return false;
    }

    *m = (ldm_mem_t *)malloc(sizeof(ldm_mem_t));

    if(*m == NULL){
        SET_ERROR(LDMERR_MALLOC_FAILED);
        return false;
    }

    (*m)->mem_name = (char *)malloc(sizeof(char) * (strlen(mem_name) + 1));

    if((*m)->mem_name == NULL){
        SET_ERROR(LDMERR_MALLOC_FAILED);
        return false;
    }

    strcpy((*m)->mem_name, mem_name);

    (*m)->begin_addr = begin_addr;
    (*m)->size = size;
    (*m)->first_item = NULL;
    (*m)->last_item = NULL;
    (*m)->prev = NULL;
    (*m)->next = NULL;

    return true;
}

bool new_item(ldm_item_t **i, isa_address_t address, isa_address_t opword){
    if(*i != NULL){
        SET_ERROR(LDMERR_PTR_NOT_NULL);
        return false;
    }

    *i = (ldm_item_t *)malloc(sizeof(ldm_item_t));

    if(*i == NULL){
        SET_ERROR(LDMERR_MALLOC_FAILED);
        return false;
    }

    (*i)->prev = NULL;
    (*i)->next = NULL;
    (*i)->address = address;
    (*i)->word = opword;

    return true;
}

bool append_item_into_mem(ldm_item_t *i, ldm_mem_t *m){
    if(i == NULL || m == NULL){
        SET_ERROR(LDMERR_NULL_PTR);
        return false;
    }

    if(m->first_item == NULL){

        if(m->last_item != NULL){
            SET_ERROR(LDMERR_BROKEN_STRUCT);
            return false;
        }

        m->first_item = i;
        m->last_item = i;
    }
    else{

        if(m->last_item == NULL){
            SET_ERROR(LDMERR_BROKEN_STRUCT);
            return false;
        }

        m->last_item->next = i;
        i->prev = m->last_item;
        m->last_item = i;
    }

    return true;
}

bool append_mem_into_file(ldm_mem_t *m, ldm_file_t *f){
    if(m == NULL || f == NULL){
        SET_ERROR(LDMERR_NULL_PTR);
        return false;
    }

    if(f->first_mem == NULL){

        if(f->last_mem != NULL){
            SET_ERROR(LDMERR_BROKEN_STRUCT);
            return false;
        }

        f->first_mem = m;
        f->last_mem = m;
    }
    else{

        if(f->last_mem == NULL){
            SET_ERROR(LDMERR_BROKEN_STRUCT);
            return false;
        }

        f->last_mem->next = m;
        m->prev = f->last_mem;
        f->last_mem = m;
    }

    return true;
}
