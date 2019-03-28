#define _GNU_SOURCE

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <obj.h>
#include <sl.h>

#define HELP_STRING "\
Example usage: %s -o test_lib.sl test_obj_1.o test_obj_2.o\n\
\n\
        This is archiver utility for "TARGET_ARCH_NAME" CPU that can be used\n\
    in order to generate static library files for linker.\n\
\n\
Arguments:\n\
    -h --help           Print this help.\n\
       --version        Print version number and exit.\n\
    -o --output         Output filename for library.\n"

static void print_version(void);
static void print_help(char *cmd_name);
static void arg_parse(int argc, char* argv[]);
static void clean_mem(void);
static void create_library(char *out_file, char **input_files, unsigned file_count);

typedef struct{
    char *out_file_name;
    char **input_files;
    char **input_files_abs;
    unsigned input_files_count;
    unsigned input_files_len;
}settings_t;

settings_t settings;


int main(int argc, char **argv){
    atexit(clean_mem);

    settings.input_files = (char **)malloc(sizeof(char **));

    if(settings.input_files == NULL){
        fprintf(stderr, "Malloc failed!\n");
        exit(EXIT_FAILURE);
    }

    settings.input_files[0] = NULL;
    settings.input_files_count = 0;
    settings.input_files_len = 1;
    settings.out_file_name = NULL;

    arg_parse(argc, argv);

    //canonicalize input files
    settings.input_files_abs = (char **)malloc(sizeof(char **) * settings.input_files_count);

    for(unsigned i = 0; i < settings.input_files_count; i++){
        settings.input_files_abs[i] = canonicalize_file_name(settings.input_files[i]);

        if(settings.input_files_abs[i] == NULL){
            fprintf(stderr, "Can't get full path of the file '%s'!\n", settings.input_files[i]);
            exit(EXIT_FAILURE);
        }
    }

    create_library(settings.out_file_name, settings.input_files_abs, settings.input_files_count);

    return 0;
}

static void print_version(void){
    printf("archiver for %s CPU %s\n", TARGET_ARCH_NAME, VERSION);
}

static void print_help(char *cmd_name){
    printf(HELP_STRING, cmd_name);
}

static void arg_parse(int argc, char* argv[]){

    for(int i = 1; i<argc; i++){
        if(strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0 ){
            print_help(basename(argv[0]));
            exit(EXIT_SUCCESS);
        }
        else if(strcmp(argv[i], "--version") == 0 ){
            print_version();
            exit(EXIT_SUCCESS);
        }
        else if(strcmp(argv[i], "-o") == 0 || strcmp(argv[i], "--output") == 0){
            if(settings.out_file_name != NULL){
                fprintf(stderr, "Too much output files given!\n");
                exit(EXIT_FAILURE);
            }
            else{
                settings.out_file_name = argv[++i];
            }
        }
        else{
            if(argv[i][0] == '-'){
                fprintf(stderr, "Wrong arg format!\n");
                exit(EXIT_FAILURE);
            }
            else{
                //put file in array
                settings.input_files[settings.input_files_count++] = argv[i];

                //realloc array if needed
                if(settings.input_files_count == settings.input_files_len){
                    //double each time (minimize system calls)
                    char **new_file_array = realloc(settings.input_files, (sizeof(char **) * 2 * settings.input_files_len));

                    if(new_file_array == NULL){
                        fprintf(stderr, "Realloc failed!\n");
                        exit(EXIT_FAILURE);
                    }

                    settings.input_files_len *= 2;
                    settings.input_files = new_file_array;
                }

            }
        }
    }

}

static void clean_mem(void){
    if(settings.input_files != NULL){
        free(settings.input_files);
    }

    if(settings.input_files_abs != NULL){
        for(unsigned i = 0; i < settings.input_files_count; i++){
            free(settings.input_files_abs[i]);
        }
        free(settings.input_files_abs);
    }
}

static void create_library(char *out_file, char **input_files, unsigned file_count){
    static_library_t *new_lib = NULL;

    if(new_sl(basename(out_file), &new_lib) != 0){
        fprintf(stderr, "Can't create library, errno: %d.\n", get_sllib_errno());
        exit(EXIT_FAILURE);
    }

    for(unsigned i = 0; i < file_count; i++){
        obj_file_t *new_obj = NULL;

        //obj_load fail if we are on wrong arch
        if(obj_load(input_files[i], &new_obj) != 0){
            fprintf(stderr, "Failed to load object file! objlib errno: %d\n", get_objlib_errno());
            free_sl(new_lib);
            exit(EXIT_FAILURE);
        }

        if(append_objfile_to_sl(new_obj, new_lib) != 0){
            fprintf(stderr, "Failed to append object file into library! sllib errno: %d\n", get_sllib_errno());
            //~ free_object_file(new_obj);
            free_sl(new_lib);
            exit(EXIT_FAILURE);
        }

        //~ free_object_file(new_obj);
    }

    if(sl_write(out_file, new_lib) != 0){
        fprintf(stderr, "Failed to write library! sllib errno: %d\n", get_sllib_errno());
        free_sl(new_lib);
        exit(EXIT_FAILURE);
    }

    free_sl(new_lib);
}
