#ifndef SECTION_TABLE_H_included
#define SECTION_TABLE_H_included

#include <platformlib.h>
#include <utillib/core.h>

typedef struct{
    char *section_name;
    isa_address_t last_location_counter;
    queue_t *items;
    list_t *symbols;
} section_t;

void section_table_init(void);
section_t *section_table_get_actual_section(void);
section_t *section_table_append_or_switch(char *section_name);
void section_table_deinit(void);
list_t *section_table_get_all(void);

#endif
