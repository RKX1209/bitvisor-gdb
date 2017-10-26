/*
 * Copyright (c) 2017 Ren Kimura <rkx1209dev@gmail.com>
 * All rights reserved.
 */

#ifndef _CORE_VT_REGS_H
#define _CORE_VT_REGS_H

#include <core/types.h>

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
#define REG_PC GENERAL_REG_MAX

void vt_read_general_reg (enum general_reg reg, ulong *val);
void vt_write_general_reg (enum general_reg reg, ulong val);
void vt_read_ip (ulong *val);
void vt_write_ip (ulong val);
#endif
