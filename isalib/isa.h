/**
 * @file isa.h
 *
 * @brief Header file of the library for dealing with ISA related tasks.
 *
 * @author Bc. Vladislav Mlejnecký <v.mlejnecky@seznam.cz>
 * @date 15.06.2019
 *
 * @note This file is part of m2tools project.
 */

#ifndef ISA_H_included
#define ISA_H_included

#include <stdint.h>
#include <stdbool.h>
#include <inttypes.h>

/**
 * @defgroup isa ISA library
 *
 * Library for things related to instruction set architecture.
 * Scripts and programms that have to work with something related to
 * ISA should do it by using this library.
 *
 * @addtogroup isa
 *
 * @{
 */

/**
 * @brief ISA lib errors.
 *
 * Define posibles error that can occure in library.
 */
typedef enum {
    ISAERR_OK = 0,                           /**< @brief There isn't any error. */
    ISAERR_INSTRUCTION_NOT_RECOGNIZED,       /**< @brief Instruction is not recognized, probably not an instruction. */
    ISAERR_INSTRUCTION_DOESNT_HAVE_24CONST,  /**< @brief Instruction dosn't have 24bit constant as operand, but you wan't to separate it anyway... */
    ISAERR_INSTRUCTION_ARG_OVERFLOW,         /**< @brief Constant argument is larger than its width in instruction word. */
    ISAERR_NULL_PTR,                         /**< @brief Given pointer argument is NULL. */
    ISAERR_MALLOC_FAIL,                      /**< @brief Failed to allocate memory, if you need this something is probably horrible wrong. */
    ISAERR_INTER_ERR,                        /**< @brief Probably bug in library, didn't you call register_callback twice? */
    ISAERR_FORMAT_ERR,                       /**< @brief Failed to decode string for object file library. */
    ISAERR_MISSING_CALLBACK,                 /**< @brief Callback function isn't registered but it was needed. */
    ISAERR_ARG_OVERFLOW,                     /**< @brief Overflow of an argument in assembling instruction. */
    ISAERR_ARG_UNKOWN_SYMBOL,                /**< @brief Cant find symbol when assembling instruction. */
    ISAERR_INSTRU_SYNTAX_ERR                 /**< @brief Error in instruction syntax. */
} tIsaError;

/**
 * @brief Type to hold one instruction word.
 */
typedef uint32_t isa_instruction_word_t;

#define PRIisa_iw "0x%"PRIx32 /**< @brief Macro for printf to use with isa_instruction_word_t. */
#define SCNisa_iw "0x%"SCNx32 /**< @brief Macro for scanf to use with isa_instruction_word_t. */

/**
 * Type to hold address in memory on target arch.
 */
typedef uint32_t isa_address_t;

#define PRIisa_addr "0x%"PRIx32 /**< @brief Macro for printf to use with isa_address_t. */
#define SCNisa_addr "0x%"SCNx32 /**< @brief Macro for scanf to use with isa_address_t. */

/**
 * @brief Structure to hold one instruction and all its informations.
 */
typedef struct sInstruction{
    char* line; /**< @brief Instruction as output from tokenizer, semicollon separed tokens. */
    isa_instruction_word_t word; /**< @brief Instruction word. */
}tInstruction;

/**
 * @brief Retarget direct operand of instruction.
 *
 * Instruction, when translated and linked by assembler and linker, are
 * located at address 0x000000. When for, example, we have CALL to
 * address 0x000100, and put our program to address 0x100000
 * (SDRAM is here) then the call have to be done to address 0x100100.
 *
 * Anyway, this can be done only with instruction that have 24bit
 * CONST in opcode. It is imposible to retarget branch using BZI instruction.
 *
 * @param base_address Adress offset given to the instruction.
 * @param i Structure with instruction to retarget.
 *
 * @return true if ok, false if failed
 */
bool retarget_instruction(tInstruction *i, uint32_t base_address);

/**
 * @brief Return error code occured in ISA lib.
 *
 * Returned error code is from tIsaError enum. If multiple errors
 * ocured, only the first one will be stored here.
 *
 * @note Error variable is simple variable, you should clear it in
 * order to be catch another error. Use clear_isalib_errno() for it.
 */
tIsaError get_isalib_errno(void);

/**
 * @brief Clear error code variable errno.
 *
 * For more informations please see get_isalib_errno()
 */
void clear_isalib_errno(void);

/**
 * @brief Check if given token is correct acording to format string given by is_instruction.
 *
 * @param t token to check
 * @param c One character from format string.
 *
 * @return 1 if valid; 0 if not valid; -1 error
 */
int is_correct_token(char *t, char c);

/**
 * @brief Test string it it is valid instruction mnemo.
 *
 * This instruction will read input string, that should be instruction
 * mnemonic and decide if it is valid or not. Pointer to format string
 * is returned. This format string will represent format of instruction
 * as written in assembler file.
 *
 * One character in the string represent one part of the instruction.
 *
 * Following characters are defined:
 * - 'I' instruction opcode
 * - 'R' reg
 * - 'c' comparision
 * - 'N' no instruction
 * - '6' 16bit const
 * - '4' 24bit const
 *
 * @note Please note this, if you are porting isa library, it is up to you how to
 * chose format characters, but at least I character have to be used as opcode, and
 * instruction with any arguments have to be definend as "I" (with '\0' at the end).
 * This is fundamental assumption made by tokenizer when parsing input files.
 *
 * @param s String to test.
 *
 * @return Pointer to format string.
 */
const char* is_instruction(char *s);

/**
 * @brief Clean up tInstruction structure.
 *
 * In instruction structure are some parts that are dynamically alocated,
 * this function will invoke free() on them in order to cleanup memory.
 *
 * @param i Structure to clean.
 */
void free_istruction_struct(tInstruction *i);

/**
 * @brief Return instruction size in bytes
 *
 * @param inst Instruction structure
 * @param size Pointer to variable where result will be written
 *
 * @return true if ok, false if failed
 */
bool get_instruction_size(tInstruction *inst, unsigned int * size);

/**
 * @brief Check valid instruction format.
 *
 * @param s Tokenized string with instruction. Separed with semicollon ';'.
 *
 * @return true if tokens are valid, false if tokens are invalid
 */
bool check_instruction_args(char *i);

/**
 * @brief Create line for writing instruction into object file.
 *
 * @param inst Pointer to structure to be written.
 * @param line Pointer to string buffer where line be stored.
 *
 * @warning line have to be allocated before entering this function
 * and have to be atleast 32 chars wide
 *
 * @return true if ok; false if fail
 */
bool export_into_object_file_line(tInstruction *inst, char *line);

/**
 * @brief Read line from object file into intruction structure.
 *
 * @param inst Pointer to structure where result will be stored.
 * @param line Pointer to string buffer where line is stored.
 *
 * @return true if ok; false if fail
 */
bool import_from_object_file_line(tInstruction *inst, char *line);

/**
 * @brief Create new instance of tInstruction structure and fill it
 * with default values.
 *
 * @return Pointer to new struct if succed, NULL otherwise.
 */
tInstruction *new_instru(void);

/**
 * @brief Assemble instruction from string.
 *
 * @warning If someone want to use this function you have to set-up callback
 * first for searching symbol table. Use register_callback_search_for_symbol function for that.
 *
 * @param i Instruction to assemble
 * @param section_ptr void pointer given to callback together with symbol label. Used for specifing section.
 *
 * @return true if ok; false if fail
 */
bool assemble_instruction(tInstruction * i, void * section_ptr);

/**
 * @brief Register call back function for ISA library module.
 *
 * This function register callback function for ISA library, this callback
 * is used when assembling instructions is required. It got called when
 * value of specified symbol from symbol table is needed.
 *
 * It suppose to return pointer to its value or NULL when no value is found.
 * First argument have to be pointer to string label of symbol, and second
 * pointer is pointer given in assemble_instruction function for specifiing
 * section.
 *
 * @param f Pointer to function for search in symbol table.
 *
 * @return true if ok, false if failed
 *
 * @warning Call this function only once and before any assemble_instruction is called.
 * Otherwise you may cause error.
 */
bool register_callback_search_for_symbol( uint32_t *(*f)(char *, void *) );

/**
 * @brief Register call back function for ISA library module.
 *
 * This function register callback function for translating string into
 * integers. This function is provided by assembler itself. It is used when
 * calling assemble_instruction.
 *
 * Given function have to translate null terminated string into long int
 * number. If this conversion is not possible, function can terminate execution
 * of assembler.
 *
 * @param f pointer to such function.
 *
 * @return true if ok, false if failed
 *
 * @warning Call this function only once and before any assemble_instruction is called.
 * Otherwise you may cause error.
 */
bool register_callback_convert_to_int( long int (*f)(char *) );

/**
 * @brief Register call back function for ISA library module.
 *
 * This function register callback function for checking if string is number.
 *
 * Given function have to check if string is acceptable for convert_to_int callback. And if so, return 1. Zero otherwise.
 *
 * @param f pointer to such function.
 *
 * @return true if ok, false if failed
 *
 * @warning Call this function only once and before any assemble_instruction is called.
 * Otherwise you may cause error.
 */
bool register_callback_is_number( int (*f)(char *) );

/**
 * @}
 */

#endif
