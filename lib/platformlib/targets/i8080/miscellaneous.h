/**
 * @file miscellaneous.h
 * @brief Collection of different fuctions.
 */

#ifndef MISCELLANEOUS_H_included
#define MISCELLANEOUS_H_included

#include "datatypes.h"
#include "instructions_description.h"

/**
 * @brief Convert translated instruction into mnemotechnic
 * string that is understood by assembler.
 * @note Returning string can't be dynamically allocated.
 * @note String have to contain only instruction, args have to be stripped.
 * @param word Word to be translated.
 * @param opcode Pointer to result.
 * @return true
 * @return false
 */
bool platformlib_get_instruction_opcode(isa_instruction_word_t word, char **opcode);

#endif
