#define _GNU_SOURCE

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "sender.h"

#include <ldm.h>
#include <isa.h>

#define HELP_STRING "\
Example usage: %s -b 0x400 -p /dev/ttyUSB0 example.ldm\n\
\n\
        Simple utility to load program into MARK-II using default serial\n\
    bootloader. For more information please see following link:\n\
    https://www.github.com/VladisM/MARK_II\n\
\n\
Arguments:\n\
    -h --help           Print this help.\n\
    -b <address>        Base address, using hex C like syntax, to store source.\n\
                        Loader also perform relocation of the given source to\n\
                        this address.\n\
    -p <port>           Port where MARK-II is connected. For example\n\
                        /dev/ttyUSB0.\n\
       --version        Print version number and exit.\n\
    -e --emulator       Add this option if you are connecting to emulator.\n"


typedef struct{
    //mandatory
    char port[128];
    char file[128];
    unsigned int base_adress;
    //flags
    int emulator;
}settings_t;

static void clean_mem(void);
static void print_version(void);
static void print_help(char *cmd_name);
static void relocate_buffer(ldm_buffer_item_t *b, unsigned int base_address);
static void get_args(int argc, char *argv[]);

ldm_buffer_item_t * buffer;
settings_t settings;

int main(int argc, char **argv)
{
    atexit(clean_mem);
    
    strcpy(settings.file, "none");
    strcpy(settings.port, "none");
    settings.base_adress = 0;    
    settings.emulator = 0;
    
    get_args(argc, argv);        

    if(ldm_load(settings.file, &buffer) != 0){
        fprintf(stderr, "Failed to load ldm file! ldm_errno: %d\n", get_ldmlib_errno());
        exit(EXIT_FAILURE);
    }
    
    relocate_buffer(buffer, settings.base_adress);

    sender_info_t info;
    info.base_address = settings.base_adress;
    info.port = settings.port;
    info.emulator = settings.emulator;

    send_buffer(&info, buffer);
    
    exit(EXIT_SUCCESS);
}

static void clean_mem(void){
    free_ldm_buffer(buffer);
}

static void print_version(void){
    printf("loader for MARK-II CPU %s\n", VERSION);
}

static void print_help(char *cmd_name){    
    printf(HELP_STRING, cmd_name);
}

static void relocate_buffer(ldm_buffer_item_t *b, unsigned int base_address){

    for(;b != NULL ; b=b->next){
        tInstruction instruction;

        //relocate instruction adress
        b->address += base_address;
        instruction.word = b->value;

        //if I don't have retarget instruction argument I don't do so
        if(b->relocation == 0){
            continue;
        }

        if(retarget_instruction(&instruction, base_address) < 0){
            fprintf(stderr, "Error! Can't relocate instruction.\n");
            exit(EXIT_FAILURE);
        }

        //put result back to buffer item
        b->value = instruction.word;
    }
    
}

static void get_args(int argc, char *argv[]){
    int port_given = 0;
    int file_given = 0;
    int base_given = 0;
    int toomuchfiles_given = 0;

    for(int i = 1; i<argc; i++){
        if( strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0 ){
            print_help(basename(argv[0]));
            exit(EXIT_SUCCESS);
        }
        else if( strcmp(argv[i], "--version") == 0 ){
            print_version();
            exit(EXIT_SUCCESS);
        }
        else if( strcmp(argv[i], "-b") == 0 ){
            unsigned int given_base;
            sscanf(argv[++i], "0x%x", &given_base);
            settings.base_adress = given_base;
            base_given = 1;
        }
        else if( strcmp(argv[i], "-p") == 0 ){
            strcpy(settings.port, argv[++i]);
            port_given = 1;
        }
        else if( strcmp(argv[i], "-e") == 0 || strcmp(argv[i], "--emulator") == 0 ){
            settings.emulator = 1;
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

    if(port_given == 0){
        fprintf(stderr, "Error! Port name is not given.\n");
        exit(EXIT_FAILURE);
    }
    if(file_given == 0){
        fprintf(stderr, "Error! Input file name isn't given.\n");
        exit(EXIT_FAILURE);
    }
    if(base_given == 0){
        fprintf(stderr, "Error! Base adress isn' given.\n");
        exit(EXIT_FAILURE);
    }
    if(toomuchfiles_given == 1){
        fprintf(stderr, "Error! Given multiple input files.\n");
        exit(EXIT_FAILURE);
    }

}
