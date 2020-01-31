/**
 * @file common_defs.h
 *
 * @brief Two pass assembler with simple preprocessor.
 *
 * @author Bc. Vladislav Mlejnecký <v.mlejnecky@seznam.cz>
 * @date 15.06.2019
 *
 * @note This file is part of m2tools project.
 *
 * Assembler is splitted into multiple files:
 *  - assembler.c
 *  - common_defs.h
 *  - file_gen.c
 *  - file_gen.h
 *  - pass1.c
 *  - pass1.h
 *  - pass2.c
 *  - pass2.h
 *  - symbol_table.c
 *  - symbol_table.h
 *  - tokenizer.c
 *  - tokenizer.h
 *  - asm_util.c
 *  - asm_util.h
 */

#ifndef COMMON_DEFS_H_included
#define COMMON_DEFS_H_included

#include <stdint.h>

#include <isa.h>

/* -----------------------------------------------------------------------------
 * Defines related to tokenizer part.
 */

typedef enum{
    TOKEN_IS_LABEL,
    TOKEN_IS_PSEUD,
    TOKEN_IS_INSTR
}token_type_t;

typedef struct{
    char *line;
}label_t;

typedef struct{
    char * line;
}pseudo_t;

//track files
typedef struct fileInfoOut_s{
    struct fileInfoOut_s *next;
    struct fileInfoOut_s *prev;
    char *absName;
    char *absPath;
    char *name;
}fileInfoOut_t;

//all items parsed by the tokenizer are stored in this struct
typedef struct tok_s{
    struct tok_s *next;                 //it is double linked list :)
    struct tok_s *prev;
    fileInfoOut_t *fileInfo;     //pointer to fileinfo where informations about file are stored
    union{
        label_t *l;
        pseudo_t *p;
        tInstruction *i;
    }payload;
    unsigned int lineNumber;            //token was found on this line
    token_type_t type;
}tok_t;

/* -----------------------------------------------------------------------------
 * Defines related to pass1 part. (pass2 use them too)
 */

typedef enum{
    TYPE_BLOB = 0,
    TYPE_INSTRUCTION
}pass_item_type_t;

typedef struct{
    uint8_t *blob_data;
    unsigned int blob_len;
}blob_t;

typedef struct pass_item_s{
    struct pass_item_s *prev;
    struct pass_item_s *next;
    tok_t *token;
    union{
        blob_t *b;
        tInstruction *i;
    }payload;
    isa_address_t location;
    pass_item_type_t type;
    bool relocation;
    bool special;
}pass_item_t;

typedef struct pass1_section_s{
    struct pass1_section_s *prev;
    struct pass1_section_s *next;
    pass_item_t *first_element;
    pass_item_t *last_element;
    char *section_name;
    isa_address_t last_location_counter;
}pass_section_t;

/* -----------------------------------------------------------------------------
 * Defines related to symbol table.
 */

#define STYPE_ABSOLUTE    0x01U
#define STYPE_RELOCATION  0x02U
#define STYPE_EXPORT      0x04U
#define STYPE_IMPORT      0x08U

typedef struct symbol_s{
    struct symbol_s *next;
    struct symbol_s *prev;
    char *label;
    tok_t *parent;
    void *section;
    isa_address_t address;
    uint8_t stype;
}symbol_t;

/* -----------------------------------------------------------------------------
 * Pointers to lists.
 */

extern tok_t *toklist_first;
extern tok_t *toklist_last;

extern pass_section_t *pass_list_first;
extern pass_section_t *pass_list_last;

extern symbol_t *symbol_first;
extern symbol_t *symbol_last;

#endif
