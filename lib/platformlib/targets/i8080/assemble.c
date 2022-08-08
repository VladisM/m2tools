#include "assemble.h"

#include "../../src/platformlib_common.h"
#include "../../src/platformlib_private.h"

#include "opcode_decode.h"

#include <string.h>

static bool get_reg_code(char *regname, isa_instruction_word_t *regcode){
    if(strcmp(regname, "A") == 0 || strcmp(regname, "ACC") == 0){
        *regcode = DST_A_CODE;
    }
    else if(strcmp(regname, "B") == 0){
        *regcode = DST_B_CODE;
    }
    else if(strcmp(regname, "C") == 0){
        *regcode = DST_C_CODE;
    }
    else if(strcmp(regname, "D") == 0){
        *regcode = DST_D_CODE;
    }
    else if(strcmp(regname, "E") == 0){
        *regcode = DST_E_CODE;
    }
    else if(strcmp(regname, "H") == 0){
        *regcode = DST_H_CODE;
    }
    else if(strcmp(regname, "L") == 0){
        *regcode = DST_L_CODE;
    }
    else if(strcmp(regname, "M") == 0){
        *regcode = DST_M_CODE;
    }
    else{
        ERROR_WRITE("Cannot convert %s to register code!", regname);
        return false;
    }

    return true;
}

static bool get_regp_code(char *regpname, isa_instruction_word_t *regpcode){
    if(strcmp(regpname, "BC") == 0 || strcmp(regpname, "B:C") == 0){
        *regpcode = RP_BC_CODE;
    }
    else if(strcmp(regpname, "DE") == 0 || strcmp(regpname, "D:E") == 0){
        *regpcode = RP_BC_CODE;
    }
    else if(strcmp(regpname, "HL") == 0 || strcmp(regpname, "H:L") == 0){
        *regpcode = RP_BC_CODE;
    }
    else if(strcmp(regpname, "SP") == 0 || strcmp(regpname, "S:P") == 0){
        *regpcode = RP_BC_CODE;
    }
    else{
        ERROR_WRITE("Cannot convert %s to register pair name!", regpname);
        return false;
    }

    return true;
}

static bool get_db(char *db, isa_instruction_word_t *dbvalue){
    long long tmp_1 = 0;
    isa_instruction_word_t tmp_2 = 0;

    if(!is_number(db)){
        ERROR_WRITE("Trying to pass %s as data byte, but can't perform conversion to number!", db);
        return false;
    }

    if(!str_to_num(db, &tmp_1)){
        error("is_number() returned true but str_to_num returned false!");
    }

    if(!can_fit_in(tmp_1, sizeof(isa_instruction_word_t))){
        ERROR_WRITE("Converted %s but its value is too large to hold in target data isa_instruction_word_t!", db);
        return false;
    }

    tmp_2 = tmp_1;

    if((tmp_2 & ~(0x000000FF)) != 0){
        ERROR_WRITE("Converted %s to be used as data byte, but its value overflow!", db);
        return false;
    }

    *dbvalue = tmp_2;
    return true;
}

static bool get_pa(char *pa, isa_instruction_word_t *pavalue){
    return get_db(pa, pavalue);
}

static bool get_rst_n(char *n, isa_instruction_word_t *nvalue){
    long long tmp_1 = 0;
    isa_instruction_word_t tmp_2 = 0;

    if(!is_number(n)){
        ERROR_WRITE("Trying to pass %s as RST N, but can't perform conversion to number!", n);
        return false;
    }

    if(!str_to_num(n, &tmp_1)){
        error("is_number() returned true but str_to_num returned false!");
    }

    if(!can_fit_in(tmp_1, sizeof(isa_instruction_word_t))){
        ERROR_WRITE("Converted %s but its value is too large to hold in target data isa_instruction_word_t!", n);
        return false;
    }

    tmp_2 = tmp_1;

    if((tmp_2 & 0x00000008) != 0){
        ERROR_WRITE("Converted %s to be used as N in RST, but its value overflow!", n);
        return false;
    }

    *nvalue = tmp_2;
    return true;
}

static bool get_lb_hb(
    char *lb_hb,
    isa_instruction_word_t *lb, isa_instruction_word_t *hb,
    bool (*find_symbol_callback)(char *label, void *section, isa_address_t *result),
    void *section)
{
    isa_address_t addr = 0;

    if(!is_number(lb_hb)){
        if(!find_symbol_callback(lb_hb, section, &addr)){
            ERROR_WRITE("Failed to convert %s as number and even as label!", lb_hb);
            return false;
        }
    }
    else{
        long long tmp = 0;

        if(!str_to_num(lb_hb, &tmp)){
            error("is_number() returned true but str_to_num returned false!");
        }

        if(!can_fit_in(tmp, sizeof(isa_address_t))){
            ERROR_WRITE("Converted %s but its value is too large to hold in target data isa_address_t!", lb_hb);
            return false;
        }

        addr = tmp;
    }

    *lb = (0x00FF & addr);
    *hb = (0xFF00 & addr) >> 8;
    return true;
}

bool platformlib_assemble_instruction(
    char **args,
    int argc,
    bool (*find_symbol_callback)(char *label, void *section, isa_address_t *result),
    void *section,
    isa_instruction_word_t *result)
{
    CHECK_NULL_ARGUMENT(args);
    CHECK_NULL_ARGUMENT(section);
    CHECK_NULL_ARGUMENT(result);
    CHECK_NULL_ARGUMENT(find_symbol_callback);
    for(int i = 0; i < argc; i++){
        CHECK_NULL_ARGUMENT(args + i);
    }

    isa_instruction_word_t instruction = 0;
    instruction_signature_t *signature = platformlib_get_instruction_signature(args[0]);

    isa_instruction_word_t tmp_a = 0;
    isa_instruction_word_t tmp_b = 0;
    isa_instruction_word_t tmp_c = 0;

    if(signature == NULL){
        error("Tryting to get type of something that isn't instruction!");
    }

    instruction = signature->instruction_code;

    if((unsigned)argc != signature->argc + 1){
        ERROR_WRITE("Wrong arguments to %s! Needed args: %d given: %d", (signature)->opcode, (signature)->argc, argc);
        return false;
    }

    switch(signature->instruction_mnemonic){
        case INSTRU_MOV:
            if(!get_reg_code(args[1], &tmp_a)){
                goto _on_error;
            }

            if(!get_reg_code(args[2], &tmp_b)){
                goto _on_error;
            }

            instruction |= SHIFT_TO_SRC(tmp_b) | SHIFT_TO_DST(tmp_a);
            break;

        case INSTRU_XCHG:
        case INSTRU_DAA:
        case INSTRU_RLC:
        case INSTRU_RRC:
        case INSTRU_RAL:
        case INSTRU_RAR:
        case INSTRU_CMA:
        case INSTRU_CMC:
        case INSTRU_STC:
        case INSTRU_RET:
        case INSTRU_RNZ:
        case INSTRU_RZ:
        case INSTRU_RNC:
        case INSTRU_RC:
        case INSTRU_RPO:
        case INSTRU_RPE:
        case INSTRU_RP:
        case INSTRU_RM:
        case INSTRU_PCHL:
        case INSTRU_XTHL:
        case INSTRU_SPHL:
        case INSTRU_EI:
        case INSTRU_DI:
        case INSTRU_HLT:
        case INSTRU_NOP:
            break;  //already done

        case INSTRU_ADD:
        case INSTRU_ADC:
        case INSTRU_SUB:
        case INSTRU_SBB:
        case INSTRU_ANA:
        case INSTRU_ORA:
        case INSTRU_XRA:
        case INSTRU_CMP:
            if(!get_reg_code(args[1], &tmp_a)){
                goto _on_error;
            }

            instruction |= SHIFT_TO_SRC(tmp_a);
            break;

        case INSTRU_INR:
        case INSTRU_DCR:
            if(!get_reg_code(args[1], &tmp_a)){
                goto _on_error;
            }

            instruction |= SHIFT_TO_DST(tmp_a);
            break;

        case INSTRU_LDAX:
        case INSTRU_STAX:
            if(!get_regp_code(args[1], &tmp_a)){
                goto _on_error;
            }

            if(tmp_a != RP_BC_CODE && tmp_a != RP_DE_CODE){
                ERROR_WRITE("For instruction %s only BS or DE reg pairs are allowed!", signature->opcode);
                goto _on_error;
            }

            instruction |= SHIFT_TO_RP(tmp_a);
            break;

        case INSTRU_INX:
        case INSTRU_DCX:
        case INSTRU_DAD:
        case INSTRU_PUSH:
        case INSTRU_POP:
            if(!get_regp_code(args[1], &tmp_a)){
                goto _on_error;
            }

            instruction |= SHIFT_TO_RP(tmp_a);
            break;

        case INSTRU_MVI:
            if(!get_reg_code(args[1], &tmp_a)){
                goto _on_error;
            }

            if(!get_db(args[2], &tmp_b)){
                goto _on_error;
            }

            instruction = APPEND_DB((instruction | SHIFT_TO_DST(tmp_a)), tmp_b);
            break;

        case INSTRU_IN:
        case INSTRU_OUT:
            if(!get_pa(args[1], &tmp_a)){
                goto _on_error;
            }

            instruction = APPEND_DB(instruction, tmp_a);
            break;

        case INSTRU_ADI:
        case INSTRU_ACI:
        case INSTRU_SUI:
        case INSTRU_SBI:
        case INSTRU_ANI:
        case INSTRU_ORI:
        case INSTRU_XRI:
        case INSTRU_CPI:
            if(!get_db(args[1], &tmp_a)){
                goto _on_error;
            }

            instruction = APPEND_DB(instruction, tmp_a);
            break;

        case INSTRU_LDA:
        case INSTRU_STA:
        case INSTRU_LHLD:
        case INSTRU_SHLD:
        case INSTRU_JMP:
        case INSTRU_JNZ:
        case INSTRU_JZ:
        case INSTRU_JNC:
        case INSTRU_JC:
        case INSTRU_JPO:
        case INSTRU_JPE:
        case INSTRU_JP:
        case INSTRU_JM:
        case INSTRU_CALL:
        case INSTRU_CNZ:
        case INSTRU_CZ:
        case INSTRU_CNC:
        case INSTRU_CC:
        case INSTRU_CPO:
        case INSTRU_CPE:
        case INSTRU_CP:
        case INSTRU_CM:
            if(!get_lb_hb(args[1], &tmp_a, &tmp_b, find_symbol_callback, section)){
                goto _on_error;
            }

            instruction = APPEND_LB_HB(instruction, tmp_a, tmp_b);
            break;

        case INSTRU_LXI:
            if(!get_regp_code(args[1], &tmp_a)){
                goto _on_error;
            }

            if(!get_lb_hb(args[2], &tmp_b, &tmp_c, find_symbol_callback, section)){
                goto _on_error;
            }

            instruction = APPEND_LB_HB((instruction | SHIFT_TO_RP(tmp_a)), tmp_b, tmp_c);
            break;

        case INSTRU_RST:
            if(!get_rst_n(args[1], &tmp_a)){
                goto _on_error;
            }

            instruction |= SHIFT_TO_DST(tmp_a);
            break;

        case INSTRU_UNKOWN:
            error("Error in backend, have to assemble INSTRU_UNKNOWN!");
            break;
    }

    *result = instruction;
    return true;

_on_error:
    return false;
}

bool platformlib_relocate_instruction(
    isa_instruction_word_t input,
    isa_instruction_word_t *output,
    isa_address_t offset)
{
    CHECK_NULL_ARGUMENT(output);

    instruction_signature_t *signature = platformlib_get_instruction_signature_1(input);
    isa_address_t addr = 0;

    switch(signature->instruction_mnemonic){
        case INSTRU_LDA:
        case INSTRU_STA:
        case INSTRU_LHLD:
        case INSTRU_SHLD:
        case INSTRU_JMP:
        case INSTRU_JNZ:
        case INSTRU_JZ:
        case INSTRU_JNC:
        case INSTRU_JC:
        case INSTRU_JPO:
        case INSTRU_JPE:
        case INSTRU_JP:
        case INSTRU_JM:
        case INSTRU_CALL:
        case INSTRU_CNZ:
        case INSTRU_CZ:
        case INSTRU_CNC:
        case INSTRU_CC:
        case INSTRU_CPO:
        case INSTRU_CPE:
        case INSTRU_CP:
        case INSTRU_CM:
        case INSTRU_LXI:
            addr = GET_HBLB(input);
            addr += offset;
            *output = APPEND_LB_HB(((input & 0xFFFF0000) >> 16), GET_LB(addr), GET_HB(addr));
            break;
        default:
            error("Cannot relocate instruction that doesn't have LB_HB part inside!");  //will crash app
            break;
    }

    return true;
}

bool platformlib_retarget_instruction(
    isa_instruction_word_t input,
    isa_instruction_word_t *output,
    isa_address_t target)
{
    CHECK_NULL_ARGUMENT(output);

    instruction_signature_t *signature = platformlib_get_instruction_signature_1(input);

    switch(signature->instruction_mnemonic){
        case INSTRU_LDA:
        case INSTRU_STA:
        case INSTRU_LHLD:
        case INSTRU_SHLD:
        case INSTRU_JMP:
        case INSTRU_JNZ:
        case INSTRU_JZ:
        case INSTRU_JNC:
        case INSTRU_JC:
        case INSTRU_JPO:
        case INSTRU_JPE:
        case INSTRU_JP:
        case INSTRU_JM:
        case INSTRU_CALL:
        case INSTRU_CNZ:
        case INSTRU_CZ:
        case INSTRU_CNC:
        case INSTRU_CC:
        case INSTRU_CPO:
        case INSTRU_CPE:
        case INSTRU_CP:
        case INSTRU_CM:
        case INSTRU_LXI:
            *output = APPEND_LB_HB(((input & 0xFFFF0000) >> 16), GET_LB(target), GET_HB(target));
            break;
        default:
            error("Cannot retarget instruction that doesn't have LB_HB part inside!");  //will crash app
            break;
    }

    return true;
}
