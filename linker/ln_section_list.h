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
    SECTION_MULTIPLE,
    SECTION_MALLOC_FAIL,
    SECTION_WRONG_ARG,
    SECTION_OBJLIB_ERROR,
    SECTION_ISALIB_ERROR
}ln_section_list_errno_t;

typedef struct section_list_item_s{
    section_t *section;
    ldm_mem_t *assinged_mem;
    struct section_list_item_s *next;
    isa_address_t begin_addr;
    bool used;
}section_list_item_t;

extern section_list_item_t *firts_section_item;

bool append_into_section_list_sl(static_library_t *sl);
bool append_into_section_list_obj(obj_file_t *obj);

void clear_section_list_errno(void);
ln_section_list_errno_t get_section_list_errno(void);

void clean_up_section_list(void);

#endif
