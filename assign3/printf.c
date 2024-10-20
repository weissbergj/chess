/* File: printf.c
 * --------------
 * ***** This is the printf function in C *****
 */
#include "printf.h"
#include <stdarg.h>
#include <stdint.h>
#include "strings.h"
#include "uart.h"


void num_to_string(unsigned long num, int base, char *outstr);
const char *hex_string(unsigned long val);
const char *decimal_string(long val);

#define MAX_DIGITS 25

const char *hex_string(unsigned long val) {
    static char buf[MAX_DIGITS]; // note that a static buffer here is unusual
    num_to_string(val, 16, buf); // num_to_string does the hard work
    return buf;
}

const char *decimal_string(long val) {
    static char buf[MAX_DIGITS];
    if (val < 0) {
        buf[0] = '-';   // add negative sign in front first
        num_to_string(-val, 10, buf + 1); // pass positive val as arg, start writing at buf + 1
    } else {
        num_to_string(val, 10, buf);
    }
    return buf;
}

void num_to_string(unsigned long num, int base, char *outstr) {
    int digits_in_num = 0;
    unsigned long temp = num;

    do {
        temp /= base;
        digits_in_num++;
    } while (temp > 0);     // makes sure the 0-case has one digit_in_num

    for (int i = digits_in_num - 1; i >= 0; i--) {
        int modulo = num % base;
        outstr[i] = (char)(modulo <=9 ? modulo + '0' : modulo + 'a' - 10); //if 0-9 add ASCII 0; else add 'a' - 10
        num /= base;
    }
    outstr[digits_in_num] = '\0';
}

int parser(const char **str) {
    const char *end;
    unsigned long result = strtonum(*str, &end);
    *str = end;
    return (int)result;
}

struct insn {
    uint32_t opcode: 7;
    uint32_t reg_d:  5;
    uint32_t funct3: 3;
    uint32_t reg_s1: 5;
    uint32_t reg_s2: 5;
    uint32_t funct7: 7;
};

typedef enum {
    OPCODE_RTYPE   = 0b0110011, // R-type: ADD, SUB, SLL, SLT, SLTU, XOR, SRL, SRA, OR, AND
    OPCODE_ITYPE   = 0b0010011, // I-type: ADDI, SLTI, SLTIU, XORI, ORI, ANDI, SLLI, SRLI, SRAI
    OPCODE_LOAD    = 0b0000011, // I-type: LB, LH, LW, LBU, LHU (Load instructions)
    OPCODE_STYPE   = 0b0100011, // S-type: SB, SH, SW (Store instructions)
    OPCODE_BTYPE   = 0b1100011, // B-type: BEQ, BNE, BLT, BGE, BLTU, BGEU (Branch instructions)
    OPCODE_JAL     = 0x6F, // J-type: JAL (Jump and Link)
    OPCODE_JALR    = 0x67, // I-type: JALR (Jump and Link Register)
    OPCODE_LUI     = 0x37, // U-type: LUI (Load Upper Immediate)
    OPCODE_AUIPC   = 0x17, // U-type: AUIPC (Add Upper Immediate to PC)
    OPCODE_FENCE   = 0x0F, // FENCE: Memory barrier
    OPCODE_SYSTEM  = 0x73,  // System: ECALL, EBREAK, CSR operations
    OPCODE_ADDIW     = 0b0011011, // ADDIW and shift immediate word instructions
    OPCODE_ADDW      = 0b0111011,  // Add/subtract word and shift word instructions
    OPCODE_LOAD_FP   = 0x07,
    OPCODE_STORE_FP  = 0x27,
    OPCODE_MADD      = 0x43,
    OPCODE_MSUB      = 0x47,
    OPCODE_NMSUB     = 0x4B,
    OPCODE_NMADD     = 0x4F,
    OPCODE_OP_FP     = 0x53
} OPCODE;

static const char *reg_names[32] = {"zero", "ra", "sp", "gp", "tp", "t0", "t1", "t2",
                                    "s0", "s1", "a0", "a1", "a2", "a3", "a4", "a5",
                                    "a6", "a7", "s2", "s3", "s4", "s5", "s6", "s7",
                                    "s8", "s9", "s10", "s11", "t3", "t4", "t5", "t6" };  // s0/fp

const char *funct3_rtype0[] = {"ADD", "SLL", "SLT", "SLTU", "XOR", "SRL", "OR", "AND"};
const char *funct3_rtype1[] = {"SUB", NULL, NULL, NULL, NULL, "SRA"};
const char *funct3_itype0[] = {"ADDI", NULL, "SLTI", "SLTIU", "XORI", NULL, "ORI", "ANDI"};
const char *funct3_load[] = {"LB", "LH", "LW", "LD", "LBU", "LHU", "LWU"};
const char *funct3_store[] = {"SB", "SH", "SW", "SD"};
const char *funct3_btype[] = {"BEQ", "BNE", NULL, NULL, "BLT", "BGE", "BLTU", "BGEU"};
const char *funct3_addiw[] = {"ADDIW", "SLLIW", NULL, NULL, NULL, "SRLIW/SRAIW"};
const char *funct3_addw[] = {"ADDW/SUBW", "SLLW", NULL, NULL, NULL, "SRLW/SRAW"};
const char *funct3_zicsr[] = {"CSRRW", "CSRRS", "CSRRC", "CSRRWI", "CSRRSI", "CSRRCI"};
const char *funct3_rv32m[] = {"MUL", "MULH", "MULHSU", "MULHU", "DIV", "DIVU", "REM", "REMU"};
const char *funct3_rv64m[] = {"MULW", "DIVW", "DIVUW", "REMW", "REMUW"};
const char *funct5_rv32a[] = {NULL, NULL, "LR.W", "SC.W", "AMOSWAP.W", "AMOADD.W", "AMOXOR.W", "AMOAND.W", "AMOOR.W", "AMOMIN.W", "AMOMAX.W", "AMOMINU.W", "AMOMAXU.W"};
const char *funct5_rv64a[] = {NULL, NULL, "LR.D", "SC.D", "AMOSWAP.D", "AMOADD.D", "AMOXOR.D", "AMOAND.D", "AMOOR.D", "AMOMIN.D", "AMOMAX.D", "AMOMINU.D", "AMOMAXU.D"};
const char *funct3_float_load[] = {"FLW", "FLD", "FLQ"};
const char *funct3_float_store[] = {"FSW", "FSD", "FSQ"};
const char *rm_names[8] = { "rne", "rtz", "rdn", "rup", "rmm", "Reserved", "Reserved", "dyn" };
const char *f_reg_names[] = {"ft0", "ft1", "ft2", "ft3", "ft4", "ft5", "ft6", "ft7", "fs0", "fs1", "fa0", "fa1", "fa2", "fa3", "fa4", "fa5", "fa6", "fa7", "fs2", "fs3", "fs4", "fs5", "fs6", "fs7", "fs8", "fs9", "fs10", "fs11", "ft8", "ft9", "ft10", "ft11"};

static const struct {
    uint32_t csr_addr;
    const char *csr_name;
} csr_table[] = {
    {0x341, "mepc"},
    {0x342, "mcause"},
    {0x343, "mtval"},
    // can add more!
};

static const char* get_csr_name(uint32_t csr_addr) {
    for (size_t i = 0; i < sizeof(csr_table) / sizeof(csr_table[0]); i++) {
        if (csr_table[i].csr_addr == csr_addr) {
            return csr_table[i].csr_name;
        }
    }
    return NULL;
}

int disassemble(void *p, char *disassemble_buf, size_t buf_size) {
    struct insn instruction = *(struct insn *)p;

    int rd = instruction.reg_d;
    int funct3 = instruction.funct3;
    int rs1 = instruction.reg_s1;
    int rs2 = instruction.reg_s2;
    int funct7 = instruction.funct7;
    const char *rd_char = reg_names[rd];
    const char *rs1_char = reg_names[rs1];
    const char *rs2_char = reg_names[rs2];

    int immediate_I = ((int32_t)(instruction.funct7 << 5 | instruction.reg_s2) << 20) >> 20;
    int immediate_S = ((int32_t)((instruction.funct7 << 5) | instruction.reg_d) << 20) >> 20;

    int immediate_B = ((int32_t)(
        ((instruction.funct7 & 0x40) << 6) |  // imm[12]
        ((instruction.funct7 & 0x3f) << 5) |  // imm[10:5]
        ((instruction.reg_d & 0x1) << 11) |   // imm[11]
        ((instruction.reg_d & 0x1e) << 1)     // imm[4:1]
    ) << 19) >> 19;  // Sign extend

    int immediate_U = instruction.funct7 << 25 | instruction.reg_s2 << 20 | instruction.reg_s1 << 15 | instruction.funct3 << 12;
    int immediate_J = ((int32_t)(
        (instruction.funct7 << 25) |               // imm[20|10:5]
        (instruction.reg_s2 << 20) |               // imm[4:1]
        (instruction.reg_s1 << 15) |               // imm[11]
        (instruction.funct3 << 12) |               // imm[12]
        (instruction.reg_d)                        // imm[19:12]
    ) << 11) >> 11;  // Sign extend

    // For J
    uint32_t raw_insn = *(uint32_t *)p;  // Get the raw 32-bit instruction
    int imm20 = (*(uint32_t *)p >> 31) & 0x1;
    int imm10_1 = (*(uint32_t *)p >> 21) & 0x3FF;
    int imm11 = (*(uint32_t *)p >> 20) & 0x1;
    int imm19_12 = (*(uint32_t *)p >> 12) & 0xFF;

    uint32_t funct2 = (raw_insn >> 25) & 0x3;  // Add this line to extract funct2

    int immediate_J_ = (imm20 << 20) | (imm19_12 << 12) | (imm11 << 11) | (imm10_1 << 1);
    immediate_J = ((int32_t)immediate_J_ << 11) >> 11;  // Sign extend

    switch (instruction.opcode) {
        case OPCODE_RTYPE: // R-type: Handle register-to-register operations
            {
            if (funct7 == 0x00) {
                const char *funct3_char = funct3_rtype0[funct3];
                snprintf(disassemble_buf, buf_size, "%s %s, %s, %s", funct3_char, rd_char, rs1_char, rs2_char);
            } else if (funct7 == 0b0100000) {
                const char *funct3_char = funct3_rtype1[funct3];
                snprintf(disassemble_buf, buf_size, "%s %s, %s, %s", funct3_char, rd_char, rs1_char, rs2_char);
            } else if (funct7 == 0x01) {
                const char *funct3_char;
                if (funct3 < 4) {
                    funct3_char = funct3_rv32m[funct3];
                } else {
                    funct3_char = funct3_rv64m[funct3 - 4];
                }
                snprintf(disassemble_buf, buf_size, "%s %s, %s, %s", funct3_char, rd_char, rs1_char, rs2_char);
            }
            else {
                snprintf(disassemble_buf, buf_size, "Unknown R-type %s, %s, %s", rd_char, rs1_char, rs2_char);
            }
            }
            break;
        case OPCODE_ITYPE: // I-type: Handle immediate arithmetic operations
            {
            if (funct3 == 0 || funct3 == 2 || funct3 == 3 || funct3 == 4 || funct3 == 6 || funct3 == 7) {  // ADDI, SLTI, SLTIU, XORI, ORI, ANDI
                const char *funct3_char = funct3_itype0[funct3];
                snprintf(disassemble_buf, buf_size, "%s %s, %s, %d", funct3_char, rd_char, rs1_char, immediate_I);
            } else if (funct3 == 1 || funct3 == 5) {  // SLLI, SRLI/SRAI
                const char *shift_type = (funct3 == 1) ? "SLLI" : ((immediate_I & 0xfe0) == 0 ? "SRLI" : "SRAI");
                int shamt = immediate_I & 0x3F;  // Lower 6 bits for RV64I
                snprintf(disassemble_buf, buf_size, "%s %s, %s, %d", shift_type, rd_char, rs1_char, shamt);
            }
            }
            break;
        case OPCODE_LOAD: // Load instructions (I-type, including 64-bit loads)
            {
            const char *funct3_char = funct3_load[funct3];
            if (funct3_char) {
                snprintf(disassemble_buf, buf_size, "%s %s, %d(%s)", funct3_char, rd_char, immediate_I, rs1_char);
            } else {
                snprintf(disassemble_buf, buf_size, "Unknown load instruction");
            }
            }
            break;
        case OPCODE_STYPE: // Store instructions (S-type, including 64-bit stores)
            {
            const char *funct3_char = funct3_store[funct3];
            if (funct3 <= 3 && funct3_char) {
                snprintf(disassemble_buf, buf_size, "%s %s, %d(%s)", 
                        funct3_char, rs2_char, immediate_S, rs1_char);
            } else {
                snprintf(disassemble_buf, buf_size, "Unknown store instruction");
            }
            }
            break;
        case OPCODE_BTYPE: // Branch instructions (B-type)
            {
            const char *funct3_char = funct3_btype[funct3];
            if (funct3_char) {
                snprintf(disassemble_buf, buf_size, "%s %s, %s, %d", funct3_char, rs1_char, rs2_char, (immediate_B / 2));
            } else {
                snprintf(disassemble_buf, buf_size, "Unknown branch instruction");
            }
            }
            break;
        case OPCODE_JAL:
            snprintf(disassemble_buf, buf_size, "JAL %s, %d", rd_char, immediate_J);
            break;
        case OPCODE_JALR: // JALR (I-type control)
            {
            snprintf(disassemble_buf, buf_size, "JALR %s, %d(%s)", rd_char, immediate_I, rs1_char);
            }
            break;
        case OPCODE_LUI:
            snprintf(disassemble_buf, buf_size, "LUI %s, %x", rd_char, immediate_U >> 12);
            break;
        case OPCODE_AUIPC:
            snprintf(disassemble_buf, buf_size, "AUIPC %s, %x", rd_char, immediate_U >> 12);
            break;
        case OPCODE_FENCE:
            if (funct3 == 0) {
                snprintf(disassemble_buf, buf_size, "FENCE");
            } else if (funct3 == 1) {
                snprintf(disassemble_buf, buf_size, "FENCE.I");
            } else {
                snprintf(disassemble_buf, buf_size, "Unknown FENCE instruction");
            }
            break;
        case OPCODE_SYSTEM: // System instructions (ECALL, EBREAK)
            {
                uint32_t funct3 = (raw_insn >> 12) & 0x7;
                uint32_t csr = (raw_insn >> 20) & 0xFFF;
                const char *csr_name = get_csr_name(csr);
                
                if (funct3 == 0) {
                    // ECALL, EBREAK, etc.
                    const char *sys_op = (raw_insn == 0x00000073) ? "ECALL" :
                                         (raw_insn == 0x00100073) ? "EBREAK" : "Unknown SYSTEM";
                    snprintf(disassemble_buf, buf_size, "%s", sys_op);
                } else {
                    // CSR instructions
                    const char *csr_op = (funct3 == 1) ? "CSRRW" :
                                         (funct3 == 2) ? "CSRRS" :
                                         (funct3 == 3) ? "CSRRC" :
                                         (funct3 == 5) ? "CSRRWI" :
                                         (funct3 == 6) ? "CSRRSI" :
                                         (funct3 == 7) ? "CSRRCI" : "Unknown CSR";
                    
                    if (csr_name) {
                        snprintf(disassemble_buf, buf_size, "%s %s, %s", csr_op, csr_name, reg_names[rs1]);
                    } else {
                        snprintf(disassemble_buf, buf_size, "%s 0x%x, %s", csr_op, csr, reg_names[rs1]);
                    }
                }
            }
            break;
        case OPCODE_ADDIW:
            {
            const char *funct3_char = funct3_addiw[funct3];
            if (funct3_char) {
                if (funct3 == 0) {  // ADDIW
                    snprintf(disassemble_buf, buf_size, "%s %s, %s, %d", funct3_char, rd_char, rs1_char, immediate_I);
                } else if (funct3 == 1 || funct3 == 5) {  // SLLIW, SRLIW, SRAIW
                    const char *shift_type = (funct3 == 1) ? "SLLIW" : (funct7 & 0x20 ? "SRAIW" : "SRLIW");
                    int shamt = rs2 & 0x1F;  // Only lower 5 bits for RV64I
                    snprintf(disassemble_buf, buf_size, "%s %s, %s, %d", shift_type, rd_char, rs1_char, shamt);
                }
            } else {
                snprintf(disassemble_buf, buf_size, "Unknown ADDIW-type instruction");
            }
            }
            break;
        case OPCODE_ADDW:  // This also covers RV64M instructions
            if (funct7 == 0x01) {  // RV64M
                const char *funct3_char;
                switch (funct3) {
                    case 0: funct3_char = "MULW"; break;
                    case 4: funct3_char = "DIVW"; break;
                    case 5: funct3_char = "DIVUW"; break;
                    case 6: funct3_char = "REMW"; break;
                    case 7: funct3_char = "REMUW"; break;
                    default: funct3_char = "Unknown RV64M";
                }
                snprintf(disassemble_buf, buf_size, "%s %s, %s, %s", funct3_char, rd_char, rs1_char, rs2_char);
            } else {
                const char *funct3_char = funct3_addw[funct3];
                if (funct3_char) {
                    if (funct3 == 0) {  // ADDW/SUBW
                        const char *op = (funct7 & 0x20) ? "SUBW" : "ADDW";
                        snprintf(disassemble_buf, buf_size, "%s %s, %s, %s", op, rd_char, rs1_char, rs2_char);
                    } else if (funct3 == 1 || funct3 == 5) {  // SLLW, SRLW, SRAW
                        const char *shift_type = (funct3 == 1) ? "SLLW" : (funct7 & 0x20 ? "SRAW" : "SRLW");
                        snprintf(disassemble_buf, buf_size, "%s %s, %s, %s", shift_type, rd_char, rs1_char, rs2_char);
                    } else {
                        snprintf(disassemble_buf, buf_size, "Unknown ADDW-type instruction");
                    }
                } else {
                    snprintf(disassemble_buf, buf_size, "Unknown ADDW-type instruction");
                }
            }
            break;
        case 0x2F:  // RV32A and RV64A
            {
            uint32_t funct5 = (raw_insn >> 27) & 0x1F;
            uint32_t aq = (raw_insn >> 26) & 0x1;
            uint32_t rl = (raw_insn >> 25) & 0x1;
            const char *insn_name;
            int is_64bit = (funct3 == 3);  // Check if it's a 64-bit instruction

            switch (funct5) {
                case 0x02: insn_name = is_64bit ? "LR.D" : "LR.W"; break;
                case 0x03: insn_name = is_64bit ? "SC.D" : "SC.W"; break;
                case 0x01: insn_name = is_64bit ? "AMOSWAP.D" : "AMOSWAP.W"; break;
                case 0x00: insn_name = is_64bit ? "AMOADD.D" : "AMOADD.W"; break;
                case 0x04: insn_name = is_64bit ? "AMOXOR.D" : "AMOXOR.W"; break;
                case 0x0C: insn_name = is_64bit ? "AMOAND.D" : "AMOAND.W"; break;
                case 0x08: insn_name = is_64bit ? "AMOOR.D" : "AMOOR.W"; break;
                case 0x10: insn_name = is_64bit ? "AMOMIN.D" : "AMOMIN.W"; break;
                case 0x14: insn_name = is_64bit ? "AMOMAX.D" : "AMOMAX.W"; break;
                case 0x18: insn_name = is_64bit ? "AMOMINU.D" : "AMOMINU.W"; break;
                case 0x1C: insn_name = is_64bit ? "AMOMAXU.D" : "AMOMAXU.W"; break;
                default: insn_name = "Unknown RV32A/RV64A"; break;
            }
            if (funct5 == 0x02) {  // LR.W or LR.D
                snprintf(disassemble_buf, buf_size, "%s %s, (%s)", insn_name, rd_char, rs1_char);
            } else {
                snprintf(disassemble_buf, buf_size, "%s %s, %s, (%s)", insn_name, rd_char, rs2_char, rs1_char);
            }
            if (aq) strlcat(disassemble_buf, " aq", buf_size);
            if (rl) strlcat(disassemble_buf, " rl", buf_size);
            }
            break;
        case 0x07:  // Floating-point load instructions
            {
                uint32_t funct3 = (raw_insn >> 12) & 0x7;
                uint32_t rs1 = (raw_insn >> 15) & 0x1F;
                uint32_t rd  = (raw_insn >> 7) & 0x1F;
                int32_t imm = (int32_t)(raw_insn) >> 20;  // Sign-extend immediate

                const char *fmt = (funct3 == 2) ? "FLW" :
                                  (funct3 == 3) ? "FLD" :
                                  (funct3 == 4) ? "FLQ" : "Unknown";

                snprintf(disassemble_buf, buf_size, "%s %s, %d(%s)", fmt, f_reg_names[rd], imm, reg_names[rs1]);
            }
            break;
        case 0x27:  // Floating-point store instructions
            {
                uint32_t funct3 = (raw_insn >> 12) & 0x7;
                uint32_t rs1 = (raw_insn >> 15) & 0x1F;
                uint32_t rs2 = (raw_insn >> 20) & 0x1F;
                // Immediate is split between bits [31:25] and [11:7]
                int32_t imm = (((raw_insn >> 25) & 0x7F) << 5) | ((raw_insn >> 7) & 0x1F);
                if (imm & 0x800)  // Sign-extend if necessary
                    imm |= 0xFFFFF000;

                const char *fmt = (funct3 == 2) ? "FSW" :
                                  (funct3 == 3) ? "FSD" :
                                  (funct3 == 4) ? "FSQ" : "Unknown";

                snprintf(disassemble_buf, buf_size, "%s %s, %d(%s)", fmt, f_reg_names[rs2], imm, reg_names[rs1]);
            }
            break;
        case 0x43:  // FMADD.S, FMADD.D, FMADD.Q
        case 0x47:  // FMSUB.S, FMSUB.D, FMSUB.Q
        case 0x4B:  // FNMSUB.S, FNMSUB.D, FNMSUB.Q
        case 0x4F:  // FNMADD.S, FNMADD.D, FNMADD.Q
            {
                uint32_t funct2 = (raw_insn >> 25) & 0x3;
                const char *fmt = (funct2 == 0b00) ? "S" :
                                  (funct2 == 0b01) ? "D" :
                                  (funct2 == 0b11) ? "Q" : "Unknown";

                uint32_t rd  = (raw_insn >> 7) & 0x1F;
                uint32_t rs1 = (raw_insn >> 15) & 0x1F;
                uint32_t rs2 = (raw_insn >> 20) & 0x1F;
                uint32_t rs3 = (raw_insn >> 27) & 0x1F;
                uint32_t rm  = (raw_insn >> 12) & 0x7;  // Rounding mode

                const char *op_name = (instruction.opcode == 0x43) ? "FMADD" :
                                      (instruction.opcode == 0x47) ? "FMSUB" :
                                      (instruction.opcode == 0x4B) ? "FNMSUB" : "FNMADD";

                const char *rm_str = rm_names[rm];

                snprintf(disassemble_buf, buf_size, "%s.%s %s, %s, %s, %s, %s",
                         op_name, fmt,
                         f_reg_names[rd], f_reg_names[rs1],
                         f_reg_names[rs2], f_reg_names[rs3], rm_str);
            }
            break;
        case 0x53:  // Floating-point operations
            {
                uint32_t funct7 = (raw_insn >> 25) & 0x7F;
                uint32_t funct3 = (raw_insn >> 12) & 0x7;
                uint32_t rs2 = (raw_insn >> 20) & 0x1F;
                uint32_t rs1 = (raw_insn >> 15) & 0x1F;
                uint32_t rd  = (raw_insn >> 7) & 0x1F;

                // Determine the format based on funct7[1:0]
                uint32_t fmt_bits = funct7 & 0x3;
                const char *fmt = (fmt_bits == 0b00) ? "S" :
                                  (fmt_bits == 0b01) ? "D" :
                                  (fmt_bits == 0b11) ? "Q" : "Unknown";

                // Rounding mode is in funct3
                uint32_t rm = funct3;
                const char *rm_str = rm_names[rm];

                // Decode instruction based on funct7
                switch (funct7 & 0xFC) {  // Masking lower 2 bits which are format bits
                    case 0x00:  // FADD
                        snprintf(disassemble_buf, buf_size, "FADD.%s %s, %s, %s, %s",
                                 fmt, f_reg_names[rd], f_reg_names[rs1], f_reg_names[rs2], rm_str);
                        break;
                    case 0x04:  // FSUB
                        snprintf(disassemble_buf, buf_size, "FSUB.%s %s, %s, %s, %s",
                                 fmt, f_reg_names[rd], f_reg_names[rs1], f_reg_names[rs2], rm_str);
                        break;
                    case 0x08:  // FMUL
                        snprintf(disassemble_buf, buf_size, "FMUL.%s %s, %s, %s, %s",
                                 fmt, f_reg_names[rd], f_reg_names[rs1], f_reg_names[rs2], rm_str);
                        break;
                    case 0x0C:  // FDIV
                        snprintf(disassemble_buf, buf_size, "FDIV.%s %s, %s, %s, %s",
                                 fmt, f_reg_names[rd], f_reg_names[rs1], f_reg_names[rs2], rm_str);
                        break;
                    case 0x2C:  // FSQRT
                        snprintf(disassemble_buf, buf_size, "FSQRT.%s %s, %s, %s",
                                 fmt, f_reg_names[rd], f_reg_names[rs1], rm_str);
                        break;
                    case 0x10:  // FSGNJ, FSGNJN, FSGNJX
                        switch (funct3) {
                            case 0:
                                snprintf(disassemble_buf, buf_size, "FSGNJ.%s %s, %s, %s", fmt, f_reg_names[rd], f_reg_names[rs1], f_reg_names[rs2]);
                                break;
                            case 1:
                                snprintf(disassemble_buf, buf_size, "FSGNJN.%s %s, %s, %s", fmt, f_reg_names[rd], f_reg_names[rs1], f_reg_names[rs2]);
                                break;
                            case 2:
                                snprintf(disassemble_buf, buf_size, "FSGNJX.%s %s, %s, %s", fmt, f_reg_names[rd], f_reg_names[rs1], f_reg_names[rs2]);
                                break;
                            default:
                                snprintf(disassemble_buf, buf_size, "Unknown FSGNJ variant");
                        }
                        break;
                    case 0x14:  // FMIN, FMAX
                        if (funct3 == 0)
                            snprintf(disassemble_buf, buf_size, "FMIN.%s %s, %s, %s", fmt, f_reg_names[rd], f_reg_names[rs1], f_reg_names[rs2]);
                        else if (funct3 == 1)
                            snprintf(disassemble_buf, buf_size, "FMAX.%s %s, %s, %s", fmt, f_reg_names[rd], f_reg_names[rs1], f_reg_names[rs2]);
                        else
                            snprintf(disassemble_buf, buf_size, "Unknown FMIN/FMAX variant");
                        break;
                    case 0x50:  // FCVT between floating-point formats
                        {
                            uint32_t src_fmt_bits = rs2 & 0x3;
                            const char *src_fmt = (src_fmt_bits == 0b00) ? "S" :
                                                  (src_fmt_bits == 0b01) ? "D" :
                                                  (src_fmt_bits == 0b11) ? "Q" : "Unknown";
                            snprintf(disassemble_buf, buf_size, "FCVT.%s.%s %s, %s, %s", fmt, src_fmt, f_reg_names[rd], f_reg_names[rs1], rm_str);
                        }
                        break;
                    case 0x70:  // FCLASS, FMV.X
                        if (funct3 == 0 && rs2 == 0)
                            snprintf(disassemble_buf, buf_size, "FMV.X.%s %s, %s", fmt, reg_names[rd], f_reg_names[rs1]);
                        else if (funct3 == 1 && rs2 == 0)
                            snprintf(disassemble_buf, buf_size, "FCLASS.%s %s, %s", fmt, reg_names[rd], f_reg_names[rs1]);
                        else
                            snprintf(disassemble_buf, buf_size, "Unknown FMV.X/FCLASS variant");
                        break;
                    case 0x60:  // FCVT from floating-point to integer
                        {
                            const char *int_fmt = (rs2 == 0) ? "W" :
                                                  (rs2 == 1) ? "WU" :
                                                  (rs2 == 2) ? "L" :
                                                  (rs2 == 3) ? "LU" : "Unknown";
                            snprintf(disassemble_buf, buf_size, "FCVT.%s.%s %s, %s, %s", int_fmt, fmt, reg_names[rd], f_reg_names[rs1], rm_str);
                        }
                        break;
                    case 0x68:  // FCVT from integer to floating-point
                        {
                            const char *int_fmt = (rs2 == 0) ? "W" :
                                                  (rs2 == 1) ? "WU" :
                                                  (rs2 == 2) ? "L" :
                                                  (rs2 == 3) ? "LU" : "Unknown";
                            snprintf(disassemble_buf, buf_size, "FCVT.%s.%s %s, %s, %s", fmt, int_fmt, f_reg_names[rd], reg_names[rs1], rm_str);
                        }
                        break;
                    case 0x78:  // FMV to float from integer
                        if (funct3 == 0 && rs2 == 0)
                            snprintf(disassemble_buf, buf_size, "FMV.%s.X %s, %s", fmt, f_reg_names[rd], reg_names[rs1]);
                        else
                            snprintf(disassemble_buf, buf_size, "Unknown FMV variant");
                        break;
                    case 0x54:  // FLE, FLT, FEQ
                        switch (funct3) {
                            case 0:
                                snprintf(disassemble_buf, buf_size, "FLE.%s %s, %s, %s", fmt, reg_names[rd], f_reg_names[rs1], f_reg_names[rs2]);
                                break;
                            case 1:
                                snprintf(disassemble_buf, buf_size, "FLT.%s %s, %s, %s", fmt, reg_names[rd], f_reg_names[rs1], f_reg_names[rs2]);
                                break;
                            case 2:
                                snprintf(disassemble_buf, buf_size, "FEQ.%s %s, %s, %s", fmt, reg_names[rd], f_reg_names[rs1], f_reg_names[rs2]);
                                break;
                            default:
                                snprintf(disassemble_buf, buf_size, "Unknown compare instruction");
                        }
                        break;
                    default:
                        snprintf(disassemble_buf, buf_size, "Unknown floating-point instruction");
                }
            }
            break;
        default:
            snprintf(disassemble_buf, buf_size, "Unknown instruction");
            break;
    }
    return 0;
}


void add_char(char c, char **buf_ptr, size_t *buf_remaining, int *total_written) {
    if (*buf_remaining > 1) {
        *(*buf_ptr)++ = c;
        (*buf_remaining)--;
    }
    (*total_written)++;
}

void add_padding(int count, char pad_char, char **buf_ptr, size_t *buf_remaining, int *total_written) {
    for (int k = 0; k < count; k++) {
        add_char(pad_char, buf_ptr, buf_remaining, total_written);
    }
}

void add_padded_string(const char *str, int field_width, int zero_pad, int is_char, char **buf_ptr, size_t *buf_remaining, int *total_written) {
    int str_len = is_char ? 1 : strlen(str);
    int padding = (field_width > str_len) ? field_width - str_len : 0;
    add_padding(padding, zero_pad ? '0' : ' ', buf_ptr, buf_remaining, total_written);
    if (is_char) {
        add_char(*str, buf_ptr, buf_remaining, total_written);
    }
    else {
        while (*str) {
            add_char(*str++, buf_ptr, buf_remaining, total_written);
        }
    }
}

int vsnprintf(char *buf, size_t bufsize, const char *format, va_list args) {
    size_t i = 0;
    int is_char = 0;
    char *buf_ptr = buf;
    size_t buf_remaining = bufsize;
    int total_written = 0;

    while (format[i] != '\0') {
        if (format[i] == '%' && format[i+1] != '\0') {
            i++;
            int field_width = 0;
            int zero_pad = 0;
            
            if (format[i] == '0') {
                zero_pad = 1;
                i++;
            }
            if (format[i] >= '1' && format[i] <= '9') {  // Parse field width
                const char *width_start = &format[i];
                field_width = parser(&width_start);
                i += width_start - &format[i];           // Move i past the parsed digits
            }

            switch (format[i]) {
                case 'c': {
                    is_char = 1;                         // Set flag for character handling
                    char c = (char)va_arg(args, int);
                    add_padded_string(&c, field_width, zero_pad, is_char, &buf_ptr, &buf_remaining, &total_written);
                    is_char = 0;                         // Reset flag
                    break;
                }
                case 's': {
                    const char *str = va_arg(args, char*);
                    add_padded_string(str, field_width, zero_pad, is_char, &buf_ptr, &buf_remaining, &total_written);
                    break;
                }
                case 'd': {
                    int d = va_arg(args, int);
                    add_padded_string(decimal_string(d), field_width, zero_pad, is_char, &buf_ptr, &buf_remaining, &total_written);
                    break;
                }
                case 'x': {
                    unsigned int x = va_arg(args, unsigned int);
                    add_padded_string(hex_string(x), field_width, zero_pad, is_char, &buf_ptr, &buf_remaining, &total_written);
                    break;
                }
                case 'l': {
                    i++;                                 // Move to next character ('d' or 'x')
                    if (format[i] == 'd') {
                        long ld = va_arg(args, long);
                        add_padded_string(decimal_string(ld), field_width, zero_pad, is_char, &buf_ptr, &buf_remaining, &total_written);
                    } else if (format[i] == 'x') {
                        unsigned long lx = va_arg(args, unsigned long);
                        add_padded_string(hex_string(lx), field_width, zero_pad, is_char, &buf_ptr, &buf_remaining, &total_written);
                    } break;
                }
                case 'p': {
                    if (format[i + 1] == 'I') {
                        i++;
                        void *ptr = va_arg(args, void*);
                        char disassembled_buf[100];
                        disassemble(ptr, disassembled_buf, sizeof(disassembled_buf));
                        add_padded_string(disassembled_buf, field_width, zero_pad, is_char, &buf_ptr, &buf_remaining, &total_written);
                    } else {
                        void *ptr = va_arg(args, void*);
                        add_char('0', &buf_ptr, &buf_remaining, &total_written);
                        add_char('x', &buf_ptr, &buf_remaining, &total_written);
                        add_padded_string(hex_string((unsigned long)ptr), 8, 1, is_char, &buf_ptr, &buf_remaining, &total_written);  // 8 digits, zero-padded
                    } break;
                }
                case '%': {
                    add_char('%', &buf_ptr, &buf_remaining, &total_written);
                    break;
                }
            }
        } else {
            add_char(format[i], &buf_ptr, &buf_remaining, &total_written);
        }
        i++;
    }

    if (buf_remaining > 0) {
        *buf_ptr = '\0';   // null-termination
    }
    return total_written;   // Return total characters (not necessarily written)
}

int snprintf(char *buf, size_t bufsize, const char *format, ...) {
    va_list args;
    va_start(args, format);
    int print = vsnprintf(buf, bufsize, format, args);
    va_end(args);
    return print;
}

// ok to assume printf output is never longer that MAX_OUTPUT_LEN
#define MAX_OUTPUT_LEN 1024

int printf(const char *format, ...) {
    char buf[MAX_OUTPUT_LEN];
    va_list args;
    va_start(args, format);
    int num_characters_written = vsnprintf(buf, MAX_OUTPUT_LEN, format, args);
    va_end(args);

    uart_putstring(buf);

    return num_characters_written;
}