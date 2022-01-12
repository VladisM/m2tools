#ifndef SECTION_CACHE_H_included
#define SECTION_CACHE_H_included

#include "ldparser.h"

#include <utillib/core.h>
#include <filelib.h>
#include <platformlib.h>

#include <stdbool.h>

typedef struct{
    obj_section_t *section;
    ldm_memory_t *assigned_memory;
    bool used;
    isa_address_t size;
    isa_address_t offset;
} cache_section_item_t;

typedef enum{
    SYMBOL_EXPORT,
    SYMBOL_IMPORT,
    SYMBOL_LINKER_SCRIPT_ABS,
    SYMBOL_LINKER_SCRIPT_EVAL
} symbol_type_t;

typedef struct{
    symbol_type_t symbol_type;
    obj_symbol_t *symbol;
    cache_section_item_t *assigned_section;
    string_t *eval_string;
    bool evaluated;
} cache_symbol_item_t;

typedef struct{
    ldm_memory_t *ldm_mem;
    isa_address_t next_offset;
} cache_ldm_mem_holder_t;

typedef struct{
    struct{
        list_t *sections;
        list_t *symbols;
    }all;
    struct{
        list_t *obj_files;
        list_t *sl_files;
    }files;
    struct{
        list_t *exported;
        list_t *imported;
    }symbols;
    list_t * offsets;
} cache_t;

void cache_new(cache_t **cache);
void cache_destroy(cache_t *this);

bool cache_load_object_file(cache_t *this, char *filename);
bool cache_load_library_file(cache_t *this, char *filename);

bool cache_build_symbol_table(cache_t *this, list_t *ld_symbols, char *entry_point_label);

void cache_strip_down_unused_sections(cache_t *this);

void cache_assing_section_into_memory(cache_t *this, char *section_name, ldm_memory_t *ldm_mem);

void cache_calculate_real_exported_addresses(cache_t *this);

bool cache_evaluate_labels(cache_t *this, ldm_file_t *ldm);

bool cache_relocate_data(cache_t *this);

bool cache_link_specials(cache_t *this);

void cache_write_data_into_associated_ldm(cache_t *this);

void print_cache(cache_t *this);

#endif
