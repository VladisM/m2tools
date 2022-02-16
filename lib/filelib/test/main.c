#include <stdlib.h>
#include <stdio.h>

#include <utillib/cli.h>
#include <filelib.h>

#include "ldm_test.h"
#include "obj_test.h"
#include "sl_test.h"
#include "mif_test.h"

typedef struct{
    bool help;
    bool version;
    int argc;
    char **argv;
    ldm_test_settings_t ldm_settings;
    obj_test_settings_t obj_settings;
    sl_test_settings_t sl_settings;
    mif_test_settings_t mif_settings;
}settings_t;

options_t *args = NULL;
settings_t settings;

void argparse(int argc, char **argv);
void memclean(void);

int main(int argc, char **argv){
    atexit_init();
    atexit_register(memclean);

    filelib_init();

    argparse(argc, argv);

    if(settings.help){
        options_print_help(args);
        exit(EXIT_SUCCESS);
    }
    else if(settings.version){
        options_print_version(args);
        exit(EXIT_SUCCESS);
    }
    else if(ldm_test_should_run(&(settings.ldm_settings))){
        if(ldm_test_run(&(settings.ldm_settings), settings.argc, settings.argv) == true){
            exit(EXIT_SUCCESS);
        }
        else{
            exit(EXIT_FAILURE);
        }
    }
    else if(obj_test_should_run(&(settings.obj_settings))){
        if(obj_test_run(&(settings.obj_settings), settings.argc, settings.argv) == true){
            exit(EXIT_SUCCESS);
        }
        else{
            exit(EXIT_FAILURE);
        }
    }
    else if(sl_test_should_run(&(settings.sl_settings))){
        if(sl_test_run(&(settings.sl_settings), settings.argc, settings.argv) == true){
            exit(EXIT_SUCCESS);
        }
        else{
            exit(EXIT_FAILURE);
        }
    }
    else if(mif_test_should_run(&(settings.mif_settings))){
        if(mif_test_run(&(settings.mif_settings), settings.argc, settings.argv) == true){
            exit(EXIT_SUCCESS);
        }
        else{
            exit(EXIT_FAILURE);
        }
    }
    else{
        printf("No action specified!\r\n");
        exit(EXIT_FAILURE);
    }

    return 0;
}

void argparse(int argc, char **argv){
    options_init(&args, VERSION, PROG_NAME);

    options_append_section(args, "General", NULL);
    options_append_flag_3(args,
        "h", "help",
        "Print this help."
    );
    options_append_flag_2(args,
        "version",
        "Print version info."
    );

    ldm_test_args_init(args, &(settings.ldm_settings));
    obj_test_args_init(args, &(settings.obj_settings));
    sl_test_args_init(args, &(settings.sl_settings));
    mif_test_args_init(args, &(settings.mif_settings));

    settings.help = false;
    settings.version = false;
    settings.argv = NULL;

    settings.argc = options_parse(args, argc, argv);
    settings.argv = options_get_argv(args);

    if(options_is_flag_set(args, "h") || options_is_flag_set(args, "help")){
        settings.help = true;
    }

    if(options_is_flag_set(args, "version")){
        settings.version = true;
    }

    ldm_test_args_parse(args, &(settings.ldm_settings));
    obj_test_args_parse(args, &(settings.obj_settings));
    sl_test_args_parse(args, &(settings.sl_settings));
    mif_test_args_parse(args, &(settings.mif_settings));

    return;
}

void memclean(void){
    if(args != NULL){
        options_destroy(args);
    }

    filelib_deinit();
}
