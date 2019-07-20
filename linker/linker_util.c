#include "linker_util.h"

#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>

#include "ldparser.h"

bool is_section_in_mem(char *sname, mem_t *m){

    if(sname == NULL || m == NULL){
        fprintf(stderr, "Internall error, null ptr!\n");
        exit(EXIT_FAILURE);
    }

    if(m->section_count == 0){
        return false;
    }

}

