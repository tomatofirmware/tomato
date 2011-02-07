/*
 *  addrmap.c -- address mapping routines
 *
 *  part of TAYGA <http://www.litech.org/tayga/>
 *  Copyright (C) 2010  Nathan Lutchansky <lutchann@litech.org>
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
 */

#include <tayga.h>

extern struct config *gcfg;
extern time_t now;

int validate_ip4_addr(const struct in_addr *a)
{
	/* First octet == 0 */
	if (!(a->s_addr & htonl(0xff000000)))
		return -1;

	/* First octet == 127 */
	if ((a->s_addr & htonl(0xff000000)) == htonl(0x7f000000))
		return -1;

	/* Link-local block 169.254.0.0/16 */
	if ((a->s_addr & htonl(0xffff0000)) == htonl(0xa9fe0000))
		return -1;

	/* Class D & E */
	if ((a->s_addr & htonl(0xe0000000)) == htonl(0xe0000000))
		return -1;

	return 0;
}

int validate_ip6_addr(const struct in6_addr *a)
{
	/* Well-known prefix for NAT64 */
	if (a->s6_addr32[0] == WKPF && !a->s6_addr32[1] && !a->s6_addr32[2])
		return 0;

	/* Reserved per RFC 2373 */
	if (!a->s6_addr[0])
		return -1;

	/* Multicast addresses */
	if (a->s6_addr[0] == 0xff)
		return -1;

	/* Link-local unicast addresses */
	if ((a->s6_addr16[0] & htons(0xffc0)) == htons(0xfe80))
		return -1;

	return 0;
}

int is_private_ip4_addr(const struct in_addr *a)
{
	/* 10.0.0.0/8 */
	if ((a->s_addr & htonl(0xff000000)) == htonl(0x0a000000))
		return -1;

	/* 172.16.0.0/12 */
	if ((a->s_addr & htonl(0xfff00000)) == htonl(0xac100000))
		return -1;

	/* 192.0.2.0/24 */
	if ((a->s_addr & htonl(0xffffff00)) == htonl(0xc0000200))
		return -1;

	/* 192.168.0.0/16 */
	if ((a->s_addr & htonl(0xffff0000)) == htonl(0xc0a80000))
		return -1;

	/* 198.18.0.0/15 */
	if ((a->s_addr & htonl(0xfffe0000)) == htonl(0xc6120000))
		return -1;

	/* 198.51.100.0/24 */
	if ((a->s_addr & htonl(0xffffff00)) == htonl(0xc6336400))
		return -1;

	/* 203.0.113.0/24 */
	if ((a->s_addr & htonl(0xffffff00)) == htonl(0xcb007100))
		return -1;

	return 0;
}

int calc_ip4_mask(struct in_addr *mask, const struct in_addr *addr, int len)
{
	mask->s_addr = htonl(~((1 << (32 - len)) - 1));
	if (addr && (addr->s_addr & ~mask->s_addr))
		return -1;
	return 0;

}

int calc_ip6_mask(struct in6_addr *mask, const struct in6_addr *addr, int len)
{
	if (len > 32) {
		mask->s6_addr32[0] = ~0;
		if (len > 64) {
			mask->s6_addr32[1] = ~0;
			if (len > 96) {
				mask->s6_addr32[2] = ~0;
				mask->s6_addr32[3] =
					htonl(~((1 << (128 - len)) - 1));
			} else {
				mask->s6_addr32[2] =
					htonl(~((1 << (96 - len)) - 1));
				mask->s6_addr32[3] = 0;
			}
		} else {
			mask->s6_addr32[1] = htonl(~((1 << (64 - len)) - 1));
			mask->s6_addr32[2] = 0;
			mask->s6_addr32[3] = 0;
		}
	} else {
		mask->s6_addr32[0] = htonl(~((1 << (32 - len)) - 1));
		mask->s6_addr32[1] = 0;
		mask->s6_addr32[2] = 0;
		mask->s6_addr32[3] = 0;
	}
	if (!addr)
		return 0;
	if ((addr->s6_addr32[0] & ~mask->s6_addr32[0]) ||
			(addr->s6_addr32[1] & ~mask->s6_addr32[1]) ||
			(addr->s6_addr32[2] & ~mask->s6_addr32[2]) ||
			(addr->s6_addr32[3] & ~mask->s6_addr32[3]))
		return -1;
	return 0;
}

static uint32_t hash_ip4(const struct in_addr *addr4)
{
	return ((uint32_t)(addr4->s_addr *
				gcfg->rand[0])) >> (32 - gcfg->hash_bits);
}

static uint32_t hash_ip6(const struct in6_addr *addr6)
{
	uint32_t h;

	h = ((uint32_t)addr6->s6_addr16[0] + gcfg->rand[0]) *
		((uint32_t)addr6->s6_addr16[1] + gcfg->rand[1]);
	h ^= ((uint32_t)addr6->s6_addr16[2] + gcfg->rand[2]) *
		((uint32_t)addr6->s6_addr16[3] + gcfg->rand[3]);
	h ^= ((uint32_t)addr6->s6_addr16[4] + gcfg->rand[4]) *
		((uint32_t)addr6->s6_addr16[5] + gcfg->rand[5]);
	h ^= ((uint32_t)addr6->s6_addr16[6] + gcfg->rand[6]) *
		((uint32_t)addr6->s6_addr16[7] + gcfg->rand[7]);
	return h >> (32 - gcfg->hash_bits);
}

static void add_to_hash_table(struct cache_entry *c, uint32_t hash4,
		uint32_t hash6)
{
	list_add(&c->hash4, &gcfg->hash_table4[hash4]);
	list_add(&c->hash6, &gcfg->hash_table6[hash6]);
}

void create_cache(void)
{
	int i, hash_size = 1 << gcfg->hash_bits;
	struct list_head *entry;
	struct cache_entry *c;

	if (gcfg->hash_table4) {
		free(gcfg->hash_table4);
		free(gcfg->hash_table6);
	}

	gcfg->hash_table4 = (struct list_head *)
				malloc(hash_size * sizeof(struct list_head));
	gcfg->hash_table6 = (struct list_head *)
				malloc(hash_size * sizeof(struct list_head));
	if (!gcfg->hash_table4 || !gcfg->hash_table6) {
		slog(LOG_CRIT, "unable to allocate %d bytes for hash table\n",
				hash_size * sizeof(struct list_head));
		exit(1);
	}
	for (i = 0; i < hash_size; ++i) {
		INIT_LIST_HEAD(&gcfg->hash_table4[i]);
		INIT_LIST_HEAD(&gcfg->hash_table6[i]);
	}

	if (list_empty(&gcfg->cache_pool) && list_empty(&gcfg->cache_active)) {
		c = calloc(gcfg->cache_size, sizeof(struct cache_entry));
		for (i = 0; i < gcfg->cache_size; ++i) {
			INIT_LIST_HEAD(&c->list);
			INIT_LIST_HEAD(&c->hash4);
			INIT_LIST_HEAD(&c->hash6);
			list_add_tail(&c->list, &gcfg->cache_pool);
			++c;
		}
	} else {
		list_for_each(entry, &gcfg->cache_active) {
			c = list_entry(entry, struct cache_entry, list);
			INIT_LIST_HEAD(&c->hash4);
			INIT_LIST_HEAD(&c->hash6);
			add_to_hash_table(c, hash_ip4(&c->addr4),
						hash_ip6(&c->addr6));
		}
	}
}

static struct cache_entry *cache_insert(const struct in_addr *addr4,
		const struct in6_addr *addr6,
		uint32_t hash4, uint32_t hash6)
{
	struct cache_entry *c;

	if (list_empty(&gcfg->cache_pool))
		return NULL;
	c = list_entry(gcfg->cache_pool.next, struct cache_entry, list);
	c->addr4 = *addr4;
	c->addr6 = *addr6;
	c->last_use = now;
	c->flags = 0;
	c->ip4_ident = 1;
	list_add(&c->list, &gcfg->cache_active);
	add_to_hash_table(c, hash4, hash6);
	return c;
}

struct map4 *find_map4(const struct in_addr *addr4)
{
	struct list_head *entry;
	struct map4 *m;

	list_for_each(entry, &gcfg->map4_list) {
		m = list_entry(entry, struct map4, list);
		if (m->addr.s_addr == (m->mask.s_addr & addr4->s_addr))
			return m;
	}
	return NULL;
}

struct map6 *find_map6(const struct in6_addr *addr6)
{
	struct list_head *entry;
	struct map6 *m;

	list_for_each(entry, &gcfg->map6_list) {
		m = list_entry(entry, struct map6, list);
		if (IN6_IS_IN_NET(addr6, &m->addr, &m->mask))
			return m;
	}
	return NULL;
}

int insert_map4(struct map4 *m, struct map4 **conflict)
{
	struct list_head *entry;
	struct map4 *s;

	list_for_each(entry, &gcfg->map4_list) {
		s = list_entry(entry, struct map4, list);
		if (s->prefix_len < m->prefix_len)
			break;
		if (s->prefix_len == m->prefix_len &&
				s->addr.s_addr == m->addr.s_addr)
			goto conflict;
	}
	list_add_tail(&m->list, entry);
	return 0;

conflict:
	if (conflict)
		*conflict = s;
	return -1;
}

int insert_map6(struct map6 *m, struct map6 **conflict)
{
	struct list_head *entry, *insert_pos = NULL;
	struct map6 *s;

	list_for_each(entry, &gcfg->map6_list) {
		s = list_entry(entry, struct map6, list);
		if (s->prefix_len < m->prefix_len) {
			if (IN6_IS_IN_NET(&m->addr, &s->addr, &s->mask))
				goto conflict;
			if (!insert_pos)
				insert_pos = entry;
		} else {
			if (IN6_IS_IN_NET(&s->addr, &m->addr, &m->mask))
				goto conflict;
		}
	}
	list_add_tail(&m->list, insert_pos ? insert_pos : &gcfg->map6_list);
	return 0;

conflict:
	if (conflict)
		*conflict = s;
	return -1;
}

int append_to_prefix(struct in6_addr *addr6, const struct in_addr *addr4,
		const struct in6_addr *prefix, int prefix_len)
{
	switch (prefix_len) {
	case 32:
		addr6->s6_addr32[0] = prefix->s6_addr32[0];
		addr6->s6_addr32[1] = addr4->s_addr;
		addr6->s6_addr32[2] = 0;
		addr6->s6_addr32[3] = 0;
		return 0;
	case 40:
		addr6->s6_addr32[0] = prefix->s6_addr32[0];
#if __BYTE_ORDER == __BIG_ENDIAN
		addr6->s6_addr32[1] = prefix->s6_addr32[1] |
					(addr4->s_addr >> 8);
		addr6->s6_addr32[2] = (addr4->s_addr << 16) & 0x00ff0000;
#else
# if __BYTE_ORDER == __LITTLE_ENDIAN
		addr6->s6_addr32[1] = prefix->s6_addr32[1] |
					(addr4->s_addr << 8);
		addr6->s6_addr32[2] = (addr4->s_addr >> 16) & 0x0000ff00;
# endif
#endif
		addr6->s6_addr32[3] = 0;
		return 0;
	case 48:
		addr6->s6_addr32[0] = prefix->s6_addr32[0];
#if __BYTE_ORDER == __BIG_ENDIAN
		addr6->s6_addr32[1] = prefix->s6_addr32[1] |
					(addr4->s_addr >> 16);
		addr6->s6_addr32[2] = (addr4->s_addr << 8) & 0x00ffff00;
#else
# if __BYTE_ORDER == __LITTLE_ENDIAN
		addr6->s6_addr32[1] = prefix->s6_addr32[1] |
					(addr4->s_addr << 16);
		addr6->s6_addr32[2] = (addr4->s_addr >> 8) & 0x00ffff00;
# endif
#endif
		addr6->s6_addr32[3] = 0;
		return 0;
	case 56:
		addr6->s6_addr32[0] = prefix->s6_addr32[0];
#if __BYTE_ORDER == __BIG_ENDIAN
		addr6->s6_addr32[1] = prefix->s6_addr32[1] |
					(addr4->s_addr >> 24);
		addr6->s6_addr32[2] = addr4->s_addr & 0x00ffffff;
#else
# if __BYTE_ORDER == __LITTLE_ENDIAN
		addr6->s6_addr32[1] = prefix->s6_addr32[1] |
					(addr4->s_addr << 24);
		addr6->s6_addr32[2] = addr4->s_addr & 0xffffff00;
# endif
#endif
		addr6->s6_addr32[3] = 0;
		return 0;
	case 64:
		addr6->s6_addr32[0] = prefix->s6_addr32[0];
		addr6->s6_addr32[1] = prefix->s6_addr32[1];
#if __BYTE_ORDER == __BIG_ENDIAN
		addr6->s6_addr32[2] = addr4->s_addr >> 8;
		addr6->s6_addr32[3] = addr4->s_addr << 24;
#else
# if __BYTE_ORDER == __LITTLE_ENDIAN
		addr6->s6_addr32[2] = addr4->s_addr << 8;
		addr6->s6_addr32[3] = addr4->s_addr >> 24;
# endif
#endif
		return 0;
	case 96:
		if (prefix->s6_addr32[0] == WKPF &&
				is_private_ip4_addr(addr4))
			return -1;
		addr6->s6_addr32[0] = prefix->s6_addr32[0];
		addr6->s6_addr32[1] = prefix->s6_addr32[1];
		addr6->s6_addr32[2] = prefix->s6_addr32[2];
		addr6->s6_addr32[3] = addr4->s_addr;
		return 0;
	default:
		return -1;
	}
}

int map_ip4_to_ip6(struct in6_addr *addr6, const struct in_addr *addr4,
		struct cache_entry **c_ptr)
{
	uint32_t hash;
	struct list_head *entry;
	struct cache_entry *c;
	struct map4 *map4;
	struct map_static *s;
	struct map_dynamic *d = NULL;

	if (gcfg->cache_size) {
		hash = hash_ip4(addr4);

		list_for_each(entry, &gcfg->hash_table4[hash]) {
			c = list_entry(entry, struct cache_entry, hash4);
			if (addr4->s_addr == c->addr4.s_addr) {
				*addr6 = c->addr6;
				c->last_use = now;
				if (c_ptr)
					*c_ptr = c;
				return 0;
			}
		}
	}

	map4 = find_map4(addr4);

	if (!map4)
		return -1;

	switch (map4->type) {
	case MAP_TYPE_STATIC:
		s = container_of(map4, struct map_static, map4);
		*addr6 = s->map6.addr;
		break;
	case MAP_TYPE_RFC6052:
		s = container_of(map4, struct map_static, map4);
		if (append_to_prefix(addr6, addr4, &s->map6.addr,
					s->map6.prefix_len) < 0)
			return -1;
		break;
	case MAP_TYPE_DYNAMIC_POOL:
		return -1;
	case MAP_TYPE_DYNAMIC_HOST:
		d = container_of(map4, struct map_dynamic, map4);
		*addr6 = d->map6.addr;
		d->last_use = now;
		break;
	default:
		return -1;
	}

	if (gcfg->cache_size) {
		c = cache_insert(addr4, addr6, hash, hash_ip6(addr6));

		if (c_ptr)
			*c_ptr = c;
		if (d) {
			d->cache_entry = c;
			if (c)
				c->flags |= CACHE_F_REP_AGEOUT;
		}
	}

	return 0;
}

static int extract_from_prefix(struct in_addr *addr4,
		const struct in6_addr *addr6, int prefix_len)
{
	switch (prefix_len) {
	case 32:
		if (addr6->s6_addr32[2] || addr6->s6_addr32[3])
			return -1;
		addr4->s_addr = addr6->s6_addr32[1];
		break;
	case 40:
		if (addr6->s6_addr32[2] & htonl(0xff00ffff) ||
				addr6->s6_addr32[3])
			return -1;
#if __BYTE_ORDER == __BIG_ENDIAN
		addr4->s_addr = (addr6->s6_addr32[1] << 8) | addr6->s6_addr[9];
#else
# if __BYTE_ORDER == __LITTLE_ENDIAN
		addr4->s_addr = (addr6->s6_addr32[1] >> 8) |
				(addr6->s6_addr32[2] << 16);
# endif
#endif
		break;
	case 48:
		if (addr6->s6_addr32[2] & htonl(0xff0000ff) ||
				addr6->s6_addr32[3])
			return -1;
#if __BYTE_ORDER == __BIG_ENDIAN
		addr4->s_addr = (addr6->s6_addr16[3] << 16) |
				(addr6->s6_addr32[2] >> 8);
#else
# if __BYTE_ORDER == __LITTLE_ENDIAN
		addr4->s_addr = addr6->s6_addr16[3] |
				(addr6->s6_addr32[2] << 8);
# endif
#endif
		break;
	case 56:
		if (addr6->s6_addr[8] || addr6->s6_addr32[3])
			return -1;
#if __BYTE_ORDER == __BIG_ENDIAN
		addr4->s_addr = (addr6->s6_addr[7] << 24) |
				addr6->s6_addr32[2];
#else
# if __BYTE_ORDER == __LITTLE_ENDIAN
		addr4->s_addr = addr6->s6_addr[7] |
				addr6->s6_addr32[2];
# endif
#endif
		break;
	case 64:
		if (addr6->s6_addr[8] ||
				addr6->s6_addr32[3] & htonl(0x00ffffff))
			return -1;
#if __BYTE_ORDER == __BIG_ENDIAN
		addr4->s_addr = (addr6->s6_addr32[2] << 8) |
				addr6->s6_addr[12];
#else
# if __BYTE_ORDER == __LITTLE_ENDIAN
		addr4->s_addr = (addr6->s6_addr32[2] >> 8) |
				(addr6->s6_addr32[3] << 24);
# endif
#endif
		break;
	case 96:
		addr4->s_addr = addr6->s6_addr32[3];
		break;
	default:
		return -1;
	}
	return validate_ip4_addr(addr4);
}

int map_ip6_to_ip4(struct in_addr *addr4, const struct in6_addr *addr6,
		struct cache_entry **c_ptr, int dyn_alloc)
{
	uint32_t hash;
	struct list_head *entry;
	struct cache_entry *c;
	struct map6 *map6;
	struct map_static *s;
	struct map_dynamic *d = NULL;

	if (gcfg->cache_size) {
		hash = hash_ip6(addr6);

		list_for_each(entry, &gcfg->hash_table6[hash]) {
			c = list_entry(entry, struct cache_entry, hash6);
			if (IN6_ARE_ADDR_EQUAL(addr6, &c->addr6)) {
				*addr4 = c->addr4;
				c->last_use = now;
				if (c_ptr)
					*c_ptr = c;
				return 0;
			}
		}
	}

	map6 = find_map6(addr6);

	if (!map6) {
		if (dyn_alloc)
			map6 = assign_dynamic(addr6);
		if (!map6)
			return -1;
	}

	switch (map6->type) {
	case MAP_TYPE_STATIC:
		s = container_of(map6, struct map_static, map6);
		*addr4 = s->map4.addr;
		break;
	case MAP_TYPE_RFC6052:
		if (extract_from_prefix(addr4, addr6, map6->prefix_len) < 0)
			return -1;
		if (map6->addr.s6_addr32[0] == WKPF &&
				is_private_ip4_addr(addr4))
			return -1;
		s = container_of(map6, struct map_static, map6);
		if (find_map4(addr4) != &s->map4)
			return -1;
		break;
	case MAP_TYPE_DYNAMIC_HOST:
		d = container_of(map6, struct map_dynamic, map6);
		*addr4 = d->map4.addr;
		d->last_use = now;
		break;
	default:
		return -1;
	}

	if (gcfg->cache_size) {
		c = cache_insert(addr4, addr6, hash_ip4(addr4), hash);

		if (c_ptr)
			*c_ptr = c;
		if (d) {
			d->cache_entry = c;
			if (c)
				c->flags |= CACHE_F_REP_AGEOUT;
		}
	}

	return 0;
}

static void report_ageout(struct cache_entry *c)
{
	struct map4 *m4;
	struct map_dynamic *d;

	m4 = find_map4(&c->addr4);
	if (!m4 || m4->type != MAP_TYPE_DYNAMIC_HOST)
		return;
	d = container_of(m4, struct map_dynamic, map4);
	d->last_use = c->last_use;
	d->cache_entry = NULL;
}

void addrmap_maint(void)
{
	struct list_head *entry, *next;
	struct cache_entry *c;

	list_for_each_safe(entry, next, &gcfg->cache_active) {
		c = list_entry(entry, struct cache_entry, list);
		if (c->last_use + CACHE_MAX_AGE < now) {
			if (c->flags & CACHE_F_REP_AGEOUT)
				report_ageout(c);
			list_add(&c->list, &gcfg->cache_pool);
			list_del(&c->hash4);
			list_del(&c->hash6);
		}
	}
}
