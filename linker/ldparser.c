#include "ldparser.h"

#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>

lds_t *parse_lds(char *path){
    //TODO: add parsing script
}

void free_lds_t(lds_t *l){
    //TODO: finish
    if(l != NULL){
        free(l->entry_point);
    }
}

#ifndef NDEBUG
void print_lds(lds_t *l){
    if(l == NULL){
        printf("(null)\n");
        return;
    }

    //TODO: finish this
}
#endif
