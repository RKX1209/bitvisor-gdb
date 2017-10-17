/*
 * Copyright (c) 2017 Ren Kimura <rkx1209dev@gmail.com>
 * All rights reserved.
 */

#include "gdbstub.h"

static void gdb_read_byte(u8 ch) {

}

void gdb_chr_receive (u8 *buf, u16 size) {
  int i;
  for (i = 0; i < size; i++) {
      gdb_read_byte(buf[i]);
  }
}
