#include "ldmdump.h"

#include <string.h>
#include <limits.h>

static mif_backend_settings_t *settings_ptr = NULL;

static mif_radix_t str_to_radix(char *s){
    CHECK_NULL_ARGUMENT(s);

    if(strcmp(s, "hex") == 0){
        return RADIX_HEX;
    }
    else if(strcmp(s, "dec") == 0){
        return RADIX_DEC;
    }
    else if(strcmp(s, "oct") == 0){
        return RADIX_OCT;
    }
    else if(strcmp(s, "bin") == 0){
        return RADIX_BIN;
    }
    else{
        return RADIX_UNK;
    }
}

void mif_backend_args_init(options_t *args, mif_backend_settings_t *mif_settings){
    CHECK_NULL_ARGUMENT(args);
    CHECK_NULL_ARGUMENT(mif_settings);

    options_append_section(args, "MIF", "Options valid for MIF backend");
    options_append_string_option_2(args, "mif-data-radix", "Can be hex, dec, oct or bin. Default hex.");
    options_append_string_option_2(args, "mif-address-radix", "Can be hex, dec, oct or bin. Default hex.");
    options_append_number_option_2(args, "mif-address-depth", "Enforce size of output file.");

    settings_ptr = mif_settings;
}

bool mif_backend_args_parse(options_t *args, mif_backend_settings_t *mif_settings){
    CHECK_NULL_ARGUMENT(args);
    CHECK_NULL_ARGUMENT(mif_settings);

    mif_settings->address_radix = RADIX_HEX;
    mif_settings->data_radix = RADIX_HEX;
    mif_settings->force_address_depth = false;
    mif_settings->address_depth = 0;

    if(options_is_option_set(args, "mif-data-radix")){
        char *tmp = NULL;
        mif_radix_t data_radix = RADIX_UNK;

        options_get_option_value_string(args, "mif-data-radix", &tmp);
        data_radix = str_to_radix(tmp);

        if(data_radix == RADIX_UNK){
            ERROR_WRITE("Failed to decode data radix argument! %s", tmp);
            return false;
        }

        mif_settings->data_radix = data_radix;
    }

    if(options_is_option_set(args, "mif-address-radix")){
        char *tmp = NULL;
        mif_radix_t address_radix = RADIX_UNK;

        options_get_option_value_string(args, "mif-address-radix", &tmp);
        address_radix = str_to_radix(tmp);

        if(address_radix == RADIX_UNK){
            ERROR_WRITE("Failed to decode address radix argument! %s", tmp);
            return false;
        }

        mif_settings->address_radix = address_radix;
    }

    if(options_is_option_set(args, "mif-address-depth")){
        long long val = 0;

        options_get_option_value_number(args, "mif-address-depth", &val);

        if(val < 1){
            ERROR_WRITE("Depth of mif file can not be smaller than 1!");
            return false;
        }

        mif_settings->address_depth = (unsigned)val;
        mif_settings->force_address_depth = true;
    }

    settings_ptr = mif_settings;

    return true;
}

bool mif_backend_convert(ldm_memory_t *memory, char *output_filename){
    CHECK_NULL_ARGUMENT(memory);
    CHECK_NULL_ARGUMENT(output_filename);

    mif_file_t *mif = NULL;
    isa_address_t size = memory->size;

    if(settings_ptr->force_address_depth)
        size = settings_ptr->address_depth;

    mif_init(&mif, size, sizeof(isa_memory_element_t) * CHAR_BIT);
    mif_config_radixes(mif, settings_ptr->data_radix, settings_ptr->address_radix);

    for(unsigned i = 0; i < list_count(memory->items); i++){
        ldm_item_t *head = NULL;
        uintmax_t tmpVar = 0;

        list_at(memory->items, i, (void *)&head);
        tmpVar = head->word;

        if(!mif_set(mif, head->address - memory->begin_addr, 1, &tmpVar)){
            ERROR_WRITE("Failed in MIF building.");
            return false;
        }
    }

    if(!mif_write(mif, output_filename)){
        ERROR_WRITE("Failed to write out MIF %s.", output_filename);
        mif_destroy(mif);
        return false;
    }

    mif_destroy(mif);
    return true;
}
