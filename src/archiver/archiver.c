/**
 * @file archiver.c
 *
 * @brief Utility for manipulating static library files.
 *
 * @author Bc. Vladislav Mlejnecký <v.mlejnecky@seznam.cz>
 * @date 15.06.2019
 *
 * @note This file is part of m2tools project.
 *
 * Archiver is used in order to create static libraries. Simply, you translate bunch of
 * asm files into object files with assembler. These asm files should contain some
 * exported symbols like some functions and so on. Then you can create single file
 * contains all these object files by using this utility.
 *
 * $archiver -c -o my_lib.sl obj_a.o obj_b.o obj_c.o
 *
 * And it is done. :)
 *
 * You can also use this tool to extract static library into object files from
 * which it was created. For this --extract is used. Or you can simply list them
 * by using --list.
 *
 * Source code of this utility is pretty straight forward and doesn't need any futher
 * explanation. All hard work is done by sllib and objlib.
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <libgen.h>

#include <utillib/core.h>
#include <utillib/cli.h>

#include <filelib.h>

#define HELP_STRING "\
This is archiver utility for "TARGET_ARCH_NAME" CPU that can be used in order \
to generate static library files for linker."

static void arg_parse(int argc, char **argv);
static void clean_mem(void);
static void failure(char *errmsg);
static void create_library(char *out_file, char **input_files, unsigned file_count);
static void list_library(char *input_archive);
static void extract_library(char *input_archive);

typedef enum{
    ACTION_CREATE_ARCHIVE = 0,
    ACTION_LIST_ARCHIVE,
    ACTION_EXTRACT_ARCHIVE,
    ACTION_NOT_SPECIFIED,
    ACTION_HELP,
    ACTION_VERSION
}action_t;

typedef struct{
    action_t action;
    char *out_file_name;
    int input_files_count;
    char **input_files;
    bool verbose;
}settings_t;

settings_t settings;
options_t *args = NULL;

int main(int argc, char **argv){
    atexit_init();
    atexit_register(clean_mem);
    filelib_init();

    arg_parse(argc, argv);

    switch(settings.action){
        case ACTION_HELP:
            options_print_help(args);
            break;
        case ACTION_VERSION:
            options_print_version(args);
            break;
        case ACTION_CREATE_ARCHIVE:
            if(settings.out_file_name == NULL){
                failure("Missing output file name!");
            }

            if(settings.input_files_count < 1){
                failure("You have to set at least one input object file!");
            }

            create_library(settings.out_file_name, settings.input_files, settings.input_files_count);
            break;
        case ACTION_LIST_ARCHIVE:

            if(settings.input_files_count > 1){
                failure("Too much input files!");
            }

            if(settings.input_files_count < 1){
                failure("You didn't specified input file!");
            }

            list_library(settings.input_files[0]);
            break;
        case ACTION_EXTRACT_ARCHIVE:

            if(settings.input_files_count > 1){
                failure("Too much input files!");
            }

            if(settings.input_files_count < 1){
                failure("You didn't specified input file!");
            }

            extract_library(settings.input_files[0]);
            break;
        default:
            failure("Action didn't specified!");
    }

    return 0;
}

static void clean_mem(void){
    if(args != NULL){
        options_destroy(args);
    }

    filelib_deinit();
}

static void create_library(char *out_file, char **input_files, unsigned file_count){
    sl_file_t *new_lib = NULL;

    sl_file_new(&new_lib);

    for(unsigned i = 0; i < file_count; i++){
        obj_file_t *tmp_obj = NULL;
        sl_holder_t *tmp_holder = NULL;

        char *filename = input_files[i];

        if(!obj_load(filename, &tmp_obj)){
            failure(filelib_error());
        }

        sl_holder_new(&tmp_holder, filename, tmp_obj);
        sl_holder_into_file(new_lib, tmp_holder);
    }

    if(!sl_write(new_lib, out_file)){
        failure(filelib_error());
    }

    sl_file_destroy(new_lib);
}

static void extract_library(char *input_archive){
    sl_file_t *lib = NULL;

    if(!sl_load(input_archive, &lib)){
        failure(filelib_error());
    }

    for(unsigned i = 0; i < list_count(lib->objects); i++){
        sl_holder_t *holder = NULL;
        list_at(lib->objects, i, (void *)&holder);

        if(settings.verbose == true){
            fprintf(stdout, "Extracting %s\r\n", holder->object_name);
        }

        if(!obj_write(holder->object, holder->object_name)){
            sl_file_destroy(lib);
            failure(filelib_error());
        }
    }

    sl_file_destroy(lib);
}

static void list_library(char *input_archive){

    sl_file_t *lib = NULL;

    if(!sl_load(input_archive, &lib)){
        failure(filelib_error());
    }

    printf("Static library %s:\r\n", input_archive);

    if(list_count(lib->objects) > 0){
        for(unsigned i = 0; i < list_count(lib->objects); i++){
            sl_holder_t *holder = NULL;
            list_at(lib->objects, i, (void *)&holder);

            printf("- %s\r\n", holder->object_name);
        }
    }
    else{
        printf("EMPTY\r\n");
    }

    sl_file_destroy(lib);
}

static void arg_parse(int argc, char **argv){
    options_init(&args, VERSION, PROG_NAME);
    options_append_about(args, HELP_STRING);

    options_append_flag_3(args,
    "h", "help",
    "Print this help.");

    options_append_flag_2(args,
    "version",
    "Print version number and exit.");

    options_append_string_option_3(args,
    "o", "output",
    "Output filename for library.");

    options_append_flag_3(args,
    "c", "create",
    "Create an archive from given object files.");

    options_append_flag_2(args,
    "list",
    "Print object files in archive.");

    options_append_flag_2(args,
    "extract",
    "Extract all object files from archive into current dir.");

    options_append_flag_2(args,
    "verbose",
    "Be verbose when extracting.");

    int _argc = options_parse(args, argc, argv);
    char **_argv = options_get_argv(args);

    settings.verbose = options_is_flag_set(args, "verbose");

    if(options_is_flag_set(args, "help") || options_is_flag_set(args, "h")){
        settings.action = ACTION_HELP;
    }
    else if(options_is_flag_set(args, "version")){
        settings.action = ACTION_VERSION;
    }
    else if(options_is_flag_set(args, "list")){
        settings.action = ACTION_LIST_ARCHIVE;
    }
    else if(options_is_flag_set(args, "extract")){
        settings.action = ACTION_EXTRACT_ARCHIVE;
    }
    else if(options_is_flag_set(args, "c") || options_is_flag_set(args, "create")){
        settings.action = ACTION_CREATE_ARCHIVE;
    }
    else{
        settings.action = ACTION_NOT_SPECIFIED;
    }

    if(options_is_flag_set(args, "o")){
        options_get_option_value_string(args, "o", &settings.out_file_name);
    }
    else if(options_is_flag_set(args, "output")){
        options_get_option_value_string(args, "output", &settings.out_file_name);
    }
    else{
        settings.out_file_name = NULL;
    }

    settings.input_files_count = _argc;
    settings.input_files = _argv;
}

static void failure(char *errmsg){
    fprintf(stderr, "%s\r\n", errmsg);
    fflush(stderr);
    exit(EXIT_FAILURE);
}
