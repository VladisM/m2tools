#define _GNU_SOURCE

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <ldm.h>
#include <mif.h>
#include <isa.h>

#define HELP_STRING "\
Example usage: %s example.ldm\n\
\n\
        This is simple utility to convert load module (.ldm) file from linker\n\
    into memory inicialization file for Quartus II. Default address range of\n\
    output file is 2^8.\n\
\n\
Arguments:\n\
    -h --help           Print this help.\n\
    -o <file>           Output MIF name. If not specified default name\n\
                        will be used.\n\
    -r <address>        Relocate source. Addjust immediate addresses of these\n\
                        instructions that use relative addresing using labels.\n\
                        You have to specify <address> in hex where the code\n\
                        will be stored. Default value is 0x000000.\n\
    -s <size>           Size of memory, default value is 8. Memory range is\n\
                        from 0 to 2^<size>.\n\
       --version        Print version number and exit.\n"

typedef struct{
    //mandatory
    char out_file[128];
    char file[128];
    unsigned int base_adress;
    unsigned int size;
}settings_t;

ldm_buffer_item_t * ldm_buffer;
mif_buffer_item_t * mif_buffer;
settings_t settings;

static void clean_mem(void);
static void print_help(char *cmd_name);
static void print_version(void);
static void get_args(int argc, char *argv[]);
static void ldm2mif(ldm_buffer_item_t* lb, mif_buffer_item_t** mb, unsigned int size_pow);
static void relocate_buffer(ldm_buffer_item_t *b, unsigned int base_address);

int main(int argc, char *argv[]){
    
    atexit(clean_mem);
    
    strcpy(settings.out_file, "out.mif");
    settings.file[0] = '\0';
    settings.base_adress = 0x000000;
    settings.size = 8;    
    
    get_args(argc, argv);

    if(ldm_load(settings.file, &ldm_buffer) != 0){
        fprintf(stderr, "Failed to load ldm file! ldm_errno: %d\n", get_ldmlib_errno());
        exit(EXIT_FAILURE);
    }
    
    relocate_buffer(ldm_buffer, settings.base_adress);
    ldm2mif(ldm_buffer, &mif_buffer, settings.size);
    
    if(mif_write(settings.out_file, mif_buffer) != 0){
        fprintf(stderr, "Failed to write mif file! mif_errno: %d\n", get_miflib_errno());
        exit(EXIT_FAILURE);
    }

    exit(EXIT_SUCCESS);
}

static void clean_mem(void){
    free_ldm_buffer(ldm_buffer);
    free_mif_buffer(mif_buffer);
}

static void print_version(void){
    printf("ldm2mif for MARK-II CPU %s\n", VERSION);
}

static void print_help(char *cmd_name){
    printf(HELP_STRING, cmd_name);
}

static void get_args(int argc, char *argv[]){
    int file_given = 0;
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
        fprintf(stderr, "Missing input file!\n");
        exit(EXIT_FAILURE);
    }
    if(toomuchfiles_given == 1){
        fprintf(stderr, "Too much input files given!\n");
        exit(EXIT_FAILURE);
    }
}

static void ldm2mif(ldm_buffer_item_t* lb, mif_buffer_item_t** mb, unsigned int size_pow){

    mif_buffer_item_t* mif_current = NULL;
    mif_buffer_item_t* mif_buf_top = NULL;
    unsigned int size = 1;

    //calculate count of items in mif
    for(unsigned int i = 0; i < size_pow; i++) size *= 2;


    for(unsigned int i = 0; i < size; i++){
        mif_current = (mif_buffer_item_t *)malloc(sizeof(mif_buffer_item_t));

        if(lb != NULL){
            mif_current->value = lb->value;
            mif_current->address = lb->address;
            lb = lb->next;
        }
        else{
            mif_current->value = 0x00000000;
            mif_current->address = i;
        }

        if(mif_buf_top == NULL){
            mif_buf_top = mif_current;
            *mb = mif_buf_top;
        }
        else{
            mif_buf_top->next = mif_current;
            mif_buf_top = mif_buf_top->next;
        }
    }

    mif_buf_top->next = NULL;
    
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
            fprintf(stderr, "Error, can't relocate instruction!\n");
            exit(EXIT_FAILURE);
        }

        //put result back to buffer item
        b->value = instruction.word;
    }
    
}