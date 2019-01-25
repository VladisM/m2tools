#include <stdint.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <inttypes.h>

#include "isa.h"

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
 * @brief Read string and return instruction opcode.
 *
 * @param s Pointer to string with instruction opcode
 *
 * @return type of instruction decoded from string
 */
static i_opcode get_opcode_from_name(char *s);

/*
 *
 * End of statit functions declarations
 * ---------------------------------------------------------------------
 * Static variables
 *
 */

static tIsaError isalib_errno;
static uint32_t *(*search_for_symbol_handler)(char *, void *)  = NULL;


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

static i_opcode get_opcode_from_name(char *s){
         if( strcmp(s,"RET")   == 0) return ISA_RET;
    else if( strcmp(s,"RETI")  == 0) return ISA_RETI;
    else if( strcmp(s,"CALLI") == 0) return ISA_CALLI;
    else if( strcmp(s,"PUSH")  == 0) return ISA_PUSH;
    else if( strcmp(s,"POP")   == 0) return ISA_POP;
    else if( strcmp(s,"LDI")   == 0) return ISA_LDI;
    else if( strcmp(s,"STI")   == 0) return ISA_STI;
    else if( strcmp(s,"BZI")   == 0) return ISA_BZI;
    else if( strcmp(s,"BNZI")  == 0) return ISA_BNZI;
    else if( strcmp(s,"CMPI")  == 0) return ISA_CMPI;
    else if( strcmp(s,"CMPF")  == 0) return ISA_CMPF;
    else if( strcmp(s,"MULU")  == 0) return ISA_MULU;
    else if( strcmp(s,"MUL")   == 0) return ISA_MUL;
    else if( strcmp(s,"ADD")   == 0) return ISA_ADD;
    else if( strcmp(s,"SUB")   == 0) return ISA_SUB;
    else if( strcmp(s,"INC")   == 0) return ISA_INC;
    else if( strcmp(s,"DEC")   == 0) return ISA_DEC;
    else if( strcmp(s,"AND")   == 0) return ISA_AND;
    else if( strcmp(s,"OR")    == 0) return ISA_OR;
    else if( strcmp(s,"XOR")   == 0) return ISA_XOR;
    else if( strcmp(s,"NOT")   == 0) return ISA_NOT;
    else if( strcmp(s,"DIVU")  == 0) return ISA_DIVU;
    else if( strcmp(s,"DIV")   == 0) return ISA_DIV;
    else if( strcmp(s,"REMU")  == 0) return ISA_REMU;
    else if( strcmp(s,"REM")   == 0) return ISA_REM;
    else if( strcmp(s,"LSL")   == 0) return ISA_LSL;
    else if( strcmp(s,"LSR")   == 0) return ISA_LSR;
    else if( strcmp(s,"ROL")   == 0) return ISA_ROL;
    else if( strcmp(s,"ROR")   == 0) return ISA_ROR;
    else if( strcmp(s,"ASL")   == 0) return ISA_ASL;
    else if( strcmp(s,"ASR")   == 0) return ISA_ASR;
    else if( strcmp(s,"FSUB")  == 0) return ISA_FSUB;
    else if( strcmp(s,"FADD")  == 0) return ISA_FADD;
    else if( strcmp(s,"FMUL")  == 0) return ISA_FMUL;
    else if( strcmp(s,"FDIV")  == 0) return ISA_FDIV;
    else if( strcmp(s,"MVIL")  == 0) return ISA_MVIL;
    else if( strcmp(s,"MVIH")  == 0) return ISA_MVIH;
    else if( strcmp(s,"CALL")  == 0) return ISA_CALL;
    else if( strcmp(s,"LD")    == 0) return ISA_LD;
    else if( strcmp(s,"ST")    == 0) return ISA_ST;
    else if( strcmp(s,"BZ")    == 0) return ISA_BZ;
    else if( strcmp(s,"BNZ")   == 0) return ISA_BNZ;
    else if( strcmp(s,"MVIA")  == 0) return ISA_MVIA;
    else if( strcmp(s,"SWI")   == 0) return ISA_SWI;
    else                             return ISA_UNDEF;
}

/*
 *
 * End of static functions
 * ---------------------------------------------------------------------
 * Exported functions definitions
 *
 */

int get_instruction_opcode(tInstruction *i, i_opcode *opcode){

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

int get_instruction_24CONST_operand(tInstruction *i, uint32_t *operand){
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

int set_instruction_24CONST_operand(tInstruction *i, uint32_t operand){
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

         if(strcmp(s, "EQ")  == 0) return 1;
    else if(strcmp(s, "NEQ") == 0) return 1;
    else if(strcmp(s, "L")   == 0) return 1;
    else if(strcmp(s, "LU")  == 0) return 1;
    else if(strcmp(s, "LE")  == 0) return 1;
    else if(strcmp(s, "LEU") == 0) return 1;
    else if(strcmp(s, "G")   == 0) return 1;
    else if(strcmp(s, "GU")  == 0) return 1;
    else if(strcmp(s, "GE")  == 0) return 1;
    else if(strcmp(s, "GEU") == 0) return 1;
    else return 0;

}

int is_reg(char *s){

         if( strcmp(s, "R0")  == 0) return 1;
    else if( strcmp(s, "R1")  == 0) return 1;
    else if( strcmp(s, "R2")  == 0) return 1;
    else if( strcmp(s, "R3")  == 0) return 1;
    else if( strcmp(s, "R4")  == 0) return 1;
    else if( strcmp(s, "R5")  == 0) return 1;
    else if( strcmp(s, "R6")  == 0) return 1;
    else if( strcmp(s, "R7")  == 0) return 1;
    else if( strcmp(s, "R8")  == 0) return 1;
    else if( strcmp(s, "R9")  == 0) return 1;
    else if( strcmp(s, "R10") == 0) return 1;
    else if( strcmp(s, "R11") == 0) return 1;
    else if( strcmp(s, "R12") == 0) return 1;
    else if( strcmp(s, "R13") == 0) return 1;
    else if( strcmp(s, "R14") == 0) return 1;
    else if( strcmp(s, "R15") == 0) return 1;
    else if( strcmp(s, "SP")  == 0) return 1;
    else if( strcmp(s, "PC")  == 0) return 1;
    else                            return 0;

}

const char* is_instruction(char *s){

         if( strcmp(s,"RET")   == 0) return "I";
    else if( strcmp(s,"RETI")  == 0) return "I";
    else if( strcmp(s,"CALLI") == 0) return "IR";
    else if( strcmp(s,"PUSH")  == 0) return "IR";
    else if( strcmp(s,"POP")   == 0) return "IR";
    else if( strcmp(s,"LDI")   == 0) return "IRR";
    else if( strcmp(s,"STI")   == 0) return "IRR";
    else if( strcmp(s,"BZI")   == 0) return "IRR";
    else if( strcmp(s,"BNZI")  == 0) return "IRR";
    else if( strcmp(s,"CMPI")  == 0) return "IcRRR";
    else if( strcmp(s,"CMPF")  == 0) return "IcRRR";
    else if( strcmp(s,"MULU")  == 0) return "IRRR";
    else if( strcmp(s,"MUL")   == 0) return "IRRR";
    else if( strcmp(s,"ADD")   == 0) return "IRRR";
    else if( strcmp(s,"SUB")   == 0) return "IRRR";
    else if( strcmp(s,"INC")   == 0) return "IRR";
    else if( strcmp(s,"DEC")   == 0) return "IRR";
    else if( strcmp(s,"AND")   == 0) return "IRRR";
    else if( strcmp(s,"OR")    == 0) return "IRRR";
    else if( strcmp(s,"XOR")   == 0) return "IRRR";
    else if( strcmp(s,"NOT")   == 0) return "IRR";
    else if( strcmp(s,"DIVU")  == 0) return "IRRR";
    else if( strcmp(s,"DIV")   == 0) return "IRRR";
    else if( strcmp(s,"REMU")  == 0) return "IRRR";
    else if( strcmp(s,"REM")   == 0) return "IRRR";
    else if( strcmp(s,"LSL")   == 0) return "IRRR";
    else if( strcmp(s,"LSR")   == 0) return "IRRR";
    else if( strcmp(s,"ROL")   == 0) return "IRRR";
    else if( strcmp(s,"ROR")   == 0) return "IRRR";
    else if( strcmp(s,"ASL")   == 0) return "IRRR";
    else if( strcmp(s,"ASR")   == 0) return "IRRR";
    else if( strcmp(s,"FSUB")  == 0) return "IRRR";
    else if( strcmp(s,"FADD")  == 0) return "IRRR";
    else if( strcmp(s,"FMUL")  == 0) return "IRRR";
    else if( strcmp(s,"FDIV")  == 0) return "IRRR";
    else if( strcmp(s,"MVIL")  == 0) return "IR6";
    else if( strcmp(s,"MVIH")  == 0) return "IR6";
    else if( strcmp(s,"CALL")  == 0) return "I4";
    else if( strcmp(s,"LD")    == 0) return "I4R";
    else if( strcmp(s,"ST")    == 0) return "IR4";
    else if( strcmp(s,"BZ")    == 0) return "IR4";
    else if( strcmp(s,"BNZ")   == 0) return "IR4";
    else if( strcmp(s,"MVIA")  == 0) return "IR4";
    else if( strcmp(s,"SWI")   == 0) return "I";
    else                             return "N";

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

#if defined ( __GNUC__ )
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#endif

unsigned int get_instruction_size(tInstruction *i){
    return 4;
}

#if defined ( __GNUC__ )
#pragma GCC diagnostic pop
#endif

//TODO: remove this pragmas for ignore flag
#if defined ( __GNUC__ )
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#endif

int check_instruction_args(char *i){
    //TODO: add valid check
    return 1;
}

#if defined ( __GNUC__ )
#pragma GCC diagnostic pop
#endif

int export_into_object_file_line(tInstruction *inst, char *line){
    if(inst == NULL || line == NULL){
        SET_ERROR(ISAERR_NULL_PTR);
        return 0;
    }

    if(inst->relocation > 1 || inst->special > 1){
        SET_ERROR(ISAERR_INTER_ERR);
        return 0;
    }

    int ret = snprintf(line, 32, "0x%"PRIx32":%"PRId8":%"PRId8, inst->word, inst->relocation, inst->special);

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

    for(int i = 0; line[i] != '\0'; i++) if(line[i] == ':') line[i] = ' ';

    if(sscanf(line, "%"SCNx32" %"SCNd8" %"SCNd8, &(inst->word), &(inst->relocation), &(inst->special)) != 3){
        SET_ERROR(ISAERR_FORMAT_ERR);
        return 0;
    }

    if(inst->relocation > 1 || inst->special > 1){
        SET_ERROR(ISAERR_INTER_ERR);
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
    tmp->relocation = 0;
    tmp->special = 0;

    return tmp;

}

int assemble_instruction(tInstruction * inst, void * section_ptr){
    //TODO: implement this!

    if(inst == NULL || section_ptr == NULL){
        SET_ERROR(ISAERR_NULL_PTR);
        return 0;
    }

    if(inst->line != NULL){
        SET_ERROR(ISAERR_NULL_PTR);
        return 0;
    }

    //create local copy of line
    char * line_copy = (char *)malloc(sizeof(char) * (strlen(inst->line) + 1));
    char * opcode_copy = (char *)malloc(sizeof(char) * (strlen(inst->line) + 1));

    if(line_copy == NULL || opcode_copy == NULL){
        SET_ERROR(ISAERR_MALLOC_FAIL);
        return 0;
    }

    strcpy(line_copy, inst->line);
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
    i_opcode op = get_opcode_from_name(opcode_copy);

    switch(op){
        case ISA_UNDEF:
            {
                SET_ERROR(ISAERR_INTER_ERR);
                return 0;
            }
            break;
        case ISA_RET:
            {
                inst->word = 0x01000000;
            }
            break;
        case ISA_RETI:
            {
                inst->word = 0x0200000;
            }
            break;
        case ISA_CALLI:
            {
            }
            break;
        case ISA_PUSH:
            {
            }
            break;
        case ISA_POP:
            {
            }
            break;
        case ISA_LDI:
            {
            }
            break;
        case ISA_STI:
            {
            }
            break;
        case ISA_BZI:
            {
            }
            break;
        case ISA_BNZI:
            {
            }
            break;
        case ISA_CMPI:
            {
            }
            break;
        case ISA_CMPF:
            {
            }
            break;
        case ISA_MULU:
            {
            }
            break;
        case ISA_MUL:
            {
            }
            break;
        case ISA_ADD:
            {
            }
            break;
        case ISA_SUB:
            {
            }
            break;
        case ISA_INC:
            {
            }
            break;
        case ISA_DEC:
            {
            }
            break;
        case ISA_AND:
            {
            }
            break;
        case ISA_OR:
            {
            }
            break;
        case ISA_XOR:
            {
            }
            break;
        case ISA_NOT:
            {
            }
            break;
        case ISA_DIVU:
            {
            }
            break;
        case ISA_DIV:
            {
            }
            break;
        case ISA_REMU:
            {
            }
            break;
        case ISA_REM:
            {
            }
            break;
        case ISA_LSL:
            {
            }
            break;
        case ISA_LSR:
            {
            }
            break;
        case ISA_ROL:
            {
            }
            break;
        case ISA_ROR:
            {
            }
            break;
        case ISA_ASL:
            {
            }
            break;
        case ISA_ASR:
            {
            }
            break;
        case ISA_FSUB:
            {
            }
            break;
        case ISA_FADD:
            {
            }
            break;
        case ISA_FMUL:
            {
            }
            break;
        case ISA_FDIV:
            {
            }
            break;
        case ISA_MVIL:
            {
            }
            break;
        case ISA_MVIH:
            {
            }
            break;
        case ISA_CALL:
            {
            }
            break;
        case ISA_LD:
            {
            }
            break;
        case ISA_ST:
            {
            }
            break;
        case ISA_BZ:
            {
            }
            break;
        case ISA_BNZ:
            {
            }
            break;
        case ISA_MVIA:
            {
            }
            break;
        case ISA_SWI:
            {
                inst->word = 0x14000000;
            }
            break;
        default:
            SET_ERROR(ISAERR_INTER_ERR);
            return 0;
    }

    free(line_copy);
    free(opcode_copy);

    return 1;
}

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

/*
 *
 * End of exported functions definitions
 * ---------------------------------------------------------------------
 */
