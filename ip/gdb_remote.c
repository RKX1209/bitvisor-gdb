/*
 * Copyright (c) 2017 Ren Kimura <rkx1209dev@gmail.com>
 * All rights reserved.
 */
#include <debug/gdbstub.h>
#include "lwip/tcp.h"

static struct tcp_pcb *gdb_pcb;

enum server_states {
  ES_NONE = 0,
  ES_ACCEPTED,
  ES_RECEIVED,
  ES_CLOSING
};

struct gdb_server_state {
  enum server_states state;
  struct tcp_pcb *pcb;
};

static void gdb_server_close(struct tcp_pcb *tpcb, struct gdb_server_state *gs)
{
  tcp_arg(tpcb, NULL);
  tcp_sent(tpcb, NULL);
  tcp_recv(tpcb, NULL);

  if (gs != NULL) {
    mem_free(gs);
  }
  tcp_close(tpcb);
}

static err_t gdb_server_recv(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err)
{
  struct gdb_server_state *gs;
  err_t ret_err;
  gs = (struct gdb_server_state *)arg;
  printf ("%s: Recv\n", __func__);
  if (p == NULL) {
    printf("gdbserver: Disconnected from client\n");
    gs->state = ES_CLOSING;
    gdb_server_close (tpcb, gs);
    ret_err = ERR_OK;
  } else {
    gdb_chr_receive (p->payload, p->len);
  }
  return ret_err;
}

static err_t gdb_server_accept(void *arg, struct tcp_pcb *newpcb, err_t err)
{
  struct gdb_server_state *gs;

  LWIP_UNUSED_ARG(arg);
  LWIP_UNUSED_ARG(err);
  printf ("%s: Accept\n", __func__);
  /* commonly observed practive to call tcp_setprio(), why? */
  tcp_setprio(newpcb, TCP_PRIO_MIN);

  gs = (struct gdb_server_state *)mem_malloc(sizeof(struct gdb_server_state));
  if (!gs)
    panic ("%s: mem_malloc failed\n", __func__);
  gs->pcb = newpcb;
  /* pass newly allocated gs to our callbacks */
  tcp_arg(newpcb, gs);
  tcp_recv(newpcb, gdb_server_recv);
  printf("gdbserver: connected from client!\n");
  return ERR_OK;
}

void gdb_server_init (int port)
{
  gdb_stub_init();
  gdb_pcb = tcp_new();
  if (gdb_pcb != NULL)
  {
    err_t err;

    err = tcp_bind(gdb_pcb, IP_ADDR_ANY, port);
    if (err == ERR_OK)
    {
      gdb_pcb = tcp_listen(gdb_pcb);
      tcp_accept(gdb_pcb, gdb_server_accept);
      printf ("%s: Initilization complete\n", __func__);
      printf ("Server IP: 0x%016x\n", ntohl(gdb_pcb->local_ip.addr));
    }
    else
    {
      panic ("%s: tcp_bind failed\n", __func__);
    }
  }
  else
  {
    panic ("%s: tcp_new failed\n", __func__);
  }
}
