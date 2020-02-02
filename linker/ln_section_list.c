#include "ln_section_list.h"

#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include <isa.h>
#include <ldm.h>
#include <obj.h>

#define SET_ERROR(n) if(section_list_errno == SECTION_OK) section_list_errno = n

section_list_item_t *firts_section_item = NULL;
static ln_section_list_errno_t section_list_errno = SECTION_OK;



/**
 * @brief Create deep copy of section.
 *
 * That mean - every object stored in previous structure will be recreated again.
 */
static section_t * make_deep_copy_of_section(section_t *section);

/**
 * @brief Append section into our list in firts_section_item.
 *
 * @return true if append OK; false if problem occurs (if same section exist)
 */
static bool append_into_list(section_t *section);

/**
 * @brief Compare two sections if are the same.
 *
 * @return true if same; false if not
 */
static bool is_same_section(section_t *A, section_t *B);



bool append_into_section_list_sl(static_library_t *sl){
    //todo: go thru all object files and append them into list
    return false;
}

bool append_into_section_list_obj(obj_file_t *obj){
    //todo: go thru all sections, make deep copy of them and append them into list
    return false;
}

static section_t * make_deep_copy_of_section(section_t *section){
    //todo: make deep copy of section
    return false;
}

static bool append_into_list(section_t *section){
    section_list_item_t *head = NULL;

    for(head = firts_section_item; head != NULL; head = head->next){
        if(is_same_section(head->section, section) == true){
            return false;
        }

        if(head->next == NULL){
            //TODO: tutaj je chybka


            head->next = section;
            break;
        }
    }

    return true;
}

static bool is_same_section(section_t *A, section_t *B){
    if(strcmp(A->section_name, B->section_name) == 0){
        return true;
    }
    return false;
}

void clear_section_list_errno(void){
    section_list_errno = SECTION_OK;
}

obj_file_err_t get_section_list_errno(void){
    return section_list_errno;
}

void clean_up_section_list(void){
    section_list_item_t *head = NULL;
    section_list_item_t *tmp = NULL;

    head = firts_section_item;
    while(head != NULL){
        tmp = head;
        head = head->next;

        free_obj_section(tmp->section);
        free(tmp);
    }
}
