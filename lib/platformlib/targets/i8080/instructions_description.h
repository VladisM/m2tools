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

/** @brief Enum with all instructions to simplify backend. */
typedef enum {
    INSTRU_MOV = 0,
    INSTRU_MVI,
    INSTRU_LXI,
    INSTRU_LDA,
    INSTRU_STA,
    INSTRU_LHLD,
    INSTRU_SHLD,
    INSTRU_LDAX,
    INSTRU_STAX,
    INSTRU_XCHG,
    INSTRU_ADD,
    INSTRU_ADI,
    INSTRU_ADC,
    INSTRU_ACI,
    INSTRU_SUB,
    INSTRU_SUI,
    INSTRU_SBB,
    INSTRU_SBI,
    INSTRU_INR,
    INSTRU_DCR,
    INSTRU_INX,
    INSTRU_DCX,
    INSTRU_DAD,
    INSTRU_DAA,
    INSTRU_ANA,
    INSTRU_ANI,
    INSTRU_ORA,
    INSTRU_ORI,
    INSTRU_XRA,
    INSTRU_XRI,
    INSTRU_CMP,
    INSTRU_CPI,
    INSTRU_RLC,
    INSTRU_RRC,
    INSTRU_RAL,
    INSTRU_RAR,
    INSTRU_CMA,
    INSTRU_CMC,
    INSTRU_STC,
    INSTRU_JMP,
    INSTRU_JNZ,
    INSTRU_JZ,
    INSTRU_JNC,
    INSTRU_JC,
    INSTRU_JPO,
    INSTRU_JPE,
    INSTRU_JP,
    INSTRU_JM,
    INSTRU_CALL,
    INSTRU_CNZ,
    INSTRU_CZ,
    INSTRU_CNC,
    INSTRU_CC,
    INSTRU_CPO,
    INSTRU_CPE,
    INSTRU_CP,
    INSTRU_CM,
    INSTRU_RET,
    INSTRU_RNZ,
    INSTRU_RZ,
    INSTRU_RNC,
    INSTRU_RC,
    INSTRU_RPO,
    INSTRU_RPE,
    INSTRU_RP,
    INSTRU_RM,
    INSTRU_RST,
    INSTRU_PCHL,
    INSTRU_PUSH,
    INSTRU_POP,
    INSTRU_XTHL,
    INSTRU_SPHL,
    INSTRU_IN,
    INSTRU_OUT,
    INSTRU_EI,
    INSTRU_DI,
    INSTRU_HLT,
    INSTRU_NOP,
    INSTRU_UNKOWN
}instruction_mnemonic_t;

/**
 * @brief Structure to hold informations about instruction.
 */
typedef struct{
    char *opcode;           /**< @b String representation of mnemotechnic. */
    unsigned int argc;      /**< @b Required arguments count. */
    isa_address_t size;     /**< @b Size of instruction in count of isa_memory_element_t. */
    instruction_mnemonic_t instruction_mnemonic; /**< @b Used to simplify backend. */
    isa_instruction_word_t instruction_code; /**< @b Used to simplify backend. */
} instruction_signature_t;

/**
 * @brief List of instruction_signature_t instancies filled with values.
 * @note Last item of list have to be {NULL, 0, 0}!
 * All tools will go thru this array to get neccesary informations.
 */
extern instruction_signature_t platformlib_instruction_signatures[];

#endif
