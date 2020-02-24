/**
 * @file ln_section_list.h
 *
 * @brief Portable linker from m2tools.
 *
 * @author Bc. Vladislav Mlejnecký <v.mlejnecky@seznam.cz>
 * @date 07.02.2020
 *
 * @note This file is part of m2tools project.
 *
 * Linker is splitted into multiple files:
 *  - ldparser.c
 *  - ldparser.h
 *  - linker.c
 *  - linker_util.c
 *  - linker_util.h
 *  - ln_section_list.c
 *  - ln_section_lish.h
 *  - ln_symbol_list.c
 *  - ln_symbol_list.h
 */

#ifndef LN_SECTION_LIST_included
#define LN_SECTION_LIST_included

#include <stdbool.h>

#include <isa.h>
#include <ldm.h>
#include <obj.h>
#include <sl.h>

//TODO: append comments

typedef enum{
    SECTION_OK = 0,
    SECTION_MALLOC_FAIL,
    SECTION_WRONG_ARG,
    SECTION_OBJLIB_ERROR,
    SECTION_ISALIB_ERROR,
    SECTION_FAIL_SECTION_MERGE,
    SECTION_MULTIPLE_SYMBOL
}ln_section_list_errno_t;

typedef struct section_list_item_s{
    section_t *section;
    ldm_mem_t *assinged_mem;
    struct section_list_item_s *next;
    isa_address_t begin_addr;
    bool used;
}section_list_item_t;

extern section_list_item_t *first_section_item;

bool append_into_section_list_sl(static_library_t *sl);
bool append_into_section_list_obj(obj_file_t *obj);

void clear_section_list_errno(void);
ln_section_list_errno_t get_section_list_errno(void);

void clean_up_section_list(void);

#ifndef NDEBUG
void print_section_list(void);
#endif

#endif
