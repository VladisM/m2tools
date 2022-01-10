#include "datatypes.h"

#include "../../src/platformlib_common.h"
#include "../../src/platformlib_private.h"

#include <utillib/core.h>

#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stdlib.h>

#define PRIisa_iw "0x%08"PRIx32
#define SCNisa_iw "0x%08"SCNx32
#define PRIisa_addr "0x%08"PRIx16
#define SCNisa_addr "0x%08"SCNx16
#define PRIisa_me "0x%08"PRIx8
#define SCNisa_me "0x%08"SCNx8

bool platformlib_read_isa_address(char *s, isa_address_t *value){
    CHECK_NULL_ARGUMENT(value);
    CHECK_NULL_ARGUMENT(s);

    if(sscanf(s, SCNisa_addr, value) != 1){
        *value = 0;
        return false;
    }
    else{
        return true;
    }
}

bool platformlib_read_isa_instruction_word(char *s, isa_instruction_word_t *value){
    CHECK_NULL_ARGUMENT(value);
    CHECK_NULL_ARGUMENT(s);

    if(sscanf(s, SCNisa_iw, value) != 1){
        *value = 0;
        return false;
    }
    else{
        return true;
    }
}

bool platformlib_read_isa_memory_element(char *s, isa_memory_element_t *value){
    CHECK_NULL_ARGUMENT(value);
    CHECK_NULL_ARGUMENT(s);

    if(sscanf(s, SCNisa_me, value) != 1){
        *value = 0;
        return false;
    }
    else{
        return true;
    }
}

char *platformlib_write_isa_address(isa_address_t value){
    int length = snprintf(NULL, 0, PRIisa_addr, value);
    char *tmp = dynmem_malloc(length + 1);
    sprintf(tmp, PRIisa_addr, value);
    return tmp;
}

char *platformlib_write_isa_instruction_word(isa_instruction_word_t value){
    int length = snprintf(NULL, 0, PRIisa_iw, value);
    char *tmp = dynmem_malloc(length + 1);
    sprintf(tmp, PRIisa_iw, value);
    return tmp;
}

char *platformlib_write_isa_memory_element(isa_memory_element_t value){
    int length = snprintf(NULL, 0, PRIisa_me, value);
    char *tmp = dynmem_malloc(length + 1);
    sprintf(tmp, PRIisa_me, value);
    return tmp;
}

void platformlib_convert_isa_word_to_element(isa_instruction_word_t word, array_t **output){
    CHECK_NULL_ARGUMENT(output);
    CHECK_NOT_NULL_ARGUMENT(*output);

    instruction_signature_t *signature = platformlib_get_instruction_signature_1(word);

    if(signature == NULL){
        error("Converting isa_instruction_word_t into elements but it is not instruction!");
    }

    array_init(output, sizeof(isa_memory_element_t), signature->size);

    unsigned pos = 0;
    for(isa_address_t i = signature->size; i > 0; i--){
        isa_memory_element_t data = 0;
        data = (word >> ((i - 1) * 8)) & 0xFF;
        array_set(*output, pos++, &data);
    }
}
