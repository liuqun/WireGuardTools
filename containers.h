/* Copyright (C) 2015-2017 Jason A. Donenfeld <Jason@zx2c4.com>. All Rights Reserved. */


#ifndef CONTAINERS_H
#define CONTAINERS_H

#include <stdint.h>
#include <stdlib.h>
#include <net/if.h>
#include <netinet/in.h>
#include <sys/time.h>
#include <sys/socket.h>

#include "../uapi/wireguard.h"

struct wgallowedip {
	uint16_t family;
	union {
		struct in_addr ip4;
		struct in6_addr ip6;
	};
	uint8_t cidr;
	struct wgallowedip *next_allowedip;
};

enum {
	WGPEER_REMOVE_ME = (1 << 0),
	WGPEER_REPLACE_ALLOWEDIPS = (1 << 1),
	WGPEER_HAS_PRESHARED_KEY = (1 << 2),
	WGPEER_HAS_PERSISTENT_KEEPALIVE_INTERVAL = (1 << 3)
};

struct wgpeer {
	uint32_t flags;

	uint8_t public_key[WG_KEY_LEN];
	uint8_t preshared_key[WG_KEY_LEN];

	union {
		struct sockaddr addr;
		struct sockaddr_in addr4;
		struct sockaddr_in6 addr6;
	} endpoint;

	struct timeval last_handshake_time;
	uint64_t rx_bytes, tx_bytes;
	uint16_t persistent_keepalive_interval;

	struct wgallowedip *first_allowedip;
	struct wgpeer *next_peer;
};

enum {
	WGDEVICE_REPLACE_PEERS = (1 << 0),
	WGDEVICE_HAS_PRIVATE_KEY = (1 << 1),
	WGDEVICE_HAS_LISTEN_PORT = (1 << 2),
	WGDEVICE_HAS_FWMARK = (1 << 3)
};

enum {
	WG_API_VERSION_MAGIC = 0xbeef0003
};

struct wgdevice {
	char name[IFNAMSIZ];

	uint32_t flags;

	uint8_t public_key[WG_KEY_LEN];
	uint8_t private_key[WG_KEY_LEN];

	uint32_t fwmark;
	uint16_t listen_port;

	struct wgpeer *first_peer;
};

#define for_each_wgpeer(__dev, __peer) for ((__peer) = (__dev)->first_peer; (__peer); (__peer) = (__peer)->next_peer)
#define for_each_wgallowedip(__peer, __allowedip) for ((__allowedip) = (__peer)->first_allowedip; (__allowedip); (__allowedip) = (__allowedip)->next_allowedip)
#define max(a, b) ((a) > (b) ? (a) : (b))

static inline void free_wgdevice(struct wgdevice *dev)
{
	if (!dev)
		return;
	for (struct wgpeer *peer = dev->first_peer, *np = peer ? peer->next_peer : NULL; peer; peer = np, np = peer ? peer->next_peer : NULL) {
		for (struct wgallowedip *allowedip = peer->first_allowedip, *na = allowedip ? allowedip->next_allowedip : NULL; allowedip; allowedip = na, na = allowedip ? allowedip->next_allowedip : NULL)
			free(allowedip);
		free(peer);
	}
	free(dev);
}

#endif
