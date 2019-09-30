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

#include "ldm.h"

#define SET_ERROR(n) if(ldmlib_errno == 0) ldmlib_errno = n

static tLdmError ldmlib_errno;

tLdmError get_ldmlib_errno(void){
    return ldmlib_errno;
}

void clear_ldmlib_errno(void){
    ldmlib_errno = 0;
}

bool ldm_load(char *filename, ldm_file_t **f){
    //TODO: dopsat
    return false;
}

bool ldm_write(char *filename, ldm_file_t *f){
    //TODO: dopsat
    return false;
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

        head_item = head_mem->first_item;

        while(head_item != NULL){
            tmp_item = head_item;
            head_item = head_item->next;

            free(head_item);
        }

        free(head_mem->mem_name);
        free(head_mem);
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

    *f->file_name = (char *)malloc(sizeof(char) * (strlen(filename) + 1));
    *f->target_arch_name = (char *)malloc(sizeof(char) * (strlen(TARGET_ARCH) + 1));

    if(*f->file_name == NULL || *f->target_arch_name == NULL){
        SET_ERROR(LDMERR_MALLOC_FAILED);
        return false;
    }

    strcpy(*f->file_name, filename);
    strcpy(*f->target_arch_name, TARGET_ARCH);

    *f->entry_point = entry_point;
    *f->first_mem = NULL;
    *f->last_mem = NULL;

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

    *m->mem_name = (char *)malloc(sizeof(char) * (strlen(mem_name) + 1));

    if(*m->mem_name == NULL){
        SET_ERROR(LDMERR_MALLOC_FAILED);
        return false;
    }

    strcpy(*m->mem_name, mem_name);

    *m->being_addr = begin_addr;
    *m->size = size;
    *m->first_item = NULL;
    *m->last_item = NULL;
    *m->prev = NULL;
    *m->next = NULL;

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

    *i->prev = NULL;
    *i->next = NULL;
    *i->address = address;
    *i = opword = opword;

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
