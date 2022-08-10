#ifndef MIF_BACKEND_H_included
#define MIF_BACKEND_H_included

#include <filelib.h>
#include <utillib/cli.h>
#include <utillib/files.h>

#include <stdbool.h>

typedef struct{
    mif_radix_t data_radix;
    mif_radix_t address_radix;
    unsigned address_depth;
    bool force_address_depth;
}mif_backend_settings_t;

void mif_backend_args_init(options_t *args, mif_backend_settings_t *mif_settings);
bool mif_backend_args_parse(options_t *args, mif_backend_settings_t *mif_settings);
bool mif_backend_convert(ldm_memory_t *memory, char *output_filename);

#endif
