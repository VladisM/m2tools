/**
 * @file assembler.c
 *
 * @brief Two pass assembler with simple preprocessor.
 *
 * @author Bc. Vladislav Mlejnecký <v.mlejnecky@seznam.cz>
 * @date 15.06.2019
 *
 * @note This file is part of m2tools project.
 *
 * Assembler is splitted into multiple files:
 *  - assembler.c
 *  - common_defs.h
 *  - file_gen.c
 *  - file_gen.h
 *  - pass1.c
 *  - pass1.h
 *  - pass2.c
 *  - pass2.h
 *  - symbol_table.c
 *  - symbol_table.h
 *  - tokenizer.c
 *  - tokenizer.h
 *  - util.c
 *  - util.h
 *
 * Simple two pass assembler with very simple preprocesor. Preprocessor is
 * implemented in tokenizer.c file. First pass is then in pass1.c and second
 * in pass2.c. Then object file is written out by functions in file_gen.c.
 *
 * Input into tokenizer is path to file, and output is stored into
 * toklist_first and toklist_last. Like double linked list.
 *
 * Then pass1() is called, this function will take toklist like input and
 * transform it into pass_list_first and pass_list_last. Second pass, the
 * pass2() function then complete this list. In these two pass, symbol table
 * in symbol_first and symbol_last is generated too.
 *
 * At the end, object file is generated from these pass_list buffer.
 *
 * When porting to new architecture, any changes in assembler should be done. All
 * architecture specific code is in isa library, see files isa.c and isa.h.
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <libgen.h>
#include <limits.h>

#include "tokenizer.h"
#include "pass1.h"
#include "symbol_table.h"
#include "pass2.h"
#include "common_defs.h"
#include "util.h"
#include "file_gen.h"

typedef struct{
    char *i_file;
    char *i_file_abs;
    char *given_o_file;
    char *o_file;
#ifndef NDEBUG
    int verbose;
#endif
}settings_t;

#define HELP_STRING "\
Example usage: \n\
 "PROG_NAME" main.asm\n\
\n\
        This is two pass assembler for "TARGET_ARCH_NAME" CPU.\n\
\n\
Arguments:\n\
    -h --help           Print this help.\n\
    -o --output         Output object file name.\n\
       --version        Print version number and exit.\n"

#ifndef NDEBUG
#define DEBUG_HELP_STRING "\
       --verbose        Be a lot verbose. :) This can be used for\n\
                        toolchain debug.\n"
#else
#define DEBUG_HELP_STRING ""
#endif

static void arg_parse(int argc, char* argv[]);
static void print_version(void);
static void print_help(void);
static void cast_paths(void);
static void clean_up_mem(void);

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
    atexit(clean_up_mem);

    //get args
    settings.i_file = NULL;
    settings.o_file = NULL;
#ifndef NDEBUG
    settings.verbose = 0;
#endif

    arg_parse(argc, argv);

    //get absolute paths for files
    cast_paths();

    //process file
#ifndef NDEBUG
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
#ifndef NDEBUG
    }
#endif

    return EXIT_SUCCESS;

}

static void clean_up_mem(void){
    //free file paths
    if(settings.i_file_abs != NULL){
        free(settings.i_file_abs);
    }
    if(settings.o_file){
        free(settings.o_file);
    }
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
#ifndef NDEBUG
        else if(strcmp(argv[i], "--verbose") == 0){
            settings.verbose  = 1;
        }
#endif
        else if(strcmp(argv[i], "-o") == 0 || strcmp(argv[i], "--output") == 0 ){
            if(settings.given_o_file != NULL){
                fprintf(stderr, "Too much output files given!\n");
                exit(EXIT_FAILURE);
            }
            else if((i+1)<argc){
                settings.given_o_file = argv[++i];
            }
            else{
                fprintf(stderr, "Wrong arg format!\n");
                exit(EXIT_FAILURE);
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
    printf("assembler for %s CPU %s\n", TARGET_ARCH_NAME, VERSION);
}

static void print_help(void){
    printf(HELP_STRING DEBUG_HELP_STRING);
}

static void cast_paths(void){

    char buff[PATH_MAX + 1];
    char *res = realpath(settings.i_file, buff);

    if(res == NULL){
        fprintf(stderr, "Failed to find real path to file '%s'.\n", settings.i_file);
        exit(EXIT_FAILURE);
    }
    else{
        settings.i_file_abs = strdup(buff);
        if(settings.i_file_abs == NULL){
            fprintf(stderr, "Internal error, strdup failed!\n");
            exit(EXIT_FAILURE);
        }
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
