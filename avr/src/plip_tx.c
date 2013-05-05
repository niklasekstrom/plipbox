/*
 * plip_tx.h: send plip packets
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

#include "plip_tx.h"
#include "plip.h"
#include "pkt_buf.h"
#include "enc28j60.h"
#include "uartutil.h"
#include "net/eth.h"
#include "dump.h"
#include "param.h"
#include "uart.h"

static u16 send_pos = 0;
static u16 max_pos = 0;

// callback to retrieve next byte for plip packet send
static u08 get_plip_data(u08 *data)
{
  // fetch data from buffer
  if(send_pos < max_pos) {
    *data = pkt_buf[send_pos++];
    return PLIP_STATUS_OK;
  }  
  // fetch data directly from packet buffer on enc28j60
  else {
    *data = enc28j60_packet_rx_byte();
    return PLIP_STATUS_OK;
  }
}

static void uart_send_prefix(void)
{
  uart_send_time_stamp_spc();
  uart_send_pstring(PSTR("plip(TX): "));
}

void plip_tx_init(void)
{
  // setup plip
  plip_send_init(get_plip_data); 
}

static u08 retry;

static void dump_begin(void)
{
  uart_send_prefix();
  dump_line(pkt_buf, pkt.size);
}

static void dump_end(void)
{
  if(param.dump_plip) {
    dump_plip();
  }
  uart_send_crlf();
}

void plip_tx_worker(u08 plip_online)
{
  // shall we retry to send the last packet
  if(plip_online && (retry > 0)) {

    if(param.dump_dirs & DUMP_DIR_PLIP_TX) {
      dump_begin();
    }
    
    u08 status = plip_send(&pkt);
    if(status == PLIP_STATUS_OK) {
      uart_send_pstring(PSTR("retry: OK "));
      retry = 0;
    } else {
      uart_send_pstring(PSTR("retry: failed: "));
      uart_send_hex_byte(status);
      uart_send(' ');
      retry --;
    }

    if(param.dump_dirs & DUMP_DIR_PLIP_TX) {
      dump_end();
    }
  }  
}

u08 plip_tx_send(u08 mem_offset, u16 mem_size, u16 total_size)
{
  send_pos = mem_offset;
  max_pos = mem_offset + mem_size;
  pkt.size = total_size;
  pkt.crc_type = PLIP_NOCRC;

  if(param.dump_dirs & DUMP_DIR_PLIP_TX) {
    dump_begin();
  }
  
  u08 status = plip_send(&pkt);
  
  if(status != PLIP_STATUS_OK) {
    uart_send_pstring(PSTR("first: failed: "));
    uart_send_hex_byte(status);
    uart_send(' ');
    retry = 5;
  } else {
    retry = 0;
  }
  
  if(param.dump_dirs & DUMP_DIR_PLIP_TX) {
    dump_end();
  }
  
  return status;
}
