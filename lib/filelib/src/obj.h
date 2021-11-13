#ifndef FILELIB_OBJ_H_included
#define FILELIB_OBJ_H_included

#include <stdbool.h>

#include <platformlib.h>
#include <utillib/core.h>

typedef struct{
    isa_address_t address;
    union{
        isa_instruction_word_t data_value;
        isa_memory_element_t blob_value;
    }payload;
    isa_address_t special_value;
    bool relocation;
    bool special;
    bool blob;
} obj_data_t;

typedef struct{
    char *name;
    isa_address_t value;
} obj_symbol_t;

typedef struct{
    char *section_name;
    list_t *exported_symbol_list;
    list_t *imported_symbol_list;
    list_t *data_symbol_list;
} obj_section_t;

typedef struct{
    char *target_arch_name;
    list_t *section_list;
} obj_file_t;

bool obj_load(char *filename, obj_file_t **f);
bool obj_write(obj_file_t *f, char *filename);

void obj_file_new(obj_file_t **f);
void obj_file_destroy(obj_file_t *f);

void obj_section_new(char *section_name, obj_section_t **section);
void obj_section_destroy(obj_section_t *section);
void obj_section_into_file(obj_file_t *file, obj_section_t *section);

void obj_data_new(obj_data_t **symbol, isa_address_t address, isa_instruction_word_t value, bool relocation, bool special, isa_address_t special_value);
void obj_data_destroy(obj_data_t *symbol);
void obj_data_into_section(obj_section_t *section, obj_data_t *symbol);

void obj_blob_new(obj_data_t **symbol, isa_address_t address, isa_memory_element_t value);
void obj_blob_destroy(obj_data_t *symbol);
void obj_blob_into_section(obj_section_t *section, obj_data_t *symbol);

void obj_symbol_new(obj_symbol_t **symbol, char *name, isa_address_t value);
void obj_symbol_destroy(obj_symbol_t *symbol);
void obj_exported_symbol_into_section(obj_section_t *section, obj_symbol_t *symbol);
void obj_imported_symbol_into_section(obj_section_t *section, obj_symbol_t *symbol);

#endif
