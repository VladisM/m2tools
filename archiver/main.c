#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <libgen.h>

#include <obj.h>
#include <sl.h>

//TODO: refactoring případně
//TODO: doplnit komentáře

#define HELP_STRING "\
Example usage: \n\
 "PROG_NAME" -c -o test_lib.sl test_obj_1.o test_obj_2.o\n\
 "PROG_NAME" --list test.sl \n\
 "PROG_NAME" --extract test.sl \n\
\n\
        This is archiver utility for "TARGET_ARCH_NAME" CPU that can be used\n\
    in order to generate static library files for linker.\n\
\n\
Arguments:\n\
    -h --help           Print this help.\n\
       --version        Print version number and exit.\n\
    -o --output         Output filename for library.\n\
    -c --create         Create an archive from given object files.\n\
       --list           Print object files in archive.\n\
       --extract        Extract all object files from archive into current dir.\n\
       --verbose        Be verbose when extracting. Only with --extract\n"

static void print_version(void);
static void print_help(void);
static void arg_parse(int argc, char* argv[]);
static void clean_mem(void);
static void create_library(char *out_file, char **input_files, unsigned file_count);
static void list_library(char *input_archive);
static void extract_library(char *input_archive);

typedef enum{
    CREATE_ARCHIVE = 0,
    LIST_ARCHIVE,
    EXTRACT_ARCHIVE,
    ACTION_NOT_SPECIFIED
}action_t;

typedef struct{
    char *out_file_name;
    char **input_files;
    unsigned input_files_count;
    unsigned input_files_len;
    action_t action;
    int verbose;
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
    settings.action = ACTION_NOT_SPECIFIED;
    settings.verbose = 0;

    arg_parse(argc, argv);

    if(settings.action == CREATE_ARCHIVE){

        if(settings.out_file_name == NULL){
            fprintf(stderr, "Missing output file name!\n");
            exit(EXIT_FAILURE);
        }

        if(settings.input_files_count < 1){
            fprintf(stderr, "You have to set at least one input object file!\n");
            exit(EXIT_FAILURE);
        }

        create_library(settings.out_file_name, settings.input_files, settings.input_files_count);
    }
    else if(settings.action == LIST_ARCHIVE){

        if(settings.input_files_count > 1){
            fprintf(stderr, "Too much input files!\n");
            exit(EXIT_FAILURE);
        }

        if(settings.input_files_count < 1){
            fprintf(stderr, "You didn't specified input file!\n");
            exit(EXIT_FAILURE);
        }

        list_library(settings.input_files[0]);

    }
    else if(settings.action == EXTRACT_ARCHIVE){

        if(settings.input_files_count > 1){
            fprintf(stderr, "Too much input files!\n");
            exit(EXIT_FAILURE);
        }

        if(settings.input_files_count < 1){
            fprintf(stderr, "You didn't specified input file!\n");
            exit(EXIT_FAILURE);
        }

        extract_library(settings.input_files[0]);

    }
    else{
        fprintf(stderr, "Action didn't specified!\n");
        exit(EXIT_FAILURE);
    }

    return 0;
}

static void print_version(void){
    printf("archiver for %s CPU %s\n", TARGET_ARCH_NAME, VERSION);
}

static void print_help(void){
    printf(HELP_STRING);
}

static void arg_parse(int argc, char* argv[]){

    for(int i = 1; i<argc; i++){
        if(strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0 ){
            print_help();
            exit(EXIT_SUCCESS);
        }
        else if(strcmp(argv[i], "--version") == 0 ){
            print_version();
            exit(EXIT_SUCCESS);
        }
        else if(strcmp(argv[i], "-o") == 0 || strcmp(argv[i], "--output") == 0){
            if(settings.action == ACTION_NOT_SPECIFIED){
                fprintf(stderr, "You have to specify action before anything else!\n");
                exit(EXIT_FAILURE);
            }
            if(settings.action == CREATE_ARCHIVE){
                if(settings.out_file_name != NULL){
                    fprintf(stderr, "Too much output files given!\n");
                    exit(EXIT_FAILURE);
                }
                else{
                    settings.out_file_name = argv[++i];
                }
            }
            else{
                fprintf(stderr, "You cannot set this with this action!");
                exit(EXIT_FAILURE);
            }
        }
        else if(strcmp(argv[i], "-c") == 0 || strcmp(argv[i], "--create") == 0){
            if(settings.action != ACTION_NOT_SPECIFIED){
                fprintf(stderr, "Action already specified!\n");
                exit(EXIT_FAILURE);
            }
            settings.action = CREATE_ARCHIVE;
        }
        else if(strcmp(argv[i], "--list") == 0){
            if(settings.action != ACTION_NOT_SPECIFIED){
                fprintf(stderr, "Action already specified!\n");
                exit(EXIT_FAILURE);
            }
            settings.action = LIST_ARCHIVE;
        }
        else if(strcmp(argv[i], "--extract") == 0){
            if(settings.action != ACTION_NOT_SPECIFIED){
                fprintf(stderr, "Action already specified!\n");
                exit(EXIT_FAILURE);
            }
            settings.action = EXTRACT_ARCHIVE;
        }
        else if(strcmp(argv[i], "--verbose") == 0){
            if(settings.action == ACTION_NOT_SPECIFIED){
                fprintf(stderr, "You have to specify action before anything else!\n");
                exit(EXIT_FAILURE);
            }
            else if(settings.action == EXTRACT_ARCHIVE){
                settings.verbose = 1;
            }
            else{
                fprintf(stderr, "You cannot set this with this action!");
                exit(EXIT_FAILURE);
            }
        }
        else{
            if(argv[i][0] == '-'){
                fprintf(stderr, "Wrong arg format!\n");
                exit(EXIT_FAILURE);
            }
            else{
                if(settings.action == ACTION_NOT_SPECIFIED){
                    fprintf(stderr, "You have to specify action before anything else!\n");
                    exit(EXIT_FAILURE);
                }

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
            free_sl(new_lib);
            exit(EXIT_FAILURE);
        }

    }

    if(sl_write(out_file, new_lib) != 0){
        fprintf(stderr, "Failed to write library! sllib errno: %d\n", get_sllib_errno());
        free_sl(new_lib);
        exit(EXIT_FAILURE);
    }

    free_sl(new_lib);
}

static void extract_library(char *input_archive){
    static_library_t *loaded_lib = NULL;

    //fail if wrong arch
    if(sl_load(input_archive, &loaded_lib) != 0){
        fprintf(stderr, "Failed to load library '%s'!\n", input_archive);
        exit(EXIT_FAILURE);
    }

    if(settings.verbose == 1){
        printf("Archive %s:\n", loaded_lib->library_name);
    }

    obj_file_t *head_obj_file = loaded_lib->first_obj;

    while(head_obj_file != NULL){

        if(settings.verbose == 1){
            printf(" %s\n", head_obj_file->object_file_name);
        }

        if(obj_write_to_file(head_obj_file->object_file_name, head_obj_file) != 0){
            fprintf(stderr, "Failed to create object file '%s'! objlib errno: %d", head_obj_file->object_file_name, get_objlib_errno());
            free_sl(loaded_lib);
            exit(EXIT_FAILURE);
        }

        head_obj_file = head_obj_file->next;
    }

    free_sl(loaded_lib);
}

static void list_library(char *input_archive){

    static_library_t *loaded_lib = NULL;

    //fail if wrong arch
    if(sl_load(input_archive, &loaded_lib) != 0){
        fprintf(stderr, "Failed to load library '%s'!\n", input_archive);
        exit(EXIT_FAILURE);
    }

    printf("Static library from: '%s'\n", input_archive);
    printf("  |- Library name: '%s'\n", loaded_lib->library_name);
    printf("  |- Architecture: '%s'\n", loaded_lib->target_arch_name);
    printf("  '- Objects:\n");

    obj_file_t *head_obj_file = loaded_lib->first_obj;

    while(head_obj_file != NULL){
        if(head_obj_file->next == NULL){
            printf("     '- %s\n", head_obj_file->object_file_name);
        }
        else{
            printf("     |- %s\n", head_obj_file->object_file_name);
        }
        head_obj_file = head_obj_file->next;
    }

    free_sl(loaded_lib);

}
