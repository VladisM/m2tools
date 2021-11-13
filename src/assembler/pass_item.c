#include "pass_item.h"

#include <utillib/core.h>

#define CHECK_IF_INITIALIZED() {if(item_db == NULL){ error("Item db is not initialized!"); }}
#define CHECK_IF_NOT_INITIALIZED() {if(item_db != NULL){ error("item db is already initialized!"); }}

static void pass_item_destroy(pass_item_t *item);
static pass_item_t *pass_item_create(void);

static list_t *item_db = NULL;
static pass_item_t *last_item = NULL;

void pass_item_db_init(void){
    CHECK_IF_NOT_INITIALIZED();
    list_init(&item_db, sizeof(pass_item_t *));
    last_item = NULL;
}

void pass_item_db_deinit(void){
    CHECK_IF_INITIALIZED();

    for(unsigned int i = 0; i < list_count(item_db); i++){
        pass_item_t *item = NULL;
        list_at(item_db, i, (void *)&item);

        pass_item_destroy(item);
    }

    list_destroy(item_db);

    item_db = NULL;
    last_item = NULL;
}

static pass_item_t *pass_item_create(void){
    pass_item_t *tmp = (pass_item_t *)dynmem_calloc(1, sizeof(pass_item_t));

    tmp->address = 0;
    tmp->args = NULL;
    tmp->section = NULL;
    tmp->type = ITEM_INST;
    tmp->relocation = false;
    tmp->special = false;
    tmp->value.instr = 0;
    tmp->value.blob = 0;
    tmp->special_value = 0;

    list_init(&(tmp->args), sizeof(preprocessed_token_t *));

    return tmp;
}

static void pass_item_destroy(pass_item_t *item){
    CHECK_NULL_ARGUMENT(item);
    list_destroy(item->args);
    dynmem_free(item);
}

void pass_item_db_create_item(isa_address_t address, section_t *section, pass_item_type_t type){
    CHECK_NULL_ARGUMENT(section);
    CHECK_IF_INITIALIZED();

    last_item = pass_item_create();

    last_item->address = address;
    last_item->section = section;
    last_item->type = type;

    queue_append(section->items, (void *)&last_item);
    list_append(item_db, (void *)&last_item);
}

pass_item_t *pass_item_db_get_last(void){
    CHECK_IF_INITIALIZED();
    if(last_item == NULL){
        error("Requesting NULL last item!");
    }
    return last_item;
}

void pass_item_db_append_arg(pass_item_t *item, preprocessed_token_t *arg){
    CHECK_NULL_ARGUMENT(item);
    CHECK_NULL_ARGUMENT(arg);
    CHECK_IF_INITIALIZED();
    list_append(item->args, (void *)&arg);
}

list_t *pass_item_db_get_all(void){
    CHECK_IF_INITIALIZED();
    return item_db;
}
