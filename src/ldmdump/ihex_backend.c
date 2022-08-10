#include "ldmdump.h"

#include <string.h>
#include <limits.h>

static ihex_backend_settings *settings_ptr = NULL;

void ihex_backend_args_init(options_t *args, ihex_backend_settings *ihex_settings){
    CHECK_NULL_ARGUMENT(args);
    CHECK_NULL_ARGUMENT(ihex_settings);

    options_append_section(args, "Intel HEX", "Options valid for Intel HEX backend");
    options_append_flag_2(args, "ihex-i8hex-only", "Stick only with 8bit Intel HEX.");
    options_append_number_option_2(args, "ihex-start-address", "Set start linear address record to value.");

    settings_ptr = ihex_settings;
}

bool ihex_backend_args_parse(options_t *args, ihex_backend_settings *ihex_settings){
    ihex_settings->i8hex_force = false;
    ihex_settings->start_linear_address.requested = false;
    ihex_settings->start_linear_address.address = 0;

    if(options_is_flag_set(args, "ihex-i8hex-only")){
        ihex_settings->i8hex_force = true;
    }

    if(options_is_option_set(args, "ihex-start-address")){
        long long tmp = 0;

        options_get_option_value_number(args, "ihex-start-address", &tmp);

        unsigned size = sizeof(ihex_settings->start_linear_address.address);

        if(!can_fit_in(tmp, size)){
            ERROR_WRITE("Given address for ihex-start-address cannot be larger than %d bytes!", size);
            return false;
        }

        ihex_settings->start_linear_address.address = (uint32_t)tmp;
        ihex_settings->start_linear_address.requested = true;
    }

    settings_ptr = ihex_settings;

    return true;
}

bool ihex_backend_convert(ldm_memory_t *memory, char *output_filename){
    CHECK_NULL_ARGUMENT(memory);
    CHECK_NULL_ARGUMENT(output_filename);

    if((sizeof(isa_memory_element_t) * CHAR_BIT) != 8){
        ERROR_WRITE("Error. Intel HEX can be generated only for byte addressable architectures!");
        return false;
    }

    if(sizeof(isa_address_t) > sizeof(uint32_t)){
        ERROR_WRITE("Error. Intel HEX can be generated only for addresses up to 32bit!");
        return false;
    }

    if((settings_ptr->i8hex_force == true) && (memory->begin_addr + memory->size > 0xFFFF)){
        ERROR_WRITE("Memory %s cannot be converted to i8hex. Memory is too large!", memory->memory_name);
        return false;
    }

    ihex_file_t *file = NULL;
    bool retVal = false;

    ihex_init(&file, memory->size, memory->begin_addr);

    if(settings_ptr->start_linear_address.requested == true){
        ihex_set_start_linear_address(file, settings_ptr->start_linear_address.address);
    }

    for(unsigned i = 0; i < list_count(memory->items); i++){
        ldm_item_t *head = NULL;
        uint8_t tmpVar = 0;

        list_at(memory->items, i, (void *)&head);
        tmpVar = head->word;

        ihex_set_relative(file, i, 1, &tmpVar);
    }

    retVal = ihex_write(file, output_filename);

    if(retVal == false){
        ERROR_WRITE("Failed to write out IHEX %s.", output_filename);
    }

    ihex_destroy(file);
    return retVal;
}
