/****************************************************************************
 * Copyright (C) 1998 WIDE Project. All rights reserved.
 * Copyright (C) 1999,2000,2001,2002 University of Tromso. All rights reserved.
 * Copyright (C) 2002 Invenia Innovation AS. All rights reserved.
 *
 * Author: Feike W. Dillema, feico@pasta.cs.uit.no.
 *         based on newbie code by Yusuke DOI, Keio Univ. Murai Lab.
 ****************************************************************************/

/*
 * <$Id: protos.h,v 3.62 2005/01/27 15:29:33 dillema Exp $>
 */

#if !HAVE_STRLCPY
size_t strlcpy(char *, const char *, size_t);
#endif
#if !HAVE_STRLCAT
size_t strlcat(char *, const char *, size_t);
#endif
#if !HAVE_DAEMON
int daemon(int, int);
#endif
#if !HAVE_DAEMON
int inet_aton(const char *, struct in_addr *);
#endif

RRset *search_name (G_List *, char *, int, int);
char *sprint_inet(struct sockaddr *, char *);
int request_start (Context *, int);
int request_retry(Context *);
int request_abort (Context *, int);
int request_finish (Context *cont);
int assemble_response (Context *);
int do_forward(struct context *, u_char *, uint16_t, uint16_t, int);
int recursive_process (Context *);
int tcp_writemesg (Context *, int);
int response_abort (Context *, int);
int tcp_request_start (struct context *);
int tcp_request_wait_connect_process (Context *);
int tcp_request_wait_connect_retry (Context *);
int tcp_request_writing_process (Context *);
int tcp_request_writing_retry (Context *);
int tcp_request_readlen_process (Context *);
int tcp_request_readlen_retry (Context *);
int tcp_request_reading_process (Context *);
int tcp_request_reading_retry (Context *);
int tcp_request_finish (Context *);
int tcp_response_start (int, struct sockaddr *);
int tcp_response_readlen_process (Context *);
int tcp_response_readlen_retry (Context *);
int tcp_response_reading_process (Context *);
int tcp_response_reading_retry (Context *);
int tcp_response_recursive_process (Context *);
int tcp_response_recursive_retry (Context *);
int tcp_response_writing_process (Context *);
int tcp_response_writing_retry (Context *);
int tcp_response_waiting_client_close_process (Context *);
int tcp_response_waiting_client_close_retry (Context *);
int tcp_response_finish (Context *);
int udp_request_start (struct context *);
int udp_request_process (Context *);
int udp_request_retry (Context *);
int udp_request_finish (Context *);
int udp_response_start (u_char *, int, struct sockaddr *, Nia *);
int udp_response_recursive_process (Context *);
int udp_response_recursive_retry (Context *);
int udp_response_finish (Context *);
int context_timeout_handler (struct ev_to_data *);
int context_timeout_register (Context *, int);
Context *context_create (void);
void *context_destroy (Context *);
void conv_trick_list (G_List *, int, int);
void conv_trick_ptr (G_List *, u_char *);
void conv_trick_newptr (G_List *, u_char *);
int conv_trick_conf (u_char *);
int conv_trick_is_tot_ptr (u_char *);
void conv_trick_ptr_rq (u_char * qname);
int conv_trick_is_tot_newptr (u_char *, struct in6_addr *);
void conv_trick_newptr_rq (u_char *, struct in6_addr *);
int conv_stf_is_stf_ptr (u_char *);
int conv_stf_ptr (u_char *);
void conv_stf_ns_list (G_List *);
struct sockaddr *conv_stf_get_nsaddr(Context *);
void ev_dup_init (void);
int ev_dup (struct sockaddr *, uint16_t, uint16_t);
void ev_main ();
int ev_signal_init (void);
void ev_signal_process (void);
void ev_signal_finish (void);
int ev_tcp_common_eq (void *, void *);
G_List *ev_tcp_common_init (void);
int ev_tcp_common_register (G_List *, int, Context *);
int ev_tcp_common_remove (G_List *, int);
int ev_tcp_common_fds (G_List *, fd_set *);
int ev_tcp_common_fd_check (G_List *, fd_set *);
int ev_tcp_conn_in_init (void);
void ev_tcp_conn_in_finish (void);
int ev_tcp_conn_in_register (int, Context *);
int ev_tcp_conn_in_remove (int);
int ev_tcp_conn_in_fds (fd_set *);
int ev_tcp_conn_in_fd_check (fd_set *);
int ev_tcp_out_init (void);
void ev_tcp_out_finish (void);
int ev_tcp_out_register (int, Context *);
int ev_tcp_out_remove (int);
int ev_tcp_out_fds (fd_set *);
int ev_tcp_out_fd_check (fd_set *);
int ev_tcp_srv_accept (int);
int ev_to_init (void);
int ev_to_register (Ev_TO_Data *);
time_t ev_timeout_process (void);
void ev_to_data_free (Ev_TO_Data *);
void ev_to_data_free_v (void *);
void ev_to_finish (void);
int ev_to_handler_ifcheck (Ev_TO_Data *);
int ev_to_register_ifcheck (void);
int ev_udp_in_eq (void *, void *);
void ev_udp_in_data_free (Ev_UDP_In_Data * _p);
void ev_udp_in_data_free_v (void *_vp);
int ev_udp_in_init (void);
void ev_udp_in_finish (void);
int ev_udp_in_register (Context *, struct sockaddr *, int, uint16_t);
int ev_udp_in_remove (struct sockaddr *, int);
int ev_udp_in_read (int);
struct sockaddr *parse_and_alloc_addr (char *caddr, int port, int *sa_len_ret);
void fwd_free (Fwd *);
void fwd_freev (void *);
Fwd *fwd_alloc (void);
void fwd_init (void);
int fwd_add (char *hostname, int port);
int fwd_del (struct sockaddr *, int);
void fwd_select (void);
void fwd_mark (struct sockaddr *, int);
G_List *fwd_socketlist (void);
G_List *list_init (void);
int list_add (G_List *, void *);
int list_add_tail (G_List *, void *);
G_List *list_search (G_List *, void *, int (*comp_func) (void *, void *));
void *list_delete (G_List *);
void list_destroy (G_List *, void (*freefnc) (void *));
G_List *list_dup (G_List *, void (freefunc) (void *), void *(dupfunc) (void *));
void list_cat (G_List * list, G_List * toadd);
void nia_free_closev (void *);
void nia_free (Nia *, int);
Nia *nia_find_by_sock (int);
void nia_fds_set (fd_set *, int *);
int nia_fds_isset (fd_set *, int *);
Nia *nia_copy (Nia *);
int net_init_socketlist (int);
int net_reinit_socketlist (int, int);
int net_bind_socketlist (void);
int net_delete_socketlist (void);
int net_mesg_socket (Nia *);
int net_stream_socket (Nia *);
int net_mesg_send (Nia *, u_char *, int, struct sockaddr *);
uint16_t mesg_id (void);
int mesg_make_query (u_char *, uint16_t, uint16_t, uint32_t, int, u_char *, int);
int mesg_write_dname (u_char *, u_char *, uint16_t *, int, u_char *, u_char *);
u_char *mesg_skip_dname (u_char * dname, u_char * end);
int labellen (const u_char * dname);
u_char *dname_decompress (u_char *, int, u_char *, u_char *, u_char *, int *);
int mesg_dname_cmp (u_char *, u_char *, u_char *);
int mesg_write_rrset_list (G_List *, u_char *, u_char *, uint16_t *, int, u_char **, uint16_t *);
int mesg_assemble (G_List *, G_List *, G_List *, u_char *, uint16_t, u_char *mesg, int);
void rrset_couple_free (RRset_Couple *);
void rrset_couple_freev (void *);
int mesg_extract_rr (u_char *, u_char *, uint16_t, uint16_t, u_char *, u_char *, int);
int mesg_parse (u_char *, int, G_List *, G_List *, G_List *);
Q_Set *queue_create (void);
void queue_destroy (Q_Set *, void (*freefnc) (void *));
int enqueue (Q_Set *, void *);
void *dequeue (Q_Set *);
void *queue_peek (Q_Set *);
void *queue_disable (G_Queue *);
int read_config (char *);
RR *rr_alloc (uint32_t, int, u_char *);
RR_List *rr_list_alloc (void);
void rr_list_free (RR_List *);
RR_List *rr_list_add (RR_List *, uint32_t, int, u_char *);
RR *rrset_get_rr_f (int, RRset *);
RR_List *rr_list_of_rrset (RRset *);
RRset *rrset_alloc (void);
RRset *rrset_create (uint16_t, uint16_t, uint16_t, u_char *, RR_List *);
RRset *rrset_create_single (u_char *, int, uint16_t, uint16_t, uint32_t, uint16_t, u_char *);
void rrset_freev (void *);
void rrset_free (RRset *);
RRset *rrset_copy (RRset *);
void *rrset_copyv (void *);
void *rrset_dupv (void *);
RRset *rrset_dup (RRset *);
void rrset_list_dump (G_List *, FILE *);
char *string_rclass (uint16_t);
char *string_rtype (uint16_t);

void totd_eventloop (void);
int totd_exit (int);

int conv_scoped_query (Context *);
int conv_scoped_conf (const char *, const char *, int);
void conv_scoped_list (G_List *);
int conv_is_scoped_ptr (u_char *, int);
void conv_scoped_ptr (G_List *, u_char *);
void conv_scoped_ptr_rq (u_char *);
#ifdef SWILL
void print_stats(FILE *);
void add_prefix(FILE *);
void del_prefix(FILE *);
#endif
