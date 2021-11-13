#ifndef PASS_ITEM_H_included
#define PASS_ITEM_H_included

#include "section_table.h"
#include "preprocessor.h"

#include <platformlib.h>

#include <stdbool.h>

typedef enum{
    ITEM_BLOB = 0,
    ITEM_INST,
} pass_item_type_t;

typedef struct{
    union{
        isa_instruction_word_t instr;
        isa_memory_element_t blob;
    }value;
    list_t *args;
    section_t *section;
    isa_address_t address;
    pass_item_type_t type;
    isa_address_t special_value;
    bool relocation;
    bool special;
}pass_item_t;

void pass_item_db_init(void);
void pass_item_db_deinit(void);

void pass_item_db_create_item(isa_address_t address, section_t *section, pass_item_type_t type);
pass_item_t *pass_item_db_get_last(void);
void pass_item_db_append_arg(pass_item_t *item, preprocessed_token_t *arg);

list_t *pass_item_db_get_all(void);

#endif
