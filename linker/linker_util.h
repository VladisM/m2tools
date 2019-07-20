#ifndef LINKER_UTIL_H_included
#define LINKER_UTIL_H_included

#include <stdbool.h>

#include "ldparser.h"

/**
 * @brief Check if section is assigned into memory. Also solve '*'.
 *
 * @return true if section have to be in this mem
 */
bool is_section_in_mem(char *sname, mem_t *m);
void check_malloc(void *p);

#endif
