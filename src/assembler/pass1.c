#include "pass1.h"

#include "symbol_table.h"
#include "section_table.h"
#include "common.h"
#include "pass_item.h"

#include <utillib/core.h>
#include <utillib/utils.h>

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#define UNUSED(x) (void)x

typedef enum{
    PSEUDO_ORG,
    PSEUDO_CONS,
    PSEUDO_DAT,
    PSEUDO_DS,
    PSEUDO_EXPORT,
    PSEUDO_IMPORT,
    PSEUDO_SECTION
} pseudo_type_t;

static bool save_symbol(char *name, isa_address_t value, symbol_type_t type, preprocessed_token_t *parent);
static void increment_location_counter(isa_address_t n);
static void set_location_counter(isa_address_t n);
static isa_address_t get_location_counter(void);
static unsigned int get_token_count_at_line(queue_t *preprocessor_output, unsigned int postion);
static bool is_pseudo(preprocessed_token_t *token);
static bool eval_pseudo(queue_t *preprocessor_output, unsigned int *position);
static bool is_label(preprocessed_token_t *token);
static bool eval_label(queue_t *preprocessor_output, unsigned int *position);
static bool check_openned_section(void);
static bool is_instru(preprocessed_token_t *token);
static bool eval_instru(queue_t *preprocessor_output, unsigned int *position);

bool pass1(queue_t *preprocessor_output){
    unsigned int position = 0;

    while(position < list_count(preprocessor_output)){
        bool retVal = false;

        preprocessed_token_t *head = NULL;
        list_at(preprocessor_output, position, (void *)&head);


        if(is_pseudo(head)){
            retVal = eval_pseudo(preprocessor_output, &position);
        }
        else if(is_instru(head)){
            retVal = eval_instru(preprocessor_output, &position);
        }
        else if(is_label(head)){
            retVal = eval_label(preprocessor_output, &position);
        }
        else{
            ERROR_WRITE("Token '%s' from %s+%ld is not recognized as valid instruction, label or pseudoinstruction!", head->token, head->origin.filename, head->origin.line_number);
            error_buffer_append_if_defined(head);
            return false;
        }

        if(retVal == false){
            ERROR_WRITE("Failed to execute token from %s+%ld!", head->origin.filename, head->origin.line_number);
            error_buffer_append_if_defined(head);
            return false;
        }

        position++;
    }

    return true;
}

static bool save_symbol(char *name, isa_address_t value, symbol_type_t type, preprocessed_token_t *parent){
    CHECK_NULL_ARGUMENT(name);
    CHECK_NULL_ARGUMENT(parent);

    section_t *opened_section = section_table_get_actual_section();

    unsigned long len = strlen(name);

    if(name[len - 1] == ':'){
        name[len - 1] = '\0';
    }

    if(!check_openned_section()){
        ERROR_WRITE("Cannot resolve symbol due to lack of opened section at %s+%ld! Create a section first.", parent->origin.filename, parent->origin.line_number);
        error_buffer_append_if_defined(parent);
        return false;
    }

    if(!symbol_table_append(name, value, type, parent, opened_section)){
        ERROR_WRITE("Failed to pass symbol %s into symbol table.", name);
        return false;
    }

    return true;
}

static void increment_location_counter(isa_address_t n){
    section_t *opened_section = section_table_get_actual_section();

    if(opened_section == NULL){
        error("No section opened but wanted to update location counter!");
    }

    opened_section->last_location_counter += n;
}

static void set_location_counter(isa_address_t n){
    section_t *opened_section = section_table_get_actual_section();

    if(opened_section == NULL){
        error("No section opened but wanted to update location counter!");
    }

    opened_section->last_location_counter = n;
}

static isa_address_t get_location_counter(void){
    section_t *opened_section = section_table_get_actual_section();

    if(opened_section == NULL){
        error("No section opened but wanted to get location counter!");
    }

    return opened_section->last_location_counter;
}

static unsigned int get_token_count_at_line(queue_t *preprocessor_output, unsigned int position){
    unsigned int result = 1;
    unsigned int line = 0;

    preprocessed_token_t *head = NULL;
    list_at(preprocessor_output, position++, (void *)&head);

    line = head->origin.line_number;

    while(position < list_count(preprocessor_output)){
        list_at(preprocessor_output, position++, (void *)&head);

        if(head->origin.line_number == line){
            result++;
            continue;
        }
        else{
            break;
        }
    }

    return result - 1;
}

static bool check_openned_section(void){
    section_t *section = section_table_get_actual_section();

    if(section == NULL){
        ERROR_WRITE("No section specified!");
        return false;
    }
    else{
        return true;
    }
}

//------------------------------------------------------------------------------
// Pseudo instructions implementation

static bool _strcmp(char *x, char *y){
    CHECK_NULL_ARGUMENT(x);
    CHECK_NULL_ARGUMENT(y);
    if(strcmp(x, y) == 0) return true;
    else return false;
}

static bool is_org(char *x){
    return _strcmp(x, ".ORG");
}

static bool is_cons(char *x){
    return _strcmp(x, ".CONS");
}

static bool is_dat(char *x){
    return _strcmp(x, ".DAT");
}

static bool is_ds(char *x){
    return _strcmp(x, ".DS");
}

static bool is_export(char *x){
    return _strcmp(x, ".EXPORT");
}

static bool is_import(char *x){
    return _strcmp(x, ".IMPORT");
}

static bool is_section(char *x){
    return _strcmp(x, ".SECTION");
}

static bool is_pseudo(preprocessed_token_t *token){
    CHECK_NULL_ARGUMENT(token);

    char *code = token->token;
    bool retVal = false;

    if(is_org(code))
        retVal = true;

    if(is_cons(code))
        retVal = true;

    if(is_dat(code))
        retVal = true;

    if(is_ds(code))
        retVal = true;

    if(is_export(code))
        retVal = true;

    if(is_import(code))
        retVal = true;

    if(is_section(code))
        retVal = true;

    return retVal;
}

static pseudo_type_t get_pseudo_type(preprocessed_token_t *token){
    CHECK_NULL_ARGUMENT(token);

    char *code = token->token;
    pseudo_type_t retVal = PSEUDO_ORG;

    if(is_pseudo(token) == false){
        error("Trying to get type of something that isn't pseudoinstru!");
    }

    if(is_org(code)){
        retVal = PSEUDO_ORG;
    }
    else if(is_cons(code)){
        retVal = PSEUDO_CONS;
    }
    else if(is_dat(code)){
        retVal = PSEUDO_DAT;
    }
    else if(is_ds(code)){
        retVal = PSEUDO_DS;
    }
    else if(is_export(code)){
        retVal = PSEUDO_EXPORT;
    }
    else if(is_import(code)){
        retVal = PSEUDO_IMPORT;
    }
    else if(is_section(code)){
        retVal = PSEUDO_SECTION;
    }
    else{
        error("Internal error! Wrong condition!");
    }

    return retVal;
}

static unsigned int get_pseudo_argc(preprocessed_token_t *token){
    CHECK_NULL_ARGUMENT(token);

    unsigned int retVal = 0;

    if(is_pseudo(token) == false){
        error("Trying to get type of something that isn't pseudoinstru!");
    }

    switch(get_pseudo_type(token)){
        case PSEUDO_ORG:     retVal = 1; break;
        case PSEUDO_CONS:    retVal = 2; break;
        case PSEUDO_DAT:     retVal = 1; break;
        case PSEUDO_DS:      retVal = 1; break;
        case PSEUDO_EXPORT:  retVal = 1; break;
        case PSEUDO_IMPORT:  retVal = 1; break;
        case PSEUDO_SECTION: retVal = 1; break;
        default:
            error("Internal error! Wrong switch!");
            break;
    }

    return retVal;
}

static void append_blob(preprocessed_token_t *blob_token){
    pass_item_db_create_item(get_location_counter(), section_table_get_actual_section(), ITEM_BLOB);
    pass_item_db_append_arg(pass_item_db_get_last(), blob_token);
}

static bool eval_pseudo(queue_t *preprocessor_output, unsigned int *position){
    CHECK_NULL_ARGUMENT(preprocessor_output);
    CHECK_NULL_ARGUMENT(position);

    bool retVal = false;
    preprocessed_token_t *head = NULL;
    list_at(preprocessor_output, *position, (void *)&head);

    unsigned int argc_requested = get_pseudo_argc(head);

    if(argc_requested + *position + 1 > list_count(preprocessor_output)){
        ERROR_WRITE("Unexpected end of input when executing %s from %s+%ld!", head->token, head->origin.filename, head->origin.line_number);
        return false;
    }

    unsigned int argc_given = get_token_count_at_line(preprocessor_output, *position);

    if(argc_requested != argc_given){
        ERROR_WRITE("Pseudoinstruction %s from %s+%ld requesting %u arguments. %u given!", head->token, head->origin.filename, head->origin.line_number, argc_requested, argc_given);
        return false;
    }

    switch(get_pseudo_type(head)){
        case PSEUDO_ORG:
            {
                preprocessed_token_t *arg = NULL;
                isa_address_t address = 0;

                *position = *position + 1;
                list_at(preprocessor_output, *position, (void *)&arg);

                if(!check_openned_section()){
                    ERROR_WRITE("Cannot execute %s due to lack of opened section at %s+%ld! Create a section first.", head->token, head->origin.filename, head->origin.line_number);
                    error_buffer_append_if_defined(head);
                    retVal = false;
                    break;
                }

                if(!platformlib_read_isa_address(arg->token, &address)){
                    ERROR_WRITE("Cannot decode address in argument of pseudoinstruction at %s%+ld!", arg->origin.filename, arg->origin.line_number);
                    error_buffer_append_if_defined(arg);
                    retVal = false;
                    break;
                }

                set_location_counter(address);
                retVal = true;
            }
            break;
        case PSEUDO_CONS:
            {
                preprocessed_token_t *arg_1 = NULL;
                preprocessed_token_t *arg_2 = NULL;
                isa_address_t address = 0;

                *position = *position + 1;
                list_at(preprocessor_output, *position, (void *)&arg_1);

                *position = *position + 1;
                list_at(preprocessor_output, *position, (void *)&arg_2);

                if(!platformlib_read_isa_address(arg_2->token, &address)){
                    ERROR_WRITE("Cannot decode address in argument of pseudoinstruction at %s%+ld!", arg_2->origin.filename, arg_2->origin.line_number);
                    error_buffer_append_if_defined(arg_2);
                    retVal = false;
                    break;
                }

                retVal = save_symbol(arg_1->token, address, SYMBOL_TYPE_ABSOLUTE, arg_1);
            }
            break;
        case PSEUDO_DAT:
            {
                preprocessed_token_t *arg = NULL;
                *position = *position + 1;
                list_at(preprocessor_output, *position, (void *)&arg);
                append_blob(arg);
                increment_location_counter(1);
                retVal = true;
            }
            break;
        case PSEUDO_DS:
            {
                preprocessed_token_t *arg = NULL;
                long long num = 0;
                *position = *position + 1;
                list_at(preprocessor_output, *position, (void *)&arg);

                if(!is_number(arg->token)){
                    ERROR_WRITE("Error! Expected number at %s+%ld!", arg->defined.filename, arg->defined.line_number);
                    error_buffer_append_if_defined(arg);
                    retVal = false;
                    break;
                }

                if(!str_to_num(arg->token, &num)){
                    error("Failed to convert number!");
                }

                if(!can_fit_in(num, sizeof(isa_address_t))){
                    ERROR_WRITE("Error! Argument at %s+%ld is out of range!", arg->defined.filename, arg->defined.line_number);
                    error_buffer_append_if_defined(arg);
                    retVal = false;
                    break;
                }

                if(!check_openned_section()){
                    ERROR_WRITE("Cannot execute %s due to lack of opened section at %s+%ld! Create a section first.", head->token, head->origin.filename, head->origin.line_number);
                    error_buffer_append_if_defined(head);
                    retVal = false;
                    break;
                }

                increment_location_counter((isa_address_t)num);
                retVal = true;
            }
            break;
        case PSEUDO_EXPORT:
            {
                preprocessed_token_t *arg = NULL;
                *position = *position + 1;
                list_at(preprocessor_output, *position, (void *)&arg);

                retVal = save_symbol(arg->token, 0, SYMBOL_TYPE_EXPORT, arg);
            }
            break;
        case PSEUDO_IMPORT:
            {
                preprocessed_token_t *arg = NULL;
                *position = *position + 1;
                list_at(preprocessor_output, *position, (void *)&arg);

                retVal = save_symbol(arg->token, 0, SYMBOL_TYPE_IMPORT, arg);
            }
            break;
        case PSEUDO_SECTION:
            {
                preprocessed_token_t *arg = NULL;
                *position = *position + 1;
                list_at(preprocessor_output, *position, (void *)&arg);

                section_table_append_or_switch(arg->token);
                retVal = true;
            }
            break;
        default:
            error("Internal error! Wrong switch!");
            break;
    }

    return retVal;
}

//------------------------------------------------------------------------------
// Labels implementation

static bool is_label(preprocessed_token_t *token){
    CHECK_NULL_ARGUMENT(token);

    char *label = token->token;
    unsigned long len = strlen(label);

    if(label[len - 1] == ':'){
        return true;
    }
    else{
        return false;
    }
}

static bool eval_label(queue_t *preprocessor_output, unsigned int *position){
    CHECK_NULL_ARGUMENT(preprocessor_output);
    CHECK_NULL_ARGUMENT(position);

    preprocessed_token_t *head;
    list_at(preprocessor_output, *position, (void *)&head);

    if(!check_openned_section()){
        ERROR_WRITE("Cannot process a label ue to lack of opened section at %s+%ld! Create a section first.", head->origin.filename, head->origin.line_number);
        error_buffer_append_if_defined(head);
        return false;
    }

    isa_address_t current_counter = get_location_counter();

    if(!save_symbol(head->token, current_counter, SYMBOL_TYPE_RELOCATION, head)){
        ERROR_WRITE("Cannot process a label at %s+%ld!", head->origin.filename, head->origin.line_number);
        error_buffer_append_if_defined(head);
        return false;
    }

    return true;
}

//------------------------------------------------------------------------------
// Instructions implementation

static bool is_instru(preprocessed_token_t *token){
    CHECK_NULL_ARGUMENT(token);
    return platformlib_is_instruction_opcode(token->token);
}

static instruction_signature_t * get_instru_signature(preprocessed_token_t *token){
    CHECK_NULL_ARGUMENT(token);

    instruction_signature_t *signature = platformlib_get_instruction_signature(token->token);

    if(signature == NULL){
        error("Tryting to get type of something that isn't instruction!");
    }

    return signature;
}

static unsigned int get_instru_argc(preprocessed_token_t *token){
    CHECK_NULL_ARGUMENT(token);
    instruction_signature_t *signature = get_instru_signature(token);
    return signature->argc;
}

static isa_address_t get_instru_size(preprocessed_token_t *token){
    CHECK_NULL_ARGUMENT(token);
    instruction_signature_t *signature = get_instru_signature(token);
    return signature->size;
}

static bool eval_instru(queue_t *preprocessor_output, unsigned int *position){
    CHECK_NULL_ARGUMENT(preprocessor_output);
    CHECK_NULL_ARGUMENT(position);

    preprocessed_token_t *head = NULL;
    list_at(preprocessor_output, *position, (void *)&head);

    unsigned int argc_requested = get_instru_argc(head);

    if(argc_requested + *position + 1 > list_count(preprocessor_output)){
        ERROR_WRITE("Unexpected end of input when processing %s from %s+%ld!", head->token, head->origin.filename, head->origin.line_number);
        return false;
    }

    unsigned int argc_given = get_token_count_at_line(preprocessor_output, *position);

    if(argc_requested != argc_given){
        ERROR_WRITE("Instruction %s at %s+%ld requesting %u arguments. %u given!", head->token, head->origin.filename, head->origin.line_number, argc_requested, argc_given);
        error_buffer_append_if_defined(head);
        return false;
    }

    pass_item_db_create_item(get_location_counter(), section_table_get_actual_section(), ITEM_INST);
    pass_item_db_append_arg(pass_item_db_get_last(), head);

    for(unsigned int argc_processed = 0; argc_processed < argc_requested; argc_processed++){
        *position = *position + 1;
        preprocessed_token_t *arg = NULL;
        list_at(preprocessor_output, *position, (void *)&arg);
        pass_item_db_append_arg(pass_item_db_get_last(), arg);
    }

    increment_location_counter(get_instru_size(head));
    return true;
}
