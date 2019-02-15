#include <stdint.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <inttypes.h>
#include <ctype.h>

#include "isa.h"

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

typedef struct{
    const char * line;
    uint32_t code;
}my_register_t;

typedef struct{
    const char * line;
    const char * format;
    i_opcode opcode;
    unsigned int size;
    uint32_t base_of_opcode;
}my_instruction_t;

typedef struct{
    const char * line;
    uint32_t code_CMPI;
    uint32_t code_CMPF;
    uint8_t can_be_CMPF;
}my_comp_t;

#define MOVE_TO_REGC(X) (X)
#define MOVE_TO_REGB(X) (X << 4)
#define MOVE_TO_REGA(X) (X << 8)
#define MOVE_TO_FLAG(X) (X << 20)
#define MOVE_TO_COND(X) (X << 20)

#define MY_REGS_LEN (sizeof(my_regs) / sizeof(my_register_t))
my_register_t my_regs[] = {
    {"R0", 0x0},
    {"R1", 0x1},
    {"R2", 0x2},
    {"R3", 0x3},
    {"R4", 0x4},
    {"R5", 0x5},
    {"R6", 0x6},
    {"R7", 0x7},
    {"R8", 0x8},
    {"R9", 0x9},
    {"R10", 0xA},
    {"R11", 0xB},
    {"R12", 0xC},
    {"R13", 0xD},
    {"R14", 0xE},
    {"R15", 0xF},
    {"SP", 0xF},
    {"PC", 0xE}
};

#define MY_INSTRS_LEN (sizeof(my_instrs) / sizeof(my_instruction_t))
my_instruction_t my_instrs[] = {
    {"RET"  , "I"    , ISA_RET   , 4, 0x01000000},
    {"RETI" , "I"    , ISA_RETI  , 4, 0x02000000},
    {"CALLI", "IR"   , ISA_CALLI , 4, 0x03000000},
    {"PUSH" , "IR"   , ISA_PUSH  , 4, 0x04000000},
    {"POP"  , "IR"   , ISA_POP   , 4, 0x05000000},
    {"LDI"  , "IRR"  , ISA_LDI   , 4, 0x06000000},
    {"STI"  , "IRR"  , ISA_STI   , 4, 0x07000000},
    {"BNZI" , "IRR"  , ISA_BNZI  , 4, 0x08000000},
    {"BZI"  , "IRR"  , ISA_BZI   , 4, 0x09000000},
    {"CMPI" , "IcRRR", ISA_CMPI  , 4, 0x0A000000},
    {"CMPF" , "IcRRR", ISA_CMPF  , 4, 0x0B000000},
    {"MULU" , "IRRR" , ISA_MULU  , 4, 0x0C000000},
    {"MUL"  , "IRRR" , ISA_MUL   , 4, 0x0C100000},
    {"ADD"  , "IRRR" , ISA_ADD   , 4, 0x0C600000},
    {"SUB"  , "IRRR" , ISA_SUB   , 4, 0x0C700000},
    {"INC"  , "IRR"  , ISA_INC   , 4, 0x0C800000},
    {"DEC"  , "IRR"  , ISA_DEC   , 4, 0x0C900000},
    {"AND"  , "IRRR" , ISA_AND   , 4, 0x0CA00000},
    {"OR"   , "IRRR" , ISA_OR    , 4, 0x0CB00000},
    {"XOR"  , "IRRR" , ISA_XOR   , 4, 0x0CC00000},
    {"NOT"  , "IRR"  , ISA_NOT   , 4, 0x0CD00000},
    {"DIVU" , "IRRR" , ISA_DIVU  , 4, 0x0D200000},
    {"DIV"  , "IRRR" , ISA_DIV   , 4, 0x0D300000},
    {"REMU" , "IRRR" , ISA_REMU  , 4, 0x0D400000},
    {"REM"  , "IRRR" , ISA_REM   , 4, 0x0D500000},
    {"LSL"  , "IRRR" , ISA_LSL   , 4, 0x0E000000},
    {"LSR"  , "IRRR" , ISA_LSR   , 4, 0x0E100000},
    {"ROL"  , "IRRR" , ISA_ROL   , 4, 0x0E200000},
    {"ROR"  , "IRRR" , ISA_ROR   , 4, 0x0E300000},
    {"ASL"  , "IRRR" , ISA_ASL   , 4, 0x0E400000},
    {"ASR"  , "IRRR" , ISA_ASR   , 4, 0x0E500000},
    {"FSUB" , "IRRR" , ISA_FSUB  , 4, 0x0F000000},
    {"FADD" , "IRRR" , ISA_FADD  , 4, 0x0F300000},
    {"FMUL" , "IRRR" , ISA_FMUL  , 4, 0x10100000},
    {"FDIV" , "IRRR" , ISA_FDIV  , 4, 0x11200000},
    {"MVIL" , "IR6"  , ISA_MVIL  , 4, 0x12000000},
    {"MVIH" , "IR6"  , ISA_MVIH  , 4, 0x13000000},
    {"CALL" , "I4"   , ISA_CALL  , 4, 0x80000000},
    {"LD"   , "I4R"  , ISA_LD    , 4, 0x90000000},
    {"ST"   , "IR4"  , ISA_ST    , 4, 0xA0000000},
    {"BZ"   , "IR4"  , ISA_BZ    , 4, 0xB0000000},
    {"BNZ"  , "IR4"  , ISA_BNZ   , 4, 0xC0000000},
    {"MVIA" , "IR4"  , ISA_MVIA  , 4, 0xD0000000},
    {"SWI"  , "I"    , ISA_SWI   , 4, 0x14000000}
};

#define MY_COMP_LEN (sizeof(my_comp) / sizeof(my_comp_t))
my_comp_t my_comp[] = {
    {"EQ" , 6 , 0, 1},
    {"NEQ", 7 , 5, 1},
    {"L"  , 10, 3, 1},
    {"LU" , 14, 0, 0},
    {"LE" , 11, 4, 1},
    {"LEU", 15, 0, 0},
    {"G"  , 8 , 1, 1},
    {"GU" , 12, 2, 1},
    {"GE" , 9 , 0, 0},
    {"GEU", 13, 0, 0}
};

/*
 * ---------------------------------------------------------------------
 * Macro definitions
 *
 */

#define SET_ERROR(n) if(isalib_errno == 0) isalib_errno = n

/*
 *
 * End of macro definitions
 * ---------------------------------------------------------------------
 * Static functions declarations
 *
 */

/**
 * @brief Find symbol in actual section. Search in symbol table.
 *
 * @param l string with label to find
 * @param s pointer given when calling function assemble_instruction() - point to section
 *
 * @return pointer to value, NULL if not found
 */
static uint32_t * seach_for_symbol(char *l, void *s);

/**
 * @brief Decode name of register into code for instruction opcode
 *
 * @param name String with register name.
 *
 * @return Code of register if exist, NULL othervise
 */
static uint32_t decode_register_name_for_opcode(char *name);

/**
 * @brief Convert string to long int.
 *
 * @param l String to convert.
 *
 * @return Converted value.
 *
 * This function is implemented as calling call back from user code.
 * If converting fail, user code may terminate execution.
 */
static long int convert_to_int(char *l);

/**
 * @brief Try to gues if given string can be label or not.
 *
 * @param s String to consider
 *
 * @return 1 if can be; 0 if cannot
 *
 * This function have to check if given string contain only
 * characters that can be used for labels. If so it will return 1. It will newer
 * look into symbol table as it is used in first pass.
 */
static int can_be_label_or_cons(char *s);


/**
 * @brief Get instruction opcode.
 *
 * Try to disasm instruction and return its opcode.
 *
 * @return -1 on failrule, 0 otherwise
 */
static int get_instruction_opcode(tInstruction *i, i_opcode *opcode);

/**
 * @brief Get 24CONST operand from instruction.
 *
 * If instruction have 24 bit wide operand, it will return it.
 *
 * @return -1 on failrule, 0 otherwise
 */
static int get_instruction_24CONST_operand(tInstruction *i, uint32_t *operand);

/**
 * @brief Set 24CONST operand to instruction.
 *
 * Put 24bit const operand back into instruction word. It will rewrite it.
 *
 * @return -1 on failrule, 0 otherwise
 */
static int set_instruction_24CONST_operand(tInstruction *i, uint32_t operand);

/*
 *
 * End of statit functions declarations
 * ---------------------------------------------------------------------
 * Static variables
 *
 */

static tIsaError isalib_errno = ISAERR_OK;
static uint32_t *(*search_for_symbol_handler)(char *, void *) = NULL;
static long int (*convert_to_int_handler)(char *) = NULL;
static int (*is_number_handler)(char *) = NULL;
inline static int put_adr_arg_into_inst(tInstruction * inst, char * adr_ptr, void * section_ptr);

/*
 *
 * End of static variables
 * ---------------------------------------------------------------------
 * Static functions definitions
 *
 */

static uint32_t * seach_for_symbol(char *l, void *s){
    if(search_for_symbol_handler == NULL) {
        SET_ERROR(ISAERR_MISSING_CALLBACK);
        return NULL;
    }

    return (*search_for_symbol_handler)(l, s);

}

static long int convert_to_int(char *l){
    if(convert_to_int_handler == NULL) {
        SET_ERROR(ISAERR_MISSING_CALLBACK);
        return -1;
    }

    return (*convert_to_int_handler)(l);

}

static uint32_t decode_register_name_for_opcode(char *name){

    for(unsigned int i = 0; i < MY_REGS_LEN; i++){
        if(strcmp(my_regs[i].line, name) == 0) return my_regs[i].code;
    }

    return (uint32_t)-1;
}

static int is_number(char *s){
    if(is_number_handler == NULL) {
        SET_ERROR(ISAERR_MISSING_CALLBACK);
        return -1;
    }

    return (*is_number_handler)(s);
}

inline static int put_adr_arg_into_inst(tInstruction * inst, char * adr_ptr, void * section_ptr){
    int ret = is_number(adr_ptr);

    if(ret == 1){
        //instruction have constant as argument
        long int adr = convert_to_int(adr_ptr);

        if(!((adr >= -8388608 && adr <= 8388607) || (unsigned)adr < 0xFFFFFF)){
            SET_ERROR(ISAERR_ARG_OVERFLOW);
            return 0;
        }

        set_instruction_24CONST_operand(inst, (uint32_t)adr);
    }
    else if(ret == 0){
        //instruction have symbol as arg
        uint32_t *adr = seach_for_symbol(adr_ptr, section_ptr);

        if(adr == NULL){
            SET_ERROR(ISAERR_ARG_UNKOWN_SYMBOL);
            return 0;
        }

        set_instruction_24CONST_operand(inst, *adr);
    }
    else{
        SET_ERROR(ISAERR_INTER_ERR);
        return 0;
    }

    return 1;
}

static int can_be_label_or_cons(char *s){
    for(int i = 0; s[i] != '\0'; i++){
        if( isalnum(s[i]) == 0 && s[i] != '_' && s[i] != '-' ){
            return 0;
        }
    }
    return 1;
}

static int get_instruction_opcode(tInstruction *i, i_opcode *opcode){

    switch((i->word & 0xF0000000) >> 28){
        case 0:
            switch((i->word & 0x0F000000) >> 24){
                case 1:
                    *opcode = ISA_RET;
                    return 0;
                case 2:
                    *opcode = ISA_RETI;
                    return 0;
                case 3:
                    *opcode = ISA_CALLI;
                    return 0;
                case 4:
                    *opcode = ISA_PUSH;
                    return 0;
                case 5:
                    *opcode = ISA_POP;
                    return 0;
                case 6:
                    *opcode = ISA_LDI;
                    return 0;
                case 7:
                    *opcode = ISA_STI;
                    return 0;
                case 8:
                    *opcode = ISA_BNZI;
                    return 0;
                case 9:
                    *opcode = ISA_BZI;
                    return 0;
                case 10:
                    *opcode = ISA_CMPI;
                    return 0;
                case 11:
                    *opcode = ISA_CMPF;
                    return 0;
                case 12:
                    switch((i->word & 0x0F000000) >> 20){
                        case 0:
                            *opcode = ISA_MULU;
                            return 0;
                        case 1:
                            *opcode = ISA_MUL;
                            return 0;
                        case 6:
                            *opcode = ISA_ADD;
                            return 0;
                        case 7:
                            *opcode = ISA_SUB;
                            return 0;
                        case 8:
                            *opcode = ISA_INC;
                            return 0;
                        case 9:
                            *opcode = ISA_DEC;
                            return 0;
                        case 10:
                            *opcode = ISA_AND;
                            return 0;
                        case 11:
                            *opcode = ISA_OR;
                            return 0;
                        case 12:
                            *opcode = ISA_XOR;
                            return 0;
                        case 13:
                            *opcode = ISA_NOT;
                            return 0;
                        default:
                            *opcode = ISA_UNDEF;
                            SET_ERROR(ISAERR_INSTRUCTION_NOT_RECOGNIZED);
                            return -1;
                    }
                case 13:
                    switch((i->word & 0x0F000000) >> 20){
                        case 2:
                            *opcode = ISA_DIVU;
                            return 0;
                        case 3:
                            *opcode = ISA_DIV;
                            return 0;
                        case 4:
                            *opcode = ISA_REMU;
                            return 0;
                        case 5:
                            *opcode = ISA_REM;
                            return 0;
                        default:
                            *opcode = ISA_UNDEF;
                            SET_ERROR(ISAERR_INSTRUCTION_NOT_RECOGNIZED);
                            return -1;
                    }
                case 14:
                    switch((i->word & 0x0F000000) >> 20){
                        case 0:
                            *opcode = ISA_LSL;
                            return 0;
                        case 1:
                            *opcode = ISA_LSR;
                            return 0;
                        case 2:
                            *opcode = ISA_ROL;
                            return 0;
                        case 3:
                            *opcode = ISA_ROR;
                            return 0;
                        case 4:
                            *opcode = ISA_ASL;
                            return 0;
                        case 5:
                            *opcode = ISA_ASR;
                            return 0;
                        default:
                            *opcode = ISA_UNDEF;
                            SET_ERROR(ISAERR_INSTRUCTION_NOT_RECOGNIZED);
                            return -1;
                    }
                case 15:
                    switch((i->word & 0x0F000000) >> 20){
                        case 0:
                            *opcode = ISA_FSUB;
                            return 0;
                        case 3:
                            *opcode = ISA_FADD;
                            return 0;
                        default:
                            *opcode = ISA_UNDEF;
                            SET_ERROR(ISAERR_INSTRUCTION_NOT_RECOGNIZED);
                            return -1;
                    }
                default:
                    *opcode = ISA_UNDEF;
                    SET_ERROR(ISAERR_INSTRUCTION_NOT_RECOGNIZED);
                    return -1;
            }
            break;
        case 1:
            switch((i->word & 0x0F000000) >> 24){
                case 0:
                    *opcode = ISA_FMUL;
                    return 0;
                case 1:
                    *opcode = ISA_FDIV;
                    return 0;
                case 2:
                    *opcode = ISA_MVIL;
                    return 0;
                case 3:
                    *opcode = ISA_MVIH;
                    return 0;
                default:
                    SET_ERROR(ISAERR_INSTRUCTION_NOT_RECOGNIZED);
                    return -1;
            }
        case 8:
            *opcode = ISA_CALL;
            return 0;
        case 9:
            *opcode = ISA_LD;
            return 0;
        case 10:
            *opcode = ISA_ST;
            return 0;
        case 11:
            *opcode = ISA_BZ;
            return 0;
        case 12:
            *opcode = ISA_BNZ;
            return 0;
        case 13:
            *opcode = ISA_MVIA;
            return 0;
        case 14:
            *opcode = ISA_SWI;
            return 0;
        default:
            SET_ERROR(ISAERR_INSTRUCTION_NOT_RECOGNIZED);
            return -1;
    };
}

static int get_instruction_24CONST_operand(tInstruction *i, uint32_t *operand){
    int ret;
    i_opcode opcode;

    ret = get_instruction_opcode(i, &opcode);

    if(ret != 0){
        return -1;
    }

    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wswitch-enum"

    switch(opcode){
        case ISA_CALL:
            *operand = (i->word & 0x0FFFFFF0) >> 4;
            break;
        case ISA_LD:
            *operand = (i->word & 0x0FFFFFF0) >> 4;
            break;
        case ISA_ST:
            *operand = ((i->word & 0x0FFFFF00) >> 4) | (i->word & 0x0000000F);
            break;
        case ISA_BZ:
            *operand = ((i->word & 0x0F000000) >> 4) | (i->word & 0x000FFFFF);
            break;
        case ISA_BNZ:
            *operand = ((i->word & 0x0F000000) >> 4) | (i->word & 0x000FFFFF);
            break;
        case ISA_MVIA:
            *operand = (i->word & 0x0FFFFFF0) >> 4;
            break;
        default:
            SET_ERROR(ISAERR_INSTRUCTION_DOESNT_HAVE_24CONST);
            return -1;
            break;
    }

    #pragma GCC diagnostic pop

    return 0;
}

static int set_instruction_24CONST_operand(tInstruction *i, uint32_t operand){
    int ret;
    i_opcode opcode;

    ret = get_instruction_opcode(i, &opcode);

    if(ret != 0){
        return -1;
    }

    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wswitch-enum"

    switch(opcode){
        case ISA_CALL:
            operand = operand << 4;
            i->word = operand | (i->word & 0xF000000F);
            break;
        case ISA_LD:
            operand = operand << 4;
            i->word = operand | (i->word & 0xF000000F);
            break;
        case ISA_ST:
            operand = ((operand & 0x00FFFFF0) << 4) | (operand & 0x0000000F);
            i->word = operand | (i->word & 0xF00000F0);
            break;
        case ISA_BZ:
            operand = ((operand & 0x00F00000) << 4) | (operand & 0x000FFFFF);
            i->word = operand | (i->word & 0xF0F00000);
            break;
        case ISA_BNZ:
            operand = ((operand & 0x00F00000) << 4) | (operand & 0x000FFFFF);
            i->word = operand | (i->word & 0xF0F00000);
            break;
        case ISA_MVIA:
            operand = operand << 4;
            i->word = operand | (i->word & 0xF000000F);
            break;
        default:
            SET_ERROR(ISAERR_INSTRUCTION_DOESNT_HAVE_24CONST);
            return -1;
            break;
    }

    #pragma GCC diagnostic pop

    return 0;
}

/*
 *
 * End of static functions
 * ---------------------------------------------------------------------
 * Exported functions definitions
 *
 */

int retarget_instruction(tInstruction *i, uint32_t base_address){

    uint32_t operand;

    if(get_instruction_24CONST_operand(i, &operand) < 0){
        return -1;
    }

    //relocate operand
    operand += base_address;

    //check if instruction argument dont overflow
    if(operand > 0x00FFFFFF){
        SET_ERROR(ISAERR_INSTRUCTION_ARG_OVERFLOW);
        return -1;
    }

    //put operand back to instruction word
    if(set_instruction_24CONST_operand(i, operand) < 0){
        return -1;
    }

    return 0;
}

tIsaError get_isalib_errno(void){
    return isalib_errno;
}

void clear_isalib_errno(void){
    isalib_errno = 0;
}

int is_comparison(char *s){

    for(unsigned int i = 0; i < MY_COMP_LEN; i++){
        if(strcmp(my_comp[i].line, s) == 0) return 1;
    }

    return 0;

}

int is_reg(char *s){

    for(unsigned int i = 0; i < MY_REGS_LEN; i++){
        if(strcmp(my_regs[i].line, s) == 0) return 1;
    }

    return 0;

}

const char* is_instruction(char *s){

    for(unsigned int i = 0; i < MY_INSTRS_LEN; i++){
        if(strcmp(s, my_instrs[i].line) == 0) return my_instrs[i].format;
    }

    return "N";

}

void free_istruction_struct(tInstruction *i){
    if(i == NULL){
        return;
    }

    if(i->line != NULL){
        free(i->line);
    }

    free(i);

}

int get_instruction_size(tInstruction *inst, unsigned int *size){

    //create local copy of line
    char * line_copy = (char *)malloc(sizeof(char) * (strlen(inst->line) + 1));

    int ret_val = 0;

    if(line_copy == NULL){
        SET_ERROR(ISAERR_MALLOC_FAIL);
        return 0;
    }

    strcpy(line_copy, inst->line);

    for(int i = 0; line_copy[i] != '\0'; i++){
        if(line_copy[i] == ';'){
            line_copy[i] = '\0';
        }
    }

    for(unsigned int i = 0; i < MY_INSTRS_LEN; i++){
        if(strcmp(line_copy, my_instrs[i].line) == 0){
            *size = my_instrs[i].size;
            ret_val = 1;
            break;
        }
    }

    free(line_copy);
    return ret_val;

}

int check_instruction_args(char *i){
    //TODO: add valid check

    char *line_dup = strdup(i);

    if(line_dup == NULL){
        SET_ERROR(ISAERR_INTER_ERR);
        return 0;
    }

    for(int index = 0; line_dup[index] != '\0'; index++){
        if(line_dup[index] == ';'){
            line_dup[index] = '\0';
        }
    }

    my_instruction_t *found_instr = NULL;

    for(unsigned int index = 0; index < MY_INSTRS_LEN; index++){
        if(strcmp(line_dup, my_instrs[index].line) == 0){
            found_instr = &(my_instrs[index]);
            break;
        }
    }

    if(found_instr == NULL){
        SET_ERROR(ISAERR_INSTRUCTION_NOT_RECOGNIZED);
        return 0;
    }

    switch(found_instr->opcode){
        case ISA_RET:
        case ISA_RETI:
        case ISA_SWI:
            return 1;
        case ISA_CALLI:
        case ISA_PUSH:
        case ISA_POP:
            {
                char * reg_ptr = (line_dup + strlen(line_dup) + 1);

                if(is_reg(reg_ptr) == 0){
                    return 0;
                }
            }
            break;
        case ISA_LDI:
        case ISA_STI:
        case ISA_BZI:
        case ISA_BNZI:
        case ISA_INC:
        case ISA_DEC:
        case ISA_NOT:
            {
                char * reg1_ptr = (line_dup + strlen(line_dup) + 1);
                char * reg2_ptr = (line_dup + strlen(line_dup) + strlen(reg1_ptr) + 2);

                if(is_reg(reg1_ptr) == 0 || is_reg(reg2_ptr) == 0){
                    return 0;
                }
            }
            break;
        case ISA_MULU:
        case ISA_MUL:
        case ISA_ADD:
        case ISA_SUB:
        case ISA_AND:
        case ISA_OR:
        case ISA_XOR:
        case ISA_DIVU:
        case ISA_DIV:
        case ISA_REMU:
        case ISA_REM:
        case ISA_LSL:
        case ISA_LSR:
        case ISA_ROL:
        case ISA_ROR:
        case ISA_ASL:
        case ISA_ASR:
        case ISA_FSUB:
        case ISA_FADD:
        case ISA_FMUL:
        case ISA_FDIV:
            {
                char * reg1_ptr = (line_dup + strlen(line_dup) + 1);
                char * reg2_ptr = (line_dup + strlen(line_dup) + strlen(reg1_ptr) + 2);
                char * reg3_ptr = (line_dup + strlen(line_dup) + strlen(reg1_ptr) + strlen(reg2_ptr) + 3);

                if(is_reg(reg1_ptr) == 0 || is_reg(reg2_ptr) == 0 || is_reg(reg3_ptr) == 0){
                    return 0;
                }
            }
            break;
        case ISA_CMPI:
        case ISA_CMPF:
            {
                char * cmp_flag = (line_dup + strlen(line_dup) + 1);
                char * reg1_ptr = (line_dup + strlen(cmp_flag) + strlen(line_dup) + 2);
                char * reg2_ptr = (line_dup + strlen(cmp_flag) + strlen(line_dup) + strlen(reg1_ptr) + 3);
                char * reg3_ptr = (line_dup + strlen(cmp_flag) + strlen(line_dup) + strlen(reg1_ptr) + strlen(reg2_ptr) + 4);

                if(is_reg(reg1_ptr) == 0 || is_reg(reg2_ptr) == 0 || is_reg(reg3_ptr) == 0|| is_comparison(cmp_flag) == 0){
                    return 0;
                }
            }
            break;
        case ISA_MVIL:
        case ISA_MVIH:
            {
                char * reg_ptr = (line_dup + strlen(line_dup) + 1);
                char * cons_ptr = (line_dup + strlen(line_dup) + strlen(reg_ptr) + 2);

                if(is_reg(reg_ptr) == 0 || can_be_label_or_cons(cons_ptr) == 0){
                    return 0;
                }
            }
            break;
        case ISA_CALL:
            {
                char * cons_ptr = (line_dup + strlen(line_dup) + 1);

                if(can_be_label_or_cons(cons_ptr) == 0){
                    return 0;
                }
            }
            break;
        case ISA_LD:
            {
                char * adr_ptr = (line_dup + strlen(line_dup) + 1);
                char * reg_ptr = (line_dup + strlen(line_dup) + strlen(adr_ptr) + 2);

                if(is_reg(reg_ptr) == 0 || can_be_label_or_cons(adr_ptr) == 0){
                    return 0;
                }
            }
            break;
        case ISA_ST:
        case ISA_BZ:
        case ISA_BNZ:
        case ISA_MVIA:
            {
                char * reg_ptr = (line_dup + strlen(line_dup) + 1);
                char * adr_ptr = (line_dup + strlen(line_dup) + strlen(reg_ptr) + 2);

                if(is_reg(reg_ptr) == 0 || can_be_label_or_cons(adr_ptr) == 0){
                    return 0;
                }
            }
            break;
        case ISA_UNDEF:
        default:
            SET_ERROR(ISAERR_INTER_ERR);
            return 0;
    }

    return 1;
}

int export_into_object_file_line(tInstruction *inst, char *line){
    if(inst == NULL || line == NULL){
        SET_ERROR(ISAERR_NULL_PTR);
        return 0;
    }

    int ret = snprintf(line, 32, "0x%"PRIx32, inst->word);

    if(ret < 0){
        SET_ERROR(ISAERR_INTER_ERR);
        return 0;
    }

    return 1;

}

int import_from_object_file_line(tInstruction *inst, char *line){

    if(line == NULL || inst == NULL){
        SET_ERROR(ISAERR_NULL_PTR);
        return 0;
    }

    if(sscanf(line, "%"SCNx32, &(inst->word)) != 1){
        SET_ERROR(ISAERR_FORMAT_ERR);
        return 0;
    }

    return 1;
}

tInstruction *new_instru(void){

    tInstruction *tmp = (tInstruction *)malloc(sizeof(tInstruction));

    if(tmp == NULL){
        SET_ERROR(ISAERR_MALLOC_FAIL);
        return NULL;
    }

    tmp->line = NULL;
    tmp->word = 0;

    return tmp;

}

int assemble_instruction(tInstruction * inst, void * section_ptr){

    if(inst == NULL || section_ptr == NULL){
        SET_ERROR(ISAERR_NULL_PTR);
        return 0;
    }

    if(inst->line == NULL){
        SET_ERROR(ISAERR_NULL_PTR);
        return 0;
    }

    //create local copy of line
    char * opcode_copy = (char *)malloc(sizeof(char) * (strlen(inst->line) + 1));

    if(opcode_copy == NULL){
        SET_ERROR(ISAERR_MALLOC_FAIL);
        return 0;
    }

    strcpy(opcode_copy, inst->line);

    //replace ';' with '\0'
    for(int i = 0; opcode_copy[i] != '\0'; i++) if(opcode_copy[i] == ';') opcode_copy[i] = '\0';

    //check if i is valid instruction
    const char * format_string = is_instruction(opcode_copy);

    if(format_string[0] != 'I'){
        SET_ERROR(ISAERR_INSTRUCTION_NOT_RECOGNIZED);
        return 0;
    }

    //detect instruction
    i_opcode op;
    uint32_t word;

    for(unsigned int i = 0; i < MY_INSTRS_LEN; i++){
        if(strcmp(opcode_copy, my_instrs[i].line) == 0){
            op = my_instrs[i].opcode;
            word = my_instrs[i].base_of_opcode;
        }
    }

    switch(op){
        case ISA_RET:
        case ISA_RETI:
        case ISA_SWI:
            {
                inst->word = word;
            }
            break;
        case ISA_CALLI:
        case ISA_PUSH:
        case ISA_POP:
            {
                char * reg_ptr = (opcode_copy + strlen(opcode_copy) + 1);

                uint32_t reg = decode_register_name_for_opcode(reg_ptr);

                inst->word = word;

                if(op == ISA_CALLI)
                    inst->word |= MOVE_TO_REGA(reg);
                if(op == ISA_PUSH)
                    inst->word |= MOVE_TO_REGB(reg);
                if(op == ISA_POP)
                    inst->word |= MOVE_TO_REGC(reg);
            }
            break;
        case ISA_LDI:
        case ISA_STI:
        case ISA_BZI:
        case ISA_BNZI:
        case ISA_INC:
        case ISA_DEC:
        case ISA_NOT:
            {
                char * reg1_ptr = (opcode_copy + strlen(opcode_copy) + 1);
                char * reg2_ptr = (opcode_copy + strlen(opcode_copy) + strlen(reg1_ptr) + 2);

                uint32_t reg1 = decode_register_name_for_opcode(reg1_ptr);
                uint32_t reg2 = decode_register_name_for_opcode(reg2_ptr);

                inst->word = word;

                if(op == ISA_LDI)
                    inst->word |= (MOVE_TO_REGA(reg1) | MOVE_TO_REGC(reg2));

                if(op == ISA_STI)
                    inst->word |= (MOVE_TO_REGA(reg2) | MOVE_TO_REGC(reg1));

                if(op == ISA_BZI || op == ISA_BNZI)
                    inst->word |= (MOVE_TO_FLAG(reg1) | MOVE_TO_REGA(reg2));

                if(op == ISA_INC || op == ISA_DEC || op == ISA_NOT)
                    inst->word |= (MOVE_TO_REGA(reg1) | MOVE_TO_REGC(reg2));

            }
            break;
        case ISA_MULU:
        case ISA_MUL:
        case ISA_ADD:
        case ISA_SUB:
        case ISA_AND:
        case ISA_OR:
        case ISA_XOR:
        case ISA_DIVU:
        case ISA_DIV:
        case ISA_REMU:
        case ISA_REM:
        case ISA_LSL:
        case ISA_LSR:
        case ISA_ROL:
        case ISA_ROR:
        case ISA_ASL:
        case ISA_ASR:
        case ISA_FSUB:
        case ISA_FADD:
        case ISA_FMUL:
        case ISA_FDIV:
            {
                char * reg1_ptr = (opcode_copy + strlen(opcode_copy) + 1);
                char * reg2_ptr = (opcode_copy + strlen(opcode_copy) + strlen(reg1_ptr) + 2);
                char * reg3_ptr = (opcode_copy + strlen(opcode_copy) + strlen(reg1_ptr) + strlen(reg2_ptr) + 3);

                uint32_t reg1 = decode_register_name_for_opcode(reg1_ptr);
                uint32_t reg2 = decode_register_name_for_opcode(reg2_ptr);
                uint32_t reg3 = decode_register_name_for_opcode(reg3_ptr);

                inst->word = word;

                inst->word |= (MOVE_TO_REGA(reg1) | MOVE_TO_REGB(reg2) | MOVE_TO_REGC(reg3));

            }
            break;
        case ISA_CMPI:
        case ISA_CMPF:
            {
                char * cmp_flag = (opcode_copy + strlen(opcode_copy) + 1);
                char * reg1_ptr = (opcode_copy + strlen(cmp_flag) + strlen(opcode_copy) + 2);
                char * reg2_ptr = (opcode_copy + strlen(cmp_flag) + strlen(opcode_copy) + strlen(reg1_ptr) + 3);
                char * reg3_ptr = (opcode_copy + strlen(cmp_flag) + strlen(opcode_copy) + strlen(reg1_ptr) + strlen(reg2_ptr) + 4);

                uint32_t reg1 = decode_register_name_for_opcode(reg1_ptr);
                uint32_t reg2 = decode_register_name_for_opcode(reg2_ptr);
                uint32_t reg3 = decode_register_name_for_opcode(reg3_ptr);

                inst->word = word;

                inst->word |= (MOVE_TO_REGA(reg1) | MOVE_TO_REGB(reg2) | MOVE_TO_REGC(reg3));


                my_comp_t * flag = NULL;

                //find flag in table
                for(unsigned int i = 0; i < MY_COMP_LEN; i++){
                    if(strcmp(my_comp[i].line, cmp_flag) == 0){
                        flag = &(my_comp[i]);
                        break;
                    }
                }

                //put cond reg into word
                if(op == ISA_CMPF){
                    if(flag->can_be_CMPF == 0){
                        SET_ERROR(ISAERR_INSTRU_SYNTAX_ERR);
                        return 0;
                    }
                    else{
                        inst->word |= MOVE_TO_COND(flag->code_CMPF);
                    }
                }
                else{
                    inst->word |= MOVE_TO_COND(flag->code_CMPI);
                }

            }
            break;
        case ISA_MVIL:
        case ISA_MVIH:
            {
                char * reg_ptr = (opcode_copy + strlen(opcode_copy) + 1);
                char * cons_ptr = (opcode_copy + strlen(opcode_copy) + strlen(reg_ptr) + 2);

                uint32_t reg = decode_register_name_for_opcode(reg_ptr);
                long int cons = convert_to_int(cons_ptr);

                inst->word = word;

                inst->word |= (MOVE_TO_REGB(reg) | MOVE_TO_REGC(reg));

                if(!((cons >= -32768 && cons <= 32767) || (unsigned)cons < 0xFFFF)){
                    SET_ERROR(ISAERR_ARG_OVERFLOW);
                    return 0;
                }

                inst->word |= ((uint32_t)cons << 8);

            }
            break;
        case ISA_CALL:
            {
                char *adr_ptr = (opcode_copy + strlen(opcode_copy) + 1);

                inst->word = word;

                if(put_adr_arg_into_inst(inst, adr_ptr, section_ptr) == 0) return 0;
            }
            break;
        case ISA_LD:
        case ISA_ST:
        case ISA_BZ:
        case ISA_BNZ:
        case ISA_MVIA:
            {
                char *reg_ptr;
                char *adr_ptr;

                if(op == ISA_LD){
                    adr_ptr = (opcode_copy + strlen(opcode_copy) + 1);
                    reg_ptr = (opcode_copy + strlen(opcode_copy) + strlen(adr_ptr) + 2);
                }
                else{
                    reg_ptr = (opcode_copy + strlen(opcode_copy) + 1);
                    adr_ptr = (opcode_copy + strlen(opcode_copy) + strlen(reg_ptr) + 2);
                }

                inst->word = word;

                uint32_t reg = decode_register_name_for_opcode(reg_ptr);

                if(op == ISA_MVIA || op == ISA_LD){
                    inst->word |= MOVE_TO_REGC(reg);
                }
                else if(op == ISA_BZ || op == ISA_BNZ){
                    inst->word |= MOVE_TO_FLAG(reg);
                }
                else{
                    inst->word |= MOVE_TO_REGB(reg);
                }

                if(put_adr_arg_into_inst(inst, adr_ptr, section_ptr) == 0) return 0;

            }
            break;
        case ISA_UNDEF:
        default:
            SET_ERROR(ISAERR_INTER_ERR);
            return 0;
    }

    free(opcode_copy);

    return 1;
}

/*
 *
 * End of exported functions definitions
 * ---------------------------------------------------------------------
 * Callback register functions.
 *
 */

int register_callback_search_for_symbol( uint32_t *(*f)(char *, void *) ){
    if(f == NULL){
        SET_ERROR(ISAERR_NULL_PTR);
        return 0;
    }

    if(search_for_symbol_handler != NULL){
        SET_ERROR(ISAERR_INTER_ERR);
        return 0;
    }

    search_for_symbol_handler = f;

    return 1;
}

int register_callback_convert_to_int( long int (*f)(char *) ){
    if(f == NULL){
        SET_ERROR(ISAERR_NULL_PTR);
        return 0;
    }

    if(convert_to_int_handler != NULL){
        SET_ERROR(ISAERR_INTER_ERR);
        return 0;
    }

    convert_to_int_handler = f;

    return 1;
}

int register_callback_is_number( int (*f)(char *) ){
    if(f == NULL){
        SET_ERROR(ISAERR_NULL_PTR);
        return 0;
    }

    if(is_number_handler != NULL){
        SET_ERROR(ISAERR_INTER_ERR);
        return 0;
    }

    is_number_handler = f;

    return 1;
}

