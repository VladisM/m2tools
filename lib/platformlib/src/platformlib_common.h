#ifndef PLATFORMLIB_COMMON_H_included
#define PLATFORMLIB_COMMON_H_included

#include "../targets/platformlib_target_specific.h"

#include <stdbool.h>

void platformlib_init(void);
void platformlib_deinit(void);
char *platformlib_error(void);

bool platformlib_is_instruction_opcode(char *opcode);
instruction_signature_t *platformlib_get_instruction_signature(char *opcode);
instruction_signature_t *platformlib_get_instruction_signature_1(isa_instruction_word_t word);

#endif
