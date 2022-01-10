/**
 * @file opcode_decode.h
 * @brief This file contain bunch of defines to compute instruction opcodes.
 */

#ifndef OPCODE_DECODE_H_included
#define OPCODE_DECODE_H_included

#define SHIFT(x, y) ((x) << (y))

#define CCC_SHIFT_OFFSET 3
#define RP_SHIFT_OFFSET 4
#define DST_SHIFT_OFFSET 3
#define SRC_SHIFT_OFFSET 0

#define SHIFT_TO_CCC(x) (SHIFT((x), CCC_SHIFT_OFFSET))
#define SHIFT_TO_RP(x)  (SHIFT((x), RP_SHIFT_OFFSET))
#define SHIFT_TO_DST(x) (SHIFT((x), DST_SHIFT_OFFSET))
#define SHIFT_TO_SRC(x) (SHIFT((x), SRC_SHIFT_OFFSET))

#define APPEND_DB(x, d)        ((SHIFT((x), 8)) | (d))
#define APPEND_LB_HB(x, l, h)  ((SHIFT((x), 16)) | (SHIFT((l), 8)) | (h))
#define APPEND_PA(x, P)        ((SHIFT((x), 8)) | (p))

#define GET_LB(x)       (((x) & 0xFF00) >> 8)
#define GET_HB(x)       ((x) & 0xFF)
#define GET_HBLB(x)     ((GET_HB(x) << 8) | GET_LB(x))

#define CCC_NZ_CODE     0x00
#define CCC_Z_CODE      0x01
#define CCC_NC_CODE     0x02
#define CCC_C_CODE      0x03
#define CCC_PO_CODE     0x04
#define CCC_PE_CODE     0x05
#define CCC_P_CODE      0x06
#define CCC_M_CODE      0x07

#define RP_BC_CODE      0x00
#define RP_DE_CODE      0x01
#define RP_HL_CODE      0x02
#define RP_SP_CODE      0x03

#define DST_A_CODE      0x07
#define DST_B_CODE      0x00
#define DST_C_CODE      0x01
#define DST_D_CODE      0x02
#define DST_E_CODE      0x03
#define DST_H_CODE      0x04
#define DST_L_CODE      0x05
#define DST_M_CODE      0x06

#define SRC_A_CODE DST_A_CODE
#define SRC_B_CODE DST_B_CODE
#define SRC_C_CODE DST_C_CODE
#define SRC_D_CODE DST_D_CODE
#define SRC_E_CODE DST_E_CODE
#define SRC_H_CODE DST_H_CODE
#define SRC_L_CODE DST_L_CODE
#define SRC_M_CODE DST_M_CODE

#define MOV_BASE        0x40
#define MVI_BASE        0x06
#define LXI_BASE        0x01
#define LDA_BASE        0x3A
#define STA_BASE        0x32
#define LHLD_BASE       0x2A
#define SHLD_BASE       0x22
#define LDAX_BASE       0x0A
#define STAX_BASE       0x02
#define XCHG_BASE       0xEB
#define ADD_BASE        0x80
#define ADI_BASE        0xC6
#define ADC_BASE        0x88
#define ACI_BASE        0xCE
#define SUB_BASE        0x90
#define SUI_BASE        0xD6
#define SBB_BASE        0x98
#define SBI_BASE        0xDE
#define INR_BASE        0x04
#define DCR_BASE        0x05
#define INX_BASE        0x03
#define DCX_BASE        0x0B
#define DAD_BASE        0x09
#define DAA_BASE        0x27
#define ANA_BASE        0xA0
#define ANI_BASE        0xE6
#define ORA_BASE        0xB0
#define ORI_BASE        0xF6
#define XRA_BASE        0xA8
#define XRI_BASE        0xEE
#define CMP_BASE        0xB8
#define CPI_BASE        0xFE
#define RLC_BASE        0x07
#define RRC_BASE        0x0F
#define RAL_BASE        0x17
#define RAR_BASE        0x1F
#define CMA_BASE        0x2F
#define CMC_BASE        0x3F
#define STC_BASE        0x37
#define JMP_BASE        0xC3
#define Jccc_BASE       0xC2
#define CALL_BASE       0xCD
#define Cccc_BASE       0xC4
#define RET_BASE        0xC9
#define Rcc_BASE        0xC0
#define RST_BASE        0xC7
#define PCHL_BASE       0xE9
#define PUSH_BASE       0xC5
#define POP_BASE        0xC1
#define XTHL_BASE       0xE3
#define SPHL_BASE       0xF9
#define IN_BASE         0xDB
#define OUT_BASE        0xD3
#define EI_BASE         0xFB
#define DI_BASE         0xF3
#define HLT_BASE        0x76
#define NOP_BASE        0x00

#endif
