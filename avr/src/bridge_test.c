/*
 * bridge_test.c - test packet I/O and PB loop back
 *
 * Written by
 *  Christian Vogelgsang <chris@vogelgsang.org>
 *
 * This file is part of plipbox.
 * See README for copyright notice.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
 *  02111-1307  USA.
 *
 */

#include "bridge_test.h"

#include "pio.h"
#include "pio_util.h"
#include "param.h"
#include "uartutil.h"
#include "main.h"
#include "stats.h"
#include "cmd.h"
#include "pb_util.h"
#include "pb_proto.h"
#include "net/net.h"
#include "net/eth.h"
#include "pkt_buf.h"
#include "dump.h"

static u16 pio_pkt_size;

/* a RECV command arrived from Amiga.
   this should only happen if we got a packet here from PIO
   in the first place
*/
static u08 fill_pkt(u08 *buf, u16 max_size, u16 *size)
{
  *size = pio_pkt_size;
  if(*size > max_size) {
    return PBPROTO_STATUS_PACKET_TOO_LARGE;
  }

  // in test mode 0 send via internal device loopback
  if(param.test_mode == 0) {
    // switch eth type to magic for loop back
    net_put_word(buf + ETH_OFF_TYPE, ETH_TYPE_MAGIC_LOOPBACK);
  }

  // consumed packet
  pio_pkt_size = 0;

  return PBPROTO_STATUS_OK;  
}

/* a SEND command arrvied from Amiga.
   we got our packet back. forward to PIO
*/
static u08 proc_pkt(const u08 *buf, u16 size)
{
  // make sure its the expected packet type
  u16 type = net_get_word(pkt_buf + ETH_OFF_TYPE);

  // in test mode 0 packet was sent by internal device loopback
  if(param.test_mode == 0) {
    if(type != ETH_TYPE_MAGIC_LOOPBACK) {
      uart_send_pstring(PSTR("NO MAGIC!!\r\n"));
      return PBPROTO_STATUS_OK;
    } else {
      // switch eth type back to IPv4
      net_put_word(pkt_buf + ETH_OFF_TYPE, ETH_TYPE_IPV4);
    }
  } else {
    if(type != ETH_TYPE_IPV4) {
      uart_send_pstring(PSTR("NO IPV4!!\r\n"));
      return PBPROTO_STATUS_OK;      
    }
  }

  // send packet via pio
  pio_util_send_packet(size);

  return PBPROTO_STATUS_OK;
}

u08 bridge_test_loop(void)
{
  u08 result = CMD_WORKER_IDLE;

  uart_send_time_stamp_spc();
  uart_send_pstring(PSTR("[BRIDGE_TEST] on\r\n"));

  pb_proto_init(fill_pkt, proc_pkt, pkt_buf, PKT_BUF_SIZE);
  pio_init(param.mac_addr, pio_util_get_init_flags());
  stats_reset();
  
  while(run_mode == RUN_MODE_BRIDGE_TEST) {
    // handle commands
    result = cmd_worker();
    if(result & CMD_WORKER_RESET) {
      break;
    }

    // handle pbproto
    pb_util_handle();

    // incoming packet via PIO?
    if(pio_has_recv()) {
      u16 size;
      if(pio_util_recv_packet(&size) == PIO_OK) {
        // handle ARP?
        if(!pio_util_handle_arp(size)) {
          // is it a UDP test packet?
          if(pio_util_handle_udp_test(size)) {

            // oops! overwrite??
            if(pio_pkt_size != 0) {
              uart_send_pstring(PSTR("OVERWRITE?!\r\n"));
            }

            // request receive
            pio_pkt_size = size;
            pb_proto_request_recv();
          }          
        }
      }
    }
  }

  stats_dump_all();
  pio_exit();

  uart_send_time_stamp_spc();
  uart_send_pstring(PSTR("[BRIDGE_TEST] off\r\n"));

  return result;
}
