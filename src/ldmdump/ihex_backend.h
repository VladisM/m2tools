#ifndef IHEX_BACKEND_H_included
#define IHEX_BACKEND_H_included

#include <filelib.h>
#include <utillib/cli.h>
#include <utillib/files.h>

#include <stdbool.h>

typedef struct{
    bool i8hex_force;
    struct{
        bool requested;
        uint32_t address;
    }start_linear_address;
}ihex_backend_settings;

void ihex_backend_args_init(options_t *args, ihex_backend_settings *ihex_settings);
bool ihex_backend_args_parse(options_t *args, ihex_backend_settings *ihex_settings);
bool ihex_backend_convert(ldm_memory_t *memory, char *output_filename);

#endif
