#include "section_table.h"

#include "common.h"
#include "symbol_table.h"
#include "pass_item.h"

#include <platformlib.h>
#include <utillib/core.h>
#include <utillib/utils.h>

#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#define CHECK_IF_INITIALIZED() {if(section_table == NULL){ error("Section table is not initialized!"); }}
#define CHECK_IF_NOT_INITIALIZED() {if(section_table != NULL){ error("Section table is already initialized!"); }}

static section_t *find_section(char *section_name);
static bool does_section_exist(char *section_name);
static section_t *section_new(char *section_name);
static void section_destroy(section_t *section);

static list_t *section_table = NULL;
static section_t *actual_section = NULL;

void section_table_init(void){
    CHECK_IF_NOT_INITIALIZED();
    list_init(&section_table, sizeof(section_t *));
}

section_t *section_table_get_actual_section(void){
    CHECK_IF_INITIALIZED();

    if(actual_section == NULL){
        error("No section opened at this time!");
    }

    return actual_section;
}

section_t *section_table_append_or_switch(char *section_name){
    CHECK_NULL_ARGUMENT(section_name);
    CHECK_IF_INITIALIZED();

    if(does_section_exist(section_name)){
        actual_section = find_section(section_name);
    }
    else{
        section_t *tmp = section_new(section_name);
        list_append(section_table, (void *)&tmp);
        actual_section = tmp;
    }

    return section_table_get_actual_section();
}

void section_table_deinit(void){
    CHECK_IF_INITIALIZED();

    for(unsigned int i = 0; i < list_count(section_table); i++){
        section_t *section = NULL;
        list_at(section_table, i, (void *)&section);

        section_destroy(section);
    }

    list_destroy(section_table);
    actual_section = NULL;
    section_table = NULL;
}

list_t *section_table_get_all(void){
    CHECK_IF_INITIALIZED();
    return section_table;
}

static section_t *find_section(char *section_name){
    CHECK_NULL_ARGUMENT(section_name);

    section_t *retVal = NULL;

    for(unsigned int i = 0; i < list_count(section_table); i++){
        section_t *section = NULL;
        list_at(section_table, i, (void *)&section);

        if(strcmp(section->section_name, section_name) == 0){
            retVal = section;
            break;
        }
    }

    return retVal;
}

static bool does_section_exist(char *section_name){
    CHECK_NULL_ARGUMENT(section_name);

    if(find_section(section_name) == NULL){
        return false;
    }
    else{
        return true;
    }
}

static section_t *section_new(char *section_name){
    CHECK_NULL_ARGUMENT(section_name);

    section_t *tmp = NULL;

    tmp = (section_t *)dynmem_calloc(1, sizeof(section_t));

    tmp->section_name = dynmem_strdup(section_name);
    tmp->last_location_counter = 0;
    tmp->items = NULL;

    list_init(&(tmp->symbols), sizeof(symbol_t *));
    list_init(&(tmp->items), sizeof(pass_item_t *));

    return tmp;
}

static void section_destroy(section_t *section){
    CHECK_NULL_ARGUMENT(section);

    list_destroy(section->symbols);
    list_destroy(section->items);
    dynmem_free(section->section_name);
    dynmem_free(section);
}
