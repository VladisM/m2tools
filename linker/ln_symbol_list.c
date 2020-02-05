#include "ln_symbol_list.h"

#include <stdbool.h>
#include <stdlib.h>

#include <obj.h>
#include <isa.h>

#include "ln_section_list.h"

symbol_holder_t *exported_symbols = NULL;
symbol_holder_t *imported_symbols = NULL;

bool parse_symbols(section_list_item_t *section){
    return false;
}

void clean_up_symbol_lists(void){
    return;
}
