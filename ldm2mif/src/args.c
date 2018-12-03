#include "args.h"

#include <string.h>
#include <stdio.h>

#include "error.h"

settings_t settings;

int get_args(int argc, char *argv[]){
    int file_given = 0;
    int toomuchfiles_given = 0;

    for(int i = 1; i<argc; i++){
        if( strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0 ){
            settings.help = 1;
            return -1;
        }
        else if( strcmp(argv[i], "--version") == 0 ){
            settings.version = 1;
            return -1;
        }
        else if( strcmp(argv[i], "-r") == 0 ){
            unsigned int given_base;
            sscanf(argv[++i], "0x%x", &given_base);
            settings.base_adress = given_base;
        }
        else if( strcmp(argv[i], "-s") == 0 ){
            unsigned int given_size;
            sscanf(argv[++i], "%u", &given_size);
            settings.size = given_size;
        }
        else if( strcmp(argv[i], "-o") == 0 ){
            strcpy(settings.out_file, argv[++i]);
        }
        else{
            if(file_given == 0){
                strcpy(settings.file, argv[i]);
                file_given = 1;
            }
            else{
                toomuchfiles_given = 1;
            }
        }
    }

    if(file_given == 0){
        SET_ERROR(ERR_FILE_NOT_GIVEN);
        return -1;
    }
    if(toomuchfiles_given == 1){
        SET_ERROR(ERR_MULTIPLE_FILES_GIVEN);
        return -1;
    }

    return 0;
}
