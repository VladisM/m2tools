#include "sl_test.h"

#include "test_common.h"

#include <stdlib.h>
#include <stdio.h>

#include <filelib.h>
#include <utillib/cli.h>

static bool create_test(sl_settings_t *settings, int argc, char **argv){
    UNUSED(settings);

    if(!requested_arguments_more(argc, 2)){
        return false;
    }

    char *outname = argv[0];
    sl_file_t *lib = NULL;

    sl_file_new(&lib);

    for(int i = 1; i < argc; i++){
        char *obj_name = argv[i];

        sl_holder_t *tmp = NULL;
        obj_file_t *obj_file = NULL;

        if(!obj_load(obj_name, &obj_file)){
            printf("%s\r\n", filelib_error());
            sl_file_destroy(lib);
            return false;
        }

        sl_holder_new(&tmp, obj_name, obj_file);
        sl_holder_into_file(lib, tmp);
    }

    if(!sl_write(lib, outname)){
        printf("%s\r\n", filelib_error());
        sl_file_destroy(lib);
        return false;
    }

    sl_file_destroy(lib);

    return true;
}

static bool print_test(sl_settings_t *settings, int argc, char **argv){
    UNUSED(settings);

    if(!requested_arguments_exact(argc, 1)){
        return false;
    }

    sl_file_t *lib = NULL;
    char *filename = argv[0];

    if(!sl_load(filename, &lib)){
        printf("%s\r\n", filelib_error());
        return false;
    }

    printf("Static library %s:\r\n", filename);

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

    return true;
}

static bool load_save_test(sl_settings_t *settings, int argc, char **argv){
    UNUSED(settings);

    if(!requested_arguments_exact(argc, 2)){
        return false;
    }

    char *filename_input = argv[0];
    char *filename_output = argv[1];

    sl_file_t *lib = NULL;

    if(!sl_load(filename_input, &lib)){
        printf("%s\r\n", filelib_error());
        return false;
    }

    if(!sl_write(lib, filename_output)){
        printf("%s\r\n", filelib_error());
        sl_file_destroy(lib);
        return false;
    }

    sl_file_destroy(lib);
    return true;
}

static bool unpack_test(sl_settings_t *settings, int argc, char **argv){
    UNUSED(settings);

    if(!requested_arguments_exact(argc, 1)){
        return false;
    }

    sl_file_t *lib = NULL;
    char *filename = argv[0];

    if(!sl_load(filename, &lib)){
        printf("%s\r\n", filelib_error());
        return false;
    }

    for(unsigned i = 0; i < list_count(lib->objects); i++){
        sl_holder_t *holder = NULL;
        list_at(lib->objects, i, (void *)&holder);

        if(!obj_write(holder->object, holder->object_name)){
            printf("%s\r\n", filelib_error());
            sl_file_destroy(lib);
            return false;
        }

    }

    sl_file_destroy(lib);

    return true;
}

void sl_test_args_init(options_t *args, sl_settings_t *settings){
    options_append_section(args, "SL Tests", NULL);
    options_append_flag_2(args, "sl-create", "Generate static library from given object files.");
    options_append_flag_2(args, "sl-print", "Print content of library.");
    options_append_flag_2(args, "sl-load-save", "Load library and save it again as another file.");
    options_append_flag_2(args, "sl-unpack", "Unpack static library back into object files.");

    settings->create = false;
    settings->print = false;
    settings->load_save = false;
    settings->unpack = false;
}

void sl_test_args_parse(options_t *args, sl_settings_t *settings){
    if(options_is_flag_set(args, "sl-create")){
        settings->create = true;
    }
    if(options_is_flag_set(args, "sl-print")){
        settings->print = true;
    }
    if(options_is_flag_set(args, "sl-load-save")){
        settings->load_save = true;
    }
    if(options_is_flag_set(args, "sl-unpack")){
        settings->unpack = true;
    }
}

bool sl_test_should_run(sl_settings_t *settings){
    return (settings->create || settings->load_save || settings->print || settings->unpack);
}

bool sl_test_run(sl_settings_t *settings, int argc, char **argv){
    if(settings->create == true){
        return create_test(settings, argc, argv);
    }
    else if(settings->print == true){
        return print_test(settings, argc, argv);
    }
    else if(settings->load_save == true){
        return load_save_test(settings, argc, argv);
    }
    else if(settings->unpack == true){
        return unpack_test(settings, argc, argv);
    }
    else{
        return false;
    }
}
