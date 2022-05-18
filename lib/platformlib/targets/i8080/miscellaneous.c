#include "miscellaneous.h"

#include "../../src/platformlib_common.h"
#include "../../src/platformlib_private.h"

#include "instructions_description.h"
#include "opcode_decode.h"

#include <stddef.h>
#include <string.h>

bool platformlib_get_instruction_opcode(isa_instruction_word_t word, char **opcode){
    CHECK_NULL_ARGUMENT(opcode);
    CHECK_NOT_NULL_ARGUMENT(*opcode);

    for(int i = 0; platformlib_instruction_signatures[i].opcode != NULL; i++){
        isa_instruction_word_t head = platformlib_instruction_signatures[i].instruction_code;

        head = SHIFT(head, (platformlib_instruction_signatures[i].size - 1) * 8);

        if((word & head) == head){
            *opcode = platformlib_instruction_signatures[i].opcode;
            return true;
        }
    }

    return false;
}
