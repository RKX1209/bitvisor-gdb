/*
 * Copyright (c) 2017 Ren Kimura <rkx1209dev@gmail.com>
 * All rights reserved.
 */

#ifndef _CORE_VT_REGS_H
#define _CORE_VT_REGS_H

#include <core/types.h>

enum sreg {
	SREG_ES = 0,
	SREG_CS = 1,
	SREG_SS = 2,
	SREG_DS = 3,
	SREG_FS = 4,
	SREG_GS = 5,
	SREG_DEFAULT = 7,
};

enum control_reg {
	CONTROL_REG_CR0 = 0,
	CONTROL_REG_CR2 = 2,
	CONTROL_REG_CR3 = 3,
	CONTROL_REG_CR4 = 4,
	CONTROL_REG_CR8 = 8,
};

enum debug_reg {
	DEBUG_REG_DR0 = 0,
	DEBUG_REG_DR1 = 1,
	DEBUG_REG_DR2 = 2,
	DEBUG_REG_DR3 = 3,
	DEBUG_REG_DR7 = 7,
};

enum general_reg {
	GENERAL_REG_RAX = 0,
	GENERAL_REG_RCX = 1,
	GENERAL_REG_RDX = 2,
	GENERAL_REG_RBX = 3,
	GENERAL_REG_RSP = 4,
	GENERAL_REG_RBP = 5,
	GENERAL_REG_RSI = 6,
	GENERAL_REG_RDI = 7,
	GENERAL_REG_R8 = 8,
	GENERAL_REG_R9 = 9,
	GENERAL_REG_R10 = 10,
	GENERAL_REG_R11 = 11,
	GENERAL_REG_R12 = 12,
	GENERAL_REG_R13 = 13,
	GENERAL_REG_R14 = 14,
	GENERAL_REG_R15 = 15,
	GENERAL_REG_MAX = 16,
};

/* NOTE: $pc register index is 16 */
#define IDX_IP_REG 			GENERAL_REG_MAX
#define IDX_FLAGS_REG   (IDX_IP_REG + 1)
#define IDX_SEG_REGS    (IDX_FLAGS_REG + 1)
#define IDX_FP_REGS     (IDX_SEG_REGS + 6)
#define IDX_XMM_REGS    (IDX_FP_REGS + 16)
#define IDX_MXCSR_REG   (IDX_XMM_REGS + CPU_NB_REGS)

#define DEBUG_REG_MAX		4

void vt_read_general_reg (enum general_reg reg, ulong *val);
void vt_write_general_reg (enum general_reg reg, ulong val);
void vt_read_sreg_sel (enum sreg s, ulong *val);
void vt_read_ip (ulong *val);
void vt_write_ip (ulong val);
void vt_read_flags (ulong *val);
void vt_write_flags (ulong val);
void vt_read_dr (enum debug_reg reg, ulong *val);
void vt_write_dr (enum debug_reg reg, ulong val);
void _vt_start_vm (void);
#endif
