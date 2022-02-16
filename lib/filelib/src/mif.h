#ifndef MIF_H_included
#define MIF_H_included

#include <stdbool.h>

#include <platformlib.h>
#include <utillib/core.h>

typedef enum{
    RADIX_UNK = 0,
    RADIX_HEX,
    RADIX_BIN,
    RADIX_OCT,
    RADIX_DEC,
    RADIX_UNS
} mif_radix_t;

typedef struct{
    isa_memory_element_t data;
    isa_address_t address;
} mif_item_t;

typedef struct{
    mif_radix_t data_radix;
    unsigned data_width;
    mif_radix_t address_radix;
    unsigned depth;
} mif_settings_t;

typedef struct{
    list_t *content;
    mif_settings_t settings;
} mif_file_t;

bool mif_write(mif_file_t *f, char *filename);

void mif_file_new(mif_file_t **f);
void mif_file_destroy(mif_file_t *f);

void mif_config_data(mif_file_t *f, unsigned data_width);
void mif_config_address(mif_file_t *f, unsigned address_depth);
bool mif_config_radixes(mif_file_t *f, mif_radix_t data_radix, mif_radix_t address_radix);

void mif_item_new(mif_item_t **item ,isa_address_t address, isa_memory_element_t data);
void mif_item_destroy(mif_item_t *item);
void mif_item_into_file(mif_file_t *file, mif_item_t *item);

#endif
