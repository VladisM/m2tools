#include "mif_test.h"

#include "test_common.h"

#include <stdlib.h>
#include <stdio.h>

#include <filelib.h>
#include <utillib/cli.h>

static bool generate_test(mif_test_settings_t *settings, int argc, char **argv){
    UNUSED(settings);

    if(!requested_arguments_exact(argc, 1)){
        return false;
    }

    char *filename = argv[0];

    mif_file_t *file = NULL;
    mif_item_t *item_1 = NULL;
    mif_item_t *item_2 = NULL;

    mif_file_new(&file);
    mif_item_new(&item_1, 0, 0xAA);
    mif_item_new(&item_2, 0, 0xBB);

    mif_config_address(file, 0x10);
    mif_config_data(file, 8);
    mif_config_radixes(file, RADIX_HEX, RADIX_HEX);

    mif_item_into_file(file, item_1);
    mif_item_into_file(file, item_2);

    if(!mif_write(file, filename)){
        printf("%s\r\n", filelib_error());
        return false;
    }

    mif_file_destroy(file);

    return true;
}

void mif_test_args_init(options_t *args, mif_test_settings_t *settings){
    options_append_section(args, "MIF Tests", NULL);
    options_append_flag_2(args, "mif-generate", "Generate one small mif file.");

    settings->generate = false;
}

void mif_test_args_parse(options_t *args, mif_test_settings_t *settings){
    if(options_is_flag_set(args, "mif-generate")){
        settings->generate = true;
    }
}

bool mif_test_should_run(mif_test_settings_t *settings){
    return (settings->generate);
}

bool mif_test_run(mif_test_settings_t *settings, int argc, char **argv){
    if(settings->generate == true){
        return generate_test(settings, argc, argv);
    }
    else{
        return false;
    }
}
