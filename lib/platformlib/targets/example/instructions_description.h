/**
 * @file instructions_description.h
 * @brief Description of instructions that assembler need to know to correctly
 * prepare them.
 *
 * This module consist from definition of instruction_signature_t that is used
 * to tell assembler what is valid instruction opcode, what is not, how many
 * arguments each opcode require and how much space does each instruction need
 * in term of isa_memory_element_t count.
 */

#ifndef INSTRUCTION_DESCRIPTION_H_included
#define INSTRUCTION_DESCRIPTION_H_included

#include "datatypes.h"

/**
 * @brief Structure to hold informations about instruction.
 */
typedef struct{
    char *opcode;           /**< @b String representation of mnemotechnic. */
    unsigned int argc;      /**< @b Required arguments count. */
    isa_address_t size;     /**< @b Size of instruction in count of isa_memory_element_t. */
} instruction_signature_t;

/**
 * @brief List of instruction_signature_t instancies filled with values.
 * @note Last item of list have to be {NULL, 0, 0}!
 * All tools will go thru this array to get neccesary informations.
 */
extern instruction_signature_t platformlib_instruction_signatures[];

#endif
