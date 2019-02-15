#define _GNU_SOURCE

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "tokenizer.h"
#include "pass1.h"
#include "symbol_table.h"
#include "pass2.h"
#include "common_defs.h"
#include "util.h"
#include "file_gen.h"

#ifndef VERSION
#define VERSION "0.1 dev"
#endif

typedef struct{
    char *i_file;
    char *i_file_abs;
    char *given_o_file;
    char *o_file;
#ifdef DEBUG
    int verbose;
#endif
}settings_t;

const char * help_string = "\
Example usage: assembler main.asm\n\
\n\
        This is two pass assembler for MARK II CPU. For informations about\n\
    MARK II please see: https://github.com/VladisM/MARK_II/\n\
\n\
Arguments:\n\
    -h --help           Print this help.\n\
    -o --output         Output object file name.\n\
       --version        Print version number and exit.\n\
";

static void arg_parse(int argc, char* argv[]);
static void print_version(void);
static void print_help(void);
static void cast_paths(void);

tok_t *toklist_first = NULL;
tok_t *toklist_last = NULL;
pass_section_t *pass_list_first = NULL;
pass_section_t *pass_list_last = NULL;
static settings_t settings;

int main(int argc, char* argv[]){

    //bind clean up functions
    atexit(tokenizer_cleanup);
    atexit(pass1_cleanup);
    atexit(symbol_table_cleanup);
    atexit(pass2_cleanup);
    atexit(filegen_cleanup);

    //get args
    settings.i_file = NULL;
    settings.o_file = NULL;
#ifdef DEBUG
    settings.verbose = 0;
#endif

    arg_parse(argc, argv);

    //get absolute paths for files
    cast_paths();

    //process file
#ifdef DEBUG
    if(settings.verbose == 1){
        print_start(0);

        tokenizer(settings.i_file_abs);
        print_filelist();
        print_toklist();
        print_defs();
        print_cons();

        print_end(0);
        print_start(1);

        pass1();
        print_pass1_buffer();
        print_symboltable();

        print_end(1);
        print_start(2);

        pass2();
        print_pass2_buffer();
        print_symboltable();

        print_end(2);
        print_start(3);

        filegen_create_object_file(settings.o_file);

        print_end(3);
    }
    else{
#endif
    tokenizer(settings.i_file_abs);
    pass1();
    pass2();
    filegen_create_object_file(settings.o_file);
#ifdef DEBUG
    }
#endif
    //free file paths
    free(settings.i_file_abs);
    free(settings.o_file);

    return EXIT_SUCCESS;

}

static void arg_parse(int argc, char* argv[]){
    int file_given = 0;
    int toomuchfiles_given = 0;

    for(int i = 1; i<argc; i++){
        if(strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0 ){
            print_help();
            exit(EXIT_SUCCESS);
        }
        else if(strcmp(argv[i], "--version") == 0 ){
            print_version();
            exit(EXIT_SUCCESS);
        }
#ifdef DEBUG
        else if(strcmp(argv[i], "--verbose") == 0){
            settings.verbose  = 1;
        }
#endif
        else if(strcmp(argv[i], "-o") == 0 || strcmp(argv[i], "--output") == 0 ){
            if(settings.given_o_file != NULL){
                fprintf(stderr, "Too much output files given!\n");
                exit(EXIT_FAILURE);
            }
            else{
                settings.given_o_file = argv[++i];
            }
        }
        else{
            if(argv[i][0] != '-'){
                if(file_given == 0){
                    settings.i_file = argv[i];
                    file_given = 1;
                }
                else{
                    toomuchfiles_given = 1;
                }
            }
            else{
                fprintf(stderr, "Wrong arg format!\n");
                exit(EXIT_FAILURE);
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

static void print_version(void){
    printf("assembler for MARK-II CPU %s\n", VERSION);
}

static void print_help(void){
    printf("%s", help_string);
}

static void cast_paths(void){

    settings.i_file_abs = canonicalize_file_name(settings.i_file);

    if(settings.i_file_abs == NULL){
        fprintf(stderr, "Failed to find real path to file '%s'.\n", settings.i_file);
        exit(EXIT_FAILURE);
    }

    if(settings.given_o_file == NULL){
        char *i_file_dup = (char *)malloc(sizeof(char) * (strlen(settings.i_file_abs) + 3));

        if(i_file_dup == NULL){
            fprintf(stderr, "Error! Malloc failed!\n");
            exit(EXIT_FAILURE);
        }

        strcpy(i_file_dup, settings.i_file_abs);
        strcat(i_file_dup, ".o");

        char *filename = basename(i_file_dup);

        settings.o_file = strdup(filename);

        free(i_file_dup);
    }
    else{
        settings.o_file = strdup(settings.given_o_file);
    }

}
