/**
 * @file assemble.h
 * @brief Let's put instruction together!
 *
 * This module implement key part of assembler itself, there is one function
 * called target-assemble_instruction() that have to take all arguments of
 * instruction and put it into isa_instruction_word_t.
 */

#ifndef ASSEMBLE_H_included
#define ASSEMBLE_H_included

#include "datatypes.h"

#include <stdbool.h>

/**
 * @brief Assemble instruction.
 * @param args Strings from tokenized input.
 * @param argc Count of element of array args.
 * @param find_symbol_callback This function can be called to resolve symbol into address.
 * @param section Put this into call back, otherwise do not touch it!
 * @param result Pointer where this function should store its output.
 * @return true Return true if everything was parsed correctly.
 * @return false Return false if there was something wrong.
 */
bool platformlib_assemble_instruction(
    char **args,
    int argc,
    bool (*find_symbol_callback)(char *label, void *section, isa_address_t *result),
    void *section,
    isa_instruction_word_t *result);

/**
 * @brief Relocate instruction that have relative argument and relocation flag set.
 * @note Used in linker.
 * @param input Input instruction on old location.
 * @param output Relocated output to new location.
 * @param offset Offset to previous position.
 * @return true Return true if everything was ok.
 * @return false Return false on failure.
 */
bool platformlib_relocate_instruction(
    isa_instruction_word_t input,
    isa_instruction_word_t *output,
    isa_address_t offset);

/**
 * @brief Retarget instruction that have symbol address in argument.
 * @note Used in linker.
 * @param input Input instruction pointing to old locations.
 * @param output Relocated output to new targer.
 * @param target New target address.
 * @return true Return tru if everything was ok.
 * @return false Return false on failure.
 */
bool platformlib_retarget_instruction(
    isa_instruction_word_t input,
    isa_instruction_word_t *output,
    isa_address_t target);

#endif
