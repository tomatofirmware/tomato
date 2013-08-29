/*
 * Proxy ARP capabilitiy enables an AP to indicate that the non-AP STA will not
 * receive ARP frames.  The Proxy ARP cappability enables the non-AP STA to remain
 * in power-save for longer period of time.
 * Original implements in wlc_l2_filter.c for HSPOT2.0.  Network Power Save(NPS)
 * was also defined to support the capability.  We abstract core Proxy ARP implement
 * and moved it from wlc_l2_filter.c to a standalone module to support
 * dual band usbap.
 *
 * Copyright (C) 2012, Broadcom Corporation
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 *
 * $Id: $
 */
#include <typedefs.h>
#include <bcmdefs.h>
#include <osl.h>
#include <bcmutils.h>
#include <siutils.h>
#include <bcmendian.h>
#include <proto/802.11.h>
#include <proto/802.3.h>
#include <proto/vlan.h>
#include <proto/bcmip.h>
#include <proto/bcmarp.h>
#include <proto/bcmipv6.h>
#include <proto/bcmudp.h>
#include <proto/bcmdhcp.h>
#include <proto/bcmproto.h>
#include <proxyarp/proxyarp.h>

#define ALIGN_ADJ_BUFLEN			2 /* Adjust for ETHER_HDR_LEN pull in linux */
#define	EAIP_TUPLE_TIMEOUT			600 * 1000
#define	EAIP_TUPLE_TABLE_SIZE			256

/* This marks the start of a packed structure section. */
#include <packed_section_start.h>
struct aging_link {
	struct aging_link *prev;
	struct aging_link *next;
};

BWL_PRE_PACKED_STRUCT struct eaip_tuple {
	struct eaip_tuple	*next;
	struct ether_addr	ea;
	struct aging_link	alink;
	uint32			used;		/* time stamp */
	bcm_tlv_t ip;
} BWL_POST_PACKED_STRUCT;
/* This marks the end of a packed structure section. */
#include <packed_section_end.h>

typedef struct eaip_tuple eaip_tuple_t;
static eaip_tuple_t *eaip_tbl[EAIP_TUPLE_TABLE_SIZE];
static struct aging_link *aq_head, *aq_tail;	/* aging queue head/tail */

static proxyarp_info_t *pa_info;
#ifdef BCMDBG
static uint8 proxyarp_msglevel = 0xff;
#endif /* BCMDBG */

#define ND_OPTION_LEN_ETHER	1

static inline void
aging_queue_enq(struct aging_link *node)
{
	node->prev = NULL;
	node->next = aq_head;

	if (aq_head != NULL)
		aq_head->prev = node;
	aq_head = node;

	if (aq_tail == NULL)
		aq_tail = node;
}

static inline void
aging_queue_deq(struct aging_link *node)
{
	if (node->prev == NULL)
		aq_head = node->next;
	else
		node->prev->next = node->next;

	if (node->next == NULL)
		aq_tail = node->prev;
	else
		node->next->prev = node->prev;
}
#ifdef BCMDBG
static inline uint8 *
eaip_str(uint8 *ea, uint8 *ip, uint8 ip_ver, uint8 *buf, uint32 buflen)
{
	char eastr[18];
	char ipstr[64];

	bzero(eastr, 18);
	bzero(ipstr, 64);
	bzero(buf, buflen);

	bcm_ether_ntoa((struct ether_addr *)ea, eastr);
	if (ip_ver == IP_VER_4)
		bcm_ip_ntoa((struct ipv4_addr *)ip, ipstr);
	else
		bcm_format_hex(ipstr, ip, 16);

	snprintf(buf, buflen - 1, "%s(%s)", eastr, ipstr);

	return buf;
}
#endif /* BCMDBG */
static int
eaip_add(proxyarp_info_t *pah, uint8 *ea, uint8 *ip, uint8 ip_ver)
{
	eaip_tuple_t *node;
	uint16 idx, ip_len;

	switch (ip_ver) {
	case IP_VER_4:
		if (IPV4_ADDR_NULL(ip) || IPV4_ADDR_BCAST(ip))
			return BCME_ERROR;

		idx = ip[IPV4_ADDR_LEN - 1];
		ip_len = IPV4_ADDR_LEN;
		break;
	case IP_VER_6:
		if (IPV6_ADDR_NULL(ip))
			return BCME_ERROR;

		idx = ip[IPV6_ADDR_LEN - 1];
		ip_len = IPV6_ADDR_LEN;
		break;
	default:
		return BCME_ERROR;
		break;
	}

	if ((node = MALLOC(pah->osh, sizeof(eaip_tuple_t) + ip_len)) == NULL) {
		PROXYARP_ERR("allocating new eaip for ipv%d error !!\n", ip_ver);
		return BCME_NOMEM;
	}

	bzero(node, sizeof(eaip_tuple_t) + ip_len);
	bcopy(ea, &node->ea, ETHER_ADDR_LEN);
	node->used = OSL_SYSUPTIME();
	node->ip.id = ip_ver;
	node->ip.len = ip_len;
	bcopy(ip, node->ip.data, ip_len);
	node->next = eaip_tbl[idx];
	eaip_tbl[idx] = node;

	pah->count++;

	/* add to aging queue head */
	aging_queue_enq(&node->alink);

	return BCME_OK;
}

static int
eaip_del(proxyarp_info_t *pah, uint8 *ea, uint8 *ip, uint8 ip_ver)
{
	eaip_tuple_t *prev, *node;
	uint16 idx, ip_len;

	prev = NULL;
	switch (ip_ver) {
	case IP_VER_4:
		idx = ip[3];
		ip_len = IPV4_ADDR_LEN;
		break;
	case IP_VER_6:
		idx = ip[15];
		ip_len = IPV6_ADDR_LEN;
		break;
	default:
		return BCME_ERROR;
		break;
	}

	node = eaip_tbl[idx];
	while (node) {
		/* found target */
		if (node->ip.id == ip_ver &&
		    bcmp(node->ip.data, ip, ip_len) == 0 &&
		    bcmp((uint8 *)&node->ea, ea, ETHER_ADDR_LEN) == 0) {
			if (prev == NULL)
				eaip_tbl[idx] = node->next;
			else
				prev->next = node->next;
			break;
		}
		prev = node;
		node = node->next;
	}

	if (node != NULL) {
		MFREE(pah->osh, node, sizeof(eaip_tuple_t) + ip_len);
		pah->count--;
	}

	return BCME_OK;
}

static bool
eaip_timeout(eaip_tuple_t *node)
{
	uint32 now = OSL_SYSUPTIME();

	if (now - node->used > EAIP_TUPLE_TIMEOUT)
		return TRUE;

	return FALSE;
}

static eaip_tuple_t *
eaip_find_by_ip(uint8 *ip, uint8 ip_ver)
{
	eaip_tuple_t *node;
	uint16 ip_len;

	switch (ip_ver) {
	case IP_VER_4:
		node = eaip_tbl[ip[3]];
		ip_len = IPV4_ADDR_LEN;
		break;
	case IP_VER_6:
		node = eaip_tbl[ip[15]];
		ip_len = IPV6_ADDR_LEN;
		break;
	default:
		return NULL;
		break;
	}

	while (node) {
		/* found target */
		if (node->ip.id == ip_ver && bcmp(node->ip.data, ip, ip_len) == 0) {
			node->used = OSL_SYSUPTIME();

			/* removed from aging queue and re-add to head of aging queue */
			aging_queue_deq(&node->alink);
			aging_queue_enq(&node->alink);
			break;
		}
		node = node->next;
	}

	return node;
}

static int
proxyarp_update(uint8 *ea, uint8 *ip, uint8 ip_ver)
{
	eaip_tuple_t *node;
	int ret = BCME_OK;

	/* basic ether addr check */
	if (ETHER_ISNULLADDR(ea) || ETHER_ISBCAST(ea) || ETHER_ISMULTI(ea)) {
		PROXYARP_ERR("Invalid Ether addr\n");
		return BCME_OK;
	}

	PA_LOCK(pa_info);
	node = eaip_find_by_ip(ip, ip_ver);

	if (node != NULL) {
		if (bcmp(ea, (uint8 *)&node->ea, ETHER_ADDR_LEN) != 0) {
			/* conflct ip address ! */
			PROXYARP_ERR("Ipaddr Conflict !!!!\n");
			ret = BCME_ERROR;
		}
	}
	else {
		/* no eaip tuple.  we need to add this tuple */
		if (eaip_add(pa_info, ea, ip, ip_ver) == BCME_OK) {
#ifdef BCMDBG
			char eaip_buf[128];
#endif /* BCMDBG */
			PROXYARP_INFO("Create IPv%d %s\n", ip_ver,
				eaip_str(ea, ip, ip_ver, eaip_buf, 128));
		}
	}
	PA_UNLOCK(pa_info);

	return ret;
}

static void *
proxyarp_alloc_reply(osl_t *osh, uint32 pktlen, uint8 *src_ea, uint8 *dst_ea,
	uint16 ea_type, bool snap, void **p)
{
	void *pkt;
	uint8 *frame;

	/* adjust pktlen since skb->data is aligned to 2 */
	pktlen += ALIGN_ADJ_BUFLEN;

	if ((pkt = PKTGET(osh, pktlen, FALSE)) == NULL) {
		PROXYARP_ERR("%s %d: PKTGET failed\n", __func__, __LINE__);
		return NULL;
	}
	/* adjust for pkt->data aligned */
	PKTPULL(osh, pkt, ALIGN_ADJ_BUFLEN);
	frame = PKTDATA(osh, pkt);

	/* Create 14-byte eth header, plus snap header if applicable */
	bcopy(src_ea, frame + ETHER_SRC_OFFSET, ETHER_ADDR_LEN);
	bcopy(dst_ea, frame + ETHER_DEST_OFFSET, ETHER_ADDR_LEN);
	if (snap) {
		hton16_ua_store(pktlen, frame + ETHER_TYPE_OFFSET);
		bcopy(llc_snap_hdr, frame + ETHER_HDR_LEN, SNAP_HDR_LEN);
		hton16_ua_store(ea_type, frame + ETHER_HDR_LEN + SNAP_HDR_LEN);
	} else
		hton16_ua_store(ea_type, frame + ETHER_TYPE_OFFSET);

	*p = (void *)(frame + ETHER_HDR_LEN + (snap ? SNAP_HDR_LEN + ETHER_TYPE_LEN : 0));

	return pkt;
}
/*
 * Moved from wlc_l2_filter.c
 */
static uint8
proxyarp_arpreply(osl_t *osh, struct bcmarp *arp_req, bool snap, void **reply)
{
	struct bcmarp *arp_reply;
	eaip_tuple_t *target;
	void *pkt;
	uint16 pktlen = ETHER_HDR_LEN + ARP_DATA_LEN + (snap ? SNAP_HDR_LEN + ETHER_TYPE_LEN : 0);

	/* no valid entry, return */
	PA_LOCK(pa_info);
	target = eaip_find_by_ip(arp_req->dst_ip, IP_VER_4);
	PA_UNLOCK(pa_info);

	/* no target exist, return */
	if (target == NULL) {
		return FRAME_NOP;
	}

	/* src_ea equals to dst_ea, should be dropped */
	if (bcmp(arp_req->src_eth, &target->ea, ETHER_ADDR_LEN) == 0) {
		PROXYARP_INFO("ARP-Req ask for its own ea, avoid reply to these frame\n");
		return FRAME_DROP;
	}

	/* Create 42-byte arp-reply data frame */
	if ((pkt = proxyarp_alloc_reply(osh, pktlen, (uint8 *)&target->ea, arp_req->src_eth,
		ETHER_TYPE_ARP, snap, (void **)&arp_reply)) == NULL) {
		return FRAME_NOP;
	}

	/* construct 28-byte arp-reply data frame */
	/* copy first 6 bytes as-is; i.e., htype, ptype, hlen, plen */
	bcopy(arp_req, arp_reply, ARP_OPC_OFFSET);
	arp_reply->oper = HTON16(ARP_OPC_REPLY);
	/* Copy dst eth and ip addresses */
	bcopy(&target->ea, arp_reply->src_eth, ETHER_ADDR_LEN);
	bcopy(&target->ip.data, arp_reply->src_ip, IPV4_ADDR_LEN);
	bcopy(arp_req->src_eth, arp_reply->dst_eth, ETHER_ADDR_LEN);
	bcopy(arp_req->src_ip, arp_reply->dst_ip, IPV4_ADDR_LEN);

	*reply = (void *)pkt;

	return FRAME_TAKEN;
}
/*
 * moved from wlc_l2_filter.c.  The length of the option including
 * the type and length fields in units of 8 octets
 */
static bcm_tlv_t *
parse_nd_options(void *buf, int buflen, uint key)
{
	bcm_tlv_t *elt;
	int totlen;

	elt = (bcm_tlv_t*)buf;
	totlen = buflen;

	/* find tagged parameter */
	while (totlen >= TLV_HDR_LEN) {
		int len = elt->len * 8;

		/* validate remaining totlen */
		if ((elt->id == key) &&
		    (totlen >= len))
			return (elt);

		elt = (bcm_tlv_t*)((uint8*)elt + len);
		totlen -= len;
	}

	return NULL;
}
/*
 * moved from wlc_l2_filter.c
 */
static uint16
calc_checksum(uint8 *src_ipa, uint8 *dst_ipa, uint32 ul_len, uint8 prot, uint8 *ul_data)
{
	uint16 *startpos;
	uint32 sum = 0;
	int i;
	uint16 answer = 0;

	if (src_ipa) {
		uint8 ph[8];
		for (i = 0; i < (IPV6_ADDR_LEN / 2); i++) {
			sum += *((uint16 *)src_ipa);
			src_ipa += 2;
		}

		for (i = 0; i < (IPV6_ADDR_LEN / 2); i++) {
			sum += *((uint16 *)dst_ipa);
			dst_ipa += 2;
		}

		*((uint32 *)ph) = hton32(ul_len);
		*((uint32 *)(ph+4)) = 0;
		ph[7] = prot;
		startpos = (uint16 *)ph;
		for (i = 0; i < 4; i++) {
			sum += *startpos++;
		}
	}

	startpos = (uint16 *)ul_data;
	while (ul_len > 1) {
		sum += *startpos++;
		ul_len -= 2;
	}

	if (ul_len == 1) {
		*((uint8 *)(&answer)) = *((uint8 *)startpos);
		sum += answer;
	}

	sum = (sum >> 16) + (sum & 0xffff);
	sum += (sum >> 16);
	answer = ~sum;

	return answer;
}
/*
 * moved from wlc_l2_filter.c
 */
static uint8
proxyarp_nareply(osl_t *osh, uint8 *data, bool snap, uint8 *src_mac, uint8 dup_ip, void **reply)
{
	void *pkt;
	uint8 *na;
	uint16 pktlen = ETHER_HDR_LEN + NEIGHBOR_ADVERTISE_DATA_LEN +
	                 (snap ? SNAP_HDR_LEN + ETHER_TYPE_LEN: 0);
	uint16 checksum;
	eaip_tuple_t *target;
	uint8 ipv6_mcast_allnode[16] = {0xff, 0x2, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
		0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x1};

	/* no valid entry, return */
	PA_LOCK(pa_info);
	target = eaip_find_by_ip(data + NEIGHBOR_ADVERTISE_TGT_IPV6_OFFSET, IP_VER_6);
	PA_UNLOCK(pa_info);

	if (target == NULL) {
		return FRAME_NOP;
	}

	if (bcmp(src_mac, (uint8 *)&target->ea, 6) == 0) {
		return FRAME_DROP;
	}

	/* Create 72 bytes neighbor advertisement data frame */
	if ((pkt = proxyarp_alloc_reply(osh, pktlen, (uint8 *)&target->ea, src_mac,
		ETHER_TYPE_IPV6, snap, (void **)&na)) == NULL) {
		return FRAME_NOP;
	}

	/* construct 40 bytes ipv6 header */
	bcopy(data, na, IPV6_SRC_IP_OFFSET);
	hton16_ua_store((NEIGHBOR_ADVERTISE_DATA_LEN - NEIGHBOR_ADVERTISE_TYPE_OFFSET),
		(na + IPV6_PAYLOAD_LEN_OFFSET));
	*(na + IPV6_HOP_LIMIT_OFFSET) = 255;
	bcopy(data+NEIGHBOR_ADVERTISE_TGT_IPV6_OFFSET, na+IPV6_SRC_IP_OFFSET, IPV6_ADDR_LEN);

	if (dup_ip)
		bcopy(ipv6_mcast_allnode,  na + IPV6_DEST_IP_OFFSET, IPV6_ADDR_LEN);
	else
		bcopy(data+IPV6_SRC_IP_OFFSET, na+IPV6_DEST_IP_OFFSET, IPV6_ADDR_LEN);

	/* Create 32 bytes icmpv6 NA frame body */
	bzero((na + NEIGHBOR_ADVERTISE_TYPE_OFFSET),
		(NEIGHBOR_ADVERTISE_TGT_IPV6_OFFSET - NEIGHBOR_ADVERTISE_TYPE_OFFSET));
	*(na + NEIGHBOR_ADVERTISE_TYPE_OFFSET) = NEIGHBOR_ADVERTISE_TYPE;
	*(na + NEIGHBOR_ADVERTISE_FLAGS_OFFSET) = NEIGHBOR_ADVERTISE_FLAGS_VALUE;
	bcopy(data+NEIGHBOR_ADVERTISE_TGT_IPV6_OFFSET, na+NEIGHBOR_ADVERTISE_TGT_IPV6_OFFSET,
		IPV6_ADDR_LEN);
	*(na + NEIGHBOR_ADVERTISE_OPTION_OFFSET) = OPT_TYPE_TGT_LINK_ADDR;
	*(na + NEIGHBOR_ADVERTISE_OPTION_OFFSET + TLV_LEN_OFF) = ND_OPTION_LEN_ETHER;
	bcopy(&target->ea, na + NEIGHBOR_ADVERTISE_OPTION_OFFSET + TLV_BODY_OFF,
		ETHER_ADDR_LEN);

	/* calculate ICMPv6 check sum */
	checksum = calc_checksum(na+IPV6_SRC_IP_OFFSET, na+IPV6_DEST_IP_OFFSET,
		NEIGHBOR_ADVERTISE_DATA_LEN - NEIGHBOR_ADVERTISE_TYPE_OFFSET,
		IP_PROT_ICMP6, na + NEIGHBOR_ADVERTISE_TYPE_OFFSET);
	*((uint16 *)(na + NEIGHBOR_ADVERTISE_CHECKSUM_OFFSET)) = checksum;

	*reply = (void *)pkt;
	return FRAME_TAKEN;
}

static struct {
	uint8 tid[4];
	uint8 l2_ea[ETHER_ADDR_LEN];
	uint8 req_ea[ETHER_ADDR_LEN];
} dhcpreq;

static void
proxyarp_dhcpreq_handle(frame_proto_t *fp)
{
	struct bcmudp_hdr *udph;
	uint8 *dhcp;
	bcm_tlv_t *msg_type;
	struct ether_header *eh;
	uint16 opt_len, offset = DHCP_OPT_OFFSET;

	udph = (struct bcmudp_hdr *)fp->l4;

	/* check if the packet is a dhcp-reply */
	if (ntoh16(udph->dst_port) != DHCP_PORT_SERVER)
		return;

	/* we got dhcp-reply.  update arp entry */
	dhcp = (uint8 *)(fp->l4 + UDP_HDR_LEN);

	/* only process DHCP reply(offer/ack) packets */
	if (*(dhcp + DHCP_TYPE_OFFSET) != DHCP_TYPE_REQUEST)
		return;

	/* First option must be magic cookie */
	if ((dhcp[offset + 0] != 0x63) || (dhcp[offset + 1] != 0x82) ||
	    (dhcp[offset + 2] != 0x53) || (dhcp[offset + 3] != 0x63))
		return;

	/* shift to real opt start offst */
	offset += 4;
	opt_len = fp->l4_len - offset - UDP_HDR_LEN;
	msg_type = bcm_parse_tlvs(&dhcp[offset], opt_len, DHCP_OPT_MSGTYPE);
	if (msg_type == NULL || msg_type->data[0] != DHCP_OPT_MSGTYPE_REQ)
		return;

	eh = (struct ether_header *)fp->l2;
	bcopy(&dhcp[DHCP_TID_OFFSET], dhcpreq.tid, 4);
	bcopy(eh->ether_shost, dhcpreq.l2_ea, ETHER_ADDR_LEN);
	bcopy(&dhcp[DHCP_CHADDR_OFFSET], dhcpreq.req_ea, ETHER_ADDR_LEN);
}

static void
proxyarp_dhcpack_handle(frame_proto_t *fp)
{
	struct bcmudp_hdr *udph;
	uint8 *dhcp;
#ifdef BCMDBG
	char eaip_buf[128];
#endif /* BCMDBG */
	bcm_tlv_t *msg_type;
	uint16 opt_len, offset = DHCP_OPT_OFFSET;

	udph = (struct bcmudp_hdr *)fp->l4;

	/* check if the packet is a dhcp-reply */
	if (ntoh16(udph->dst_port) != DHCP_PORT_CLIENT)
		return;

	/* we got dhcp-reply.  update arp entry */
	dhcp = (uint8 *)(fp->l4 + UDP_HDR_LEN);

	/* only process DHCP reply(offer/ack) packets */
	if (*(dhcp + DHCP_TYPE_OFFSET) != DHCP_TYPE_REPLY)
		return;

	if (IPV4_ADDR_NULL(&dhcp[16]))
		return;

	/* First option must be magic cookie */
	if ((dhcp[offset + 0] != 0x63) || (dhcp[offset + 1] != 0x82) ||
	    (dhcp[offset + 2] != 0x53) || (dhcp[offset + 3] != 0x63))
		return;

	offset += 4;
	opt_len = fp->l4_len - offset - UDP_HDR_LEN;
	msg_type = bcm_parse_tlvs(&dhcp[offset], opt_len, DHCP_OPT_MSGTYPE);
	if (msg_type == NULL || msg_type->data[0] != DHCP_OPT_MSGTYPE_ACK)
		return;

	/* update ipv4 */

	if (bcmp((dhcp + DHCP_TID_OFFSET), dhcpreq.tid, 4) == 0	 &&
	    bcmp((dhcp + DHCP_CHADDR_OFFSET), dhcpreq.req_ea, ETHER_ADDR_LEN) == 0) {
		PROXYARP_INFO("DHCP_ACK: %s\n", eaip_str(dhcpreq.l2_ea,
			&dhcp[DHCP_YIADDR_OFFSET], IP_VER_4, eaip_buf, 128));

		proxyarp_update(dhcpreq.l2_ea, (dhcp + DHCP_YIADDR_OFFSET), IP_VER_4);
	}
	else {
		PROXYARP_ERR("DHCP matching error\n");
	}

	bzero((uint8 *)&dhcpreq, sizeof(dhcpreq));
}

static uint8
proxyarp_arp_handle(osl_t *osh, frame_proto_t *fp, void **reply)
{
#ifdef BCMDBG
	char srceaip_buf[128];
	char dsteaip_buf[128];
#endif /* BCMDBG */
	struct bcmarp *arp = (struct bcmarp *)fp->l3;
	uint8 op = ntoh16(arp->oper);

	if (op > ARP_OPC_REPLY) {
		PROXYARP_ERR("%s %d: Invalid ARP operation(%d)\n",
			__func__, __LINE__, op);
		return FRAME_NOP;
	}

	/* we proxy ARP request when receiving it */
	/* update arp info when we receiving arp packet */
#ifdef BCMDBG
	if (op == ARP_OPC_REQUEST)
		PROXYARP_INFO("ARP_REQ: src %s - dst %s\n",
			eaip_str(arp->src_eth, arp->src_ip, IP_VER_4, srceaip_buf, 128),
			eaip_str(arp->dst_eth, arp->dst_ip, IP_VER_4, dsteaip_buf, 128));
#endif /* BCMDBG */

	/* update ipv4 */
	proxyarp_update((uint8 *)&arp->src_eth, (uint8 *)&arp->src_ip, IP_VER_4);

	if (op == ARP_OPC_REQUEST) {
		bool snap = FALSE;

		if (fp->l2_t == FRAME_L2_SNAP_H || fp->l2_t == FRAME_L2_SNAPVLAN_H)
			snap = TRUE;

		return proxyarp_arpreply(osh, arp, snap, reply);
	}

	return FRAME_NOP;
}

static uint8
proxyarp_icmp6_handle(osl_t *osh, frame_proto_t *fp, void **reply)
{
	bcm_tlv_t *link_addr;
	uint16 ip_off;
	uint8 link_type;
	uint8 *ea;
	uint8 dup_ip = 0;
	uint8 is_null_ip = 0;
	uint8 *data = fp->l3;
	uint16 datalen = fp->l3_len;
	uint8 *frame = fp->l2;

	if ((datalen < NEIGHBOR_ADVERTISE_TGT_IPV6_OFFSET + IPV6_ADDR_LEN) ||
		(data[IPV6_NEXT_HDR_OFFSET] != IP_PROT_ICMP6))
		return FRAME_NOP;

	/* check for ICMPv6 neighbour operation */
	if (data[NEIGHBOR_ADVERTISE_TYPE_OFFSET] == NEIGHBOR_ADVERTISE_TYPE) {
		ip_off = NEIGHBOR_ADVERTISE_TGT_IPV6_OFFSET;
		link_type = OPT_TYPE_TGT_LINK_ADDR;
	}
	else if (data[NEIGHBOR_ADVERTISE_TYPE_OFFSET] == NEIGHBOR_SOLICITATION_TYPE) {
		link_type = OPT_TYPE_SRC_LINK_ADDR;
		if (IPV6_ADDR_NULL((uint8)(data + NEIGHBOR_ADVERTISE_SRC_IPV6_OFFSET))) {
			ip_off = NEIGHBOR_ADVERTISE_TGT_IPV6_OFFSET;
			is_null_ip = 1;
		}
		else
			ip_off = NEIGHBOR_ADVERTISE_SRC_IPV6_OFFSET;
	}
	else
		return FRAME_NOP;

	link_addr = parse_nd_options(data + NEIGHBOR_ADVERTISE_OPTION_OFFSET,
		datalen-NEIGHBOR_ADVERTISE_OPTION_OFFSET, link_type);

	if (link_addr && link_addr->len == ND_OPTION_LEN_ETHER)
		ea = (uint8 *)&link_addr->data;
	else
		ea = frame + ETHER_SRC_OFFSET;

	/* update local address */
	dup_ip = proxyarp_update(ea, data + ip_off, IP_VER_6);

	/* duplicate address detection, reply with all-node multicast address */
	if (is_null_ip) {
		PROXYARP_INFO("receive DAD(duplicate address detection) frame, dup_ip = %d\n",
			dup_ip? 1: 0);
	}

	if (link_type == OPT_TYPE_SRC_LINK_ADDR) {
		bool snap = FALSE;
		if (fp->l2_t == FRAME_L2_SNAP_H || fp->l2_t == FRAME_L2_SNAPVLAN_H)
			snap = TRUE;

		return proxyarp_nareply(osh, data, snap, frame + ETHER_SRC_OFFSET, dup_ip, reply);
	}
	else if (link_type == OPT_TYPE_TGT_LINK_ADDR) {
	}
	else {
		PROXYARP_ERR("not a src_link or target_link (%x)\n", link_type);
	}

	return FRAME_NOP;
}

/*
 * handle ARP request frame in send path, update database in both send/receive path
 */
uint8
proxyarp_packets_handle(osl_t *osh, void *sdu, void *fproto, bool send, void **reply)
{
	frame_proto_t *fp = (frame_proto_t *)fproto;
	*reply = NULL;

	if (send) {
		/* update ipv4 and reply */
		if (fp->l3_t == FRAME_L3_ARP_H)
			return proxyarp_arp_handle(osh, (frame_proto_t *)fproto, reply);

		/* update ipv6 and reply */
		if (fp->l4_t == FRAME_L4_ICMP6_H)
			return proxyarp_icmp6_handle(osh, (frame_proto_t *)fproto, reply);

		/* update ipv4 when AP sending dhcp-ack frame */
		if (fp->l4_t == FRAME_L4_UDP_H)
			proxyarp_dhcpack_handle((frame_proto_t *)fproto);

	}
	else {
		/* update eaip_tuple timestamp */
		if (fp->l3_t == FRAME_L3_IP_H || fp->l3_t == FRAME_L3_IP6_H) {
			uint8 *ip = (fp->l3_t == FRAME_L3_IP_H)?
				fp->l3 + IPV4_SRC_IP_OFFSET: fp->l3 + IPV6_SRC_IP_OFFSET;

			PA_LOCK(pa_info);
			eaip_find_by_ip(ip, fp->l3_t);
			PA_UNLOCK(pa_info);
		}

		/* update ipv4 when AP sending dhcp-req frame */
		if (fp->l4_t == FRAME_L4_UDP_H)
			proxyarp_dhcpreq_handle((frame_proto_t *)fproto);
	}
	return FRAME_NOP;
}

void
_proxyarp_watchdog(bool all, uint8 *del_ea)
{
	/* clean up database */
	eaip_tuple_t *cur;
	uint16 idx, ip_ver;
	uint8 ea[ETHER_ADDR_LEN];
	uint8 ip[IPV6_ADDR_LEN];
#ifdef BCMDBG
	uint8 eaip_buf[128];
#endif /* BCMDBG */

	if (pa_info == NULL) {
		PROXYARP_ERR("Invalid clean-up\n");
		return;
	}

	PA_LOCK(pa_info);

	/* clean all table */
	if (all == TRUE) {
		for (idx = 0; idx < EAIP_TUPLE_TABLE_SIZE; idx++) {
			cur = eaip_tbl[idx];

			while (cur != NULL) {
				bcopy((uint8 *)&cur->ea, ea, ETHER_ADDR_LEN);
				ip_ver = cur->ip.id;

				if (ip_ver == IP_VER_4) {
					bcopy((uint8 *)&cur->ip.data, ip, IPV4_ADDR_LEN);
					ip[IPV4_ADDR_LEN] = 0;
				}
				else
					bcopy((uint8 *)&cur->ip.data, ip, IPV6_ADDR_LEN);

				PROXYARP_INFO("flush IPv%d - %s\n", ip_ver,
					eaip_str(ea, ip, ip_ver, eaip_buf, 128));

				cur = cur->next;

				eaip_del(pa_info, ea, ip, ip_ver);
			}
		}
		aq_head = aq_tail = NULL;
	}
	else {
		struct aging_link *node = aq_tail;

		while (node) {
			cur = (eaip_tuple_t *)((void *)node - OFFSETOF(struct eaip_tuple, alink));

			/* remove specific node if ea exists */
			if ((del_ea && bcmp(del_ea, (uint8 *)&cur->ea, 6) == 0) ||
			    eaip_timeout(cur) == TRUE) {
				bcopy((uint8 *)&cur->ea, ea, ETHER_ADDR_LEN);
				ip_ver = cur->ip.id;

				if (ip_ver == IP_VER_4) {
					bcopy((uint8 *)&cur->ip.data, ip, IPV4_ADDR_LEN);
					ip[IPV4_ADDR_LEN] = 0;
				}
				else
					bcopy((uint8 *)&cur->ip.data, ip, IPV6_ADDR_LEN);

				aging_queue_deq(node);
				node = node->prev;

				PROXYARP_INFO("Aging IPv%d - %s\n", ip_ver,
					eaip_str(ea, ip, ip_ver, eaip_buf, 128));

				eaip_del(pa_info, ea, ip, ip_ver);
			}
			else {
				/* no eaip to aging.  Stop */
				if (del_ea == NULL)
					break;
				node = node->prev;
			}
		}
	}

	PA_UNLOCK(pa_info);
}

bool BCMFASTPATH
proxyarp_get(void)
{
	return pa_info->enabled? TRUE: FALSE;
}

void
proxyarp_set(bool enabled)
{
	pa_info->enabled = enabled;
}

void
proxyarp_init(proxyarp_info_t *spa_info)
{
	if (spa_info == NULL) {
		PROXYARP_ERR("Init Error: handle is null\n");
		return;
	}

	bzero(&eaip_tbl, sizeof(eaip_tbl));
	aq_head = aq_tail = NULL;
	pa_info = spa_info;
	pa_info->enabled = FALSE;

	return;
}

void
proxyarp_deinit(void)
{
	_proxyarp_watchdog(TRUE, NULL);
	aq_head = aq_tail = NULL;
	bzero(&eaip_tbl, sizeof(eaip_tbl));
	pa_info = NULL;
}
