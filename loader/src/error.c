#include <stdio.h>

#include "error.h"

tLoaderError loader_errno = ERR_OK;

void print_error(void){
    switch(loader_errno){
        case ERR_OK:
            break;
        case ERR_RELOCATION_FAIL:
            printf("Error! Can't relocate instruction.\n");
            break;
        case ERR_CANT_OPEN_PORT:
            printf("Error! Can't open port.\n");
            break;
        case ERR_CANT_SETUP_PORT:
            printf("Error! Can't setup port.\n");
            break;
        case ERR_CANT_CONNECT:
            printf("Error! Can't connect to the target.\n");
            break;
        case ERR_PORT_NOT_GIVEN:
            printf("Error! Port name is not given.\n");
            break;
        case ERR_FILE_NOT_GIVEN:
            printf("Error! Input file name isn't given.\n");
            break;
        case ERR_BASE_NOT_GIVEN:
            printf("Error! Base adress isn' given.\n");
            break;
        case ERR_MULTIPLE_FILES_GIVEN:
            printf("Error! Given multiple input files.\n");
            break;
        case ERR_COMUNICATION:
            printf("Error! Something went wrong with comunication.\n");
            break;
        default:
            printf("Error! Unspecified error.\n");
            break;
    }
}
