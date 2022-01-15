/**
 * @file assembler.c
 *
 * @brief Two pass assembler with preprocessor.
 *
 * @author Bc. Vladislav Mlejnecký <v.mlejnecky@seznam.cz>
 * @date 13.11.2021
 *
 * @note This file is part of m2tools project.
 */

#include "preprocessor.h"
#include "section_table.h"
#include "symbol_table.h"
#include "pass_item.h"
#include "pass1.h"
#include "pass2.h"
#include "filegen.h"
#include "common.h"
#include "verbose.h"

#include <filelib.h>
#include <platformlib.h>

#include <utillib/core.h>
#include <utillib/cli.h>
#include <utillib/utils.h>

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

typedef enum{
    ACTION_NOT_SPECIFIED,
    ACTION_HELP,
    ACTION_VERSION,
    ACTION_ASSEMBLE
} action_t;

typedef struct{
    action_t action;
    char *input_file;
    char *output_file;
    bool verbose;
}settings_t;

options_t *args = NULL;
settings_t settings;

char *about_string = "This is simple two pass assembler for "TARGET_ARCH_NAME" CPU.";

bool argparse(int argc, char **argv);
void memclean(void);
bool assembler_run(char *input_filename, char *output_filename, bool verbose);

int main(int argc, char **argv){
    bool retVal = false;

    atexit_init();
    atexit_register(memclean);

    error_buffer_init(&error_buffer);

    filelib_init();
    platformlib_init();
    section_table_init();
    symbol_table_init();
    pass_item_db_init();

    if(argparse(argc, argv)){
        switch (settings.action) {
            case ACTION_HELP:
                options_print_help(args);
                retVal = true;
                break;
            case ACTION_VERSION:
                options_print_version(args);
                retVal = true;
                break;
            case ACTION_ASSEMBLE:
                retVal = assembler_run(settings.input_file, settings.output_file, settings.verbose);
                if(retVal == false)
                    ERROR_WRITE("Failed to run assembler on %s!", settings.input_file);
                break;
            default:
                ERROR_WRITE("Action didn't specified!");
                retVal = false;
                break;
        }
    }
    else{
        retVal = false;
    }

    if(retVal == false){
        fprintf(stderr, "%s", error_buffer_get(error_buffer));
        fflush(stderr);
        return EXIT_FAILURE;
    }
    else{
        return EXIT_SUCCESS;
    }
}

bool argparse(int argc, char **argv){
    options_init(&args, VERSION, PROG_NAME);
    options_append_about(args, about_string);

    bool retVal = true;

    settings.action = ACTION_NOT_SPECIFIED;
    settings.input_file = NULL;
    settings.output_file = NULL;
    settings.verbose = false;

    options_append_flag_3(args,
        "h", "help",
        "Print this help."
    );
    options_append_flag_2(args,
        "version",
        "Print version info."
    );

#ifndef NDEBUG
    options_append_flag_2(args,
        "verbose",
        "Be verbose during run."
    );
#endif

    options_append_string_option_3(args,
        "o", "output",
        "Filename for output."
    );

    int _argc = options_parse(args, argc, argv);
    char **_argv = options_get_argv(args);

    if(options_is_flag_set(args, "h") || options_is_flag_set(args, "help")){
        settings.action = ACTION_HELP;
    }
    else if(options_is_flag_set(args, "version")){
        settings.action = ACTION_VERSION;
    }
    else{
        settings.action = ACTION_NOT_SPECIFIED;
    }

    if(options_is_flag_set(args, "verbose")){
        settings.verbose = true;
    }

    if(options_is_flag_set(args, "o")){
        options_get_option_value_string(args, "o", &(settings.output_file));
    }
    else if(options_is_flag_set(args, "output")){
        options_get_option_value_string(args, "output", &(settings.output_file));
    }
    else{
        settings.output_file = "a.obj";
    }

    if(settings.action == ACTION_NOT_SPECIFIED){
        if(_argc == 1){
            settings.input_file = _argv[0];
            settings.action = ACTION_ASSEMBLE;
        }
        else{
            if(_argc > 1){
                ERROR_WRITE("Too much input files are given!");
            }
            else{
                ERROR_WRITE("Not enough input files!");
            }
            retVal = false;
        }
    }

    return retVal;
}

void memclean(void){
    if(args != NULL){
        options_destroy(args);
    }

    if(error_buffer != NULL){
        error_buffer_destroy(error_buffer);
    }

    filelib_deinit();
    platformlib_deinit();
    section_table_deinit();
    symbol_table_deinit();
    pass_item_db_deinit();
}

bool assembler_run(char *input_filename, char *output_filename, bool verbose){
    queue_t *preprocessor_output = NULL;

    if(!preprocessor_run(input_filename, &preprocessor_output)){
        ERROR_WRITE("Failed to run preprocessor on file %s!", input_filename);
        return false;
    }

    if(verbose == true){
        verbose_print_preprocessor(preprocessor_output);
    }

    if(!pass1(preprocessor_output)){
        ERROR_WRITE("Failed to complete pass1 on file %s!", input_filename);
        preprocessor_clear_output(preprocessor_output);
        return false;
    }

    if(verbose == true){
        verbose_print_pass(1);
    }

    if(!pass2()){
        ERROR_WRITE("Failed to complete pass2 on file %s!", input_filename);
        preprocessor_clear_output(preprocessor_output);
        return false;
    }

    if(verbose == true){
        verbose_print_pass(2);
    }

    if(!generate_file(output_filename)){
        ERROR_WRITE("Failed to generate output file %s!", output_filename);
        preprocessor_clear_output(preprocessor_output);
        return false;
    }

    if(verbose == true){
        verbose_print_generate();
    }

    preprocessor_clear_output(preprocessor_output);
    return true;
}
