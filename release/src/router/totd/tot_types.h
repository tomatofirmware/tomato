/****************************************************************************
 * Copyright (C) 1998 WIDE Project. All rights reserved.
 * Copyright (C) 1999,2000,2001,2002 University of Tromso. All rights reserved.
 * Copyright (C) 2002 Invenia Innovation AS. All rights reserved.
 *
 * Author: Feike W. Dillema, feico@pasta.cs.uit.no.
 *         based on newbie code by Yusuke DOI, Keio Univ. Murai Lab.
 ****************************************************************************/

/*
 * <$Id: tot_types.h,v 3.12 2002/03/06 14:56:03 dillema Exp $>
 */

/*
 * Generic double-linked list element
 */

typedef struct generic_list {
	struct generic_list *next;
	struct generic_list *prev;
	void *list_data;
} G_List;

/*
 * Generic Queue
 */

/* queue element */
typedef struct gqueue {
	struct gqueue *next;
	void *p;		/* if null, disabled queue */
} G_Queue;

/* pointer-set to handle a queue */
typedef struct qset {
	struct gqueue *head;
	struct gqueue *tail;
} Q_Set;

/*
 * A Forwarder Nameserver
 */

typedef struct fwd {
	char hostname[MAX_DNAME+1];
	int port;
   	struct sockaddr *sa;	/* forwarder peer address */
    	int sa_len;
	/*
	 * time at which this nameserver was suspected to be down
	 * zero value means it is not down, but responsive
	 */
    	time_t went_down_at;
    	int ticks;
} Fwd;

/*
 * Network Interface/Socket Combo
 */

typedef struct nia {
	struct sockaddr *sa_p;
	int udp_sock;	/* if sock>=0, socket opened */
	int tcp_sock;	/* if sock>=0, socket opened */
} Nia;

/*
 * A DNS Message `on the wire'
 */

typedef struct mesg_hdr {
	uint16_t id;		/* query id */
#if BYTE_ORDER == BIG_ENDIAN
	/* 3rd byte */
	u_char qr:1;		/* request(0) or response(1)? */
	u_char opcode:4;	/* what's the message for? */
	u_char aa:1;		/* is it authoritive? */
	u_char tc:1;		/* is it truncated? */
	u_char rd:1;		/* recursion required? */
	/* 4th byte */
	u_char ra:1;		/* recursion available? */
	u_char zero:3;		/* for future use */
	u_char rcode:4;		/* response/error code */
#elif BYTE_ORDER == LITTLE_ENDIAN
	/* 3rd byte */
	u_char rd:1;		/* recursion required? */
	u_char tc:1;		/* is it truncated? */
	u_char aa:1;		/* is it authoritive? */
	u_char opcode:4;	/* what's the message for? */
	u_char qr:1;		/* is it request(0) or response(1)? */
	/* 4th byte */
	u_char rcode:4;		/* response/error code */
	u_char zero:3;		/* for future use */
	u_char ra:1;		/* recursion avilable(true)? */
#else
#error Specify ENDIAN in config.h
#endif
	uint16_t qdcnt;	/* question count */
	uint16_t ancnt;	/* answer count */
	uint16_t nscnt;	/* ns info count */
	uint16_t arcnt;	/* additional record count */
} Mesg_Hdr;

typedef union mesg {
	Mesg_Hdr *hdr;
	u_char *p;
} Mesg;

/*
 * DNS Resource Records
 */

/* data structure for a RR */
#define RR_HEAD_LEN (sizeof(uint16_t)+sizeof(uint32_t))
#define rr_rdata(rr) (((u_char*)(rr))+(sizeof(uint16_t)+sizeof(uint32_t)))

typedef struct rr {
	uint32_t ttl;		/* time to live -- expire data if RR is cache */
	uint16_t rd_len;	/* length of rdata -- exclusive */
} RR;

/* list of RR to be used for RRset construction */
typedef struct rr_list {
	struct rr_list *next;
	int cnt;
	int offset;
	RR *rrp;
} RR_List;

/* RRset key section */
#define KEYINFO_HEAD_LEN   (sizeof(uint16_t) * 3)
#define rrset_owner(rrset) (((u_char *)(rrset)->key.info) + KEYINFO_HEAD_LEN)
#define rrset_owner_len(rrset) (rrset->key.info->owner_len)
typedef struct key_info {
	uint16_t r_type;	/* resource type */
	uint16_t r_class;	/* resource class */
	uint16_t owner_len;	/* owner length / terminator included */
} Key_Info;

typedef union u_key {
	Key_Info *info;		/* structure */
	u_char *p;		/* a pointer */
} U_Key;

/* RRset data section */
#define DATADATA_HEAD_LEN sizeof(Data_Data)
#define data_offset(n, d) (*((uint16_t*)(((u_char*)(d))+\
					  (sizeof(uint16_t)*((n)+1)))))
typedef struct data_data {
	uint16_t data_cnt;	/* number of the datas */
}   Data_Data;

typedef union u_data {
	Data_Data *d;		/* data structure */
	uint16_t *rt_list;	/* resource type list */
	u_char *p;		/* a pointer */
} U_Data;

/* structure for an RR set */
typedef struct rr_set {
	int links;		/* number of pointers which points this RR */

	size_t key_len;		/* length of the key -- exclusive */
	U_Key key;		/* key itself */

	size_t data_len;	/* length of the all datas */
	U_Data data;		/* data itself */
} RRset;

/* used to create RRset from message */
typedef struct {
	RRset *rrs;
	RR_List *rrl;
} RRset_Couple;

/* 
 *  Transaction Context/State
 */

typedef struct context {
	struct context *parent;	/* parent context */
	struct context *child;	/* child context */

	/* what state are we in? */
	int work_state;		/* recursive work and tricks state */
#ifdef STF
	u_char stfname[MAX_DNAME];
#endif

	/* collected internal state */
	int cname_links;	/* number of cname links followed */
	G_List *nameservers;	/* nameservers we may talk to */
        G_List *current_ns;	/* the nameserver we currently talk to */

	/* the actions upon state transitions */
	struct ev_to_data *tout;		/* timeout pointer */
	int (*process) (struct context *ctx);	/* proceed the state */
	int (*retry) (struct context *ctx);	/* timeout and retry */

	/* the current question/query/request */
	u_char qname[MAX_DNAME];
	uint32_t q_id;
	uint16_t q_class;
	uint16_t q_type;

	/* the resource records we got sofar */
	G_List *an_list;	/* answer list */
	G_List *ns_list;	/* nameserver/authority list */
	G_List *ar_list;	/* additional record list */

	/* the message sent/received */
	union {
		Mesg_Hdr *hdr;	/* message header */
		u_char *p;	/* buffer pointer */
	} mesg;
	int mesg_len;		/* length of the buffer */
	u_char *wp;		/* (TCP) writing pointer */

	/* parameters */
	Nia *netifaddr;		/* input address and socket */
	uint16_t retry_count;
	uint16_t timeout;

	/* address of peer of the received query */
	struct sockaddr *peer;
	/* connection socket for tcp request */
	int conn_sock;
} Context;

/*
 * Events
 */

typedef struct ev_sig_data {
	void (*handler) (void);	/* event handler */
} Ev_Sig_Data;

typedef struct ev_tcp_common_data {
	int sock;
	Context *cptr;	/* context for this socket */
} Ev_TCP_Common_Data;

/* timeout event data section -- invalid if handler is NULL */
typedef struct ev_to_data {
	time_t at;
	int (*handler) (struct ev_to_data *td);
	int type;
	union {
		void *p;		/* generic pointer */
		struct context *cont;	/* for EV_TIMEOUT_CONTEXT event */
	}   data;
} Ev_TO_Data;

typedef struct ev_udp_in_data {
	Context *cptr;
	struct sockaddr *sa_p;
	uint16_t id;
} Ev_UDP_In_Data;

