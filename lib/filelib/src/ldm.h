#ifndef FILELIB_LDM_H_included
#define FILELIB_LDM_H_included

#include <stdbool.h>

#include <platformlib.h>
#include <utillib/core.h>

typedef struct{
    isa_address_t address;
    isa_memory_element_t word;
} ldm_item_t;

typedef struct{
    char *memory_name;
    isa_address_t begin_addr;
    isa_address_t size;
    list_t *items;
} ldm_memory_t;

typedef struct{
    char *target_arch_name;
    isa_address_t entry_point;
    list_t *memories;
} ldm_file_t;

bool ldm_load(char *filename, ldm_file_t **f);
bool ldm_write(ldm_file_t *f, char *filename);

void ldm_file_new(ldm_file_t **f);
void ldm_file_destroy(ldm_file_t *f);
void ldm_file_set_entry(ldm_file_t *f, isa_address_t entry_point);

void ldm_mem_new(char *memory_name, ldm_memory_t **mem, isa_address_t size, isa_address_t begin_addr);
void ldm_mem_into_file(ldm_file_t *f, ldm_memory_t *mem);
void ldm_mem_destroy(ldm_memory_t *mem);

void ldm_item_new(isa_address_t address, isa_memory_element_t word, ldm_item_t **item);
void ldm_item_into_mem(ldm_memory_t *mem, ldm_item_t *item);
void ldm_item_destroy(ldm_item_t *item);

#endif
