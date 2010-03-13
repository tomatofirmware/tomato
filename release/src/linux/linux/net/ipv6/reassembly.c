/*
 *	IPv6 fragment reassembly
 *	Linux INET6 implementation 
 *
 *	Authors:
 *	Pedro Roque		<roque@di.fc.ul.pt>	
 *
 *	$Id: reassembly.c,v 1.1.1.4 2003/10/14 08:09:34 sparq Exp $
 *
 *	Based on: net/ipv4/ip_fragment.c
 *
 *	This program is free software; you can redistribute it and/or
 *      modify it under the terms of the GNU General Public License
 *      as published by the Free Software Foundation; either version
 *      2 of the License, or (at your option) any later version.
 */

/* 
 *	Fixes:	
 *	Andi Kleen	Make it work with multiple hosts.
 *			More RFC compliance.
 *
 *      Horst von Brand Add missing #include <linux/string.h>
 *	Alexey Kuznetsov	SMP races, threading, cleanup.
 */
#include <linux/config.h>
#include <linux/errno.h>
#include <linux/types.h>
#include <linux/string.h>
#include <linux/socket.h>
#include <linux/sockios.h>
#include <linux/sched.h>
#include <linux/net.h>
#include <linux/netdevice.h>
#include <linux/in6.h>
#include <linux/ipv6.h>
#include <linux/icmpv6.h>
#include <linux/random.h>
#include <linux/jhash.h>

#include <net/sock.h>
#include <net/snmp.h>

#include <net/ipv6.h>
#include <net/protocol.h>
#include <net/transp_v6.h>
#include <net/rawv6.h>
#include <net/ndisc.h>
#include <net/addrconf.h>

int sysctl_ip6frag_high_thresh = 256*1024;
int sysctl_ip6frag_low_thresh = 192*1024;

int sysctl_ip6frag_time = IPV6_FRAG_TIMEOUT;

struct ip6frag_skb_cb
{
	struct inet6_skb_parm	h;
	int			offset;
};

#define FRAG6_CB(skb)	((struct ip6frag_skb_cb*)((skb)->cb))


/*
 *	Equivalent of ipv4 struct ipq
 */

struct frag_queue
{
	struct frag_queue	*next;

	__u32			id;		/* fragment id		*/
	struct in6_addr		saddr;
	struct in6_addr		daddr;

	spinlock_t		lock;
	atomic_t		refcnt;
	struct timer_list	timer;		/* expire timer		*/
	struct sk_buff		*fragments;
	int			len;
	int			meat;
	int			iif;
	struct timeval		stamp;
	unsigned int		csum;
	__u8			last_in;	/* has first/last segment arrived? */
#define COMPLETE		4
#define FIRST_IN		2
#define LAST_IN			1
	__u16			nhoffset;
	struct frag_queue	**pprev;
};

/* Hash table. */

#define IP6Q_HASHSZ	64

static struct frag_queue *ip6_frag_hash[IP6Q_HASHSZ];
static rwlock_t ip6_frag_lock = RW_LOCK_UNLOCKED;
static u32 ip6_frag_hash_rnd;
int ip6_frag_nqueues = 0;

static __inline__ void __fq_unlink(struct frag_queue *fq)
{
	if(fq->next)
		fq->next->pprev = fq->pprev;
	*fq->pprev = fq->next;
	ip6_frag_nqueues--;
}

static __inline__ void fq_unlink(struct frag_queue *fq)
{
	write_lock(&ip6_frag_lock);
	__fq_unlink(fq);
	write_unlock(&ip6_frag_lock);
}

static unsigned int ip6qhashfn(u32 id, struct in6_addr *saddr,
			       struct in6_addr *daddr)
{
	u32 a, b, c;
 
	a = saddr->s6_addr32[0];
	b = saddr->s6_addr32[1];
	c = saddr->s6_addr32[2];

	a += JHASH_GOLDEN_RATIO;
	b += JHASH_GOLDEN_RATIO;
	c += ip6_frag_hash_rnd;
	__jhash_i_mix(a, b, c);

	a += saddr->s6_addr32[3];
	b += daddr->s6_addr32[0];
	c += daddr->s6_addr32[1];
	__jhash_i_mix(a, b, c);

	a += daddr->s6_addr32[2];
	b += daddr->s6_addr32[3];
	c += id;
	__jhash_mix(a, b, c);

	return c & (IP6Q_HASHSZ - 1);
}

static struct timer_list ip6_frag_secret_timer;
static int ip6_frag_secret_interval = 10 * 60 * HZ;

static void ip6_frag_secret_rebuild(unsigned long dummy)
{
	unsigned long now = jiffies;
	int i;

	write_lock(&ip6_frag_lock);
	get_random_bytes(&ip6_frag_hash_rnd, sizeof(u32));
	for (i = 0; i < IP6Q_HASHSZ; i++) {
		struct frag_queue *q;

		q = ip6_frag_hash[i];
		while (q) {
			struct frag_queue *next = q->next;
			unsigned int hval = ip6qhashfn(q->id,
						       &q->saddr,
						       &q->daddr);

			if (hval != i) {
				/* Unlink. */
				if (q->next)
					q->next->pprev = q->pprev;
				*q->pprev = q->next;

				/* Relink to new hash chain. */
				if ((q->next = ip6_frag_hash[hval]) != NULL)
					q->next->pprev = &q->next;
				ip6_frag_hash[hval] = q;
				q->pprev = &ip6_frag_hash[hval];
			}

			q = next;
		}
	}
	write_unlock(&ip6_frag_lock);

	mod_timer(&ip6_frag_secret_timer, now + ip6_frag_secret_interval);
}

atomic_t ip6_frag_mem = ATOMIC_INIT(0);

/* Memory Tracking Functions. */
extern __inline__ void frag_kfree_skb(struct sk_buff *skb)
{
	atomic_sub(skb->truesize, &ip6_frag_mem);
	kfree_skb(skb);
}

extern __inline__ void frag_free_queue(struct frag_queue *fq)
{
	atomic_sub(sizeof(struct frag_queue), &ip6_frag_mem);
	kfree(fq);
}

extern __inline__ struct frag_queue *frag_alloc_queue(void)
{
	struct frag_queue *fq = kmalloc(sizeof(struct frag_queue), GFP_ATOMIC);

	if(!fq)
		return NULL;
	atomic_add(sizeof(struct frag_queue), &ip6_frag_mem);
	return fq;
}

/* Destruction primitives. */

/* Complete destruction of fq. */
static void ip6_frag_destroy(struct frag_queue *fq)
{
	struct sk_buff *fp;

	BUG_TRAP(fq->last_in&COMPLETE);
	BUG_TRAP(del_timer(&fq->timer) == 0);

	/* Release all fragment data. */
	fp = fq->fragments;
	while (fp) {
		struct sk_buff *xp = fp->next;

		frag_kfree_skb(fp);
		fp = xp;
	}

	frag_free_queue(fq);
}

static __inline__ void fq_put(struct frag_queue *fq)
{
	if (atomic_dec_and_test(&fq->refcnt))
		ip6_frag_destroy(fq);
}

/* Kill fq entry. It is not destroyed immediately,
 * because caller (and someone more) holds reference count.
 */
static __inline__ void fq_kill(struct frag_queue *fq)
{
	if (del_timer(&fq->timer))
		atomic_dec(&fq->refcnt);

	if (!(fq->last_in & COMPLETE)) {
		fq_unlink(fq);
		atomic_dec(&fq->refcnt);
		fq->last_in |= COMPLETE;
	}
}

static void ip6_evictor(void)
{
	int i, progress;

	do {
		if (atomic_read(&ip6_frag_mem) <= sysctl_ip6frag_low_thresh)
			return;
		progress = 0;
		for (i = 0; i < IP6Q_HASHSZ; i++) {
			struct frag_queue *fq;
			if (ip6_frag_hash[i] == NULL)
				continue;

			read_lock(&ip6_frag_lock);
			if ((fq = ip6_frag_hash[i]) != NULL) {
				/* find the oldest queue for this hash bucket */
				while (fq->next)
					fq = fq->next;
				atomic_inc(&fq->refcnt);
				read_unlock(&ip6_frag_lock);

				spin_lock(&fq->lock);
				if (!(fq->last_in&COMPLETE))
					fq_kill(fq);
				spin_unlock(&fq->lock);

				fq_put(fq);
				IP6_INC_STATS_BH(Ip6ReasmFails);
				progress = 1;
				continue;
			}
			read_unlock(&ip6_frag_lock);
		}
	} while (progress);
}

static void ip6_frag_expire(unsigned long data)
{
	struct frag_queue *fq = (struct frag_queue *) data;

	spin_lock(&fq->lock);

	if (fq->last_in & COMPLETE)
		goto out;

	fq_kill(fq);

	IP6_INC_STATS_BH(Ip6ReasmTimeout);
	IP6_INC_STATS_BH(Ip6ReasmFails);

	/* Send error only if the first segment arrived. */
	if (fq->last_in&FIRST_IN && fq->fragments) {
		struct net_device *dev = dev_get_by_index(fq->iif);

		/*
		   But use as source device on which LAST ARRIVED
		   segment was received. And do not use fq->dev
		   pointer directly, device might already disappeared.
		 */
		if (dev) {
			fq->fragments->dev = dev;
			icmpv6_send(fq->fragments, ICMPV6_TIME_EXCEED, ICMPV6_EXC_FRAGTIME, 0,
				    dev);
			dev_put(dev);
		}
	}
out:
	spin_unlock(&fq->lock);
	fq_put(fq);
}

/* Creation primitives. */


static struct frag_queue *ip6_frag_intern(unsigned int hash,
					  struct frag_queue *fq_in)
{
	struct frag_queue *fq;

	write_lock(&ip6_frag_lock);
#ifdef CONFIG_SMP
	for (fq = ip6_frag_hash[hash]; fq; fq = fq->next) {
		if (fq->id == fq_in->id && 
		    !ipv6_addr_cmp(&fq_in->saddr, &fq->saddr) &&
		    !ipv6_addr_cmp(&fq_in->daddr, &fq->daddr)) {
			atomic_inc(&fq->refcnt);
			write_unlock(&ip6_frag_lock);
			fq_in->last_in |= COMPLETE;
			fq_put(fq_in);
			return fq;
		}
	}
#endif
	fq = fq_in;

	if (!mod_timer(&fq->timer, jiffies + sysctl_ip6frag_time))
		atomic_inc(&fq->refcnt);

	atomic_inc(&fq->refcnt);
	if((fq->next = ip6_frag_hash[hash]) != NULL)
		fq->next->pprev = &fq->next;
	ip6_frag_hash[hash] = fq;
	fq->pprev = &ip6_frag_hash[hash];
	ip6_frag_nqueues++;
	write_unlock(&ip6_frag_lock);
	return fq;
}


static struct frag_queue *
ip6_frag_create(unsigned int hash, u32 id, struct in6_addr *src, struct in6_addr *dst)
{
	struct frag_queue *fq;

	if ((fq = frag_alloc_queue()) == NULL)
		goto oom;

	memset(fq, 0, sizeof(struct frag_queue));

	fq->id = id;
	ipv6_addr_copy(&fq->saddr, src);
	ipv6_addr_copy(&fq->daddr, dst);

	/* init_timer has been done by the memset */
	fq->timer.function = ip6_frag_expire;
	fq->timer.data = (long) fq;
	fq->lock = SPIN_LOCK_UNLOCKED;
	atomic_set(&fq->refcnt, 1);

	return ip6_frag_intern(hash, fq);

oom:
	IP6_INC_STATS_BH(Ip6ReasmFails);
	return NULL;
}

static __inline__ struct frag_queue *
fq_find(u32 id, struct in6_addr *src, struct in6_addr *dst)
{
	struct frag_queue *fq;
	unsigned int hash = ip6qhashfn(id, src, dst);

	read_lock(&ip6_frag_lock);
	for(fq = ip6_frag_hash[hash]; fq; fq = fq->next) {
		if (fq->id == id && 
		    !ipv6_addr_cmp(src, &fq->saddr) &&
		    !ipv6_addr_cmp(dst, &fq->daddr)) {
			atomic_inc(&fq->refcnt);
			read_unlock(&ip6_frag_lock);
			return fq;
		}
	}
	read_unlock(&ip6_frag_lock);

	return ip6_frag_create(hash, id, src, dst);
}


static void ip6_frag_queue(struct frag_queue *fq, struct sk_buff *skb, 
			   struct frag_hdr *fhdr, int nhoff)
{
	struct sk_buff *prev, *next;
	int offset, end;

	if (fq->last_in & COMPLETE)
		goto err;

	offset = ntohs(fhdr->frag_off) & ~0x7;
	end = offset + (ntohs(skb->nh.ipv6h->payload_len) -
			((u8 *) (fhdr + 1) - (u8 *) (skb->nh.ipv6h + 1)));

	if ((unsigned int)end >= 65536) {
 		icmpv6_param_prob(skb,ICMPV6_HDR_FIELD, (u8*)&fhdr->frag_off - skb->nh.raw);
 		return;
	}

 	if (skb->ip_summed == CHECKSUM_HW)
 		skb->csum = csum_sub(skb->csum,
 				     csum_partial(skb->nh.raw, (u8*)(fhdr+1)-skb->nh.raw, 0));

	/* Is this the final fragment? */
	if (!(fhdr->frag_off & htons(0x0001))) {
		/* If we already have some bits beyond end
		 * or have different end, the segment is corrupted.
		 */
		if (end < fq->len ||
		    ((fq->last_in & LAST_IN) && end != fq->len))
			goto err;
		fq->last_in |= LAST_IN;
		fq->len = end;
	} else {
		/* Check if the fragment is rounded to 8 bytes.
		 * Required by the RFC.
		 */
		if (end & 0x7) {
			/* RFC2460 says always send parameter problem in
			 * this case. -DaveM
			 */
			icmpv6_param_prob(skb, ICMPV6_HDR_FIELD, 
					  offsetof(struct ipv6hdr, payload_len));
			return;
		}
		if (end > fq->len) {
			/* Some bits beyond end -> corruption. */
			if (fq->last_in & LAST_IN)
				goto err;
			fq->len = end;
		}
	}

	if (end == offset)
		goto err;

	/* Point into the IP datagram 'data' part. */
	if (!pskb_pull(skb, (u8 *) (fhdr + 1) - skb->data))
		goto err;
	if (end-offset < skb->len) {
		if (pskb_trim(skb, end - offset))
			goto err;
		if (skb->ip_summed != CHECKSUM_UNNECESSARY)
			skb->ip_summed = CHECKSUM_NONE;
	}

	/* Find out which fragments are in front and at the back of us
	 * in the chain of fragments so far.  We must know where to put
	 * this fragment, right?
	 */
	prev = NULL;
	for(next = fq->fragments; next != NULL; next = next->next) {
		if (FRAG6_CB(next)->offset >= offset)
			break;	/* bingo! */
		prev = next;
	}

	/* We found where to put this one.  Check for overlap with
	 * preceding fragment, and, if needed, align things so that
	 * any overlaps are eliminated.
	 */
	if (prev) {
		int i = (FRAG6_CB(prev)->offset + prev->len) - offset;

		if (i > 0) {
			offset += i;
			if (end <= offset)
				goto err;
			if (!pskb_pull(skb, i))
				goto err;
			if (skb->ip_summed != CHECKSUM_UNNECESSARY)
				skb->ip_summed = CHECKSUM_NONE;
		}
	}

	/* Look for overlap with succeeding segments.
	 * If we can merge fragments, do it.
	 */
	while (next && FRAG6_CB(next)->offset < end) {
		int i = end - FRAG6_CB(next)->offset; /* overlap is 'i' bytes */

		if (i < next->len) {
			/* Eat head of the next overlapped fragment
			 * and leave the loop. The next ones cannot overlap.
			 */
			if (!pskb_pull(next, i))
				goto err;
			FRAG6_CB(next)->offset += i;	/* next fragment */
			fq->meat -= i;
			if (next->ip_summed != CHECKSUM_UNNECESSARY)
				next->ip_summed = CHECKSUM_NONE;
			break;
		} else {
			struct sk_buff *free_it = next;

			/* Old fragmnet is completely overridden with
			 * new one drop it.
			 */
			next = next->next;

			if (prev)
				prev->next = next;
			else
				fq->fragments = next;

			fq->meat -= free_it->len;
			frag_kfree_skb(free_it);
		}
	}

	FRAG6_CB(skb)->offset = offset;

	/* Insert this fragment in the chain of fragments. */
	skb->next = next;
	if (prev)
		prev->next = skb;
	else
		fq->fragments = skb;

	if (skb->dev)
		fq->iif = skb->dev->ifindex;
	skb->dev = NULL;
	fq->stamp = skb->stamp;
	fq->meat += skb->len;
	atomic_add(skb->truesize, &ip6_frag_mem);

	/* The first fragment.
	 * nhoffset is obtained from the first fragment, of course.
	 */
	if (offset == 0) {
		fq->nhoffset = nhoff;
		fq->last_in |= FIRST_IN;
	}
	return;

err:
	kfree_skb(skb);
}

/*
 *	Check if this packet is complete.
 *	Returns NULL on failure by any reason, and pointer
 *	to current nexthdr field in reassembled frame.
 *
 *	It is called with locked fq, and caller must check that
 *	queue is eligible for reassembly i.e. it is not COMPLETE,
 *	the last and the first frames arrived and all the bits are here.
 */
static int ip6_frag_reasm(struct frag_queue *fq, struct sk_buff **skb_in,
			  struct net_device *dev)
{
	struct sk_buff *fp, *head = fq->fragments;
	int    remove_fraghdr = 0;
	int    payload_len;
	int    nhoff;

	fq_kill(fq);

	BUG_TRAP(head != NULL);
	BUG_TRAP(FRAG6_CB(head)->offset == 0);

	/* Unfragmented part is taken from the first segment. */
	payload_len = (head->data - head->nh.raw) - sizeof(struct ipv6hdr) + fq->len;
	nhoff = head->h.raw - head->nh.raw;

	if (payload_len > 65535) {
		payload_len -= 8;
		if (payload_len > 65535)
			goto out_oversize;
		remove_fraghdr = 1;
	}

	/* Head of list must not be cloned. */
	if (skb_cloned(head) && pskb_expand_head(head, 0, 0, GFP_ATOMIC))
		goto out_oom;

	/* If the first fragment is fragmented itself, we split
	 * it to two chunks: the first with data and paged part
	 * and the second, holding only fragments. */
	if (skb_shinfo(head)->frag_list) {
		struct sk_buff *clone;
		int i, plen = 0;

		if ((clone = alloc_skb(0, GFP_ATOMIC)) == NULL)
			goto out_oom;
		clone->next = head->next;
		head->next = clone;
		skb_shinfo(clone)->frag_list = skb_shinfo(head)->frag_list;
		skb_shinfo(head)->frag_list = NULL;
		for (i=0; i<skb_shinfo(head)->nr_frags; i++)
			plen += skb_shinfo(head)->frags[i].size;
		clone->len = clone->data_len = head->data_len - plen;
		head->data_len -= clone->len;
		head->len -= clone->len;
		clone->csum = 0;
		clone->ip_summed = head->ip_summed;
		atomic_add(clone->truesize, &ip6_frag_mem);
	}

	/* Normally we do not remove frag header from datagram, but
	 * we have to do this and to relocate header, when payload
	 * is > 65535-8. */
	if (remove_fraghdr) {
		nhoff = fq->nhoffset;
		head->nh.raw[nhoff] = head->h.raw[0];
		memmove(head->head+8, head->head, (head->data-head->head)-8);
		head->mac.raw += 8;
		head->nh.raw += 8;
	} else {
		((struct frag_hdr*)head->h.raw)->frag_off = 0;
	}

	skb_shinfo(head)->frag_list = head->next;
	head->h.raw = head->data;
	skb_push(head, head->data - head->nh.raw);
	atomic_sub(head->truesize, &ip6_frag_mem);

	for (fp=head->next; fp; fp = fp->next) {
		head->data_len += fp->len;
		head->len += fp->len;
		if (head->ip_summed != fp->ip_summed)
			head->ip_summed = CHECKSUM_NONE;
		else if (head->ip_summed == CHECKSUM_HW)
			head->csum = csum_add(head->csum, fp->csum);
		head->truesize += fp->truesize;
		atomic_sub(fp->truesize, &ip6_frag_mem);
	}

	head->next = NULL;
	head->dev = dev;
	head->stamp = fq->stamp;
	head->nh.ipv6h->payload_len = ntohs(payload_len);

	*skb_in = head;

	/* Yes, and fold redundant checksum back. 8) */
	if (head->ip_summed == CHECKSUM_HW)
		head->csum = csum_partial(head->nh.raw, head->h.raw-head->nh.raw, head->csum);

	IP6_INC_STATS_BH(Ip6ReasmOKs);
	fq->fragments = NULL;
	return nhoff;

out_oversize:
	if (net_ratelimit())
		printk(KERN_DEBUG "ip6_frag_reasm: payload len = %d\n", payload_len);
	goto out_fail;
out_oom:
	if (net_ratelimit())
		printk(KERN_DEBUG "ip6_frag_reasm: no memory for reassembly\n");
out_fail:
	IP6_INC_STATS_BH(Ip6ReasmFails);
	return -1;
}

int ipv6_reassembly(struct sk_buff **skbp, int nhoff)
{
	struct sk_buff *skb = *skbp; 
	struct net_device *dev = skb->dev;
	struct frag_hdr *fhdr;
	struct frag_queue *fq;
	struct ipv6hdr *hdr;

	hdr = skb->nh.ipv6h;

	IP6_INC_STATS_BH(Ip6ReasmReqds);

	/* Jumbo payload inhibits frag. header */
	if (hdr->payload_len==0) {
		icmpv6_param_prob(skb, ICMPV6_HDR_FIELD, skb->h.raw-skb->nh.raw);
		return -1;
	}
	if (!pskb_may_pull(skb, (skb->h.raw-skb->data)+sizeof(struct frag_hdr))) {
		icmpv6_param_prob(skb, ICMPV6_HDR_FIELD, skb->h.raw-skb->nh.raw);
		return -1;
	}

	hdr = skb->nh.ipv6h;
	fhdr = (struct frag_hdr *)skb->h.raw;

	if (!(fhdr->frag_off & htons(0xFFF9))) {
		/* It is not a fragmented frame */
		skb->h.raw += sizeof(struct frag_hdr);
		IP6_INC_STATS_BH(Ip6ReasmOKs);

		return (u8*)fhdr - skb->nh.raw;
	}

	if (atomic_read(&ip6_frag_mem) > sysctl_ip6frag_high_thresh)
		ip6_evictor();

	if ((fq = fq_find(fhdr->identification, &hdr->saddr, &hdr->daddr)) != NULL) {
		int ret = -1;

		spin_lock(&fq->lock);

		ip6_frag_queue(fq, skb, fhdr, nhoff);

		if (fq->last_in == (FIRST_IN|LAST_IN) &&
		    fq->meat == fq->len)
			ret = ip6_frag_reasm(fq, skbp, dev);

		spin_unlock(&fq->lock);
		fq_put(fq);
		return ret;
	}

	IP6_INC_STATS_BH(Ip6ReasmFails);
	kfree_skb(skb);
	return -1;
}

void __init ipv6_frag_init(void)
{
	ip6_frag_hash_rnd = (u32) ((num_physpages ^ (num_physpages>>7)) ^
				   (jiffies ^ (jiffies >> 6)));

	init_timer(&ip6_frag_secret_timer);
	ip6_frag_secret_timer.function = ip6_frag_secret_rebuild;
	ip6_frag_secret_timer.expires = jiffies + ip6_frag_secret_interval;
	add_timer(&ip6_frag_secret_timer);
}
