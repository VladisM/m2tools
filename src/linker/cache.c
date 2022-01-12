#include "cache.h"

#include "common.h"
#include "ldparser.h"

#include <utillib/core.h>
#include <filelib.h>
#include <platformlib.h>

#include <stdbool.h>
#include <string.h>
#include <stdio.h>

static cache_section_item_t *cache_section_item_new(obj_section_t *section);
static void cache_section_item_destroy(cache_section_item_t *item);
static cache_symbol_item_t *cache_symbol_item_new(void);
static void cache_symbol_item_destroy(cache_symbol_item_t *item);
static cache_ldm_mem_holder_t *cache_ldm_mem_holder_new(ldm_memory_t *mem);
static void cache_ldm_mem_holder_destroy(cache_ldm_mem_holder_t *holder);

void cache_new(cache_t **cache){
    CHECK_NULL_ARGUMENT(cache);
    CHECK_NOT_NULL_ARGUMENT(*cache);

    *cache = (cache_t *)dynmem_malloc(sizeof(cache_t));

    (*cache)->all.sections = NULL;
    (*cache)->all.symbols = NULL;
    (*cache)->files.obj_files = NULL;
    (*cache)->files.sl_files = NULL;
    (*cache)->symbols.imported = NULL;
    (*cache)->symbols.exported = NULL;
    (*cache)->offsets = NULL;

    list_init(&((*cache)->all.sections), sizeof(cache_section_item_t *));
    list_init(&((*cache)->all.symbols), sizeof(cache_symbol_item_t *));
    list_init(&((*cache)->files.obj_files), sizeof(obj_file_t *));
    list_init(&((*cache)->files.sl_files), sizeof(sl_file_t *));
    list_init(&((*cache)->symbols.exported), sizeof(cache_symbol_item_t *));
    list_init(&((*cache)->symbols.imported), sizeof(cache_symbol_item_t *));
    list_init(&((*cache)->offsets), sizeof(cache_ldm_mem_holder_t *));
}

void cache_destroy(cache_t *this){
    if(this == NULL)
        return;

    if(this->all.sections != NULL){
        while(list_count(this->all.sections) > 0){
            cache_section_item_t *tmp = NULL;
            list_windraw(this->all.sections, (void *)&tmp);
            cache_section_item_destroy(tmp);
        }
        list_destroy(this->all.sections);
    }

    if(this->files.obj_files != NULL){
        while(list_count(this->files.obj_files) > 0){
            obj_file_t *tmp = NULL;
            list_windraw(this->files.obj_files, (void *)&tmp);
            obj_file_destroy(tmp);
        }
        list_destroy(this->files.obj_files);
    }

    if(this->files.sl_files != NULL){
        while(list_count(this->files.sl_files) > 0){
            sl_file_t *tmp = NULL;
            list_windraw(this->files.sl_files, (void *)&tmp);
            sl_file_destroy(tmp);
        }
        list_destroy(this->files.sl_files);
    }

    if(this->symbols.exported != NULL){
        list_destroy(this->symbols.exported);
    }

    if(this->symbols.imported != NULL){
        list_destroy(this->symbols.imported);
    }

    if(this->all.symbols != NULL){
        while(list_count(this->all.symbols) > 0){
            cache_symbol_item_t *tmp = NULL;
            list_windraw(this->all.symbols, (void *)&tmp);
            cache_symbol_item_destroy(tmp);
        }
        list_destroy(this->all.symbols);
    }

    if(this->offsets != NULL){
        while(list_count(this->offsets) > 0){
            cache_ldm_mem_holder_t *tmp = NULL;
            list_windraw(this->offsets, (void *)&tmp);
            cache_ldm_mem_holder_destroy(tmp);
        }
        list_destroy(this->offsets);
    }

    dynmem_free(this);
}

static cache_section_item_t *cache_section_item_new(obj_section_t *section){
    CHECK_NULL_ARGUMENT(section);

    cache_section_item_t *tmp = (cache_section_item_t *)dynmem_malloc(sizeof(cache_section_item_t));

    tmp->section = section;
    tmp->assigned_memory = NULL;
    tmp->used = false;
    tmp->size = 0;
    tmp->offset = 0;

    return tmp;
}

static void cache_section_item_destroy(cache_section_item_t *item){
    if(item == NULL)
        return;

    dynmem_free(item);
}

static cache_symbol_item_t *cache_symbol_item_new(void){
    cache_symbol_item_t *tmp = (cache_symbol_item_t *)dynmem_malloc(sizeof(cache_symbol_item_t));

    tmp->symbol = NULL;
    tmp->symbol_type = SYMBOL_EXPORT;
    tmp->assigned_section = NULL;
    tmp->eval_string = NULL;

    return tmp;
}

static void cache_symbol_item_destroy(cache_symbol_item_t *item){
    if(item == NULL)
        return;

    if((item->symbol_type == SYMBOL_LINKER_SCRIPT_ABS) || (item->symbol_type == SYMBOL_LINKER_SCRIPT_EVAL)){
        if(item->symbol != NULL){
            obj_symbol_destroy(item->symbol);
        }
    }

    dynmem_free(item);
}

static cache_ldm_mem_holder_t *cache_ldm_mem_holder_new(ldm_memory_t *mem){
    CHECK_NULL_ARGUMENT(mem);

    cache_ldm_mem_holder_t *tmp = (cache_ldm_mem_holder_t *)dynmem_malloc(sizeof(cache_ldm_mem_holder_t));

    tmp->ldm_mem = mem;
    tmp->next_offset = 0;

    return tmp;
}

static void cache_ldm_mem_holder_destroy(cache_ldm_mem_holder_t *holder){
    if(holder == NULL)
        return;

    dynmem_free(holder);
}

//-----------------------------------
// Appending data into cache

static cache_section_item_t *find_section_by_name(cache_t *this, char *section_name){
    CHECK_NULL_ARGUMENT(this);
    CHECK_NULL_ARGUMENT(section_name);

    for(unsigned i = 0; i < list_count(this->all.sections); i++){
        cache_section_item_t *tmp = NULL;
        list_at(this->all.sections, i, (void *)&tmp);

        if(strcmp(tmp->section->section_name, section_name) == 0){
            return tmp;
        }
    }

    return NULL;
}

static isa_address_t get_instru_size(isa_instruction_word_t word){
    instruction_signature_t *signature = platformlib_get_instruction_signature_1(word);

    if(signature == NULL){
        error("Tryting to get type of something that isn't instruction!");
    }

    return signature->size;
}

static isa_address_t get_section_size(obj_section_t *section){
    CHECK_NULL_ARGUMENT(section);

    isa_address_t size = 0;

    for(unsigned int i = 0; i < list_count(section->data_symbol_list); i++){
        obj_data_t *head = NULL;
        list_at(section->data_symbol_list, i, (void *)&head);

        if(head->blob){
            size += 1;
        }
        else{
            size += get_instru_size(head->payload.data_value);
        }
    }

    return size;
}

static bool merge_sections(obj_section_t *A, obj_section_t *B){
    CHECK_NULL_ARGUMENT(A);
    CHECK_NULL_ARGUMENT(B);

    //get informations about old section
    isa_address_t address_offset = get_section_size(A);
    unsigned int import_label_counter = list_count(A->imported_symbol_list);

    //copy symbols with new values
    for(unsigned int i = 0; i < list_count(B->exported_symbol_list); i++){
        obj_symbol_t *head = NULL;
        obj_symbol_t *new = NULL;

        list_at(B->exported_symbol_list, i, (void *)&head);
        obj_symbol_new(&new, head->name, head->value + address_offset);
        obj_exported_symbol_into_section(A, new);
    }

    for(unsigned int i = 0; i < list_count(B->imported_symbol_list); i++){
        obj_symbol_t *head = NULL;
        obj_symbol_t *new = NULL;

        list_at(B->imported_symbol_list, i, (void *)&head);
        obj_symbol_new(&new, head->name, head->value + import_label_counter);
        obj_imported_symbol_into_section(A, new);
    }

    //copy data
    for(unsigned int i = 0; i < list_count(B->data_symbol_list); i++){
        obj_data_t *head = NULL;

        list_at(B->data_symbol_list, i, (void *)&head);

        if(head->blob == false){
            obj_data_t *new = NULL;

            obj_data_new(
                &new,
                head->address + address_offset,
                head->payload.data_value,
                head->relocation,
                head->special,
                head->special ? head->special_value + import_label_counter : head->special_value
            );
            if(new->relocation == true){
                if(!platformlib_relocate_instruction(new->payload.data_value, &(new->payload.data_value), address_offset)){
                    ERROR_WRITE("Platform lib error in merging sections.");
                    ERROR_WRITE("%s", platformlib_error());
                    obj_data_destroy(new);
                    return false;
                }
            }
            obj_data_into_section(A, new);
        }
        else{
            obj_data_t *new = NULL;
            obj_blob_new(&new, head->address + address_offset, head->payload.blob_value);
            obj_blob_into_section(A, new);
        }
    }

    return true;
}

static bool is_section_exist(cache_t *this, char *section_name){
    if(find_section_by_name(this, section_name) != NULL){
        return true;
    }
    else{
        return false;
    }
}

static bool process_obj_file_load(cache_t *this, obj_file_t *obj){
    CHECK_NULL_ARGUMENT(this);
    CHECK_NULL_ARGUMENT(obj);

    bool retVal = true;

    for(unsigned i = 0; i < list_count(obj->section_list); i++){
        obj_section_t *section = NULL;
        cache_section_item_t *section_item = NULL;
        list_at(obj->section_list, i, (void *)&section);

        if(is_section_exist(this, section->section_name)){
            cache_section_item_t *old = find_section_by_name(this, section->section_name);
            if(!merge_sections(old->section, section)){
                retVal = false;
                break;
            }
            section_item = old;
        }
        else{
            cache_section_item_t *new_item = cache_section_item_new(section);
            list_append(this->all.sections, (void *)&new_item);
            section_item = new_item;
        }

        section_item->size = get_section_size(section_item->section);
    }

    return retVal;
}

bool cache_load_object_file(cache_t *this, char *filename){
    CHECK_NULL_ARGUMENT(this);
    CHECK_NULL_ARGUMENT(filename);

    obj_file_t *tmp = NULL;

    if(!obj_load(filename, &tmp)){
        ERROR_WRITE("Failed to load object file %s.", filename);
        ERROR_WRITE("Filelib error: %s", filelib_error());
        return false;
    }

    if(!process_obj_file_load(this, tmp)){
        obj_file_destroy(tmp);
        return false;
    }

    list_append(this->files.obj_files, (void *)&tmp);

    return true;
}

bool cache_load_library_file(cache_t *this, char *filename){
    CHECK_NULL_ARGUMENT(this);
    CHECK_NULL_ARGUMENT(filename);

    sl_file_t *tmp = NULL;

    if(!sl_load(filename, &tmp)){
        ERROR_WRITE("Failed to load static library %s.", filename);
        ERROR_WRITE("Filelib error: %s", filelib_error());
        return false;
    }

    for(unsigned i = 0; i < list_count(tmp->objects); i++){
        obj_file_t *tmp_obj = NULL;
        list_at(tmp->objects, i, (void *)&tmp_obj);

        if(!process_obj_file_load(this, tmp_obj)){
            sl_file_destroy(tmp);
            return false;
        }
    }

    list_append(this->files.sl_files, (void *)&tmp);

    return true;
}

//-----------------------------------
// Symbol cache

static bool check_if_symbol_exist_by_name(char *symbol_name, list_t *list, cache_symbol_item_t **found_symbol){
    for(unsigned int index = 0; index < list_count(list); index++){
        cache_symbol_item_t *head = NULL;
        list_at(list, index, (void *)&head);

        if(strcmp(head->symbol->name, symbol_name) == 0){
            if(found_symbol != NULL){
                *found_symbol = head;
            }

            return true;
        }
    }
    return false;
}

static bool process_symbol(cache_t *this, cache_section_item_t *section_parent, obj_symbol_t *symbol, symbol_type_t type, string_t *eval_string){
    cache_symbol_item_t *holder = cache_symbol_item_new();

    holder->symbol = symbol;
    holder->symbol_type = type;
    holder->assigned_section = section_parent;
    holder->evaluated = false;

    if(type != SYMBOL_IMPORT){
        if(check_if_symbol_exist_by_name(holder->symbol->name, this->symbols.exported, NULL) == true){
            ERROR_WRITE("Linkage error, multiple symbol definition of %s.", holder->symbol->name);
            cache_symbol_item_destroy(holder);
            return false;
        }
    }

    if(type == SYMBOL_EXPORT){
        list_append(this->symbols.exported, (void *)&holder);
    }
    else if(type == SYMBOL_IMPORT){
        list_append(this->symbols.imported, (void *)&holder);
    }
    else if(type == SYMBOL_LINKER_SCRIPT_ABS){
        list_append(this->symbols.exported, (void *)&holder);
    }
    else if(type == SYMBOL_LINKER_SCRIPT_EVAL){
        holder->eval_string = eval_string;
        list_append(this->symbols.exported, (void *)&holder);
    }
    else{
        error("unknown symbol type!");
    }

    list_append(this->all.symbols, (void *)&holder);

    return true;
}

static bool process_import_symbol(cache_t *this, cache_section_item_t *section_parent, obj_symbol_t *symbol){
    CHECK_NULL_ARGUMENT(this);
    CHECK_NULL_ARGUMENT(section_parent);
    CHECK_NULL_ARGUMENT(symbol);
    return process_symbol(this, section_parent, symbol, SYMBOL_IMPORT, NULL);
}

static bool process_export_symbol(cache_t *this, cache_section_item_t *section_parent, obj_symbol_t *symbol){
    CHECK_NULL_ARGUMENT(this);
    CHECK_NULL_ARGUMENT(section_parent);
    CHECK_NULL_ARGUMENT(symbol);
    return process_symbol(this, section_parent, symbol, SYMBOL_EXPORT, NULL);
}

static bool process_linker_abs_symbol(cache_t *this, obj_symbol_t *symbol){
    CHECK_NULL_ARGUMENT(this);
    CHECK_NULL_ARGUMENT(symbol);
    return process_symbol(this, NULL, symbol, SYMBOL_LINKER_SCRIPT_ABS, NULL);
}

static bool process_linker_eval_symbol(cache_t *this, obj_symbol_t *symbol, string_t *eval_string){
    CHECK_NULL_ARGUMENT(this);
    CHECK_NULL_ARGUMENT(symbol);
    CHECK_NULL_ARGUMENT(eval_string);
    return process_symbol(this, NULL, symbol, SYMBOL_LINKER_SCRIPT_EVAL, eval_string);
}

bool cache_build_symbol_table(cache_t *this, list_t *ld_symbols, char *entry_point_label){
    CHECK_NULL_ARGUMENT(this);
    CHECK_NULL_ARGUMENT(ld_symbols);

    for(unsigned section_index = 0; section_index < list_count(this->all.sections); section_index++){
        cache_section_item_t *head_section = NULL;
        list_at(this->all.sections, section_index, (void *)&head_section);

        for(unsigned symbol_index = 0; symbol_index < list_count(head_section->section->exported_symbol_list); symbol_index++){
            obj_symbol_t *head_symbol = NULL;
            list_at(head_section->section->exported_symbol_list, symbol_index, (void *)&head_symbol);

            if(!process_export_symbol(this, head_section, head_symbol)){
                return false;
            }
        }

        for(unsigned symbol_index = 0; symbol_index < list_count(head_section->section->imported_symbol_list); symbol_index++){
            obj_symbol_t *head_symbol = NULL;
            list_at(head_section->section->imported_symbol_list, symbol_index, (void *)&head_symbol);

            if(!process_import_symbol(this, head_section, head_symbol)){
                return false;
            }
        }
    }

    for(unsigned symbol_index = 0; symbol_index < list_count(ld_symbols); symbol_index++){
        sym_t *head_symbol = NULL;
        obj_symbol_t *new_symbol = NULL;
        list_at(ld_symbols, symbol_index, (void *)&head_symbol);

        obj_symbol_new(&new_symbol, head_symbol->name, head_symbol->value);

        if(head_symbol->type == LDS_SYMBOL_ABSOLUTE){
            if(!process_linker_abs_symbol(this, new_symbol)){
                return false;
            }
        }
        else if(head_symbol->type == LDS_SYMBOL_EVAL){
            if(!process_linker_eval_symbol(this, new_symbol, head_symbol->eval_expresion)){
                return false;
            }
        }
        else{
            error("Unknown symbol type from linker script!");
        }
    }

    //check if all symbols are exported
    for(unsigned symbol_index = 0; symbol_index < list_count(this->symbols.imported); symbol_index++){
        cache_symbol_item_t *head_symbol = NULL;
        cache_symbol_item_t *found_symbol = NULL;
        list_at(this->symbols.imported, symbol_index, (void *)&head_symbol);

        if(check_if_symbol_exist_by_name(head_symbol->symbol->name, this->symbols.exported, &found_symbol) == false){
            ERROR_WRITE("Linkage error, undefined symbol %s!", head_symbol->symbol->name);
            return false;
        }

        //mark as used (symbols from linker script doesn't have sections assigned)
        if((found_symbol->symbol_type != SYMBOL_LINKER_SCRIPT_ABS) &&
           (found_symbol->symbol_type != SYMBOL_LINKER_SCRIPT_EVAL)){
            found_symbol->assigned_section->used = true;
        }
    }

    //mark section containing entry point as used
    cache_symbol_item_t *entry_point_synbol_export = NULL;

    if(check_if_symbol_exist_by_name(entry_point_label, this->symbols.exported, &entry_point_synbol_export) == false){
        ERROR_WRITE("Linkage error, missing entry point '%s' symbol!", entry_point_label);
        return false;
    }

    entry_point_synbol_export->assigned_section->used = true;

    return true;
}

//----------------------------------------
// Remove unused sections

void cache_strip_down_unused_sections(cache_t *this){
    for(unsigned section_index = 0; section_index < list_count(this->all.sections); section_index++){
        cache_section_item_t *head_section = NULL;
        list_at(this->all.sections, section_index, (void *)&head_section);

        if(head_section->used == false){
            list_remove_at(this->all.sections, section_index);
            section_index = (section_index == 0) ? section_index : section_index - 1;
            continue;
        }
    }
}

//-----------------------------------------
// Assing mem

static cache_ldm_mem_holder_t *find_ldm_mem_holder(cache_t *this, ldm_memory_t *ldm_mem){
    CHECK_NULL_ARGUMENT(this);
    CHECK_NULL_ARGUMENT(ldm_mem);

    cache_ldm_mem_holder_t *retVal = NULL;

    for(unsigned int i = 0; i < list_count(this->offsets); i++){
        cache_ldm_mem_holder_t *tmp = NULL;
        list_at(this->offsets, i, (void *)&tmp);

        if(strcmp(tmp->ldm_mem->memory_name, ldm_mem->memory_name) == 0){
            retVal = tmp;
            break;
        }
    }

    return retVal;
}

void cache_assing_section_into_memory(cache_t *this, char *section_name, ldm_memory_t *ldm_mem){
    CHECK_NULL_ARGUMENT(this);
    CHECK_NULL_ARGUMENT(section_name);
    CHECK_NULL_ARGUMENT(ldm_mem);

    cache_section_item_t *section_holder = find_section_by_name(this, section_name);
    cache_ldm_mem_holder_t *ldm_mem_holder = find_ldm_mem_holder(this, ldm_mem);

    //if missing it was optimized out
    if(section_holder == NULL){
        return;
    }

    //holder is missing, create one (this mem is seen first time)
    if(ldm_mem_holder == NULL){
        cache_ldm_mem_holder_t *new_holder = cache_ldm_mem_holder_new(ldm_mem);
        list_append(this->offsets, (void *)&new_holder);
        ldm_mem_holder = new_holder;
    }

    section_holder->assigned_memory = ldm_mem;
    section_holder->offset = ldm_mem_holder->next_offset;
    ldm_mem_holder->next_offset += section_holder->size;
}

//-----------------------------------------
// Export symbol relocation

void cache_calculate_real_exported_addresses(cache_t *this){
    CHECK_NULL_ARGUMENT(this);

    for(unsigned int index = 0; index < list_count(this->symbols.exported); index++){
        cache_symbol_item_t *head_symbol = NULL;
        list_at(this->symbols.exported, index, (void *)&head_symbol);

        if((head_symbol->symbol_type == SYMBOL_LINKER_SCRIPT_ABS) ||
           (head_symbol->symbol_type == SYMBOL_LINKER_SCRIPT_EVAL)){
           continue;
        }

        head_symbol->symbol->value += head_symbol->assigned_section->offset;
        head_symbol->symbol->value += head_symbol->assigned_section->assigned_memory->begin_addr;
    }
}

//-----------------------------------------
// Symbol evaluator

typedef struct{
    cache_t *cache;
    ldm_file_t *ldm;
    bool getting_text;
    char *result_text;
} context_t;

static context_t context = {NULL, NULL, false, NULL};

#define CHECK_CONTEXT() { \
    if(context.cache == NULL || context.ldm == NULL){  \
        error("Called callback without context set!"); \
    } \
}

static bool variable_resolve_callback(char *name, long long *result){
    CHECK_CONTEXT();

    if(context.getting_text == true){
        context.result_text = dynmem_strdup(name);
        result = 0;
        return true;
    }

    cache_symbol_item_t *found_symbol = NULL;

    if(!check_if_symbol_exist_by_name(name, context.cache->symbols.exported, &found_symbol)){
        return false;
    }

    if(found_symbol->symbol_type == SYMBOL_LINKER_SCRIPT_EVAL && found_symbol->evaluated == false){
        return false;
    }

    *result = found_symbol->symbol->value;
    return true;
}

static ldm_memory_t *find_mem_in_context(char *name){
    CHECK_CONTEXT();
    CHECK_NULL_ARGUMENT(name);

    for(unsigned i = 0; i < list_count(context.ldm->memories); i++){
        ldm_memory_t *mem = NULL;

        list_at(context.ldm->memories, i, (void *)&mem);

        if(strcmp(mem->memory_name, name) == 0){
            return mem;
        }
    }

    return NULL;
}

static void get_text_from_stack_arg(evaluator_t *this, list_t *args, int argpos){
    CHECK_NULL_ARGUMENT(this);
    CHECK_NULL_ARGUMENT(args);

    long long dump = 0;

    if(context.result_text != NULL || context.getting_text == true){
        error("Text variable is not null in context or getting_text already set!");
    }

    context.getting_text = true;

    if(!evaluator_convert(this, args, argpos, &dump)){
        error("Something went wrong when trying to get text from evaluator args.");
    }

    context.getting_text = false;
}

static void get_text_from_stack_cleanup(void){
    dynmem_free(context.result_text);
    context.result_text = false;
}

bool evaluator_function_mem_begin(evaluator_t *this, long long *result, list_t *args){
    CHECK_CONTEXT();
    CHECK_NULL_ARGUMENT(this);
    CHECK_NULL_ARGUMENT(result);
    CHECK_NULL_ARGUMENT(args);

    if(list_count(args) != 1){
        error("Received wrong count of arguments!");
    }

    get_text_from_stack_arg(this, args, 0);

    ldm_memory_t *mem = find_mem_in_context(context.result_text);

    if(mem == NULL){
        error_buffer_write(this->error_buffer, "Failed to find memory named '%s'.", context.result_text);
        get_text_from_stack_cleanup();
        return false;
    }

    *result = mem->begin_addr;
    get_text_from_stack_cleanup();
    return true;
}

bool evaluator_function_mem_size(evaluator_t *this, long long *result, list_t *args){
    CHECK_CONTEXT();
    CHECK_NULL_ARGUMENT(this);
    CHECK_NULL_ARGUMENT(result);
    CHECK_NULL_ARGUMENT(args);

    if(list_count(args) != 1){
        error("Received wrong count of arguments!");
    }

    get_text_from_stack_arg(this, args, 0);

    ldm_memory_t *mem = find_mem_in_context(context.result_text);

    if(mem == NULL){
        error_buffer_write(this->error_buffer, "Failed to find memory named '%s'.", context.result_text);
        get_text_from_stack_cleanup();
        return false;
    }

    *result = mem->size;
    get_text_from_stack_cleanup();
    return true;
}

void register_functions_to_evaluator(evaluator_t *this){
    CHECK_NULL_ARGUMENT(this);

    evaluate_append_function(this, "mem_begin", 1, evaluator_function_mem_begin);
    evaluate_append_function(this, "mem_size", 1, evaluator_function_mem_size);
}

bool cache_evaluate_labels(cache_t *this, ldm_file_t *ldm){
    CHECK_NULL_ARGUMENT(this);

    evaluator_t *evaluator = NULL;
    evaluate_new(&evaluator);

    evaluate_load_basic_math(evaluator);
    evaluate_register_variable_callback(evaluator, &variable_resolve_callback);
    register_functions_to_evaluator(evaluator);

    context.cache = this;
    context.ldm = ldm;
    for(unsigned int index = 0; index < list_count(this->symbols.exported); index++){
        cache_symbol_item_t *symbol = NULL;
        long long result = 0;

        list_at(this->symbols.exported, index, (void *)&symbol);

        if(symbol->symbol_type != SYMBOL_LINKER_SCRIPT_EVAL)
            continue;

        if(!evaluate_expression_string(evaluator, symbol->eval_string, &result)){
            ERROR_WRITE("Evaluation of symbol %s failed!", symbol->symbol->name);
            ERROR_WRITE("%s", evaluate_error(evaluator));
            evaluate_destroy(evaluator);
            context.cache = NULL;
            context.ldm = NULL;
            return false;
        }

        if(!can_fit_in(result, sizeof(isa_address_t))){
            ERROR_WRITE("Result of symbol %s evaluation overflow isa_address_t!", symbol->symbol->name);
            evaluate_destroy(evaluator);
            context.cache = NULL;
            context.ldm = NULL;
            return false;
        }

        symbol->symbol->value = (isa_address_t)result;
        symbol->evaluated = true;
    }
    context.cache = NULL;
    context.ldm = NULL;

    evaluate_destroy(evaluator);
    return true;
}

//-----------------------------------------
// Data relocation

bool cache_relocate_data(cache_t *this){
    CHECK_NULL_ARGUMENT(this);

    for(unsigned int section_index = 0; section_index < list_count(this->all.sections); section_index++){
        cache_section_item_t *section_holder = NULL;
        isa_address_t offset = 0;

        list_at(this->all.sections, section_index, (void *)&section_holder);

        offset += section_holder->offset;
        offset += section_holder->assigned_memory->begin_addr;

        for(unsigned int data_index = 0; data_index < list_count(section_holder->section->data_symbol_list); data_index++){
            obj_data_t *data_holder = NULL;
            isa_instruction_word_t output = 0;

            list_at(section_holder->section->data_symbol_list, data_index, (void *)&data_holder);

            data_holder->address += offset;

            if(data_holder->blob == true || data_holder->relocation == false){
                continue;
            }

            if(!platformlib_relocate_instruction(data_holder->payload.data_value, &output, offset)){
                ERROR_WRITE("Can't relocate instruction in section %s!", section_holder->section->section_name);
                ERROR_WRITE("%s", platformlib_error());
                return false;
            }

            data_holder->payload.data_value = output;
        }
    }
    return true;
}

//-----------------------------------------
// Data linking

bool cache_link_specials(cache_t *this){
    CHECK_NULL_ARGUMENT(this);

    for(unsigned int section_index = 0; section_index < list_count(this->all.sections); section_index++){
        cache_section_item_t *section_holder = NULL;
        list_at(this->all.sections, section_index, (void *)&section_holder);

        for(unsigned int data_index = 0; data_index < list_count(section_holder->section->data_symbol_list); data_index++){
            obj_data_t *data_holder = NULL;
            obj_symbol_t *import_symbol = NULL;
            cache_symbol_item_t *exported_counterpart = NULL;
            isa_instruction_word_t output = 0;

            list_at(section_holder->section->data_symbol_list, data_index, (void *)&data_holder);

            if(data_holder->blob == true || data_holder->special == false){
                continue;
            }

            //find name of that symbol
            for(unsigned int symbol_index = 0; symbol_index < list_count(section_holder->section->imported_symbol_list); symbol_index++){
                obj_symbol_t *head_symbol = NULL;
                list_at(section_holder->section->imported_symbol_list, symbol_index, (void *)&head_symbol);

                if(head_symbol->value == data_holder->special_value){
                    import_symbol = head_symbol;
                    break;
                }
            }

            //this is true error in linker/assembler and not in user input
            if(import_symbol == NULL){
                error("Data symbol have special value that is not found in imported symbols!");
            }

            //again this is true error as we already checked if all symbols exist -> error in linker not in user input
            if(!check_if_symbol_exist_by_name(import_symbol->name, this->symbols.exported, &exported_counterpart)){
                error("Counterpart symbol for special symbol doesn't found!");
            }

            if(!platformlib_retarget_instruction(data_holder->payload.data_value, &output, exported_counterpart->symbol->value)){
                ERROR_WRITE("Linkage error! Failed to retarget instruction referencing symbol %s in section %s!", import_symbol->name, section_holder->section->section_name);
                ERROR_WRITE("%s", platformlib_error());
                return false;
            }

            data_holder->payload.data_value = output;
        }
    }

    return true;
}

//-----------------------------------------
// Write into LDM

void cache_write_data_into_associated_ldm(cache_t *this){
    CHECK_NULL_ARGUMENT(this);

    for(unsigned section_index = 0; section_index < list_count(this->all.sections); section_index++){
        cache_section_item_t *section_holder = NULL;
        list_at(this->all.sections, section_index, (void *)&section_holder);

        for(unsigned int data_index = 0; data_index < list_count(section_holder->section->data_symbol_list); data_index++){
            obj_data_t *data_holder = NULL;

            list_at(section_holder->section->data_symbol_list, data_index, (void *)&data_holder);

            if(data_holder->blob == true){
                ldm_item_t *new_item = NULL;
                ldm_item_new(data_holder->address, data_holder->payload.blob_value, &new_item);
                ldm_item_into_mem(section_holder->assigned_memory, new_item);
            }
            else{
                array_t *memory_elements = NULL;
                platformlib_convert_isa_word_to_element(data_holder->payload.data_value, &memory_elements);

                for(unsigned int i = 0; i < array_get_size(memory_elements); i++){
                    isa_memory_element_t *memory_element = array_at(memory_elements, i);
                    ldm_item_t *new_item = NULL;
                    ldm_item_new(data_holder->address + i, *memory_element, &new_item);
                    ldm_item_into_mem(section_holder->assigned_memory, new_item);
                }

                array_destroy(memory_elements);
            }
        }
    }
}

//-----------------------------------------
// Debugging

void print_cache(cache_t *this){
    CHECK_NULL_ARGUMENT(this);

    printf("Symbol cache (%p):\r\n", this);

    printf("|- obj files: %d\r\n", list_count(this->files.obj_files));
    printf("|- sl files: %d\r\n", list_count(this->files.sl_files));

    printf("|- sections: %d\r\n", list_count(this->all.sections));

    if(list_count(this->all.sections) > 0){
        for(unsigned section_index = 0; section_index < list_count(this->all.sections); section_index++){
            cache_section_item_t *head_section = NULL;
            list_at(this->all.sections, section_index, (void *)&head_section);

            char *size_string = platformlib_write_isa_address(head_section->size);
            char *offset_string = platformlib_write_isa_address(head_section->offset);
            char c1 = list_count(this->all.sections) == (section_index + 1) ? '\'' : '|';
            char cn = list_count(this->all.sections) == (section_index + 1) ? ' ' : '|';

            printf("|  %c- section: '%s'\r\n", c1, head_section->section->section_name);
            printf("|  %c  |- memory: '%s'\r\n", cn, head_section->assigned_memory == NULL ? "NULL" : head_section->assigned_memory->memory_name);
            printf("|  %c  |- used: '%s'\r\n", cn, head_section->used == true ? "true" : "false");
            printf("|  %c  |- size: '%s'\r\n", cn, size_string);
            printf("|  %c  '- offset: '%s'\r\n", cn, offset_string);

            dynmem_free(size_string);
            dynmem_free(offset_string);
        }
    }

    printf("|- exported: %d\r\n", list_count(this->symbols.exported));

    if(list_count(this->symbols.exported) > 0){
        for(unsigned export_index = 0; export_index < list_count(this->symbols.exported); export_index++){
            cache_symbol_item_t *head_symbol = NULL;
            list_at(this->symbols.exported, export_index, (void *)&head_symbol);

            char * tmp = platformlib_write_isa_address(head_symbol->symbol->value);
            char c1 = list_count(this->symbols.exported) == (export_index + 1) ? '\'' : '|';

            if(head_symbol->symbol_type == SYMBOL_EXPORT){
                printf("|  %c- export %s @ %s (%s)\r\n", c1,
                    head_symbol->symbol->name,
                    head_symbol->assigned_section->section->section_name,
                    tmp
                );
            }

            if(head_symbol->symbol_type == SYMBOL_LINKER_SCRIPT_ABS){
                printf("|  %c- lds abs %s (%s)\r\n", c1,
                    head_symbol->symbol->name,
                    tmp
                );
            }

            if(head_symbol->symbol_type == SYMBOL_LINKER_SCRIPT_EVAL){
                printf("|  %c- lds eval %s (%s) [%s]\r\n", c1,
                    head_symbol->symbol->name,
                    tmp,
                    string_get(head_symbol->eval_string)
                );
            }

            dynmem_free(tmp);
        }
    }

    printf("'- imported: %d\r\n", list_count(this->symbols.imported));

    if(list_count(this->symbols.imported) > 0){
        for(unsigned import_index = 0; import_index < list_count(this->symbols.imported); import_index++){
            cache_symbol_item_t *head_symbol = NULL;
            list_at(this->symbols.imported, import_index, (void *)&head_symbol);

            printf("   %c- %s @ %s\r\n",
                list_count(this->symbols.imported) == (import_index + 1) ? '\'' : '|',
                head_symbol->symbol->name,
                head_symbol->assigned_section->section->section_name
            );
        }
    }
}
