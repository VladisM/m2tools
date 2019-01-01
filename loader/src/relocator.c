#include "relocator.h"

#include <ldm.h>
#include <isa.h>

#include "error.h"

#include <stdlib.h>

int relocate_buffer(ldm_buffer_item_t *b, unsigned int base_address){

    for(;b != NULL ; b=b->next){
        tInstruction instruction;

        //relocate instruction adress
        b->address += base_address;
        instruction.word = b->value;

        //if I don't have retarget instruction argument I don't do so
        if(b->relocation == 0){
            continue;
        }

        if(retarget_instruction(&instruction, base_address) < 0){
            SET_ERROR(ERR_RELOCATION_FAIL);
            return -1;
        }

        //put result back to buffer item
        b->value = instruction.word;
    }
    return 0;
}
