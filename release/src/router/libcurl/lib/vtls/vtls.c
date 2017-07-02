/***************************************************************************
 *                                  _   _ ____  _
 *  Project                     ___| | | |  _ \| |
 *                             / __| | | | |_) | |
 *                            | (__| |_| |  _ <| |___
 *                             \___|\___/|_| \_\_____|
 *
<<<<<<< HEAD
 * Copyright (C) 1998 - 2013, Daniel Stenberg, <daniel@haxx.se>, et al.
=======
 * Copyright (C) 1998 - 2017, Daniel Stenberg, <daniel@haxx.se>, et al.
>>>>>>> origin/tomato-shibby-RT-AC
 *
 * This software is licensed as described in the file COPYING, which
 * you should have received as part of this distribution. The terms
 * are also available at http://curl.haxx.se/docs/copyright.html.
 *
 * You may opt to use, copy, modify, merge, publish, distribute and/or sell
 * copies of the Software, and permit persons to whom the Software is
 * furnished to do so, under the terms of the COPYING file.
 *
 * This software is distributed on an "AS IS" basis, WITHOUT WARRANTY OF ANY
 * KIND, either express or implied.
 *
 ***************************************************************************/

/* This file is for implementing all "generic" SSL functions that all libcurl
   internals should use. It is then responsible for calling the proper
   "backend" function.

   SSL-functions in libcurl should call functions in this source file, and not
   to any specific SSL-layer.

   Curl_ssl_ - prefix for generic ones
   Curl_ossl_ - prefix for OpenSSL ones
   Curl_gtls_ - prefix for GnuTLS ones
   Curl_nss_ - prefix for NSS ones
   Curl_qssl_ - prefix for QsoSSL ones
   Curl_gskit_ - prefix for GSKit ones
   Curl_polarssl_ - prefix for PolarSSL ones
   Curl_cyassl_ - prefix for CyaSSL ones
   Curl_schannel_ - prefix for Schannel SSPI ones
   Curl_darwinssl_ - prefix for SecureTransport (Darwin) ones

   Note that this source code uses curlssl_* functions, and they are all
   defines/macros #defined by the lib-specific header files.

   "SSL/TLS Strong Encryption: An Introduction"
   http://httpd.apache.org/docs-2.0/ssl/ssl_intro.html
*/

#include "curl_setup.h"

#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif
#ifdef HAVE_FCNTL_H
#include <fcntl.h>
#endif

#include "urldata.h"

#include "vtls.h" /* generic SSL protos etc */
#include "openssl.h" /* OpenSSL versions */
#include "gtls.h"   /* GnuTLS versions */
#include "nssg.h"   /* NSS versions */
#include "qssl.h"   /* QSOSSL versions */
#include "gskit.h"  /* Global Secure ToolKit versions */
#include "polarssl.h" /* PolarSSL versions */
#include "axtls.h"  /* axTLS versions */
#include "cyassl.h"  /* CyaSSL versions */
#include "curl_schannel.h" /* Schannel SSPI version */
#include "curl_darwinssl.h" /* SecureTransport (Darwin) version */
#include "slist.h"
#include "sendf.h"
#include "strcase.h"
#include "url.h"
#include "curl_memory.h"
#include "progress.h"
#include "share.h"
#include "multiif.h"
#include "timeval.h"

#define _MPRINTF_REPLACE /* use our functions only */
#include <curl/mprintf.h>

/* The last #include file should be: */
#include "memdebug.h"

/* convenience macro to check if this handle is using a shared SSL session */
#define SSLSESSION_SHARED(data) (data->share &&                        \
                                 (data->share->specifier &             \
                                  (1<<CURL_LOCK_DATA_SSL_SESSION)))

#define CLONE_STRING(var)                    \
  if(source->var) {                          \
    dest->var = strdup(source->var);         \
    if(!dest->var)                           \
      return FALSE;                          \
  }                                          \
  else                                       \
    dest->var = NULL;

bool
Curl_ssl_config_matches(struct ssl_primary_config* data,
                        struct ssl_primary_config* needle)
{
  if((data->version == needle->version) &&
     (data->verifypeer == needle->verifypeer) &&
     (data->verifyhost == needle->verifyhost) &&
     Curl_safe_strcasecompare(data->CApath, needle->CApath) &&
     Curl_safe_strcasecompare(data->CAfile, needle->CAfile) &&
     Curl_safe_strcasecompare(data->clientcert, needle->clientcert) &&
     Curl_safe_strcasecompare(data->cipher_list, needle->cipher_list))
    return TRUE;

  return FALSE;
}

bool
Curl_clone_primary_ssl_config(struct ssl_primary_config *source,
                              struct ssl_primary_config *dest)
{
  dest->verifyhost = source->verifyhost;
  dest->verifypeer = source->verifypeer;
  dest->version = source->version;

  CLONE_STRING(CAfile);
  CLONE_STRING(CApath);
  CLONE_STRING(cipher_list);
  CLONE_STRING(egdsocket);
  CLONE_STRING(random_file);
  CLONE_STRING(clientcert);
  return TRUE;
}

void Curl_free_primary_ssl_config(struct ssl_primary_config* sslc)
{
  Curl_safefree(sslc->CAfile);
  Curl_safefree(sslc->CApath);
  Curl_safefree(sslc->cipher_list);
  Curl_safefree(sslc->egdsocket);
  Curl_safefree(sslc->random_file);
<<<<<<< HEAD
}


/*
 * Curl_rand() returns a random unsigned integer, 32bit.
 *
 * This non-SSL function is put here only because this file is the only one
 * with knowledge of what the underlying SSL libraries provide in terms of
 * randomizers.
 *
 * NOTE: 'data' may be passed in as NULL when coming from external API without
 * easy handle!
 *
 */

unsigned int Curl_rand(struct SessionHandle *data)
{
  unsigned int r;
  static unsigned int randseed;
  static bool seeded = FALSE;

#ifndef have_curlssl_random
  (void)data;
#else
  if(data) {
    Curl_ssl_random(data, (unsigned char *)&r, sizeof(r));
    return r;
  }
#endif

#ifdef RANDOM_FILE
  if(!seeded) {
    /* if there's a random file to read a seed from, use it */
    int fd = open(RANDOM_FILE, O_RDONLY);
    if(fd > -1) {
      /* read random data into the randseed variable */
      ssize_t nread = read(fd, &randseed, sizeof(randseed));
      if(nread == sizeof(randseed))
        seeded = TRUE;
      close(fd);
    }
  }
#endif

  if(!seeded) {
    struct timeval now = curlx_tvnow();
    randseed += (unsigned int)now.tv_usec + (unsigned int)now.tv_sec;
    randseed = randseed * 1103515245 + 12345;
    randseed = randseed * 1103515245 + 12345;
    randseed = randseed * 1103515245 + 12345;
    seeded = TRUE;
  }

  /* Return an unsigned 32-bit pseudo-random number. */
  r = randseed = randseed * 1103515245 + 12345;
  return (r << 16) | ((r >> 16) & 0xFFFF);
=======
  Curl_safefree(sslc->clientcert);
>>>>>>> origin/tomato-shibby-RT-AC
}

#ifdef USE_SSL

/* "global" init done? */
static bool init_ssl=FALSE;

/**
 * Global SSL init
 *
 * @retval 0 error initializing SSL
 * @retval 1 SSL initialized successfully
 */
int Curl_ssl_init(void)
{
  /* make sure this is only done once */
  if(init_ssl)
    return 1;
  init_ssl = TRUE; /* never again */

  return curlssl_init();
}


/* Global cleanup */
void Curl_ssl_cleanup(void)
{
  if(init_ssl) {
    /* only cleanup if we did a previous init */
    curlssl_cleanup();
    init_ssl = FALSE;
  }
}

<<<<<<< HEAD
CURLcode
Curl_ssl_connect(struct connectdata *conn, int sockindex)
{
  CURLcode res;
=======
static bool ssl_prefs_check(struct Curl_easy *data)
{
  /* check for CURLOPT_SSLVERSION invalid parameter value */
  if((data->set.ssl.primary.version < 0)
     || (data->set.ssl.primary.version >= CURL_SSLVERSION_LAST)) {
    failf(data, "Unrecognized parameter value passed via CURLOPT_SSLVERSION");
    return FALSE;
  }
  return TRUE;
}

static CURLcode
ssl_connect_init_proxy(struct connectdata *conn, int sockindex)
{
  DEBUGASSERT(conn->bits.proxy_ssl_connected[sockindex]);
  if(ssl_connection_complete == conn->ssl[sockindex].state &&
     !conn->proxy_ssl[sockindex].use) {
#if defined(HTTPS_PROXY_SUPPORT)
    conn->proxy_ssl[sockindex] = conn->ssl[sockindex];
    memset(&conn->ssl[sockindex], 0, sizeof(conn->ssl[sockindex]));
#else
    return CURLE_NOT_BUILT_IN;
#endif
  }
  return CURLE_OK;
}

CURLcode
Curl_ssl_connect(struct connectdata *conn, int sockindex)
{
  CURLcode result;

  if(conn->bits.proxy_ssl_connected[sockindex]) {
    result = ssl_connect_init_proxy(conn, sockindex);
    if(result)
      return result;
  }

  if(!ssl_prefs_check(conn->data))
    return CURLE_SSL_CONNECT_ERROR;

>>>>>>> origin/tomato-shibby-RT-AC
  /* mark this is being ssl-enabled from here on. */
  conn->ssl[sockindex].use = TRUE;
  conn->ssl[sockindex].state = ssl_connection_negotiating;

  res = curlssl_connect(conn, sockindex);

  if(!res)
    Curl_pgrsTime(conn->data, TIMER_APPCONNECT); /* SSL is connected */

  return res;
}

CURLcode
Curl_ssl_connect_nonblocking(struct connectdata *conn, int sockindex,
                             bool *done)
{
<<<<<<< HEAD
  CURLcode res;
=======
  CURLcode result;
  if(conn->bits.proxy_ssl_connected[sockindex]) {
    result = ssl_connect_init_proxy(conn, sockindex);
    if(result)
      return result;
  }

  if(!ssl_prefs_check(conn->data))
    return CURLE_SSL_CONNECT_ERROR;

>>>>>>> origin/tomato-shibby-RT-AC
  /* mark this is being ssl requested from here on. */
  conn->ssl[sockindex].use = TRUE;
#ifdef curlssl_connect_nonblocking
  res = curlssl_connect_nonblocking(conn, sockindex, done);
#else
  *done = TRUE; /* fallback to BLOCKING */
  res = curlssl_connect(conn, sockindex);
#endif /* non-blocking connect support */
  if(!res && *done)
    Curl_pgrsTime(conn->data, TIMER_APPCONNECT); /* SSL is connected */
  return res;
}

/*
 * Lock shared SSL session data
 */
void Curl_ssl_sessionid_lock(struct connectdata *conn)
{
  if(SSLSESSION_SHARED(conn->data))
    Curl_share_lock(conn->data,
                    CURL_LOCK_DATA_SSL_SESSION, CURL_LOCK_ACCESS_SINGLE);
}

/*
 * Unlock shared SSL session data
 */
void Curl_ssl_sessionid_unlock(struct connectdata *conn)
{
  if(SSLSESSION_SHARED(conn->data))
    Curl_share_unlock(conn->data, CURL_LOCK_DATA_SSL_SESSION);
}

/*
 * Check if there's a session ID for the given connection in the cache, and if
 * there's one suitable, it is provided. Returns TRUE when no entry matched.
 */
<<<<<<< HEAD
int Curl_ssl_getsessionid(struct connectdata *conn,
                          void **ssl_sessionid,
                          size_t *idsize) /* set 0 if unknown */
=======
bool Curl_ssl_getsessionid(struct connectdata *conn,
                           void **ssl_sessionid,
                           size_t *idsize, /* set 0 if unknown */
                           int sockindex)
>>>>>>> origin/tomato-shibby-RT-AC
{
  struct curl_ssl_session *check;
  struct Curl_easy *data = conn->data;
  size_t i;
  long *general_age;
  bool no_match = TRUE;

  const bool isProxy = CONNECT_PROXY_SSL();
  struct ssl_primary_config * const ssl_config = isProxy ?
    &conn->proxy_ssl_config :
    &conn->ssl_config;
  const char * const name = isProxy ? conn->http_proxy.host.name :
    conn->host.name;
  int port = isProxy ? (int)conn->port : conn->remote_port;
  *ssl_sessionid = NULL;

  DEBUGASSERT(data->set.general_ssl.sessionid);

  if(!data->set.general_ssl.sessionid)
    /* session ID re-use is disabled */
    return TRUE;

  /* Lock if shared */
  if(SSLSESSION_SHARED(data))
    general_age = &data->share->sessionage;
  else
    general_age = &data->state.sessionage;

  for(i = 0; i < data->set.general_ssl.max_ssl_sessions; i++) {
    check = &data->state.session[i];
    if(!check->sessionid)
      /* not session ID means blank entry */
      continue;
<<<<<<< HEAD
    if(Curl_raw_equal(conn->host.name, check->name) &&
       (conn->remote_port == check->remote_port) &&
       Curl_ssl_config_matches(&conn->ssl_config, &check->ssl_config)) {
=======
    if(strcasecompare(name, check->name) &&
       ((!conn->bits.conn_to_host && !check->conn_to_host) ||
        (conn->bits.conn_to_host && check->conn_to_host &&
         strcasecompare(conn->conn_to_host.name, check->conn_to_host))) &&
       ((!conn->bits.conn_to_port && check->conn_to_port == -1) ||
        (conn->bits.conn_to_port && check->conn_to_port != -1 &&
         conn->conn_to_port == check->conn_to_port)) &&
       (port == check->remote_port) &&
       strcasecompare(conn->handler->scheme, check->scheme) &&
       Curl_ssl_config_matches(ssl_config, &check->ssl_config)) {
>>>>>>> origin/tomato-shibby-RT-AC
      /* yes, we have a session ID! */
      (*general_age)++;          /* increase general age */
      check->age = *general_age; /* set this as used in this age */
      *ssl_sessionid = check->sessionid;
      if(idsize)
        *idsize = check->idsize;
      no_match = FALSE;
      break;
    }
  }

  return no_match;
}

/*
 * Kill a single session ID entry in the cache.
 */
void Curl_ssl_kill_session(struct curl_ssl_session *session)
{
  if(session->sessionid) {
    /* defensive check */

    /* free the ID the SSL-layer specific way */
    curlssl_session_free(session->sessionid);

    session->sessionid = NULL;
    session->age = 0; /* fresh */

    Curl_free_primary_ssl_config(&session->ssl_config);

    Curl_safefree(session->name);
  }
}

/*
 * Delete the given session ID from the cache.
 */
void Curl_ssl_delsessionid(struct connectdata *conn, void *ssl_sessionid)
{
  size_t i;
  struct Curl_easy *data=conn->data;

  for(i = 0; i < data->set.general_ssl.max_ssl_sessions; i++) {
    struct curl_ssl_session *check = &data->state.session[i];

    if(check->sessionid == ssl_sessionid) {
      Curl_ssl_kill_session(check);
      break;
    }
  }
}

/*
 * Store session id in the session cache. The ID passed on to this function
 * must already have been extracted and allocated the proper way for the SSL
 * layer. Curl_XXXX_session_free() will be called to free/kill the session ID
 * later on.
 */
CURLcode Curl_ssl_addsessionid(struct connectdata *conn,
                               void *ssl_sessionid,
                               size_t idsize,
                               int sockindex)
{
  size_t i;
  struct Curl_easy *data=conn->data; /* the mother of all structs */
  struct curl_ssl_session *store = &data->state.session[0];
  long oldest_age=data->state.session[0].age; /* zero if unused */
  char *clone_host;
  long *general_age;
  const bool isProxy = CONNECT_PROXY_SSL();
  struct ssl_primary_config * const ssl_config = isProxy ?
    &conn->proxy_ssl_config :
    &conn->ssl_config;

  DEBUGASSERT(data->set.general_ssl.sessionid);

  clone_host = strdup(isProxy ? conn->http_proxy.host.name : conn->host.name);
  if(!clone_host)
    return CURLE_OUT_OF_MEMORY; /* bail out */

  /* Now we should add the session ID and the host name to the cache, (remove
     the oldest if necessary) */

  /* If using shared SSL session, lock! */
  if(SSLSESSION_SHARED(data)) {
    general_age = &data->share->sessionage;
  }
  else {
    general_age = &data->state.sessionage;
  }

  /* find an empty slot for us, or find the oldest */
  for(i = 1; (i < data->set.general_ssl.max_ssl_sessions) &&
        data->state.session[i].sessionid; i++) {
    if(data->state.session[i].age < oldest_age) {
      oldest_age = data->state.session[i].age;
      store = &data->state.session[i];
    }
  }
  if(i == data->set.general_ssl.max_ssl_sessions)
    /* cache is full, we must "kill" the oldest entry! */
    Curl_ssl_kill_session(store);
  else
    store = &data->state.session[i]; /* use this slot */

  /* now init the session struct wisely */
  store->sessionid = ssl_sessionid;
  store->idsize = idsize;
  store->age = *general_age;    /* set current age */
<<<<<<< HEAD
  if(store->name)
    /* free it if there's one already present */
    free(store->name);
  store->name = clone_host;               /* clone host name */
  store->remote_port = conn->remote_port; /* port number */


  /* Unlock */
  if(SSLSESSION_SHARED(data))
    Curl_share_unlock(data, CURL_LOCK_DATA_SSL_SESSION);

  if(!Curl_clone_ssl_config(&conn->ssl_config, &store->ssl_config)) {
=======
  /* free it if there's one already present */
  free(store->name);
  free(store->conn_to_host);
  store->name = clone_host;               /* clone host name */
  store->conn_to_host = clone_conn_to_host; /* clone connect to host name */
  store->conn_to_port = conn_to_port; /* connect to port number */
  /* port number */
  store->remote_port = isProxy ? (int)conn->port : conn->remote_port;
  store->scheme = conn->handler->scheme;

  if(!Curl_clone_primary_ssl_config(ssl_config, &store->ssl_config)) {
>>>>>>> origin/tomato-shibby-RT-AC
    store->sessionid = NULL; /* let caller free sessionid */
    free(clone_host);
    return CURLE_OUT_OF_MEMORY;
  }

  return CURLE_OK;
}


void Curl_ssl_close_all(struct Curl_easy *data)
{
  size_t i;
  /* kill the session ID cache if not shared */
  if(data->state.session && !SSLSESSION_SHARED(data)) {
    for(i = 0; i < data->set.general_ssl.max_ssl_sessions; i++)
      /* the single-killer function handles empty table slots */
      Curl_ssl_kill_session(&data->state.session[i]);

    /* free the cache data */
    Curl_safefree(data->state.session);
  }

  curlssl_close_all(data);
}

#if defined(USE_OPENSSL) || defined(USE_GNUTLS) || defined(USE_SCHANNEL) || \
  defined(USE_DARWINSSL) || defined(USE_POLARSSL) || defined(USE_NSS) || \
  defined(USE_MBEDTLS)
int Curl_ssl_getsock(struct connectdata *conn, curl_socket_t *socks,
                     int numsocks)
{
  struct ssl_connect_data *connssl = &conn->ssl[FIRSTSOCKET];

  if(!numsocks)
    return GETSOCK_BLANK;

  if(connssl->connecting_state == ssl_connect_2_writing) {
    /* write mode */
    socks[0] = conn->sock[FIRSTSOCKET];
    return GETSOCK_WRITESOCK(0);
  }
  else if(connssl->connecting_state == ssl_connect_2_reading) {
    /* read mode */
    socks[0] = conn->sock[FIRSTSOCKET];
    return GETSOCK_READSOCK(0);
  }

  return GETSOCK_BLANK;
}
#else
int Curl_ssl_getsock(struct connectdata *conn,
                     curl_socket_t *socks,
                     int numsocks)
{
  (void)conn;
  (void)socks;
  (void)numsocks;
  return GETSOCK_BLANK;
}
/* USE_OPENSSL || USE_GNUTLS || USE_SCHANNEL || USE_DARWINSSL || USE_NSS */
#endif

void Curl_ssl_close(struct connectdata *conn, int sockindex)
{
  DEBUGASSERT((sockindex <= 1) && (sockindex >= -1));
  curlssl_close(conn, sockindex);
}

CURLcode Curl_ssl_shutdown(struct connectdata *conn, int sockindex)
{
  if(curlssl_shutdown(conn, sockindex))
    return CURLE_SSL_SHUTDOWN_FAILED;

  conn->ssl[sockindex].use = FALSE; /* get back to ordinary socket usage */
  conn->ssl[sockindex].state = ssl_connection_none;

  conn->recv[sockindex] = Curl_recv_plain;
  conn->send[sockindex] = Curl_send_plain;

  return CURLE_OK;
}

/* Selects an SSL crypto engine
 */
CURLcode Curl_ssl_set_engine(struct Curl_easy *data, const char *engine)
{
  return curlssl_set_engine(data, engine);
}

/* Selects the default SSL crypto engine
 */
CURLcode Curl_ssl_set_engine_default(struct Curl_easy *data)
{
  return curlssl_set_engine_default(data);
}

/* Return list of OpenSSL crypto engine names. */
struct curl_slist *Curl_ssl_engines_list(struct Curl_easy *data)
{
  return curlssl_engines_list(data);
}

/*
 * This sets up a session ID cache to the specified size. Make sure this code
 * is agnostic to what underlying SSL technology we use.
 */
CURLcode Curl_ssl_initsessions(struct Curl_easy *data, size_t amount)
{
  struct curl_ssl_session *session;

  if(data->state.session)
    /* this is just a precaution to prevent multiple inits */
    return CURLE_OK;

  session = calloc(amount, sizeof(struct curl_ssl_session));
  if(!session)
    return CURLE_OUT_OF_MEMORY;

  /* store the info in the SSL section */
  data->set.general_ssl.max_ssl_sessions = amount;
  data->state.session = session;
  data->state.sessionage = 1; /* this is brand new */
  return CURLE_OK;
}

size_t Curl_ssl_version(char *buffer, size_t size)
{
  return curlssl_version(buffer, size);
}

/*
 * This function tries to determine connection status.
 *
 * Return codes:
 *     1 means the connection is still in place
 *     0 means the connection has been closed
 *    -1 means the connection status is unknown
 */
int Curl_ssl_check_cxn(struct connectdata *conn)
{
  return curlssl_check_cxn(conn);
}

bool Curl_ssl_data_pending(const struct connectdata *conn,
                           int connindex)
{
  return curlssl_data_pending(conn, connindex);
}

void Curl_ssl_free_certinfo(struct Curl_easy *data)
{
  int i;
  struct curl_certinfo *ci = &data->info.certs;
  if(ci->num_of_certs) {
    /* free all individual lists used */
    for(i=0; i<ci->num_of_certs; i++) {
      curl_slist_free_all(ci->certinfo[i]);
      ci->certinfo[i] = NULL;
    }
    free(ci->certinfo); /* free the actual array too */
    ci->certinfo = NULL;
    ci->num_of_certs = 0;
  }
}

<<<<<<< HEAD
int Curl_ssl_init_certinfo(struct SessionHandle * data,
                           int num)
=======
CURLcode Curl_ssl_init_certinfo(struct Curl_easy *data, int num)
>>>>>>> origin/tomato-shibby-RT-AC
{
  struct curl_certinfo * ci = &data->info.certs;
  struct curl_slist * * table;

  /* Initialize the certificate information structures. Return 0 if OK, else 1.
   */
  Curl_ssl_free_certinfo(data);
  ci->num_of_certs = num;
  table = calloc((size_t) num, sizeof(struct curl_slist *));
  if(!table)
    return 1;

  ci->certinfo = table;
  return 0;
}

/*
 * 'value' is NOT a zero terminated string
 */
CURLcode Curl_ssl_push_certinfo_len(struct Curl_easy *data,
                                    int certnum,
                                    const char *label,
                                    const char *value,
                                    size_t valuelen)
{
<<<<<<< HEAD
  struct curl_certinfo * ci = &data->info.certs;
  char * output;
  struct curl_slist * nl;
  CURLcode res = CURLE_OK;
=======
  struct curl_certinfo *ci = &data->info.certs;
  char *output;
  struct curl_slist *nl;
  CURLcode result = CURLE_OK;
>>>>>>> origin/tomato-shibby-RT-AC
  size_t labellen = strlen(label);
  size_t outlen = labellen + 1 + valuelen + 1; /* label:value\0 */

  output = malloc(outlen);
  if(!output)
    return CURLE_OUT_OF_MEMORY;

  /* sprintf the label and colon */
  snprintf(output, outlen, "%s:", label);

  /* memcpy the value (it might not be zero terminated) */
  memcpy(&output[labellen+1], value, valuelen);

  /* zero terminate the output */
  output[labellen + 1 + valuelen] = 0;

  nl = Curl_slist_append_nodup(ci->certinfo[certnum], output);
  if(!nl) {
    free(output);
    curl_slist_free_all(ci->certinfo[certnum]);
    res = CURLE_OUT_OF_MEMORY;
  }

  ci->certinfo[certnum] = nl;
  return res;
}

/*
 * This is a convenience function for push_certinfo_len that takes a zero
 * terminated value.
 */
CURLcode Curl_ssl_push_certinfo(struct Curl_easy *data,
                                int certnum,
                                const char *label,
                                const char *value)
{
  size_t valuelen = strlen(value);

  return Curl_ssl_push_certinfo_len(data, certnum, label, value, valuelen);
}

<<<<<<< HEAD
/* these functions are only provided by some SSL backends */

#ifdef have_curlssl_random
void Curl_ssl_random(struct SessionHandle *data,
                     unsigned char *entropy,
                     size_t length)
=======
CURLcode Curl_ssl_random(struct Curl_easy *data,
                         unsigned char *entropy,
                         size_t length)
>>>>>>> origin/tomato-shibby-RT-AC
{
  curlssl_random(data, entropy, length);
}
<<<<<<< HEAD
=======

/*
 * Generic pinned public key check.
 */

CURLcode Curl_pin_peer_pubkey(struct Curl_easy *data,
                              const char *pinnedpubkey,
                              const unsigned char *pubkey, size_t pubkeylen)
{
  FILE *fp;
  unsigned char *buf = NULL, *pem_ptr = NULL;
  long filesize;
  size_t size, pem_len;
  CURLcode pem_read;
  CURLcode result = CURLE_SSL_PINNEDPUBKEYNOTMATCH;
#ifdef curlssl_sha256sum
  CURLcode encode;
  size_t encodedlen, pinkeylen;
  char *encoded, *pinkeycopy, *begin_pos, *end_pos;
  unsigned char *sha256sumdigest = NULL;
>>>>>>> origin/tomato-shibby-RT-AC
#endif

#ifdef have_curlssl_md5sum
void Curl_ssl_md5sum(unsigned char *tmp, /* input */
                     size_t tmplen,
                     unsigned char *md5sum, /* output */
                     size_t md5len)
{
  curlssl_md5sum(tmp, tmplen, md5sum, md5len);
}
#endif

#endif /* USE_SSL */
