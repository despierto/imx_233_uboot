/**
 * Top layer ethernel driver header file
 *
 * Copyright (c) 2014 Alex Winter (eterno.despierto@gmail.com)
  *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

 #ifndef __NET_H__
 #define __NET_H__

#include "types.h"
#include "netdefs.h"
#include "net_datalink.h"

/************************************************
 *              PROTOCOL HEADERS DEFINITIONS                *
 ************************************************/
#define NETSTATE_CONTINUE    1
#define NETSTATE_RESTART     2
#define NETSTATE_SUCCESS     3
#define NETSTATE_FAIL        4


/************************************************
 *              FUNCTION HEADERS DEFINITIONS                 *
 ************************************************/
int     net_init(void);
int     net_close(void);
void    net_ping_req(IPaddr_t ip);



#endif /* __NET_H__ */

