/**
 * @file linker_util.c
 *
 * @brief Portable linker from m2tools.
 *
 * @author Bc. Vladislav Mlejnecký <v.mlejnecky@seznam.cz>
 * @date 31.01.2020
 *
 * @note This file is part of m2tools project.
 *
 * Linker is splitted into multiple files:
 *  - ldparser.c
 *  - ldparser.h
 *  - linker.c
 *  - linker_util.c
 *  - linker_util.h
 */

#include "linker_util.h"

#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <stdio.h>

#include "ldparser.h"

bool is_section_in_mem(char *sname, mem_t *m){

    if(sname == NULL || m == NULL){
        fprintf(stderr, "Internall error, null ptr!\n");
        exit(EXIT_FAILURE);
    }

    if(m->section_count == 0){
        return false;
    }

    bool ret_val = false;

    for(unsigned int i = 0; i < m->section_count; i++){

        if(strlen(sname) < strlen(m->sections[i])){
            continue;
        }

        char *mem_symbol_string = (char *)malloc(sizeof(char) * (strlen(m->sections[i]) + 1));
        char *sname_copy = (char *)malloc(sizeof(char) * (strlen(sname) + 1));

        check_malloc(mem_symbol_string);
        check_malloc(sname_copy);

        for(int ii = 0; mem_symbol_string[ii] != '\0' && sname_copy[ii] != '\0'; ii++){
            if(mem_symbol_string[ii] == '*'){
                mem_symbol_string[ii] = '\0';
                sname_copy[ii] = '\0';
                break;
            }
        }

        if(strcmp(sname_copy, mem_symbol_string) == 0){
            free(sname_copy);
            free(mem_symbol_string);

            ret_val = true;
            break;
        }
        else{
            free(sname_copy);
            free(mem_symbol_string);
        }

    }

    return ret_val;

}

void check_malloc(void *p){
    if(p == NULL){
        fprintf(stderr, "Malloc failed!\n");
        exit(EXIT_FAILURE);
    }
}

