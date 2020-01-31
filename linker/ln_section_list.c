#include "ln_section_list.h"

#include <stdlib.h>
#include <stdbool.h>

#include <isa.h>
#include <ldm.h>
#include <obj.h>

#define SET_ERROR(n) if(section_list_errno == SECTION_OK) section_list_errno = n

section_list_item_t *firts_section_item = NULL;
static ln_section_list_errno_t section_list_errno = SECTION_OK;

static section_t * make_deep_copy_of_section(section_t *section);
static bool append_into_list(section_t *section);

bool append_into_section_list_sl(static_library_t *sl){
    //todo: go thru all object files and append them into list
}

bool append_into_section_list_obj(obj_file_t *obj){
    //todo: go thru all sections, make deep copy of them and append them into list
}

static section_t * make_deep_copy_of_section(section_t *section){
    //todo: make deep copy of section
}

static bool append_into_list(section_t *section){
    //todo: append section into list, also check if same section exist - if so return error! multiple section definition is not alowed
}

void clear_section_list_errno(void){
    section_list_errno = SECTION_OK;
}

obj_file_err_t get_section_list_errno(void){
    return section_list_errno;
}

void clean_up_section_list(void){
    //todo: free-up dynamic memory used by section list
}
