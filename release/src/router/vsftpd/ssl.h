#ifndef VSF_SSL_H
#define VSF_SSL_H

struct vsf_session;
struct mystr;

void ssl_getline(const struct vsf_session* p_sess, struct mystr* p_str,
                 char end_char, char* p_buf, unsigned int buflen);
int ssl_read(struct vsf_session* p_sess, char* p_buf, unsigned int len);
int ssl_write(void* p_ssl, const char* p_buf, unsigned int len);
int ssl_write_str(void* p_ssl, const struct mystr* p_str);
void ssl_init(struct vsf_session* p_sess);
int ssl_accept(struct vsf_session* p_sess, int fd);
int ssl_data_close(struct vsf_session* p_sess);
void ssl_comm_channel_init(struct vsf_session* p_sess);
void handle_auth(struct vsf_session* p_sess);
void handle_pbsz(struct vsf_session* p_sess);
void handle_prot(struct vsf_session* p_sess);

#endif /* VSF_SSL_H */

