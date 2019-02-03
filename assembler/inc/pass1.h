#ifndef PASS1_H_included
#define PASS1_H_included

#include <stdint.h>

#include <isa.h>
#include <tokenizer.h>

typedef enum{
    TYPE_BLOB = 0,
    TYPE_INSTRUCTION
}pass1_item_type_t;

typedef struct{
    unsigned int blob_len;
    uint8_t *blob_data;
}blob_t;

typedef struct pass1_item_s{
    struct pass1_item_s *prev;
    struct pass1_item_s *next;
    tok_t *token;
    union{
        blob_t *b;
        tInstruction *i;
    }payload;
    uint32_t location;
    pass1_item_type_t type;
    uint8_t relocation;
    uint8_t special;
}pass1_item_t;

typedef struct pass1_section_s{
    struct pass1_section_s *prev;
    struct pass1_section_s *next;
    pass1_item_t *first_element;
    pass1_item_t *last_element;
    uint32_t last_location_counter;
    char *section_name;
}pass1_section_t;

extern pass1_section_t *pass1_list_first;
extern pass1_section_t *pass1_list_last;

void pass1(void);
void pass1_cleanup(void);

#ifdef DEBUG
void print_pass1_buffer(void);
#endif

#endif
