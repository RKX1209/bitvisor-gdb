/*
 * Copyright (c) 2017 Ren Kimura <rkx1209dev@gmail.com>
 * All rights reserved.
 */

#include <core/mm.h>
#include <core/printf.h>
#include <common.h>
#include "gdbstub.h"

static GDBState *gdbserver_state;

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

static int is_query_packet(const char *p, const char *query, char separator)
{
  unsigned int query_len = strlen(query);

  return strncmp(p, query, query_len) == 0 &&
      (p[query_len] == '\0' || p[query_len] == separator);
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

static void gdb_put_packet(GDBState *s, char *buf) {
  gdb_write_packet(s, buf, strlen(buf));
}

static int gdb_handle_packet(GDBState *s, const char *line_buf) {
  const char *p;
  int ch;
  char buf[MAX_PACKET_LENGTH];
  printf("command='%s'\n", line_buf);

  p = line_buf;
  ch = *p++;
  switch(ch) {
    case 'q':
    case 'Q':
    if (is_query_packet(p, "Supported", ':')) {
        snprintf(buf, sizeof(buf), "PacketSize=%x", MAX_PACKET_LENGTH);
        gdb_put_packet(s, buf);
        break;
    }
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
  if (!s) {
    s = alloc (sizeof (GDBState));
    gdbserver_state = s;
  }
  s->state = RS_IDLE;
}
