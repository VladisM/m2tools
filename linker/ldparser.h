/**
 * @file ldparser.h
 *
 * @brief Portable linker from m2tools.
 *
 * @author Bc. Vladislav Mlejnecký <v.mlejnecky@seznam.cz>
 * @date 19.07.2019
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

#ifndef LDPARSE_H_included
#define LDPARSE_H_included

#include <stdint.h>

#include <isa.h>
#include <obj.h>
#include <ldm.h>
#include <sl.h>

typedef struct mem_s{
    struct mem_s *next;
    struct mem_s *prev;
    char *name;
    isa_address_t size;
    isa_address_t orig;
    char **sections;
    unsigned int section_count;
}mem_t;

typedef struct sym_s{
    struct sym_s *next;
    struct sym_s *prev;
    char *name;
    isa_address_t value;
}sym_t;

typedef struct lds_s{
    mem_t *first_mem;
    mem_t *last_mem;
    sym_t *first_sym;
    sym_t *last_sym;
    char *entry_point;
}lds_t;

lds_t *parse_lds(char *path);
void free_lds(lds_t *l);

#ifndef NDEBUG
void print_lds(lds_t *l);
#endif

#endif
