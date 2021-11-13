#include "pass2.h"

#include "symbol_table.h"
#include "section_table.h"
#include "common.h"
#include "pass_item.h"

#include <utillib/core.h>
#include <utillib/utils.h>

#include <string.h>
#include <stdbool.h>
#include <stdlib.h>

static bool assign_values_to_exported_imported_symbols(void);
static bool assemble_instructions(void);

static symbol_t *last_found_symbol = NULL;

bool pass2(void){
    if(!assign_values_to_exported_imported_symbols()){
        ERROR_WRITE("Failed to assign addresses to symbols in pass 2!");
        return false;
    }

    if(!assemble_instructions()){
        ERROR_WRITE("Failed to assemble instructions in pass 2!");
        return false;
    }

    return true;
}

static symbol_t *find_counterpart_to_exported_symbol(symbol_t *exported_symbol, list_t *symbol_list){
    CHECK_NULL_ARGUMENT(exported_symbol);
    CHECK_NULL_ARGUMENT(symbol_list);

    symbol_t *retVal = NULL;

    for(unsigned int i = 0; i < list_count(symbol_list); i++){
        symbol_t *checked_symbol = NULL;
        list_at(symbol_list, i, (void *)&checked_symbol);

        if((strcmp(exported_symbol->name, checked_symbol->name) == 0) &&
           (strcmp(exported_symbol->section->section_name, checked_symbol->section->section_name) == 0) &&
           (checked_symbol->type != SYMBOL_TYPE_EXPORT) && (checked_symbol->type != SYMBOL_TYPE_IMPORT))
        {
            retVal = checked_symbol;
            break;
        }
    }

    return retVal;
}

static bool assign_values_to_exported_imported_symbols(void){
    list_t *sections = section_table_get_all();
    list_t *symbols = symbol_table_get_all();

    for(unsigned int i = 0; i < list_count(sections); i++){
        section_t *head = NULL;
        list_at(sections, i, (void *)&head);

        head->last_location_counter = 0;
    }

    for(unsigned int i = 0; i < list_count(symbols); i++){
        symbol_t *head = NULL;
        list_at(symbols, i, (void *)&head);

        if(head->type == SYMBOL_TYPE_IMPORT){
            head->value = head->section->last_location_counter++;
        }
        else if(head->type == SYMBOL_TYPE_EXPORT){
            symbol_t *counterpart = find_counterpart_to_exported_symbol(head, symbols);

            if(counterpart == NULL){
                ERROR_WRITE("Definition of exported symbol %s from %s+%ld not found!", head->name, head->parent->origin.filename, head->parent->origin.line_number);
                error_buffer_append_if_defined(head->parent);
                return false;
            }

            head->value = counterpart->value;
        }
        else{
            //nothing to do here
        }
    }

    return true;
}

static bool find_symbol_for_instruction_assemble_callback(char *label, void *section, isa_address_t *result){
    CHECK_NULL_ARGUMENT(label);
    CHECK_NULL_ARGUMENT(section);
    CHECK_NULL_ARGUMENT(result);

    list_t *symbols = symbol_table_get_all();

    if(last_found_symbol != NULL){
        error("Last found symbol isn't null! Instructions can't have multiple labels!");
    }

    for(unsigned int i = 0; i < list_count(symbols); i++){
        symbol_t *head = NULL;
        list_at(symbols, i, (void *)&head);

        if(
            (strcmp(label, head->name) == 0) &&
            (strcmp( ((section_t *)section)->section_name, head->section->section_name) == 0) &&
            (head->type != SYMBOL_TYPE_EXPORT)
        ){
            if(head->type == SYMBOL_TYPE_IMPORT){
                *result = 0;
            }
            else{
                *result = head->value;
            }
            last_found_symbol = head;
            return true;
        }
    }

    return false;
}

static bool assemble_instructions(void){
    list_t *items = pass_item_db_get_all();

    for(unsigned int i = 0; i < list_count(items); i++){
        pass_item_t *head = NULL;
        list_at(items, i, (void *)&head);

        if(head->type == ITEM_BLOB){
            if(list_count(head->args) != 1){
                error("Internal error, blob pass_item need exactly one arg!");
            }

            preprocessed_token_t *arg = NULL;
            long long num = 0;
            list_at(head->args, 0, (void *)&arg);

            if(!is_number(arg->token)){
                ERROR_WRITE("Error! Expected number at %s+%ld!", arg->origin.filename, arg->origin.line_number);
                error_buffer_append_if_defined(arg);
                return false;
            }

            if(!str_to_num(arg->token, &num)){
                error("Failed to convert number but is_number return OK!");
            }

            if(!can_fit_in(num, sizeof(isa_memory_element_t))){
                ERROR_WRITE("Error! Argument at %s+%ld is out of range!", arg->origin.filename, arg->origin.line_number);
                error_buffer_append_if_defined(arg);
                return false;
            }

            head->value.blob = (isa_memory_element_t)num;
        }
        else if(head->type == ITEM_INST){
            int argc = (int)list_count(head->args);
            char **argv = (char **)dynmem_calloc(argc, sizeof(char *));

            for(unsigned int j = 0; j < list_count(head->args); j++){
                preprocessed_token_t *tok = NULL;
                list_at(head->args, j, (void *)&tok);
                argv[j] = tok->token;
            }

            last_found_symbol = NULL;

            if(!platformlib_assemble_instruction(argv, argc, &find_symbol_for_instruction_assemble_callback, (void *)head->section, &head->value.instr)){
                preprocessed_token_t *instruction = NULL;
                list_at(head->args, 0, (void *)&instruction);

                ERROR_WRITE("%s", platformlib_error());
                ERROR_WRITE("Error! Instruction at %s+%ld cannot be correctly assembled!", instruction->origin.filename, instruction->origin.line_number);
                error_buffer_append_if_defined(instruction);

                dynmem_free(argv);
                return false;
            }

            dynmem_free(argv);

            if(last_found_symbol != NULL){
                if(last_found_symbol->type == SYMBOL_TYPE_IMPORT){
                    head->special_value = last_found_symbol->value;
                    head->special = true;
                }

                if(last_found_symbol->type == SYMBOL_TYPE_RELOCATION){
                    head->relocation = true;
                }
            }
        }
        else{
            error("Unknown pass item type!");
        }
    }

    return true;
}
