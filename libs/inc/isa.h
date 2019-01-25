#ifndef ISA_H_included
#define ISA_H_included

/**
 * @file isa.h
 *
 * @brief Library for dealing with ISA related tasks.
 */

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

#include <stdint.h>

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
} tIsaError;

/**
 * @brief Instruction list.
 */
typedef enum {
    ISA_UNDEF = 0, /**< @brief Undefined or unknown instruction. */
    ISA_RET,       /**< @brief Return from subroutine. */
    ISA_RETI,      /**< @brief Return from interrupt. */
    ISA_CALLI,     /**< @brief Call subroutine, address is in register. */
    ISA_PUSH,      /**< @brief Push register onto stack. */
    ISA_POP,       /**< @brief Pop from stack into register. */
    ISA_LDI,       /**< @brief Load data from mem (point with reg) into register. */
    ISA_STI,       /**< @brief Store data from register into mem (point with reg). */
    ISA_BZI,       /**< @brief Branch if reg is zero, adress is in register. */
    ISA_BNZI,      /**< @brief Branch if reg is not zero, address in in register. */
    ISA_CMPI,      /**< @brief Compare two integers, store result in reg. */
    ISA_CMPF,      /**< @brief Compare two floats, store result in reg. */
    ISA_MULU,      /**< @brief Do unsigned multiply (integer). */
    ISA_MUL,       /**< @brief Do signed multiply (integer). */
    ISA_ADD,       /**< @brief Add two integers. */
    ISA_SUB,       /**< @brief Sub two intefers. */
    ISA_INC,       /**< @brief Increment register. */
    ISA_DEC,       /**< @brief Decrement register. */
    ISA_AND,       /**< @brief Do logical AND between two registers. */
    ISA_OR,        /**< @brief Do logical OR between two registers. */
    ISA_XOR,       /**< @brief Do logical XOR between two registers. */
    ISA_NOT,       /**< @brief Negate register. */
    ISA_DIVU,      /**< @brief Do integer unsigned division. */
    ISA_DIV,       /**< @brief Do integer signed division. */
    ISA_REMU,      /**< @brief Return remainer from unsigned. */
    ISA_REM,       /**< @brief Return remainer from signed. */
    ISA_LSL,       /**< @brief Logical shift left. */
    ISA_LSR,       /**< @brief Logical shift right. */
    ISA_ROL,       /**< @brief Rotate left. */
    ISA_ROR,       /**< @brief Rotate right. */
    ISA_ASL,       /**< @brief Arithmetical shift left. */
    ISA_ASR,       /**< @brief Arithmetical shift right. */
    ISA_FSUB,      /**< @brief Floating point substract. */
    ISA_FADD,      /**< @brief Floating point addition. */
    ISA_FMUL,      /**< @brief Floating point multiply. */
    ISA_FDIV,      /**< @brief Floating point division. */
    ISA_MVIL,      /**< @brief Load 16cons into lower half of reg. */
    ISA_MVIH,      /**< @brief Load 16cons into higher half of reg. */
    ISA_CALL,      /**< @brief Call subroutine (address is in opcode). */
    ISA_LD,        /**< @brief Load from mem into reg (address in opcode). */
    ISA_ST,        /**< @brief Store from reg into mem (address in opcode). */
    ISA_BZ,        /**< @brief Branch if register is zero (address in opcode). */
    ISA_BNZ,       /**< @brief Branch if register is not zero (address in opcode). */
    ISA_MVIA,      /**< @brief Move address into register (24 bit constant). */
    ISA_SWI        /**< @brief Software interrupt. */
} i_opcode;

/**
 * @brief Structure to hold one instruction and all its informations.
 */
typedef struct sInstruction{
    uint32_t word; /**< @brief Instruction word. */
    char* line; /**< @brief Instruction as output from tokenizer, semicollon separed tokens. */
    uint8_t special; /**< @brief Used in linker and assembler, specify if instruction operand pointing into special symbol table. */
    uint8_t relocation; /**< @brief Used in liner and assembler, specify if instruction operand have to be relocated during linking. */
} tInstruction;

/**
 * @brief Get instruction opcode.
 *
 * Try to disasm instruction and return its opcode.
 *
 * @return -1 on failrule, 0 otherwise
 */
int get_instruction_opcode(tInstruction *i, i_opcode *opcode);

/**
 * @brief Get 24CONST operand from instruction.
 *
 * If instruction have 24 bit wide operand, it will return it.
 *
 * @return -1 on failrule, 0 otherwise
 */
int get_instruction_24CONST_operand(tInstruction *i, uint32_t *operand);

/**
 * @brief Set 24CONST operand to instruction.
 *
 * Put 24bit const operand back into instruction word. It will rewrite it.
 *
 * @return -1 on failrule, 0 otherwise
 */
int set_instruction_24CONST_operand(tInstruction *i, uint32_t operand);

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
 * @return -1 on failrule, 0 otherwise
 */
int retarget_instruction(tInstruction *i, uint32_t base_address);

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
 * @brief Test string if it is valid register name.
 *
 * @param s String to test.
 *
 * @return non-zero value if string is valid register name.
 */
int is_reg(char *s);

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
 * @param s String to test.
 *
 * @return Pointer to format string.
 */
const char* is_instruction(char *s);

/**
 * @brief Test string if it si valid mnemo for comparison.
 *
 * Special instruction CMPI or CMPF are used for comparisons and then
 * for  branching. But CPM* instruction need to know condition to
 * compare. And that condition is given as comparison string.
 *
 * This function will check given string and return 1 if string is
 * valid, or 0 if invalid.
 *
 * @param s String to test.
 *
 * @return non-zero value if string is valid comparison mnemo.
 */
int is_comparison(char *s);

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
 * @param i Instruction structure
 *
 * @return Int value of bytes needeed by instruction.
 */
unsigned int get_instruction_size(tInstruction *i);

/**
 * @brief Check valid instruction format.
 *
 * @param s Tokenized string with instruction. Separed with semicollon ';'.
 *
 * @return 1 - if tokens are valid, 0 - if tokens are invalid
 */
int check_instruction_args(char *i);

/**
 * @brief Create line for writing instruction into object file.
 *
 * @param inst Pointer to structure to be written.
 * @param line Pointer to string buffer where line be stored.
 *
 * @warning line have to be allocated before entering this function
 * and have to be atleast 32 chars wide
 *
 * @return 1 if ok; 0 if fail
 */
int export_into_object_file_line(tInstruction *inst, char *line);

/**
 * @brief Read line from object file into intruction structure.
 *
 * @param inst Pointer to structure where result will be stored.
 * @param line Pointer to string buffer where line is stored.
 *
 * @return 1 if ok; 0 if fail
 */
int import_from_object_file_line(tInstruction *inst, char *line);

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
 * @return 1 if ok; 0 if fail
 */
int assemble_instruction(tInstruction * i, void * section_ptr);

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
 * @return 1 if ok; 0 if fail
 *
 * @warning Call this function only once and before any assemble_instruction is called.
 * Otherwise you may cause error.
 */
int register_callback_search_for_symbol( uint32_t *(*f)(char *, void *) );

/**
 * @}
 */

#endif
