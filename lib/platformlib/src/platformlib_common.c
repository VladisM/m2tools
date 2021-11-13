#include "platformlib_common.h"
#include "platformlib_private.h"

#include <stddef.h>
#include <stdbool.h>
#include <string.h>

#include <utillib/core.h>
#include <utillib/utils.h>

error_t *platformlib_error_buffer = NULL;
bool platformlib_initialized = false;

void platformlib_init(void){
    if(platformlib_initialized == true){
        return;
    }

    error_buffer_init(&platformlib_error_buffer);

    platformlib_initialized = true;
}

void platformlib_deinit(void){
    if(platformlib_initialized == false){
        return;
    }

    error_buffer_destroy(platformlib_error_buffer);

    platformlib_error_buffer = NULL;
    platformlib_initialized = false;
}

char *platformlib_error(void){
    CHECK_INITIALIZED();
    return error_buffer_get(platformlib_error_buffer);
}

bool platformlib_is_instruction_opcode(char *opcode){
    CHECK_NULL_ARGUMENT(opcode);

    bool found = false;

    for(unsigned i = 0; platformlib_instruction_signatures[i].opcode != NULL; i++){
        if(strcmp(opcode, platformlib_instruction_signatures[i].opcode) == 0){
            found = true;
            break;
        }
    }

    return found;
}

instruction_signature_t *platformlib_get_instruction_signature(char *opcode){
    CHECK_NULL_ARGUMENT(opcode);

    instruction_signature_t *signature = NULL;

    for(unsigned i = 0; platformlib_instruction_signatures[i].opcode != NULL; i++){
        if(strcmp(opcode, platformlib_instruction_signatures[i].opcode) == 0){
            signature = &(platformlib_instruction_signatures[i]);
            break;
        }
    }

    return signature;
}

instruction_signature_t *platformlib_get_instruction_signature_1(isa_instruction_word_t word){
    char *opcode = NULL;

    if(!platformlib_get_instruction_opcode(word, &opcode)){
        return NULL;
    }

    return platformlib_get_instruction_signature(opcode);
}
