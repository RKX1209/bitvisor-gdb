/*
 * Copyright (c) 2017 Ren Kimura <rkx1209dev@gmail.com>
 * All rights reserved.
 */

#include <core/gmm_access.h>
#include <core/mm.h>
#include <core/printf.h>
#include <core/strtol.h>
#include <core/vt_regs.h>
#include <common.h>
#include "gdbstub.h"

struct vcpu;
extern struct vcpu *current;
GDBState *gdbserver_state;
static LIST1_DEFINE_HEAD (struct Breakpoint, bp_list);

void gdb_server_send(u8 *buf, u16 len);

static inline bool isxdigit(char ch) {
  return ('a' <= ch && ch <= 'f') || ('A' <= ch && ch <= 'F') || ('0' <= ch && ch <= '9');
}

static inline int fromhex(int v)
{
  if (v >= '0' && v <= '9')
      return v - '0';
  else if (v >= 'A' && v <= 'F')
      return v - 'A' + 10;
  else if (v >= 'a' && v <= 'f')
      return v - 'a' + 10;
  else
      return 0;
}

static inline int tohex(int v)
{
  if (v < 10)
      return v + '0';
  else
      return v - 10 + 'a';
}

static void memtohex(char *buf, const u8 *mem, int len)
{
  int i, c;
  char *q;
  q = buf;
  for(i = 0; i < len; i++) {
      c = mem[i];
      *q++ = tohex(c >> 4);
      *q++ = tohex(c & 0xf);
  }
  *q = '\0';
}

static void hextomem(u8 *mem, const char *buf, int len)
{
  int i;
  for(i = 0; i < len; i++) {
      mem[i] = (fromhex(buf[0]) << 4) | fromhex(buf[1]);
      buf += 2;
  }
}

static int is_query_packet(const char *p, const char *query, char separator)
{
  unsigned int query_len = strlen(query);

  return strncmp(p, query, query_len) == 0 &&
      (p[query_len] == '\0' || p[query_len] == separator);
}

static int target_memory_rw(u64 addr, u8 *buf, int len, bool is_write) {
  printf("[%s] addr 0x%016lx(%d byte)\n", (is_write?"WRITE":"READ"), addr, len);
  unsigned int i;
  for (i = 0; i < len; i++) {
    if (is_write) {
      if (write_gvirt_b (addr + i, &buf[i], 0) == -1)
        return -1;
    } else {
      if (read_gvirt_b (addr + i, &buf[i], 0) == -1)
        return -1;
    }
  }
  return 0;
}

static int gdb_read_register(u8 *buf, int reg) {
  if (reg < GENERAL_REG_MAX) {
    vt_read_general_reg(reg, (ulong *)buf);
  } else {
    switch (reg) {
    case IDX_IP_REG:
      vt_read_ip((ulong *)buf);
      break;
    case IDX_FLAGS_REG:
      vt_read_flags((ulong *)buf);
      break;
    case IDX_SEG_REGS:
      vt_read_sreg_sel(SREG_CS, (ulong *)buf);
      break;
    case IDX_SEG_REGS + 1:
      vt_read_sreg_sel(SREG_SS, (ulong *)buf);
      break;
    case IDX_SEG_REGS + 2:
      vt_read_sreg_sel(SREG_DS, (ulong *)buf);
      break;
    case IDX_SEG_REGS + 3:
      vt_read_sreg_sel(SREG_ES, (ulong *)buf);
      break;
    case IDX_SEG_REGS + 4:
      vt_read_sreg_sel(SREG_FS, (ulong *)buf);
      break;
    case IDX_SEG_REGS + 5:
      vt_read_sreg_sel(SREG_GS, (ulong *)buf);
      break;
    default:
      break;
    }
  }
  return sizeof(unsigned long);
}

static int sync_debug_reg(void)
{
  const u8 type_code[] = {
      [GDB_BREAKPOINT_HW] = 0x0,
      [GDB_WATCHPOINT_WRITE] = 0x1,
      [GDB_WATCHPOINT_ACCESS] = 0x3
  };
  const u8 len_code[] = {
      [1] = 0x0, [2] = 0x1, [4] = 0x3, [8] = 0x2
  };
  Breakpoint *bp;
  unsigned long dr7 = 0x0600;
  int n = 0;
  LIST1_FOREACH (bp_list, bp) {
    vt_write_dr(n, bp->addr);
    dr7 |= (2 << (n * 2)) |
        (type_code[bp->type] << (16 + n * 4)) |
        ((u32)len_code[bp->len] << (18 + n * 4));
    n++;
  }
  vt_write_dr(DEBUG_REG_DR7, dr7);
  return 0;
}

static int hw_breakpoint_insert(unsigned long addr, unsigned long len, int type)
{
  if (nb_hw_breakpoint > DEBUG_REG_MAX)
    return -1;
  Breakpoint *bp = alloc (sizeof *bp);
  if (!bp)
    return -1;
  bp->addr = addr, bp->len = len, bp->type = type;
  LIST1_ADD (bp_list, bp);
  nb_hw_breakpoint++;
  sync_debug_reg ();
  return 0;
}

static int hw_breakpoint_remove(unsigned long addr, unsigned long len, int type) {
  if (nb_hw_breakpoint <= 0)
    return -1;
  Breakpoint *bp;
  LIST1_FOREACH (bp_list, bp) {
    if (addr == bp->addr && len == bp->len && type == bp->type) {
      nb_hw_breakpoint--;
      goto found;
    }
  }
  return -1;
found:
  LIST1_DEL (bp_list, bp);
  sync_debug_reg ();  
  return 0;
}

static int gdb_breakpoint_insert(unsigned long addr, unsigned long len, int type)
{
  int err = -1;
  switch (type) {
    case GDB_BREAKPOINT_SW:
    case GDB_BREAKPOINT_HW:
      return hw_breakpoint_insert (addr, len, type);
      break;
    case GDB_WATCHPOINT_WRITE:
    case GDB_WATCHPOINT_READ:
    case GDB_WATCHPOINT_ACCESS:
      /* TODO: */
      break;
    default:
      break;
  }
  return err;
}

static int gdb_breakpoint_remove(unsigned long addr, unsigned long len, int type)
{
  int err = -1;
  switch (type) {
    case GDB_BREAKPOINT_SW:
    case GDB_BREAKPOINT_HW:
      return hw_breakpoint_remove (addr, len, type);
    case GDB_WATCHPOINT_WRITE:
    case GDB_WATCHPOINT_READ:
    case GDB_WATCHPOINT_ACCESS:
      /* TODO: */
      break;
    default:
      break;
  }
  return err;
}

void gdb_do_sigtrap() {
  char buf [256];
  snprintf(buf, sizeof(buf), "S%02x", GDB_SIGNAL_TRAP);
  gdb_put_packet(gdbserver_state, buf);
}

static void gdb_continue(GDBState *s) {
  _vt_start_vm ();
}

static void gdb_write_buf(GDBState *s, u8 *buf, u16 size) {
  gdb_server_send(buf, size);
}

static void gdb_write_packet(GDBState *s, u8 *buf, u16 len) {
  int csum, i;
  u8 *p;
  for (;;) {
    p = s->last_packet;
    *(p++) = '$';
    memcpy(p, buf, len);
    p += len;
    csum = 0;
    for(i = 0; i < len; i++) {
        csum += buf[i];
    }
    *(p++) = '#';
    *(p++) = tohex((csum >> 4) & 0xf);
    *(p++) = tohex((csum) & 0xf);

    s->last_packet_len = p - s->last_packet;
    gdb_write_buf(s, (u8 *)s->last_packet, s->last_packet_len);
    break;
  }
}

void gdb_put_packet(GDBState *s, char *buf) {
  gdb_write_packet(s, buf, strlen(buf));
}

static int gdb_handle_packet(GDBState *s, const char *line_buf) {
  const char *p;
  u32 thread;
  int ch, reg_size, type, res;
  char buf[MAX_PACKET_LENGTH];
  u8 mem_buf[MAX_PACKET_LENGTH];
  unsigned long addr, len;

  printf("command='%s'\n", line_buf);

  p = line_buf;
  ch = *p++;
  switch(ch) {
  case '?':
    /* XXX: Is it correct to fix thread id to '1'? */
    snprintf(buf, sizeof(buf), "T%02xthread:%02x;", GDB_SIGNAL_TRAP, 1);
    gdb_put_packet(s, buf);
    break;
  case 'c':
    if (*p != '\0') {
      addr = strtol(p, (char **)&p, 16);
    }
    gdb_continue(s);
    break;
  case 'g':
    len = 0;
    for (addr = 0; addr < GENERAL_REG_MAX; addr++) {
      reg_size = gdb_read_register(mem_buf + len, addr);
      len += reg_size;
    }
    memtohex(buf, mem_buf, len);
    gdb_put_packet(s, buf);
    break;
  case 'm':
    addr = strtol(p, (char **)&p, 16);
    if (*p == ',')
        p++;
    len = strtol(p, NULL, 16);

    /* memtohex() doubles the required space */
    if (len > MAX_PACKET_LENGTH / 2) {
        gdb_put_packet (s, "E22");
        break;
    }

    if (target_memory_rw (addr, mem_buf, len, false) != 0) {
        gdb_put_packet (s, "E14");
    } else {
        memtohex(buf, mem_buf, len);
        gdb_put_packet(s, buf);
    }
    break;
  case 'p':
    addr = strtol(p, (char **)&p, 16);
    reg_size = gdb_read_register(mem_buf, addr);
    if (reg_size) {
      memtohex(buf, mem_buf, reg_size);
      gdb_put_packet(s, buf);
    } else {
      gdb_put_packet(s, "E14");
    }
    break;
  case 'q':
  case 'Q':
    if (is_query_packet(p, "Supported", ':')) {
        snprintf(buf, sizeof(buf), "PacketSize=%x", MAX_PACKET_LENGTH);
        gdb_put_packet(s, buf);
        break;
    } else if (strcmp(p,"C") == 0) {
        gdb_put_packet(s, "QC1");
        break;
    }
    goto unknown_command;
  case 'z':
  case 'Z':
    type = strtol(p, (char **)&p, 16);
    if (*p == ',')
        p++;
    addr = strtol(p, (char **)&p, 16);
    if (*p == ',')
        p++;
    len = strtol(p, (char **)&p, 16);
    if (ch == 'Z')
        res = gdb_breakpoint_insert(addr, len, type);
    else
        res = gdb_breakpoint_remove(addr, len, type);
    if (res >= 0)
        gdb_put_packet(s, "OK");
    else
        gdb_put_packet(s, "E22");
    break;

  case 'H':
    type = *p++;
    thread = strtol(p, (char **)&p, 16);
    if (thread == -1 || thread == 0) {
        gdb_put_packet(s, "OK");
        break;
    }
    switch (type) {
    case 'c':
        gdb_put_packet(s, "OK");
        break;
    case 'g':
        gdb_put_packet(s, "OK");
        break;
    default:
        gdb_put_packet(s, "E22");
        break;
    }
    break;
  case 'T':
    thread = strtol(p, (char **)&p, 16);
    gdb_put_packet(s, "OK");
    break;
  default:
    unknown_command:
      /* put empty packet */
      buf[0] = '\0';
      gdb_put_packet(s, buf);
      break;
  }
  return RS_IDLE;
}

static void gdb_read_byte(GDBState *s, u8 ch) {
  u8 reply;
  switch (s->state) {
    case RS_IDLE:
      if (ch == '$') {
        /* start of command packet */
        s->line_buf_index = 0;
        s->line_sum = 0;
        s->state = RS_GETLINE;
      } else {
        //printf("%s: received garbage between packets: 0x%x\n", __func__, ch);
      }
      break;
    case RS_GETLINE:
      if (ch == '}') {
        s->state = RS_GETLINE_ESC;
        s->line_sum += ch;
      } else if (ch == '*') {
        /* start run length encoding sequence */
        s->state = RS_GETLINE_RLE;
        s->line_sum += ch;
      } else if (ch == '#') {
        /* end of command, start of checksum*/
        s->state = RS_CHKSUM1;
      } else if (s->line_buf_index >= sizeof(s->line_buf) - 1) {
        s->state = RS_IDLE;
        printf("%s: command buffer overrun, dropping command\n", __func__);
      } else {
        /* unescaped command character */
        s->line_buf[s->line_buf_index++] = ch;
        s->line_sum += ch;
      }
      break;
    case RS_GETLINE_ESC:
        if (ch == '#') {
            /* unexpected end of command in escape sequence */
            s->state = RS_CHKSUM1;
        } else if (s->line_buf_index >= sizeof(s->line_buf) - 1) {
            /* command buffer overrun */
            s->state = RS_IDLE;
            printf("%s: command buffer overrun, dropping command\n", __func__);
        } else {
            /* parse escaped character and leave escape state */
            s->line_buf[s->line_buf_index++] = ch ^ 0x20;
            s->line_sum += ch;
            s->state = RS_GETLINE;
        }
      break;
    case RS_GETLINE_RLE:
        if (ch < ' ') {
            /* invalid RLE count encoding */
            s->state = RS_GETLINE;
            printf("%s: got invalid RLE count: 0x%x\n", __func__, ch);
        } else {
            /* decode repeat length */
            int repeat = (unsigned char)ch - ' ' + 3;
            if (s->line_buf_index + repeat >= sizeof(s->line_buf) - 1) {
                /* that many repeats would overrun the command buffer */
                printf("%s: command buffer overrun, dropping command\n", __func__);
                s->state = RS_IDLE;
            } else if (s->line_buf_index < 1) {
                /* got a repeat but we have nothing to repeat */
                printf("%s: got invalid RLE sequence\n", __func__);
                s->state = RS_GETLINE;
            } else {
                /* repeat the last character */
                memset(s->line_buf + s->line_buf_index,
                      s->line_buf[s->line_buf_index - 1], repeat);
                s->line_buf_index += repeat;
                s->line_sum += ch;
                s->state = RS_GETLINE;
            }
        }
        break;
      case RS_CHKSUM1:
          /* get high hex digit of checksum */
          if (!isxdigit(ch)) {
              printf("%s:got invalid command checksum digit\n", __func__);
              s->state = RS_GETLINE;
              break;
          }
          s->line_buf[s->line_buf_index] = '\0';
          s->line_csum = fromhex(ch) << 4;
          s->state = RS_CHKSUM2;
        break;
      case RS_CHKSUM2:
          /* get low hex digit of checksum */
          if (!isxdigit(ch)) {
              printf("%s: got invalid command checksum digit\n", __func__);
              s->state = RS_GETLINE;
              break;
          }
          s->line_csum |= fromhex(ch);

          if (s->line_csum != (s->line_sum & 0xff)) {
              printf("gdbserver: got command packet with incorrect checksum\n");
              /* send NAK reply */
              reply = '-';
              gdb_write_buf(s, &reply, 1);
              s->state = RS_IDLE;
          } else {
              printf("gdbserver: valid packet!\n");
              /* send ACK reply */
              reply = '+';
              gdb_write_buf(s, &reply, 1);
              s->state = gdb_handle_packet(s, s->line_buf);
          }
          break;
      default:
          printf("%s: got unknown status\n", __func__);
          break;
    }
}

void gdb_chr_receive(u8 *buf, u16 size) {
  int i;
  for (i = 0; i < size; i++) {
    gdb_read_byte(gdbserver_state, buf[i]);
  }
}

void gdb_stub_init(void) {
  GDBState *s;
  s = gdbserver_state;
  LIST1_HEAD_INIT (bp_list);
  nb_hw_breakpoint = 0;
  if (!s) {
    s = alloc (sizeof (GDBState));
    gdbserver_state = s;
  }
  s->state = RS_IDLE;
}
