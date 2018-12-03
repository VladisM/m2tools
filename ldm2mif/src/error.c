#include "error.h"

#include <stdio.h>

tLdm2MifError ldm2mif_errno;

void print_error(void){
    switch(ldm2mif_errno){
        case ERR_OK:
            printf("nothing special happened...\n");
            break;
        case ERR_CANT_RELOCATE_INSTRUCTION:
            printf("Error, can't relocate instruction!\n");
            break;
        case ERR_FILE_NOT_GIVEN:
            printf("Error, name of input file is not given!\n");
            break;
        case ERR_MULTIPLE_FILES_GIVEN:
            printf("Error, too much input files given!\n");
            break;
        default:
            printf("Unspecified error.\n");
            break;
    }
}
