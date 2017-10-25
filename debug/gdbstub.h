/*
 * Copyright (c) 2017 Ren Kimura <rkx1209dev@gmail.com>
 * All rights reserved.
 */
#ifndef _DEBUG_GDBSTUB_H
#define _DEBUG_GDBSTUB_H

#include <core/types.h>

#define MAX_PACKET_LENGTH 4096

enum RSState {
    RS_INACTIVE,
    RS_IDLE,
    RS_GETLINE,
    RS_GETLINE_ESC,
    RS_GETLINE_RLE,
    RS_CHKSUM1,
    RS_CHKSUM2,
};

typedef struct GDBState {
  enum RSState state;
  int line_sum; /* running checksum */
  int line_csum; /* checksum */
  int line_buf_index;
  char line_buf[MAX_PACKET_LENGTH];
  u8 last_packet[MAX_PACKET_LENGTH + 4];
  int last_packet_len;
}GDBState;

void gdb_stub_init(void);
void gdb_chr_receive (u8 *buf, u16 size);

#endif
