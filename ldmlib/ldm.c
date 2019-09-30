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

bool free_ldm_buffer(ldm_file_t *f){
    //TODO: dopsat
    return false;
}

bool new_ldm_file(ldm_file_t **f, char *filename, isa_address_t entry_point){
    if(*f != NULL){
        SET_ERROR(LDMERR_PTR_NOT_NULL);
        return false;
    }

    *f = malloc(sizeof(ldm_file_t));

    if(*f == NULL){
        SET_ERROR(LDMERR_MALLOC_FAILED);
        return false;
    }

    *f->file_name = malloc(sizeof(char) * (strlen(filename) + 1));
    *f->target_arch_name = malloc(sizeof(char) * (strlen(TARGET_ARCH) + 1));
}

bool new_mem(ldm_mem_t **m, char *mem_name, isa_address_t begin_addr, isa_address_t size){

}

bool new_item(ldm_item_t **i, isa_address_t address, isa_address_t opword){

}
