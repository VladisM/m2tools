/**
 * @file linker_util.h
 *
 * @brief Portable linker from m2tools.
 *
 * @author Bc. Vladislav Mlejnecký <v.mlejnecky@seznam.cz>
 * @date 31.01.2020
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

#ifndef LINKER_UTIL_H_included
#define LINKER_UTIL_H_included

#include <stdbool.h>

#include <isa.h>
#include <obj.h>
#include <ldm.h>
#include <sl.h>

#include "ldparser.h"

/**
 * @brief Check if section is assigned into memory. Also solve '*'.
 *
 * @return true if section have to be in this mem
 */
bool is_section_in_mem(char *sname, mem_t *m);

/**
 * @brief Check if pointer is null after malloc.
 *
 * @param p Pointer to check.
 *
 * If pointer p is null then this write out error msg call exit(EXIT_FAILRULE).
 * Call this after malloc.
 */
void check_malloc(void *p);

/**
 * @brief Count size of section.
 *
 * @param s Section to calc.
 *
 * @return size
 */
isa_address_t get_section_size(section_t *s);

#endif
