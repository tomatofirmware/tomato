/***************************************************************************
 *                                  _   _ ____  _
 *  Project                     ___| | | |  _ \| |
 *                             / __| | | | |_) | |
 *                            | (__| |_| |  _ <| |___
 *                             \___|\___/|_| \_\_____|
 *
 * Copyright (C) 2010 - 2011, Hoi-Ho Chan, <hoiho.chan@gmail.com>
 * Copyright (C) 2012 - 2014, Daniel Stenberg, <daniel@haxx.se>, et al.
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

/*
 * Source file for all PolarSSL-specific code for the TLS/SSL layer. No code
 * but vtls.c should ever call or use these functions.
 *
 */

#include "curl_setup.h"

#ifdef USE_POLARSSL

#include <polarssl/net.h>
#include <polarssl/ssl.h>
#include <polarssl/certs.h>
#include <polarssl/x509.h>
#include <polarssl/version.h>

#if POLARSSL_VERSION_NUMBER < 0x01030000
#error too old PolarSSL
#endif

#include <polarssl/error.h>
#include <polarssl/entropy.h>
#include <polarssl/ctr_drbg.h>

#include "urldata.h"
#include "sendf.h"
#include "inet_pton.h"
#include "polarssl.h"
#include "vtls.h"
#include "parsedate.h"
#include "connect.h" /* for the connect timeout */
#include "select.h"
#include "strcase.h"
#include "polarssl_threadlock.h"

#define _MPRINTF_REPLACE /* use our functions only */
#include <curl/mprintf.h>
#include "curl_memory.h"
/* The last #include file should be: */
#include "memdebug.h"

/* apply threading? */
#if defined(USE_THREADS_POSIX) || defined(USE_THREADS_WIN32)
#define THREADING_SUPPORT
#endif

#ifndef POLARSSL_ERROR_C
#define error_strerror(x,y,z)
#endif /* POLARSSL_ERROR_C */


#if defined(THREADING_SUPPORT)
static entropy_context entropy;

static int  entropy_init_initialized  = 0;

/* start of entropy_init_mutex() */
static void entropy_init_mutex(entropy_context *ctx)
{
  /* lock 0 = entropy_init_mutex() */
  polarsslthreadlock_lock_function(0);
  if(entropy_init_initialized == 0) {
    entropy_init(ctx);
    entropy_init_initialized = 1;
  }
  polarsslthreadlock_unlock_function(0);
}
/* end of entropy_init_mutex() */

/* start of entropy_func_mutex() */
static int entropy_func_mutex(void *data, unsigned char *output, size_t len)
{
<<<<<<< HEAD
    int ret;
    /* lock 1 = entropy_func_mutex() */
    polarsslthreadlock_lock_function(1);
    ret = entropy_func(data, output, len);
    polarsslthreadlock_unlock_function(1);
=======
  int ret;
  /* lock 1 = entropy_func_mutex() */
  Curl_polarsslthreadlock_lock_function(1);
  ret = entropy_func(data, output, len);
  Curl_polarsslthreadlock_unlock_function(1);
>>>>>>> origin/tomato-shibby-RT-AC

  return ret;
}
/* end of entropy_func_mutex() */

#endif /* THREADING_SUPPORT */

/* Define this to enable lots of debugging for PolarSSL */
#undef POLARSSL_DEBUG

#ifdef POLARSSL_DEBUG
static void polarssl_debug(void *context, int level, const char *line)
{
  struct Curl_easy *data = NULL;

  if(!context)
    return;

  data = (struct Curl_easy *)context;

  infof(data, "%s", line);
  (void) level;
}
#else
#endif

static Curl_recv polarssl_recv;
static Curl_send polarssl_send;


static CURLcode
polarssl_connect_step1(struct connectdata *conn,
                       int sockindex)
{
  struct Curl_easy *data = conn->data;
  struct ssl_connect_data* connssl = &conn->ssl[sockindex];
  const char *capath = SSL_CONN_CONFIG(CApath);
  const char * const hostname = SSL_IS_PROXY() ? conn->http_proxy.host.name :
    conn->host.name;
  const long int port = SSL_IS_PROXY() ? conn->port : conn->remote_port;
  int ret = -1;
<<<<<<< HEAD
#ifdef ENABLE_IPV6
  struct in6_addr addr;
#else
  struct in_addr addr;
#endif
  void *old_session = NULL;
  size_t old_session_size = 0;

=======
>>>>>>> origin/tomato-shibby-RT-AC
  char errorbuf[128];
  memset(errorbuf, 0, sizeof(errorbuf));


  /* PolarSSL only supports SSLv3 and TLSv1 */
  if(SSL_CONN_CONFIG(version) == CURL_SSLVERSION_SSLv2) {
    failf(data, "PolarSSL does not support SSLv2");
    return CURLE_SSL_CONNECT_ERROR;
  }

#ifdef THREADING_SUPPORT
  entropy_init_mutex(&entropy);

  if((ret = ctr_drbg_init(&connssl->ctr_drbg, entropy_func_mutex, &entropy,
<<<<<<< HEAD
                               connssl->ssn.id, connssl->ssn.length)) != 0) {
#ifdef POLARSSL_ERROR_C
     error_strerror(ret, errorbuf, sizeof(errorbuf));
#endif /* POLARSSL_ERROR_C */
     failf(data, "Failed - PolarSSL: ctr_drbg_init returned (-0x%04X) %s\n",
                                                            -ret, errorbuf);
=======
                          NULL, 0)) != 0) {
    error_strerror(ret, errorbuf, sizeof(errorbuf));
    failf(data, "Failed - PolarSSL: ctr_drbg_init returned (-0x%04X) %s\n",
          -ret, errorbuf);
>>>>>>> origin/tomato-shibby-RT-AC
  }
#else
  entropy_init(&connssl->entropy);

  if((ret = ctr_drbg_init(&connssl->ctr_drbg, entropy_func, &connssl->entropy,
<<<<<<< HEAD
                                connssl->ssn.id, connssl->ssn.length)) != 0) {
#ifdef POLARSSL_ERROR_C
     error_strerror(ret, errorbuf, sizeof(errorbuf));
#endif /* POLARSSL_ERROR_C */
     failf(data, "Failed - PolarSSL: ctr_drbg_init returned (-0x%04X) %s\n",
                                                            -ret, errorbuf);
=======
                          NULL, 0)) != 0) {
    error_strerror(ret, errorbuf, sizeof(errorbuf));
    failf(data, "Failed - PolarSSL: ctr_drbg_init returned (-0x%04X) %s\n",
          -ret, errorbuf);
>>>>>>> origin/tomato-shibby-RT-AC
  }
#endif /* THREADING_SUPPORT */

  /* Load the trusted CA */
  memset(&connssl->cacert, 0, sizeof(x509_crt));

  if(SSL_CONN_CONFIG(CAfile)) {
    ret = x509_crt_parse_file(&connssl->cacert,
                              SSL_CONN_CONFIG(CAfile));

    if(ret<0) {
      error_strerror(ret, errorbuf, sizeof(errorbuf));
      failf(data, "Error reading ca cert file %s - PolarSSL: (-0x%04X) %s",
            SSL_CONN_CONFIG(CAfile), -ret, errorbuf);

      if(SSL_CONN_CONFIG(verifypeer))
        return CURLE_SSL_CACERT_BADFILE;
    }
  }

<<<<<<< HEAD
=======
  if(capath) {
    ret = x509_crt_parse_path(&connssl->cacert, capath);

    if(ret<0) {
      error_strerror(ret, errorbuf, sizeof(errorbuf));
      failf(data, "Error reading ca cert path %s - PolarSSL: (-0x%04X) %s",
            capath, -ret, errorbuf);

      if(SSL_CONN_CONFIG(verifypeer))
        return CURLE_SSL_CACERT_BADFILE;
    }
  }

>>>>>>> origin/tomato-shibby-RT-AC
  /* Load the client certificate */
  memset(&connssl->clicert, 0, sizeof(x509_crt));

  if(SSL_SET_OPTION(cert)) {
    ret = x509_crt_parse_file(&connssl->clicert,
                              SSL_SET_OPTION(cert));

    if(ret) {
      error_strerror(ret, errorbuf, sizeof(errorbuf));
      failf(data, "Error reading client cert file %s - PolarSSL: (-0x%04X) %s",
            SSL_SET_OPTION(cert), -ret, errorbuf);

      return CURLE_SSL_CERTPROBLEM;
    }
  }

  /* Load the client private key */
  if(SSL_SET_OPTION(key)) {
    pk_context pk;
    pk_init(&pk);
    ret = pk_parse_keyfile(&pk, SSL_SET_OPTION(key),
                           SSL_SET_OPTION(key_passwd));
    if(ret == 0 && !pk_can_do(&pk, POLARSSL_PK_RSA))
      ret = POLARSSL_ERR_PK_TYPE_MISMATCH;
    if(ret == 0)
      rsa_copy(&connssl->rsa, pk_rsa(pk));
    else
      rsa_free(&connssl->rsa);
    pk_free(&pk);

    if(ret) {
      error_strerror(ret, errorbuf, sizeof(errorbuf));
      failf(data, "Error reading private key %s - PolarSSL: (-0x%04X) %s",
            SSL_SET_OPTION(key), -ret, errorbuf);

      return CURLE_SSL_CERTPROBLEM;
    }
  }

  /* Load the CRL */
  memset(&connssl->crl, 0, sizeof(x509_crl));

  if(SSL_SET_OPTION(CRLfile)) {
    ret = x509_crl_parse_file(&connssl->crl,
                              SSL_SET_OPTION(CRLfile));

    if(ret) {
      error_strerror(ret, errorbuf, sizeof(errorbuf));
      failf(data, "Error reading CRL file %s - PolarSSL: (-0x%04X) %s",
            SSL_SET_OPTION(CRLfile), -ret, errorbuf);

      return CURLE_SSL_CRL_BADFILE;
    }
  }

  infof(data, "PolarSSL: Connecting to %s:%d\n", hostname, port);

  if(ssl_init(&connssl->ssl)) {
    failf(data, "PolarSSL: ssl_init failed");
    return CURLE_SSL_CONNECT_ERROR;
  }

<<<<<<< HEAD
=======
  switch(SSL_CONN_CONFIG(version)) {
  case CURL_SSLVERSION_DEFAULT:
  case CURL_SSLVERSION_TLSv1:
    ssl_set_min_version(&connssl->ssl, SSL_MAJOR_VERSION_3,
                        SSL_MINOR_VERSION_1);
    break;
  case CURL_SSLVERSION_SSLv3:
    ssl_set_min_version(&connssl->ssl, SSL_MAJOR_VERSION_3,
                        SSL_MINOR_VERSION_0);
    ssl_set_max_version(&connssl->ssl, SSL_MAJOR_VERSION_3,
                        SSL_MINOR_VERSION_0);
    infof(data, "PolarSSL: Forced min. SSL Version to be SSLv3\n");
    break;
  case CURL_SSLVERSION_TLSv1_0:
    ssl_set_min_version(&connssl->ssl, SSL_MAJOR_VERSION_3,
                        SSL_MINOR_VERSION_1);
    ssl_set_max_version(&connssl->ssl, SSL_MAJOR_VERSION_3,
                        SSL_MINOR_VERSION_1);
    infof(data, "PolarSSL: Forced min. SSL Version to be TLS 1.0\n");
    break;
  case CURL_SSLVERSION_TLSv1_1:
    ssl_set_min_version(&connssl->ssl, SSL_MAJOR_VERSION_3,
                        SSL_MINOR_VERSION_2);
    ssl_set_max_version(&connssl->ssl, SSL_MAJOR_VERSION_3,
                        SSL_MINOR_VERSION_2);
    infof(data, "PolarSSL: Forced min. SSL Version to be TLS 1.1\n");
    break;
  case CURL_SSLVERSION_TLSv1_2:
    ssl_set_min_version(&connssl->ssl, SSL_MAJOR_VERSION_3,
                        SSL_MINOR_VERSION_3);
    ssl_set_max_version(&connssl->ssl, SSL_MAJOR_VERSION_3,
                        SSL_MINOR_VERSION_3);
    infof(data, "PolarSSL: Forced min. SSL Version to be TLS 1.2\n");
    break;
  case CURL_SSLVERSION_TLSv1_3:
    failf(data, "PolarSSL: TLS 1.3 is not yet supported");
    return CURLE_SSL_CONNECT_ERROR;
  default:
    failf(data, "Unrecognized parameter passed via CURLOPT_SSLVERSION");
    return CURLE_SSL_CONNECT_ERROR;
  }

>>>>>>> origin/tomato-shibby-RT-AC
  ssl_set_endpoint(&connssl->ssl, SSL_IS_CLIENT);
  ssl_set_authmode(&connssl->ssl, SSL_VERIFY_OPTIONAL);

  ssl_set_rng(&connssl->ssl, ctr_drbg_random,
              &connssl->ctr_drbg);
  ssl_set_bio(&connssl->ssl,
              net_recv, &conn->sock[sockindex],
              net_send, &conn->sock[sockindex]);

  ssl_set_ciphersuites(&connssl->ssl, ssl_list_ciphersuites());
<<<<<<< HEAD
  if(!Curl_ssl_getsessionid(conn, &old_session, &old_session_size)) {
    memcpy(&connssl->ssn, old_session, old_session_size);
    infof(data, "PolarSSL re-using session\n");
=======

  /* Check if there's a cached ID we can/should use here! */
  if(data->set.general_ssl.sessionid) {
    void *old_session = NULL;

    Curl_ssl_sessionid_lock(conn);
    if(!Curl_ssl_getsessionid(conn, &old_session, NULL, sockindex)) {
      ret = ssl_set_session(&connssl->ssl, old_session);
      if(ret) {
        Curl_ssl_sessionid_unlock(conn);
        failf(data, "ssl_set_session returned -0x%x", -ret);
        return CURLE_SSL_CONNECT_ERROR;
      }
      infof(data, "PolarSSL re-using session\n");
    }
    Curl_ssl_sessionid_unlock(conn);
>>>>>>> origin/tomato-shibby-RT-AC
  }

  ssl_set_session(&connssl->ssl,
                  &connssl->ssn);

  ssl_set_ca_chain(&connssl->ssl,
                   &connssl->cacert,
                   &connssl->crl,
                   hostname);

  ssl_set_own_cert_rsa(&connssl->ssl,
                       &connssl->clicert, &connssl->rsa);

<<<<<<< HEAD
  if(!Curl_inet_pton(AF_INET, conn->host.name, &addr) &&
#ifdef ENABLE_IPV6
     !Curl_inet_pton(AF_INET6, conn->host.name, &addr) &&
=======
  if(ssl_set_hostname(&connssl->ssl, hostname)) {
    /* ssl_set_hostname() sets the name to use in CN/SAN checks *and* the name
       to set in the SNI extension. So even if curl connects to a host
       specified as an IP address, this function must be used. */
    failf(data, "couldn't set hostname in PolarSSL");
    return CURLE_SSL_CONNECT_ERROR;
  }

#ifdef HAS_ALPN
  if(conn->bits.tls_enable_alpn) {
    static const char *protocols[3];
    int cur = 0;

#ifdef USE_NGHTTP2
    if(data->set.httpversion >= CURL_HTTP_VERSION_2) {
      protocols[cur++] = NGHTTP2_PROTO_VERSION_ID;
      infof(data, "ALPN, offering %s\n", NGHTTP2_PROTO_VERSION_ID);
    }
>>>>>>> origin/tomato-shibby-RT-AC
#endif
     sni && ssl_set_hostname(&connssl->ssl, conn->host.name)) {
     infof(data, "WARNING: failed to configure "
                 "server name indication (SNI) TLS extension\n");
  }

#ifdef POLARSSL_DEBUG
  ssl_set_dbg(&connssl->ssl, polarssl_debug, data);
#endif

  connssl->connecting_state = ssl_connect_2;

  return CURLE_OK;
}

static CURLcode
polarssl_connect_step2(struct connectdata *conn,
                       int sockindex)
{
  int ret;
  struct Curl_easy *data = conn->data;
  struct ssl_connect_data* connssl = &conn->ssl[sockindex];
  char buffer[1024];
  const char * const pinnedpubkey = SSL_IS_PROXY() ?
            data->set.str[STRING_SSL_PINNEDPUBLICKEY_PROXY] :
            data->set.str[STRING_SSL_PINNEDPUBLICKEY_ORIG];


  char errorbuf[128];
  memset(errorbuf, 0, sizeof(errorbuf));

  conn->recv[sockindex] = polarssl_recv;
  conn->send[sockindex] = polarssl_send;

<<<<<<< HEAD
  for(;;) {
    if(!(ret = ssl_handshake(&connssl->ssl)))
      break;
    else if(ret != POLARSSL_ERR_NET_WANT_READ &&
            ret != POLARSSL_ERR_NET_WANT_WRITE) {
#ifdef POLARSSL_ERROR_C
     error_strerror(ret, errorbuf, sizeof(errorbuf));
#endif /* POLARSSL_ERROR_C */
     failf(data, "ssl_handshake returned - PolarSSL: (-0x%04X) %s",
                                                    -ret, errorbuf);

     return CURLE_SSL_CONNECT_ERROR;
    }
    else {
      if(ret == POLARSSL_ERR_NET_WANT_READ) {
        connssl->connecting_state = ssl_connect_2_reading;
        return CURLE_OK;
      }
      if(ret == POLARSSL_ERR_NET_WANT_WRITE) {
        connssl->connecting_state = ssl_connect_2_writing;
        return CURLE_OK;
      }
      failf(data, "SSL_connect failed with error %d.", ret);
      return CURLE_SSL_CONNECT_ERROR;

    }
=======
  ret = ssl_handshake(&connssl->ssl);

  switch(ret) {
  case 0:
    break;

  case POLARSSL_ERR_NET_WANT_READ:
    connssl->connecting_state = ssl_connect_2_reading;
    return CURLE_OK;

  case POLARSSL_ERR_NET_WANT_WRITE:
    connssl->connecting_state = ssl_connect_2_writing;
    return CURLE_OK;

  default:
    error_strerror(ret, errorbuf, sizeof(errorbuf));
    failf(data, "ssl_handshake returned - PolarSSL: (-0x%04X) %s",
          -ret, errorbuf);
    return CURLE_SSL_CONNECT_ERROR;
>>>>>>> origin/tomato-shibby-RT-AC
  }

  infof(data, "PolarSSL: Handshake complete, cipher is %s\n",
        ssl_get_ciphersuite(&conn->ssl[sockindex].ssl)
    );

  ret = ssl_get_verify_result(&conn->ssl[sockindex].ssl);

  if(ret && SSL_CONN_CONFIG(verifypeer)) {
    if(ret & BADCERT_EXPIRED)
      failf(data, "Cert verify failed: BADCERT_EXPIRED");

    if(ret & BADCERT_REVOKED) {
      failf(data, "Cert verify failed: BADCERT_REVOKED");
      return CURLE_SSL_CACERT;
    }

    if(ret & BADCERT_CN_MISMATCH)
      failf(data, "Cert verify failed: BADCERT_CN_MISMATCH");

    if(ret & BADCERT_NOT_TRUSTED)
      failf(data, "Cert verify failed: BADCERT_NOT_TRUSTED");

    return CURLE_PEER_FAILED_VERIFICATION;
  }

  if(ssl_get_peer_cert(&(connssl->ssl))) {
    /* If the session was resumed, there will be no peer certs */
    memset(buffer, 0, sizeof(buffer));

    if(x509_crt_info(buffer, sizeof(buffer), (char *)"* ",
                     ssl_get_peer_cert(&(connssl->ssl))) != -1)
      infof(data, "Dumping cert info:\n%s\n", buffer);
  }

<<<<<<< HEAD
=======
  /* adapted from mbedtls.c */
  if(pinnedpubkey) {
    int size;
    CURLcode result;
    x509_crt *p;
    unsigned char pubkey[PUB_DER_MAX_BYTES];
    const x509_crt *peercert;

    peercert = ssl_get_peer_cert(&connssl->ssl);

    if(!peercert || !peercert->raw.p || !peercert->raw.len) {
      failf(data, "Failed due to missing peer certificate");
      return CURLE_SSL_PINNEDPUBKEYNOTMATCH;
    }

    p = calloc(1, sizeof(*p));

    if(!p)
      return CURLE_OUT_OF_MEMORY;

    x509_crt_init(p);

    /* Make a copy of our const peercert because pk_write_pubkey_der
       needs a non-const key, for now.
       https://github.com/ARMmbed/mbedtls/issues/396 */
    if(x509_crt_parse_der(p, peercert->raw.p, peercert->raw.len)) {
      failf(data, "Failed copying peer certificate");
      x509_crt_free(p);
      free(p);
      return CURLE_SSL_PINNEDPUBKEYNOTMATCH;
    }

    size = pk_write_pubkey_der(&p->pk, pubkey, PUB_DER_MAX_BYTES);

    if(size <= 0) {
      failf(data, "Failed copying public key from peer certificate");
      x509_crt_free(p);
      free(p);
      return CURLE_SSL_PINNEDPUBKEYNOTMATCH;
    }

    /* pk_write_pubkey_der writes data at the end of the buffer. */
    result = Curl_pin_peer_pubkey(data,
                                  pinnedpubkey,
                                  &pubkey[PUB_DER_MAX_BYTES - size], size);
    if(result) {
      x509_crt_free(p);
      free(p);
      return result;
    }

    x509_crt_free(p);
    free(p);
  }

#ifdef HAS_ALPN
  if(conn->bits.tls_enable_alpn) {
    const char *next_protocol = ssl_get_alpn_protocol(&connssl->ssl);

    if(next_protocol != NULL) {
      infof(data, "ALPN, server accepted to use %s\n", next_protocol);

#ifdef USE_NGHTTP2
      if(!strncmp(next_protocol, NGHTTP2_PROTO_VERSION_ID,
                  NGHTTP2_PROTO_VERSION_ID_LEN)) {
        conn->negnpn = CURL_HTTP_VERSION_2;
      }
      else
#endif
        if(!strncmp(next_protocol, ALPN_HTTP_1_1, ALPN_HTTP_1_1_LENGTH)) {
          conn->negnpn = CURL_HTTP_VERSION_1_1;
        }
    }
    else
      infof(data, "ALPN, server did not agree to a protocol\n");
  }
#endif

>>>>>>> origin/tomato-shibby-RT-AC
  connssl->connecting_state = ssl_connect_3;
  infof(data, "SSL connected\n");

  return CURLE_OK;
}

static CURLcode
polarssl_connect_step3(struct connectdata *conn,
                       int sockindex)
{
  CURLcode retcode = CURLE_OK;
  struct ssl_connect_data *connssl = &conn->ssl[sockindex];
<<<<<<< HEAD
  struct SessionHandle *data = conn->data;
  void *old_ssl_sessionid = NULL;
  ssl_session *our_ssl_sessionid = &conn->ssl[sockindex].ssn ;
  int incache;

  DEBUGASSERT(ssl_connect_3 == connssl->connecting_state);

  /* Save the current session data for possible re-use */
  incache = !(Curl_ssl_getsessionid(conn, &old_ssl_sessionid, NULL));
  if(incache) {
    if(old_ssl_sessionid != our_ssl_sessionid) {
      infof(data, "old SSL session ID is stale, removing\n");
      Curl_ssl_delsessionid(conn, old_ssl_sessionid);
      incache = FALSE;
    }
  }
  if(!incache) {
    void *new_session = malloc(sizeof(ssl_session));

    if(new_session) {
      memcpy(new_session, our_ssl_sessionid,
             sizeof(ssl_session));

      retcode = Curl_ssl_addsessionid(conn, new_session,
                                   sizeof(ssl_session));
    }
    else {
      retcode = CURLE_OUT_OF_MEMORY;
    }

    if(retcode) {
=======
  struct Curl_easy *data = conn->data;

  DEBUGASSERT(ssl_connect_3 == connssl->connecting_state);

  if(data->set.general_ssl.sessionid) {
    int ret;
    ssl_session *our_ssl_sessionid;
    void *old_ssl_sessionid = NULL;

    our_ssl_sessionid = malloc(sizeof(ssl_session));
    if(!our_ssl_sessionid)
      return CURLE_OUT_OF_MEMORY;

    ssl_session_init(our_ssl_sessionid);

    ret = ssl_get_session(&connssl->ssl, our_ssl_sessionid);
    if(ret) {
      failf(data, "ssl_get_session returned -0x%x", -ret);
      return CURLE_SSL_CONNECT_ERROR;
    }

    /* If there's already a matching session in the cache, delete it */
    Curl_ssl_sessionid_lock(conn);
    if(!Curl_ssl_getsessionid(conn, &old_ssl_sessionid, NULL, sockindex))
      Curl_ssl_delsessionid(conn, old_ssl_sessionid);

    retcode = Curl_ssl_addsessionid(conn, our_ssl_sessionid, 0, sockindex);
    Curl_ssl_sessionid_unlock(conn);
    if(retcode) {
      free(our_ssl_sessionid);
>>>>>>> origin/tomato-shibby-RT-AC
      failf(data, "failed to store ssl session");
      return retcode;
    }
  }

  connssl->connecting_state = ssl_connect_done;

  return CURLE_OK;
}

static ssize_t polarssl_send(struct connectdata *conn,
                             int sockindex,
                             const void *mem,
                             size_t len,
                             CURLcode *curlcode)
{
  int ret = -1;

  ret = ssl_write(&conn->ssl[sockindex].ssl,
                  (unsigned char *)mem, len);

  if(ret < 0) {
    *curlcode = (ret == POLARSSL_ERR_NET_WANT_WRITE) ?
      CURLE_AGAIN : CURLE_SEND_ERROR;
    ret = -1;
  }

  return ret;
}

void Curl_polarssl_close_all(struct SessionHandle *data)
{
  (void)data;
}

void Curl_polarssl_close(struct connectdata *conn, int sockindex)
{
  rsa_free(&conn->ssl[sockindex].rsa);
  x509_crt_free(&conn->ssl[sockindex].clicert);
  x509_crt_free(&conn->ssl[sockindex].cacert);
  x509_crl_free(&conn->ssl[sockindex].crl);
  ssl_free(&conn->ssl[sockindex].ssl);
}

static ssize_t polarssl_recv(struct connectdata *conn,
                             int num,
                             char *buf,
                             size_t buffersize,
                             CURLcode *curlcode)
{
  int ret = -1;
  ssize_t len = -1;

  memset(buf, 0, buffersize);
  ret = ssl_read(&conn->ssl[num].ssl, (unsigned char *)buf, buffersize);

  if(ret <= 0) {
    if(ret == POLARSSL_ERR_SSL_PEER_CLOSE_NOTIFY)
      return 0;

    *curlcode = (ret == POLARSSL_ERR_NET_WANT_READ) ?
      CURLE_AGAIN : CURLE_RECV_ERROR;
    return -1;
  }

  len = ret;

  return len;
}

void Curl_polarssl_session_free(void *ptr)
{
  free(ptr);
}

size_t Curl_polarssl_version(char *buffer, size_t size)
{
  unsigned int version = version_get_number();
  return snprintf(buffer, size, "PolarSSL/%d.%d.%d", version>>24,
                  (version>>16)&0xff, (version>>8)&0xff);
}

static CURLcode
polarssl_connect_common(struct connectdata *conn,
                        int sockindex,
                        bool nonblocking,
                        bool *done)
{
<<<<<<< HEAD
  CURLcode retcode;
  struct SessionHandle *data = conn->data;
=======
  CURLcode result;
  struct Curl_easy *data = conn->data;
>>>>>>> origin/tomato-shibby-RT-AC
  struct ssl_connect_data *connssl = &conn->ssl[sockindex];
  curl_socket_t sockfd = conn->sock[sockindex];
  long timeout_ms;
  int what;

  /* check if the connection has already been established */
  if(ssl_connection_complete == connssl->state) {
    *done = TRUE;
    return CURLE_OK;
  }

  if(ssl_connect_1==connssl->connecting_state) {
    /* Find out how much more time we're allowed */
    timeout_ms = Curl_timeleft(data, NULL, TRUE);

    if(timeout_ms < 0) {
      /* no need to continue if time already is up */
      failf(data, "SSL connection timeout");
      return CURLE_OPERATION_TIMEDOUT;
    }
    retcode = polarssl_connect_step1(conn, sockindex);
    if(retcode)
      return retcode;
  }

  while(ssl_connect_2 == connssl->connecting_state ||
        ssl_connect_2_reading == connssl->connecting_state ||
        ssl_connect_2_writing == connssl->connecting_state) {

    /* check allowed time left */
    timeout_ms = Curl_timeleft(data, NULL, TRUE);

    if(timeout_ms < 0) {
      /* no need to continue if time already is up */
      failf(data, "SSL connection timeout");
      return CURLE_OPERATION_TIMEDOUT;
    }

    /* if ssl is expecting something, check if it's available. */
    if(connssl->connecting_state == ssl_connect_2_reading
       || connssl->connecting_state == ssl_connect_2_writing) {

      curl_socket_t writefd = ssl_connect_2_writing==
        connssl->connecting_state?sockfd:CURL_SOCKET_BAD;
      curl_socket_t readfd = ssl_connect_2_reading==
        connssl->connecting_state?sockfd:CURL_SOCKET_BAD;

      what = Curl_socket_check(readfd, CURL_SOCKET_BAD, writefd,
                               nonblocking?0:timeout_ms);
      if(what < 0) {
        /* fatal error */
        failf(data, "select/poll on SSL socket, errno: %d", SOCKERRNO);
        return CURLE_SSL_CONNECT_ERROR;
      }
      else if(0 == what) {
        if(nonblocking) {
          *done = FALSE;
          return CURLE_OK;
        }
        else {
          /* timeout */
          failf(data, "SSL connection timeout");
          return CURLE_OPERATION_TIMEDOUT;
        }
      }
      /* socket is readable or writable */
    }

    /* Run transaction, and return to the caller if it failed or if
     * this connection is part of a multi handle and this loop would
     * execute again. This permits the owner of a multi handle to
     * abort a connection attempt before step2 has completed while
     * ensuring that a client using select() or epoll() will always
     * have a valid fdset to wait on.
     */
    retcode = polarssl_connect_step2(conn, sockindex);
    if(retcode || (nonblocking &&
                   (ssl_connect_2 == connssl->connecting_state ||
                    ssl_connect_2_reading == connssl->connecting_state ||
                    ssl_connect_2_writing == connssl->connecting_state)))
      return retcode;

  } /* repeat step2 until all transactions are done. */

  if(ssl_connect_3==connssl->connecting_state) {
    retcode = polarssl_connect_step3(conn, sockindex);
    if(retcode)
      return retcode;
  }

  if(ssl_connect_done==connssl->connecting_state) {
    connssl->state = ssl_connection_complete;
    conn->recv[sockindex] = polarssl_recv;
    conn->send[sockindex] = polarssl_send;
    *done = TRUE;
  }
  else
    *done = FALSE;

  /* Reset our connect state machine */
  connssl->connecting_state = ssl_connect_1;

  return CURLE_OK;
}

CURLcode
Curl_polarssl_connect_nonblocking(struct connectdata *conn,
                                  int sockindex,
                                  bool *done)
{
  return polarssl_connect_common(conn, sockindex, TRUE, done);
}


CURLcode
Curl_polarssl_connect(struct connectdata *conn,
                      int sockindex)
{
  CURLcode retcode;
  bool done = FALSE;

  retcode = polarssl_connect_common(conn, sockindex, FALSE, &done);
  if(retcode)
    return retcode;

  DEBUGASSERT(done);

  return CURLE_OK;
}

/*
 * return 0 error initializing SSL
 * return 1 SSL initialized successfully
 */
int polarssl_init(void)
{
  return polarsslthreadlock_thread_setup();
}

void polarssl_cleanup(void)
{
  (void)polarsslthreadlock_thread_cleanup();
}


int Curl_polarssl_data_pending(const struct connectdata *conn, int sockindex)
{
  return ssl_get_bytes_avail(&conn->ssl[sockindex].ssl) != 0;
}

#endif /* USE_POLARSSL */
