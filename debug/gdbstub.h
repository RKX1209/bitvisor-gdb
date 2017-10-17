/*
 * Copyright (c) 2017 Ren Kimura <rkx1209dev@gmail.com>
 * All rights reserved.
 */
#ifndef _DEBUG_GDBSTUB_H
#define _DEBUG_GDBSTUB_H

#include <core/types.h>

void gdb_chr_receive (u8 *buf, u16 size);

#endif
