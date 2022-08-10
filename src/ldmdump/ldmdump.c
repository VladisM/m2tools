/**
 * @file ldmdump.c
 *
 * @brief Tool for converting ldm into mif.
 *
 * @author Bc. Vladislav Mlejnecký <v.mlejnecky@seznam.cz>
 * @date 16.1.2022
 */

#include "ldmdump.h"

#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

static void memclean(void);
static bool argparse(int argc, char **argv);
static bool run_backend(bool (*convert_f)(ldm_memory_t *, char *));
static char *get_filename_for_mem(ldm_memory_t *mem);

options_t *args = NULL;
settings_t settings;
error_t *error_buffer = NULL;

char * about_string = "Simple utility that can convert LDM into various files.";

int main(int argc, char **argv){
    bool retVal = false;
    atexit_init();
    atexit_register(memclean);

    filelib_init();

    error_buffer_init(&error_buffer);

    if(argparse(argc, argv)){
        switch(settings.action){
            case ACTION_HELP:
                options_print_help(args);
                retVal = true;
                break;
            case ACTION_VERSION:
                options_print_version(args);
                retVal = true;
                break;
            case ACTION_LDM2MIF:
                retVal = run_backend(&mif_backend_convert);
                break;
            case ACTION_LDM2IHEX:
                retVal = run_backend(&ihex_backend_convert);
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

    filelib_deinit();

    if(retVal == false){
        fprintf(stderr, "%s", error_buffer_get(error_buffer));
        fflush(stderr);
        return EXIT_FAILURE;
    }
    else{
        return EXIT_SUCCESS;
    }
}

static bool run_backend(bool (*convert_f)(ldm_memory_t *, char *)){
    CHECK_NULL_ARGUMENT(convert_f);

    ldm_file_t *ldm_file = NULL;
    ldm_memory_t *memory = NULL;
    bool retVal = false;

    if(!ldm_load(settings.input_file, &ldm_file)){
        ERROR_WRITE("%s", filelib_error());
        ERROR_WRITE("Failed to load input LDM file %s.", settings.input_file);
        return false;
    }

    for(unsigned i = 0; i < list_count(ldm_file->memories); i++){
        list_at(ldm_file->memories, i, (void *)&memory);

        if(settings.all == true){
            char *tmp_file_name = get_filename_for_mem(memory);

            retVal = convert_f(memory, tmp_file_name);

            dynmem_free(tmp_file_name);

            if(retVal == false)
                break;
            else
                continue;
        }

        if((strcmp(memory->memory_name, settings.mem_name) == 0) && (settings.all == false)){
            retVal = convert_f(memory, settings.output_file);
            break;
        }
    }

    if(memory == NULL){
        ERROR_WRITE("File %s doesn't contain memory %s!", settings.input_file, settings.mem_name);
        retVal = false;
    }

    ldm_file_destroy(ldm_file);
    ldm_file = NULL;
    memory = NULL;

    if(retVal == false){
        ERROR_WRITE("Failed to run objdump on %s.", settings.input_file);
    }

    return retVal;
}

static char *get_filename_for_mem(ldm_memory_t *mem){
    CHECK_NULL_ARGUMENT(mem);

    int len = snprintf(NULL, 0, "%s.%s", settings.output_file, mem->memory_name);
    char *tmp = dynmem_malloc(len + 1);
    sprintf(tmp, "%s.%s", settings.output_file, mem->memory_name);
    return tmp;
}

static void memclean(void){
    if(args != NULL){
        options_destroy(args);
    }

    if(error_buffer != NULL){
        error_buffer_destroy(error_buffer);
    }
}

static bool argparse(int argc, char **argv){
    options_init(&args, VERSION, PROG_NAME);
    options_append_about(args, about_string);

    settings.action = ACTION_NONE;
    settings.input_file = NULL;
    settings.all = false;
    settings.mem_name = NULL;

    options_append_flag_3(args, "h", "help", "Print this help.");
    options_append_flag_2(args, "version", "Print version info.");

    options_append_flag_2(args, "mif", "Create MIF file.");
    options_append_flag_2(args, "ihex", "Create IHEX file.");

    options_append_string_option_2(args, "mem", "Dump only memory with specified name.");
    options_append_string_option_3(args, "o", "output", "Specify name for output file.");
    options_append_flag_2(args, "all", "Dump all memories.");

    mif_backend_args_init(args, &settings.mif_settings);
    ihex_backend_args_init(args, &settings.ihex_settings);

    int _argc = options_parse(args, argc, argv);
    char **_argv = options_get_argv(args);

    if(!mif_backend_args_parse(args, &settings.mif_settings)){
        return false;
    }

    if(options_is_flag_set(args, "h") || options_is_flag_set(args, "help")){
        settings.action = ACTION_HELP;
        return true;
    }

    if(options_is_flag_set(args, "version")){
        settings.action = ACTION_VERSION;
        return true;
    }

    if(options_is_flag_set(args, "mif")){
        settings.action = ACTION_LDM2MIF;
    }
    else if(options_is_flag_set(args, "ihex")){
        settings.action = ACTION_LDM2IHEX;
    }
    else{
        ERROR_WRITE("Output format is not set!");
        return false;
    }

    if(options_is_flag_set(args, "all")){
        settings.all = true;
    }
    else if(options_is_option_set(args, "mem")){
        options_get_option_value_string(args, "mem", &settings.mem_name);
    }
    else{
        ERROR_WRITE("Missing name of memory to be dumped!");
        return false;
    }

    if(options_is_option_set(args, "o")){
        options_get_option_value_string(args, "o", &settings.output_file);
    }
    else if(options_is_option_set(args, "output")){
        options_get_option_value_string(args, "output", &settings.output_file);
    }
    else{
        ERROR_WRITE("Missing name for output file!");
        return false;
    }

    if(_argc == 1){
        settings.input_file = _argv[0];
    }
    else{
        if(_argc == 0){
            ERROR_WRITE("Missing input file name!");
        }
        else{
            ERROR_WRITE("Multiple input files!");
        }
        return false;
    }

    return true;
}
