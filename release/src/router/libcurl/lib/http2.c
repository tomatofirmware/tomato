/***************************************************************************
 *                                  _   _ ____  _
 *  Project                     ___| | | |  _ \| |
 *                             / __| | | | |_) | |
 *                            | (__| |_| |  _ <| |___
 *                             \___|\___/|_| \_\_____|
 *
<<<<<<< HEAD
 * Copyright (C) 1998 - 2014, Daniel Stenberg, <daniel@haxx.se>, et al.
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

#include "curl_setup.h"

#ifdef USE_NGHTTP2
#define _MPRINTF_REPLACE
#include <curl/mprintf.h>

#include <nghttp2/nghttp2.h>
#include "urldata.h"
#include "http2.h"
#include "http.h"
#include "sendf.h"
#include "curl_base64.h"
<<<<<<< HEAD
#include "curl_memory.h"
#include "rawstr.h"
#include "multiif.h"

/* include memdebug.h last */
=======
#include "strcase.h"
#include "multiif.h"
#include "conncache.h"
#include "url.h"
#include "connect.h"
#include "strtoofft.h"
#include "strdup.h"
/* The last 3 #include files should be in this order */
#include "curl_printf.h"
#include "curl_memory.h"
>>>>>>> origin/tomato-shibby-RT-AC
#include "memdebug.h"

#if (NGHTTP2_VERSION_NUM < 0x000300)
#error too old nghttp2 version, upgrade!
#endif

<<<<<<< HEAD
=======
#if (NGHTTP2_VERSION_NUM > 0x010800)
#define NGHTTP2_HAS_HTTP2_STRERROR 1
#endif

#if (NGHTTP2_VERSION_NUM >= 0x010900)
/* nghttp2_session_callbacks_set_error_callback is present in nghttp2 1.9.0 or
   later */
#define NGHTTP2_HAS_ERROR_CALLBACK 1
#else
#define nghttp2_session_callbacks_set_error_callback(x,y)
#endif

#if (NGHTTP2_VERSION_NUM >= 0x010c00)
#define NGHTTP2_HAS_SET_LOCAL_WINDOW_SIZE 1
#endif

#define HTTP2_HUGE_WINDOW_SIZE (1 << 30)

/*
 * Curl_http2_init_state() is called when the easy handle is created and
 * allows for HTTP/2 specific init of state.
 */
void Curl_http2_init_state(struct UrlState *state)
{
  state->stream_weight = NGHTTP2_DEFAULT_WEIGHT;
}

/*
 * Curl_http2_init_userset() is called when the easy handle is created and
 * allows for HTTP/2 specific user-set fields.
 */
void Curl_http2_init_userset(struct UserDefined *set)
{
  set->stream_weight = NGHTTP2_DEFAULT_WEIGHT;
}

>>>>>>> origin/tomato-shibby-RT-AC
static int http2_perform_getsock(const struct connectdata *conn,
                                 curl_socket_t *sock, /* points to
                                                         numsocks
                                                         number of
                                                         sockets */
                                 int numsocks)
{
  const struct http_conn *httpc = &conn->proto.httpc;
  int bitmap = GETSOCK_BLANK;
  (void)numsocks;

  /* TODO We should check underlying socket state if it is SSL socket
     because of renegotiation. */
  sock[0] = conn->sock[FIRSTSOCKET];

<<<<<<< HEAD
  if(nghttp2_session_want_read(httpc->h2))
    bitmap |= GETSOCK_READSOCK(FIRSTSOCKET);
=======
  /* in a HTTP/2 connection we can basically always get a frame so we should
     always be ready for one */
  bitmap |= GETSOCK_READSOCK(FIRSTSOCKET);
>>>>>>> origin/tomato-shibby-RT-AC

  if(nghttp2_session_want_write(httpc->h2))
    bitmap |= GETSOCK_WRITESOCK(FIRSTSOCKET);

  return bitmap;
}

static int http2_getsock(struct connectdata *conn,
                         curl_socket_t *sock, /* points to numsocks
                                                 number of sockets */
                         int numsocks)
{
  return http2_perform_getsock(conn, sock, numsocks);
}

/*
 * http2_stream_free() free HTTP2 stream related data
 */
static void http2_stream_free(struct HTTP *http)
{
<<<<<<< HEAD
  struct http_conn *httpc = &conn->proto.httpc;
  (void)dead_connection;

  infof(conn->data, "HTTP/2 DISCONNECT starts now\n");

  nghttp2_session_del(httpc->h2);
=======
  if(http) {
    Curl_add_buffer_free(http->header_recvbuf);
    http->header_recvbuf = NULL; /* clear the pointer */
    Curl_add_buffer_free(http->trailer_recvbuf);
    http->trailer_recvbuf = NULL; /* clear the pointer */
    for(; http->push_headers_used > 0; --http->push_headers_used) {
      free(http->push_headers[http->push_headers_used - 1]);
    }
    free(http->push_headers);
    http->push_headers = NULL;
  }
}

static CURLcode http2_disconnect(struct connectdata *conn,
                                 bool dead_connection)
{
  struct http_conn *c = &conn->proto.httpc;
  (void)dead_connection;

  DEBUGF(infof(conn->data, "HTTP/2 DISCONNECT starts now\n"));

  nghttp2_session_del(c->h2);
  Curl_safefree(c->inbuf);
  http2_stream_free(conn->data->req.protop);
>>>>>>> origin/tomato-shibby-RT-AC

  Curl_safefree(httpc->header_recvbuf->buffer);
  Curl_safefree(httpc->header_recvbuf);

  Curl_safefree(httpc->inbuf);

<<<<<<< HEAD
  infof(conn->data, "HTTP/2 DISCONNECT done\n");
=======
/* called from Curl_http_setup_conn */
void Curl_http2_setup_req(struct Curl_easy *data)
{
  struct HTTP *http = data->req.protop;

  http->nread_header_recvbuf = 0;
  http->bodystarted = FALSE;
  http->status_code = -1;
  http->pausedata = NULL;
  http->pauselen = 0;
  http->error_code = NGHTTP2_NO_ERROR;
  http->closed = FALSE;
  http->close_handled = FALSE;
  http->mem = data->state.buffer;
  http->len = BUFSIZE;
  http->memlen = 0;
}
>>>>>>> origin/tomato-shibby-RT-AC

  return CURLE_OK;
}

/*
 * HTTP2 handler interface. This isn't added to the general list of protocols
 * but will be used at run-time when the protocol is dynamically switched from
 * HTTP to HTTP2.
 */
const struct Curl_handler Curl_handler_http2 = {
  "HTTP2",                              /* scheme */
  ZERO_NULL,                            /* setup_connection */
  Curl_http,                            /* do_it */
  ZERO_NULL,                            /* done */
  ZERO_NULL,                            /* do_more */
  ZERO_NULL,                            /* connect_it */
  ZERO_NULL,                            /* connecting */
  ZERO_NULL,                            /* doing */
  http2_getsock,                        /* proto_getsock */
  http2_getsock,                        /* doing_getsock */
  ZERO_NULL,                            /* domore_getsock */
  http2_perform_getsock,                /* perform_getsock */
  http2_disconnect,                     /* disconnect */
  ZERO_NULL,                            /* readwrite */
  PORT_HTTP,                            /* defport */
  CURLPROTO_HTTP,                       /* protocol */
  PROTOPT_STREAM                        /* flags */
};

const struct Curl_handler Curl_handler_http2_ssl = {
  "HTTP2",                              /* scheme */
  ZERO_NULL,                            /* setup_connection */
  Curl_http,                            /* do_it */
  ZERO_NULL,                            /* done */
  ZERO_NULL,                            /* do_more */
  ZERO_NULL,                            /* connect_it */
  ZERO_NULL,                            /* connecting */
  ZERO_NULL,                            /* doing */
  http2_getsock,                        /* proto_getsock */
  http2_getsock,                        /* doing_getsock */
  ZERO_NULL,                            /* domore_getsock */
  http2_perform_getsock,                /* perform_getsock */
  http2_disconnect,                     /* disconnect */
  ZERO_NULL,                            /* readwrite */
  PORT_HTTP,                            /* defport */
  CURLPROTO_HTTPS,                      /* protocol */
  PROTOPT_SSL | PROTOPT_STREAM          /* flags */
};

/*
 * Store nghttp2 version info in this buffer, Prefix with a space.  Return
 * total length written.
 */
int Curl_http2_ver(char *p, size_t len)
{
  nghttp2_info *h2 = nghttp2_version(0);
  return snprintf(p, len, " nghttp2/%s", h2->version_str);
}

<<<<<<< HEAD
=======
/* HTTP/2 error code to name based on the Error Code Registry.
https://tools.ietf.org/html/rfc7540#page-77
nghttp2_error_code enums are identical.
*/
const char *Curl_http2_strerror(uint32_t err)
{
#ifndef NGHTTP2_HAS_HTTP2_STRERROR
  const char *str[] = {
    "NO_ERROR",             /* 0x0 */
    "PROTOCOL_ERROR",       /* 0x1 */
    "INTERNAL_ERROR",       /* 0x2 */
    "FLOW_CONTROL_ERROR",   /* 0x3 */
    "SETTINGS_TIMEOUT",     /* 0x4 */
    "STREAM_CLOSED",        /* 0x5 */
    "FRAME_SIZE_ERROR",     /* 0x6 */
    "REFUSED_STREAM",       /* 0x7 */
    "CANCEL",               /* 0x8 */
    "COMPRESSION_ERROR",    /* 0x9 */
    "CONNECT_ERROR",        /* 0xA */
    "ENHANCE_YOUR_CALM",    /* 0xB */
    "INADEQUATE_SECURITY",  /* 0xC */
    "HTTP_1_1_REQUIRED"     /* 0xD */
  };
  return (err < sizeof str / sizeof str[0]) ? str[err] : "unknown";
#else
  return nghttp2_http2_strerror(err);
#endif
}

>>>>>>> origin/tomato-shibby-RT-AC
/*
 * The implementation of nghttp2_send_callback type. Here we write |data| with
 * size |length| to the network and return the number of bytes actually
 * written. See the documentation of nghttp2_send_callback for the details.
 */
static ssize_t send_callback(nghttp2_session *h2,
                             const uint8_t *data, size_t length, int flags,
                             void *userp)
{
  struct connectdata *conn = (struct connectdata *)userp;
  struct http_conn *httpc = &conn->proto.httpc;
  ssize_t written;
  CURLcode rc;
  (void)h2;
  (void)flags;

  rc = 0;
  written = ((Curl_send*)httpc->send_underlying)(conn, FIRSTSOCKET,
                                                 data, length, &rc);

  if(rc == CURLE_AGAIN) {
    return NGHTTP2_ERR_WOULDBLOCK;
  }

  if(written == -1) {
    failf(conn->data, "Failed sending HTTP2 data");
    return NGHTTP2_ERR_CALLBACK_FAILURE;
  }

  if(!written)
    return NGHTTP2_ERR_WOULDBLOCK;

  return written;
}

<<<<<<< HEAD
=======

/* We pass a pointer to this struct in the push callback, but the contents of
   the struct are hidden from the user. */
struct curl_pushheaders {
  struct Curl_easy *data;
  const nghttp2_push_promise *frame;
};

/*
 * push header access function. Only to be used from within the push callback
 */
char *curl_pushheader_bynum(struct curl_pushheaders *h, size_t num)
{
  /* Verify that we got a good easy handle in the push header struct, mostly to
     detect rubbish input fast(er). */
  if(!h || !GOOD_EASY_HANDLE(h->data))
    return NULL;
  else {
    struct HTTP *stream = h->data->req.protop;
    if(num < stream->push_headers_used)
      return stream->push_headers[num];
  }
  return NULL;
}

/*
 * push header access function. Only to be used from within the push callback
 */
char *curl_pushheader_byname(struct curl_pushheaders *h, const char *header)
{
  /* Verify that we got a good easy handle in the push header struct,
     mostly to detect rubbish input fast(er). Also empty header name
     is just a rubbish too. We have to allow ":" at the beginning of
     the header, but header == ":" must be rejected. If we have ':' in
     the middle of header, it could be matched in middle of the value,
     this is because we do prefix match.*/
  if(!h || !GOOD_EASY_HANDLE(h->data) || !header || !header[0] ||
     !strcmp(header, ":") || strchr(header + 1, ':'))
    return NULL;
  else {
    struct HTTP *stream = h->data->req.protop;
    size_t len = strlen(header);
    size_t i;
    for(i=0; i<stream->push_headers_used; i++) {
      if(!strncmp(header, stream->push_headers[i], len)) {
        /* sub-match, make sure that it is followed by a colon */
        if(stream->push_headers[i][len] != ':')
          continue;
        return &stream->push_headers[i][len+1];
      }
    }
  }
  return NULL;
}

static struct Curl_easy *duphandle(struct Curl_easy *data)
{
  struct Curl_easy *second = curl_easy_duphandle(data);
  if(second) {
    /* setup the request struct */
    struct HTTP *http = calloc(1, sizeof(struct HTTP));
    if(!http) {
      (void)Curl_close(second);
      second = NULL;
    }
    else {
      second->req.protop = http;
      http->header_recvbuf = Curl_add_buffer_init();
      if(!http->header_recvbuf) {
        free(http);
        (void)Curl_close(second);
        second = NULL;
      }
      else {
        Curl_http2_setup_req(second);
        second->state.stream_weight = data->state.stream_weight;
      }
    }
  }
  return second;
}


static int push_promise(struct Curl_easy *data,
                        struct connectdata *conn,
                        const nghttp2_push_promise *frame)
{
  int rv;
  DEBUGF(infof(data, "PUSH_PROMISE received, stream %u!\n",
               frame->promised_stream_id));
  if(data->multi->push_cb) {
    struct HTTP *stream;
    struct HTTP *newstream;
    struct curl_pushheaders heads;
    CURLMcode rc;
    struct http_conn *httpc;
    size_t i;
    /* clone the parent */
    struct Curl_easy *newhandle = duphandle(data);
    if(!newhandle) {
      infof(data, "failed to duplicate handle\n");
      rv = 1; /* FAIL HARD */
      goto fail;
    }

    heads.data = data;
    heads.frame = frame;
    /* ask the application */
    DEBUGF(infof(data, "Got PUSH_PROMISE, ask application!\n"));

    stream = data->req.protop;
    if(!stream) {
      failf(data, "Internal NULL stream!\n");
      rv = 1;
      goto fail;
    }

    rv = data->multi->push_cb(data, newhandle,
                              stream->push_headers_used, &heads,
                              data->multi->push_userp);

    /* free the headers again */
    for(i=0; i<stream->push_headers_used; i++)
      free(stream->push_headers[i]);
    free(stream->push_headers);
    stream->push_headers = NULL;
    stream->push_headers_used = 0;

    if(rv) {
      /* denied, kill off the new handle again */
      http2_stream_free(newhandle->req.protop);
      (void)Curl_close(newhandle);
      goto fail;
    }

    newstream = newhandle->req.protop;
    newstream->stream_id = frame->promised_stream_id;
    newhandle->req.maxdownload = -1;
    newhandle->req.size = -1;

    /* approved, add to the multi handle and immediately switch to PERFORM
       state with the given connection !*/
    rc = Curl_multi_add_perform(data->multi, newhandle, conn);
    if(rc) {
      infof(data, "failed to add handle to multi\n");
      http2_stream_free(newhandle->req.protop);
      Curl_close(newhandle);
      rv = 1;
      goto fail;
    }

    httpc = &conn->proto.httpc;
    nghttp2_session_set_stream_user_data(httpc->h2,
                                         frame->promised_stream_id, newhandle);
  }
  else {
    DEBUGF(infof(data, "Got PUSH_PROMISE, ignore it!\n"));
    rv = 1;
  }
  fail:
  return rv;
}

>>>>>>> origin/tomato-shibby-RT-AC
static int on_frame_recv(nghttp2_session *session, const nghttp2_frame *frame,
                         void *userp)
{
  struct connectdata *conn = (struct connectdata *)userp;
<<<<<<< HEAD
  struct http_conn *c = &conn->proto.httpc;
  int rv;
  (void)session;
  (void)frame;
  infof(conn->data, "on_frame_recv() was called with header %x\n",
        frame->hd.type);
=======
  struct http_conn *httpc = &conn->proto.httpc;
  struct Curl_easy *data_s = NULL;
  struct HTTP *stream = NULL;
  static int lastStream = -1;
  int rv;
  size_t left, ncopy;
  int32_t stream_id = frame->hd.stream_id;

  if(!stream_id) {
    /* stream ID zero is for connection-oriented stuff */
    if(frame->hd.type == NGHTTP2_SETTINGS) {
      uint32_t max_conn = httpc->settings.max_concurrent_streams;
      DEBUGF(infof(conn->data, "Got SETTINGS\n"));
      httpc->settings.max_concurrent_streams =
        nghttp2_session_get_remote_settings(
          session, NGHTTP2_SETTINGS_MAX_CONCURRENT_STREAMS);
      httpc->settings.enable_push =
        nghttp2_session_get_remote_settings(
          session, NGHTTP2_SETTINGS_ENABLE_PUSH);
      DEBUGF(infof(conn->data, "MAX_CONCURRENT_STREAMS == %d\n",
                   httpc->settings.max_concurrent_streams));
      DEBUGF(infof(conn->data, "ENABLE_PUSH == %s\n",
                   httpc->settings.enable_push?"TRUE":"false"));
      if(max_conn != httpc->settings.max_concurrent_streams) {
        /* only signal change if the value actually changed */
        infof(conn->data,
              "Connection state changed (MAX_CONCURRENT_STREAMS updated)!\n");
        Curl_multi_connchanged(conn->data->multi);
      }
    }
    return 0;
  }
  data_s = nghttp2_session_get_stream_user_data(session, stream_id);
  if(lastStream != stream_id) {
    lastStream = stream_id;
  }
  if(!data_s) {
    DEBUGF(infof(conn->data,
                 "No Curl_easy associated with stream: %x\n",
                 stream_id));
    return 0;
  }

  stream = data_s->req.protop;
  if(!stream) {
    DEBUGF(infof(conn->data, "No proto pointer for stream: %x\n",
                 stream_id));
    return NGHTTP2_ERR_CALLBACK_FAILURE;
  }

  DEBUGF(infof(data_s, "on_frame_recv() header %x stream %x\n",
               frame->hd.type, stream_id));

>>>>>>> origin/tomato-shibby-RT-AC
  switch(frame->hd.type) {
  case NGHTTP2_HEADERS:
    if(frame->headers.cat != NGHTTP2_HCAT_RESPONSE)
      break;
    c->bodystarted = TRUE;
    Curl_add_buffer(c->header_recvbuf, "\r\n", 2);
    c->nread_header_recvbuf = c->len < c->header_recvbuf->size_used ?
      c->len : c->header_recvbuf->size_used;

    memcpy(c->mem, c->header_recvbuf->buffer, c->nread_header_recvbuf);

<<<<<<< HEAD
    c->mem += c->nread_header_recvbuf;
    c->len -= c->nread_header_recvbuf;
=======
      /* if we receive data for another handle, wake that up */
      if(conn_s->data != data_s)
        Curl_expire(data_s, 0);
    }
>>>>>>> origin/tomato-shibby-RT-AC
    break;
  case NGHTTP2_PUSH_PROMISE:
    rv = nghttp2_submit_rst_stream(session, NGHTTP2_FLAG_NONE,
                                   frame->hd.stream_id, NGHTTP2_CANCEL);
    if(nghttp2_is_fatal(rv)) {
      return rv;
    }
    break;
  }
  return 0;
}

static int on_invalid_frame_recv(nghttp2_session *session,
                                 const nghttp2_frame *frame,
                                 nghttp2_error_code error_code, void *userp)
{
<<<<<<< HEAD
  struct connectdata *conn = (struct connectdata *)userp;
  (void)session;
  (void)frame;
  infof(conn->data, "on_invalid_frame_recv() was called, error_code = %d\n",
        error_code);
=======
  struct Curl_easy *data_s = NULL;
  (void)userp;

  data_s = nghttp2_session_get_stream_user_data(session, frame->hd.stream_id);
  if(data_s) {
    DEBUGF(infof(data_s,
                 "on_invalid_frame_recv() was called, error=%d:%s\n",
                 lib_error_code, nghttp2_strerror(lib_error_code)));
  }
>>>>>>> origin/tomato-shibby-RT-AC
  return 0;
}

static int on_data_chunk_recv(nghttp2_session *session, uint8_t flags,
                              int32_t stream_id,
                              const uint8_t *data, size_t len, void *userp)
{
<<<<<<< HEAD
=======
  struct HTTP *stream;
  struct Curl_easy *data_s;
  size_t nread;
>>>>>>> origin/tomato-shibby-RT-AC
  struct connectdata *conn = (struct connectdata *)userp;
  struct http_conn *c = &conn->proto.httpc;
  size_t nread;
  (void)session;
  (void)flags;
  (void)data;
  infof(conn->data, "on_data_chunk_recv() "
        "len = %u, stream = %x\n", len, stream_id);

  if(stream_id != c->stream_id) {
    return 0;
  }

  nread = c->len < len ? c->len : len;
  memcpy(c->mem, data, nread);

<<<<<<< HEAD
  c->mem += nread;
  c->len -= nread;
=======
  /* if we receive data for another handle, wake that up */
  if(conn->data != data_s)
    Curl_expire(data_s, 0);
>>>>>>> origin/tomato-shibby-RT-AC

  infof(conn->data, "%zu data written\n", nread);

  if(nread < len) {
    c->data = data + nread;
    c->datalen = len - nread;
    return NGHTTP2_ERR_PAUSE;
  }
  return 0;
}

static int before_frame_send(nghttp2_session *session,
                             const nghttp2_frame *frame,
                             void *userp)
{
<<<<<<< HEAD
  struct connectdata *conn = (struct connectdata *)userp;
  (void)session;
  (void)frame;
  infof(conn->data, "before_frame_send() was called\n");
=======
  struct Curl_easy *data_s;
  (void)userp;

  data_s = nghttp2_session_get_stream_user_data(session, frame->hd.stream_id);
  if(data_s) {
    DEBUGF(infof(data_s, "before_frame_send() was called\n"));
  }

>>>>>>> origin/tomato-shibby-RT-AC
  return 0;
}
static int on_frame_send(nghttp2_session *session,
                         const nghttp2_frame *frame,
                         void *userp)
{
<<<<<<< HEAD
  struct connectdata *conn = (struct connectdata *)userp;
  (void)session;
  (void)frame;
  infof(conn->data, "on_frame_send() was called\n");
=======
  struct Curl_easy *data_s;
  (void)userp;

  data_s = nghttp2_session_get_stream_user_data(session, frame->hd.stream_id);
  if(data_s) {
    DEBUGF(infof(data_s, "on_frame_send() was called, length = %zd\n",
                 frame->hd.length));
  }
>>>>>>> origin/tomato-shibby-RT-AC
  return 0;
}
static int on_frame_not_send(nghttp2_session *session,
                             const nghttp2_frame *frame,
                             int lib_error_code, void *userp)
{
<<<<<<< HEAD
  struct connectdata *conn = (struct connectdata *)userp;
  (void)session;
  (void)frame;
  infof(conn->data, "on_frame_not_send() was called, lib_error_code = %d\n",
        lib_error_code);
=======
  struct Curl_easy *data_s;
  (void)userp;

  data_s = nghttp2_session_get_stream_user_data(session, frame->hd.stream_id);
  if(data_s) {
    DEBUGF(infof(data_s,
                 "on_frame_not_send() was called, lib_error_code = %d\n",
                 lib_error_code));
  }
>>>>>>> origin/tomato-shibby-RT-AC
  return 0;
}
static int on_stream_close(nghttp2_session *session, int32_t stream_id,
                           nghttp2_error_code error_code, void *userp)
{
<<<<<<< HEAD
=======
  struct Curl_easy *data_s;
  struct HTTP *stream;
>>>>>>> origin/tomato-shibby-RT-AC
  struct connectdata *conn = (struct connectdata *)userp;
  struct http_conn *c = &conn->proto.httpc;
  (void)session;
  (void)stream_id;
  infof(conn->data, "on_stream_close() was called, error_code = %d\n",
        error_code);

  if(stream_id != c->stream_id) {
    return 0;
  }

  c->closed = TRUE;

  return 0;
}

static int on_unknown_frame_recv(nghttp2_session *session,
                                 const uint8_t *head, size_t headlen,
                                 const uint8_t *payload, size_t payloadlen,
                                 void *userp)
{
  struct connectdata *conn = (struct connectdata *)userp;
  (void)session;
  (void)head;
  (void)headlen;
  (void)payload;
  (void)payloadlen;
  infof(conn->data, "on_unknown_frame_recv() was called\n");
  return 0;
}
static int on_begin_headers(nghttp2_session *session,
                            const nghttp2_frame *frame, void *userp)
{
<<<<<<< HEAD
  struct connectdata *conn = (struct connectdata *)userp;
  (void)session;
  (void)frame;
  infof(conn->data, "on_begin_headers() was called\n");
=======
  struct HTTP *stream;
  struct Curl_easy *data_s = NULL;
  (void)userp;

  data_s = nghttp2_session_get_stream_user_data(session, frame->hd.stream_id);
  if(!data_s) {
    return 0;
  }

  DEBUGF(infof(data_s, "on_begin_headers() was called\n"));

  if(frame->hd.type != NGHTTP2_HEADERS) {
    return 0;
  }

  stream = data_s->req.protop;
  if(!stream || !stream->bodystarted) {
    return 0;
  }

  /* This is trailer HEADERS started.  Allocate buffer for them. */
  DEBUGF(infof(data_s, "trailer field started\n"));

  assert(stream->trailer_recvbuf == NULL);

  stream->trailer_recvbuf = Curl_add_buffer_init();
  if(!stream->trailer_recvbuf) {
    return NGHTTP2_ERR_TEMPORAL_CALLBACK_FAILURE;
  }

>>>>>>> origin/tomato-shibby-RT-AC
  return 0;
}

static const char STATUS[] = ":status";

/* frame->hd.type is either NGHTTP2_HEADERS or NGHTTP2_PUSH_PROMISE */
static int on_header(nghttp2_session *session, const nghttp2_frame *frame,
                     const uint8_t *name, size_t namelen,
                     const uint8_t *value, size_t valuelen,
                     uint8_t flags,
                     void *userp)
{
<<<<<<< HEAD
=======
  struct HTTP *stream;
  struct Curl_easy *data_s;
  int32_t stream_id = frame->hd.stream_id;
>>>>>>> origin/tomato-shibby-RT-AC
  struct connectdata *conn = (struct connectdata *)userp;
  struct http_conn *c = &conn->proto.httpc;
  (void)session;
  (void)frame;
  (void)flags;

<<<<<<< HEAD
  if(frame->hd.stream_id != c->stream_id) {
=======
  DEBUGASSERT(stream_id); /* should never be a zero stream ID here */

  /* get the stream from the hash based on Stream ID */
  data_s = nghttp2_session_get_stream_user_data(session, stream_id);
  if(!data_s)
    /* Receiving a Stream ID not in the hash should not happen, this is an
       internal error more than anything else! */
    return NGHTTP2_ERR_CALLBACK_FAILURE;

  stream = data_s->req.protop;
  if(!stream) {
    failf(data_s, "Internal NULL stream! 5\n");
    return NGHTTP2_ERR_CALLBACK_FAILURE;
  }

  /* Store received PUSH_PROMISE headers to be used when the subsequent
     PUSH_PROMISE callback comes */
  if(frame->hd.type == NGHTTP2_PUSH_PROMISE) {
    char *h;

    if(!stream->push_headers) {
      stream->push_headers_alloc = 10;
      stream->push_headers = malloc(stream->push_headers_alloc *
                                    sizeof(char *));
      stream->push_headers_used = 0;
    }
    else if(stream->push_headers_used ==
            stream->push_headers_alloc) {
      char **headp;
      stream->push_headers_alloc *= 2;
      headp = Curl_saferealloc(stream->push_headers,
                               stream->push_headers_alloc * sizeof(char *));
      if(!headp) {
        stream->push_headers = NULL;
        return NGHTTP2_ERR_TEMPORAL_CALLBACK_FAILURE;
      }
      stream->push_headers = headp;
    }
    h = aprintf("%s:%s", name, value);
    if(h)
      stream->push_headers[stream->push_headers_used++] = h;
    return 0;
  }

  if(stream->bodystarted) {
    /* This is trailer fields. */
    /* 3 is for ":" and "\r\n". */
    uint32_t n = (uint32_t)(namelen + valuelen + 3);

    DEBUGF(infof(data_s, "h2 trailer: %.*s: %.*s\n", namelen, name, valuelen,
                 value));

    Curl_add_buffer(stream->trailer_recvbuf, &n, sizeof(n));
    Curl_add_buffer(stream->trailer_recvbuf, name, namelen);
    Curl_add_buffer(stream->trailer_recvbuf, ": ", 2);
    Curl_add_buffer(stream->trailer_recvbuf, value, valuelen);
    Curl_add_buffer(stream->trailer_recvbuf, "\r\n\0", 3);

>>>>>>> origin/tomato-shibby-RT-AC
    return 0;
  }

  if(namelen == sizeof(":status") - 1 &&
<<<<<<< HEAD
     memcmp(STATUS, name, namelen) == 0) {
    snprintf(c->header_recvbuf->buffer, 13, "HTTP/2.0 %s", value);
    c->header_recvbuf->buffer[12] = '\r';
    return 0;
  }
  else {
    /* convert to a HTTP1-style header */
    infof(conn->data, "got header\n");
    Curl_add_buffer(c->header_recvbuf, name, namelen);
    Curl_add_buffer(c->header_recvbuf, ":", 1);
    Curl_add_buffer(c->header_recvbuf, value, valuelen);
    Curl_add_buffer(c->header_recvbuf, "\r\n", 2);
  }
=======
     memcmp(":status", name, namelen) == 0) {
    /* nghttp2 guarantees :status is received first and only once, and
       value is 3 digits status code, and decode_status_code always
       succeeds. */
    stream->status_code = decode_status_code(value, valuelen);
    DEBUGASSERT(stream->status_code != -1);

    Curl_add_buffer(stream->header_recvbuf, "HTTP/2 ", 7);
    Curl_add_buffer(stream->header_recvbuf, value, valuelen);
    /* the space character after the status code is mandatory */
    Curl_add_buffer(stream->header_recvbuf, " \r\n", 3);
    /* if we receive data for another handle, wake that up */
    if(conn->data != data_s)
      Curl_expire(data_s, 0);

    DEBUGF(infof(data_s, "h2 status: HTTP/2 %03d (easy %p)\n",
                 stream->status_code, data_s));
    return 0;
  }

  /* nghttp2 guarantees that namelen > 0, and :status was already
     received, and this is not pseudo-header field . */
  /* convert to a HTTP1-style header */
  Curl_add_buffer(stream->header_recvbuf, name, namelen);
  Curl_add_buffer(stream->header_recvbuf, ": ", 2);
  Curl_add_buffer(stream->header_recvbuf, value, valuelen);
  Curl_add_buffer(stream->header_recvbuf, "\r\n", 2);
  /* if we receive data for another handle, wake that up */
  if(conn->data != data_s)
    Curl_expire(data_s, 0);

  DEBUGF(infof(data_s, "h2 header: %.*s: %.*s\n", namelen, name, valuelen,
               value));
>>>>>>> origin/tomato-shibby-RT-AC

  return 0; /* 0 is successful */
}

/*
 * This is all callbacks nghttp2 calls
 */
static const nghttp2_session_callbacks callbacks = {
  send_callback,         /* nghttp2_send_callback */
  NULL,                  /* nghttp2_recv_callback */
  on_frame_recv,         /* nghttp2_on_frame_recv_callback */
  on_invalid_frame_recv, /* nghttp2_on_invalid_frame_recv_callback */
  on_data_chunk_recv,    /* nghttp2_on_data_chunk_recv_callback */
  before_frame_send,     /* nghttp2_before_frame_send_callback */
  on_frame_send,         /* nghttp2_on_frame_send_callback */
  on_frame_not_send,     /* nghttp2_on_frame_not_send_callback */
  on_stream_close,       /* nghttp2_on_stream_close_callback */
  on_unknown_frame_recv, /* nghttp2_on_unknown_frame_recv_callback */
  on_begin_headers,      /* nghttp2_on_begin_headers_callback */
  on_header              /* nghttp2_on_header_callback */
#if NGHTTP2_VERSION_NUM >= 0x000400
  , NULL                 /* nghttp2_select_padding_callback */
#endif
};

static ssize_t data_source_read_callback(nghttp2_session *session,
                                         int32_t stream_id,
                                         uint8_t *buf, size_t length,
                                         uint32_t *data_flags,
                                         nghttp2_data_source *source,
                                         void *userp)
{
<<<<<<< HEAD
  struct connectdata *conn = (struct connectdata *)userp;
  struct http_conn *c = &conn->proto.httpc;
=======
  struct Curl_easy *data_s;
  struct HTTP *stream = NULL;
>>>>>>> origin/tomato-shibby-RT-AC
  size_t nread;
  (void)session;
  (void)stream_id;
  (void)source;

  nread = c->upload_len < length ? c->upload_len : length;
  if(nread > 0) {
<<<<<<< HEAD
    memcpy(buf, c->upload_mem, nread);
    c->upload_mem += nread;
    c->upload_len -= nread;
    c->upload_left -= nread;
  }

  if(c->upload_left == 0)
    *data_flags = 1;
=======
    memcpy(buf, stream->upload_mem, nread);
    stream->upload_mem += nread;
    stream->upload_len -= nread;
    if(data_s->state.infilesize != -1)
      stream->upload_left -= nread;
  }

  if(stream->upload_left == 0)
    *data_flags = NGHTTP2_DATA_FLAG_EOF;
>>>>>>> origin/tomato-shibby-RT-AC
  else if(nread == 0)
    return NGHTTP2_ERR_DEFERRED;

  return nread;
}

<<<<<<< HEAD
/*
 * The HTTP2 settings we send in the Upgrade request
 */
static nghttp2_settings_entry settings[] = {
  { NGHTTP2_SETTINGS_MAX_CONCURRENT_STREAMS, 100 },
  { NGHTTP2_SETTINGS_INITIAL_WINDOW_SIZE, NGHTTP2_INITIAL_WINDOW_SIZE },
};

#define H2_BUFSIZE 4096
=======
#define H2_BUFSIZE 32768

#ifdef NGHTTP2_HAS_ERROR_CALLBACK
static int error_callback(nghttp2_session *session,
                          const char *msg,
                          size_t len,
                          void *userp)
{
  struct connectdata *conn = (struct connectdata *)userp;
  (void)session;
  infof(conn->data, "http2 error: %.*s\n", len, msg);
  return 0;
}
#endif
>>>>>>> origin/tomato-shibby-RT-AC

static void populate_settings(struct connectdata *conn,
                              struct http_conn *httpc)
{
  nghttp2_settings_entry *iv = httpc->local_settings;

  iv[0].settings_id = NGHTTP2_SETTINGS_MAX_CONCURRENT_STREAMS;
  iv[0].value = 100;

  iv[1].settings_id = NGHTTP2_SETTINGS_INITIAL_WINDOW_SIZE;
  iv[1].value = HTTP2_HUGE_WINDOW_SIZE;

  iv[2].settings_id = NGHTTP2_SETTINGS_ENABLE_PUSH;
  iv[2].value = conn->data->multi->push_cb != NULL;

  httpc->local_settings_num = 3;
}

void Curl_http2_done(struct connectdata *conn, bool premature)
{
  struct Curl_easy *data = conn->data;
  struct HTTP *http = data->req.protop;
  struct http_conn *httpc = &conn->proto.httpc;

  if(http->header_recvbuf) {
    DEBUGF(infof(data, "free header_recvbuf!!\n"));
    Curl_add_buffer_free(http->header_recvbuf);
    http->header_recvbuf = NULL; /* clear the pointer */
    Curl_add_buffer_free(http->trailer_recvbuf);
    http->trailer_recvbuf = NULL; /* clear the pointer */
    if(http->push_headers) {
      /* if they weren't used and then freed before */
      for(; http->push_headers_used > 0; --http->push_headers_used) {
        free(http->push_headers[http->push_headers_used - 1]);
      }
      free(http->push_headers);
      http->push_headers = NULL;
    }
  }

  if(premature) {
    /* RST_STREAM */
    nghttp2_submit_rst_stream(httpc->h2, NGHTTP2_FLAG_NONE, http->stream_id,
                              NGHTTP2_STREAM_CLOSED);
    if(http->stream_id == httpc->pause_stream_id) {
      infof(data, "stopped the pause stream!\n");
      httpc->pause_stream_id = 0;
    }
  }
  if(http->stream_id) {
    nghttp2_session_set_stream_user_data(httpc->h2, http->stream_id, 0);
    http->stream_id = 0;
  }
}

/*
 * Initialize nghttp2 for a Curl connection
 */
CURLcode Curl_http2_init(struct connectdata *conn)
{
  if(!conn->proto.httpc.h2) {
    int rc;
    conn->proto.httpc.inbuf = malloc(H2_BUFSIZE);
    if(conn->proto.httpc.inbuf == NULL)
      return CURLE_OUT_OF_MEMORY;

    /* The nghttp2 session is not yet setup, do it */
    rc = nghttp2_session_client_new(&conn->proto.httpc.h2,
                                    &callbacks, conn);
    if(rc) {
      failf(conn->data, "Couldn't initialize nghttp2!");
      return CURLE_OUT_OF_MEMORY; /* most likely at least */
    }
  }
  return CURLE_OK;
}

/*
 * Send a request using http2
 */
CURLcode Curl_http2_send_request(struct connectdata *conn)
{
  (void)conn;
  return CURLE_OK;
}

/*
 * Append headers to ask for a HTTP1.1 to HTTP2 upgrade.
 */
CURLcode Curl_http2_request_upgrade(Curl_send_buffer *req,
                                    struct connectdata *conn)
{
  CURLcode result;
  ssize_t binlen;
  char *base64;
  size_t blen;
  struct SingleRequest *k = &conn->data->req;
  uint8_t *binsettings = conn->proto.httpc.binsettings;
  struct http_conn *httpc = &conn->proto.httpc;

<<<<<<< HEAD
  Curl_http2_init(conn);

  /* As long as we have a fixed set of settings, we don't have to dynamically
   * figure out the base64 strings since it'll always be the same. However,
   * the settings will likely not be fixed every time in the future.
   */
=======
  populate_settings(conn, httpc);
>>>>>>> origin/tomato-shibby-RT-AC

  /* this returns number of bytes it wrote */
  binlen = nghttp2_pack_settings_payload(binsettings, H2_BINSETTINGS_LEN,
                                         httpc->local_settings,
                                         httpc->local_settings_num);
  if(!binlen) {
    failf(conn->data, "nghttp2 unexpectedly failed on pack_settings_payload");
    return CURLE_FAILED_INIT;
  }
  conn->proto.httpc.binlen = binlen;

  result = Curl_base64_encode(conn->data, (const char *)binsettings, binlen,
                              &base64, &blen);
  if(result)
    return result;

  result = Curl_add_bufferf(req,
                            "Connection: Upgrade, HTTP2-Settings\r\n"
                            "Upgrade: %s\r\n"
                            "HTTP2-Settings: %s\r\n",
                            NGHTTP2_CLEARTEXT_PROTO_VERSION_ID, base64);
  Curl_safefree(base64);

  k->upgr101 = UPGR101_REQUESTED;

  return result;
}

/*
<<<<<<< HEAD
 * If the read would block (EWOULDBLOCK) we return -1. Otherwise we return
 * a regular CURLcode value.
 */
=======
 * Returns nonzero if current HTTP/2 session should be closed.
 */
static int should_close_session(struct http_conn *httpc)
{
  return httpc->drain_total == 0 && !nghttp2_session_want_read(httpc->h2) &&
    !nghttp2_session_want_write(httpc->h2);
}

static int h2_session_send(struct Curl_easy *data,
                           nghttp2_session *h2);

/*
 * h2_process_pending_input() processes pending input left in
 * httpc->inbuf.  Then, call h2_session_send() to send pending data.
 * This function returns 0 if it succeeds, or -1 and error code will
 * be assigned to *err.
 */
static int h2_process_pending_input(struct Curl_easy *data,
                                    struct http_conn *httpc,
                                    CURLcode *err)
{
  ssize_t nread;
  char *inbuf;
  ssize_t rv;

  nread = httpc->inbuflen - httpc->nread_inbuf;
  inbuf = httpc->inbuf + httpc->nread_inbuf;

  rv = nghttp2_session_mem_recv(httpc->h2, (const uint8_t *)inbuf, nread);
  if(rv < 0) {
    failf(data,
          "h2_process_pending_input: nghttp2_session_mem_recv() returned "
          "%d:%s\n", rv, nghttp2_strerror((int)rv));
    *err = CURLE_RECV_ERROR;
    return -1;
  }

  if(nread == rv) {
    DEBUGF(infof(data,
                 "h2_process_pending_input: All data in connection buffer "
                 "processed\n"));
    httpc->inbuflen = 0;
    httpc->nread_inbuf = 0;
  }
  else {
    httpc->nread_inbuf += rv;
    DEBUGF(infof(data,
                 "h2_process_pending_input: %zu bytes left in connection "
                 "buffer\n",
                 httpc->inbuflen - httpc->nread_inbuf));
  }

  rv = h2_session_send(data, httpc->h2);
  if(rv != 0) {
    *err = CURLE_SEND_ERROR;
    return -1;
  }

  if(should_close_session(httpc)) {
    DEBUGF(infof(data,
                 "h2_process_pending_input: nothing to do in this session\n"));
    *err = CURLE_HTTP2;
    return -1;
  }

  return 0;
}

/*
 * Called from transfer.c:done_sending when we stop uploading.
 */
CURLcode Curl_http2_done_sending(struct connectdata *conn)
{
  CURLcode result = CURLE_OK;

  if((conn->handler == &Curl_handler_http2_ssl) ||
     (conn->handler == &Curl_handler_http2)) {
    /* make sure this is only attempted for HTTP/2 transfers */

    struct HTTP *stream = conn->data->req.protop;

    if(stream->upload_left) {
      /* If the stream still thinks there's data left to upload. */
      struct http_conn *httpc = &conn->proto.httpc;
      nghttp2_session *h2 = httpc->h2;

      stream->upload_left = 0; /* DONE! */

      /* resume sending here to trigger the callback to get called again so
         that it can signal EOF to nghttp2 */
      (void)nghttp2_session_resume_data(h2, stream->stream_id);

      (void)h2_process_pending_input(conn->data, httpc, &result);
    }
  }
  return result;
}


static ssize_t http2_handle_stream_close(struct connectdata *conn,
                                         struct Curl_easy *data,
                                         struct HTTP *stream, CURLcode *err)
{
  char *trailer_pos, *trailer_end;
  CURLcode result;
  struct http_conn *httpc = &conn->proto.httpc;

  if(httpc->pause_stream_id == stream->stream_id) {
    httpc->pause_stream_id = 0;
  }

  DEBUGASSERT(httpc->drain_total >= data->state.drain);
  httpc->drain_total -= data->state.drain;
  data->state.drain = 0;

  if(httpc->pause_stream_id == 0) {
    if(h2_process_pending_input(data, httpc, err) != 0) {
      return -1;
    }
  }

  DEBUGASSERT(data->state.drain == 0);

  /* Reset to FALSE to prevent infinite loop in readwrite_data function. */
  stream->closed = FALSE;
  if(stream->error_code != NGHTTP2_NO_ERROR) {
    failf(data, "HTTP/2 stream %u was not closed cleanly: %s (err %d)",
          stream->stream_id, Curl_http2_strerror(stream->error_code),
          stream->error_code);
    *err = CURLE_HTTP2_STREAM;
    return -1;
  }

  if(!stream->bodystarted) {
    failf(data, "HTTP/2 stream %u was closed cleanly, but before getting "
          " all response header fields, teated as error",
          stream->stream_id);
    *err = CURLE_HTTP2_STREAM;
    return -1;
  }

  if(stream->trailer_recvbuf && stream->trailer_recvbuf->buffer) {
    trailer_pos = stream->trailer_recvbuf->buffer;
    trailer_end = trailer_pos + stream->trailer_recvbuf->size_used;

    for(; trailer_pos < trailer_end;) {
      uint32_t n;
      memcpy(&n, trailer_pos, sizeof(n));
      trailer_pos += sizeof(n);

      result = Curl_client_write(conn, CLIENTWRITE_HEADER, trailer_pos, n);
      if(result) {
        *err = result;
        return -1;
      }

      trailer_pos += n + 1;
    }
  }

  stream->close_handled = TRUE;

  DEBUGF(infof(data, "http2_recv returns 0, http2_handle_stream_close\n"));
  return 0;
}

/*
 * h2_pri_spec() fills in the pri_spec struct, used by nghttp2 to send weight
 * and dependency to the peer. It also stores the updated values in the state
 * struct.
 */

static void h2_pri_spec(struct Curl_easy *data,
                        nghttp2_priority_spec *pri_spec)
{
  struct HTTP *depstream = (data->set.stream_depends_on?
                            data->set.stream_depends_on->req.protop:NULL);
  int32_t depstream_id = depstream? depstream->stream_id:0;
  nghttp2_priority_spec_init(pri_spec, depstream_id, data->set.stream_weight,
                             data->set.stream_depends_e);
  data->state.stream_weight = data->set.stream_weight;
  data->state.stream_depends_e = data->set.stream_depends_e;
  data->state.stream_depends_on = data->set.stream_depends_on;
}

/*
 * h2_session_send() checks if there's been an update in the priority /
 * dependency settings and if so it submits a PRIORITY frame with the updated
 * info.
 */
static int h2_session_send(struct Curl_easy *data,
                           nghttp2_session *h2)
{
  struct HTTP *stream = data->req.protop;
  if((data->set.stream_weight != data->state.stream_weight) ||
     (data->set.stream_depends_e != data->state.stream_depends_e) ||
     (data->set.stream_depends_on != data->state.stream_depends_on) ) {
    /* send new weight and/or dependency */
    nghttp2_priority_spec pri_spec;
    int rv;

    h2_pri_spec(data, &pri_spec);

    DEBUGF(infof(data, "Queuing PRIORITY on stream %u (easy %p)\n",
                 stream->stream_id, data));
    rv = nghttp2_submit_priority(h2, NGHTTP2_FLAG_NONE, stream->stream_id,
                                 &pri_spec);
    if(rv)
      return rv;
  }

  return nghttp2_session_send(h2);
}

>>>>>>> origin/tomato-shibby-RT-AC
static ssize_t http2_recv(struct connectdata *conn, int sockindex,
                          char *mem, size_t len, CURLcode *err)
{
  CURLcode rc;
  ssize_t rv;
  ssize_t nread;
  struct http_conn *httpc = &conn->proto.httpc;
<<<<<<< HEAD
=======
  struct Curl_easy *data = conn->data;
  struct HTTP *stream = data->req.protop;
>>>>>>> origin/tomato-shibby-RT-AC

  (void)sockindex; /* we always do HTTP2 on sockindex 0 */

  if(httpc->closed) {
    return 0;
  }

  /* Nullify here because we call nghttp2_session_send() and they
     might refer to the old buffer. */
<<<<<<< HEAD
  httpc->upload_mem = NULL;
  httpc->upload_len = 0;
=======
  stream->upload_mem = NULL;
  stream->upload_len = 0;

  /*
   * At this point 'stream' is just in the Curl_easy the connection
   * identifies as its owner at this time.
   */
>>>>>>> origin/tomato-shibby-RT-AC

  if(httpc->bodystarted &&
     httpc->nread_header_recvbuf < httpc->header_recvbuf->size_used) {
    size_t left =
      httpc->header_recvbuf->size_used - httpc->nread_header_recvbuf;
    size_t ncopy = len < left ? len : left;
    memcpy(mem, httpc->header_recvbuf->buffer + httpc->nread_header_recvbuf,
           ncopy);
    httpc->nread_header_recvbuf += ncopy;
    return ncopy;
  }

  if(httpc->data) {
    nread = len < httpc->datalen ? len : httpc->datalen;
    memcpy(mem, httpc->data, nread);

    httpc->data += nread;
    httpc->datalen -= nread;

    infof(conn->data, "%zu data written\n", nread);
    if(httpc->datalen == 0) {
      httpc->data = NULL;
      httpc->datalen = 0;
    }
    return nread;
  }
<<<<<<< HEAD
=======
  else if(httpc->pause_stream_id) {
    /* If a stream paused nghttp2_session_mem_recv previously, and has
       not processed all data, it still refers to the buffer in
       nghttp2_session.  If we call nghttp2_session_mem_recv(), we may
       overwrite that buffer.  To avoid that situation, just return
       here with CURLE_AGAIN.  This could be busy loop since data in
       socket is not read.  But it seems that usually streams are
       notified with its drain property, and socket is read again
       quickly. */
    DEBUGF(infof(data, "stream %x is paused, pause id: %x\n",
                 stream->stream_id, httpc->pause_stream_id));
    *err = CURLE_AGAIN;
    return -1;
  }
  else {
    char *inbuf;
    /* remember where to store incoming data for this stream and how big the
       buffer is */
    stream->mem = mem;
    stream->len = len;
    stream->memlen = 0;

    if(httpc->inbuflen == 0) {
      nread = ((Curl_recv *)httpc->recv_underlying)(
          conn, FIRSTSOCKET, httpc->inbuf, H2_BUFSIZE, &result);

      if(nread == -1) {
        if(result != CURLE_AGAIN)
          failf(data, "Failed receiving HTTP2 data");
        else if(stream->closed)
          /* received when the stream was already closed! */
          return http2_handle_stream_close(conn, data, stream, err);

        *err = result;
        return -1;
      }

      if(nread == 0) {
        failf(data, "Unexpected EOF");
        *err = CURLE_RECV_ERROR;
        return -1;
      }
>>>>>>> origin/tomato-shibby-RT-AC

  conn->proto.httpc.mem = mem;
  conn->proto.httpc.len = len;

  infof(conn->data, "http2_recv: %d bytes buffer\n",
        conn->proto.httpc.len);

  rc = 0;
  nread = ((Curl_recv*)httpc->recv_underlying)(conn, FIRSTSOCKET,
                                               httpc->inbuf, H2_BUFSIZE, &rc);

  if(rc == CURLE_AGAIN) {
    *err = rc;
    return -1;
  }

  if(nread == -1) {
    failf(conn->data, "Failed receiving HTTP2 data");
    *err = rc;
    return 0;
  }

  infof(conn->data, "nread=%zd\n", nread);
  rv = nghttp2_session_mem_recv(httpc->h2,
                                (const uint8_t *)httpc->inbuf, nread);

  if(nghttp2_is_fatal((int)rv)) {
    failf(conn->data, "nghttp2_session_mem_recv() returned %d:%s\n",
          rv, nghttp2_strerror((int)rv));
    *err = CURLE_RECV_ERROR;
    return 0;
  }
  infof(conn->data, "nghttp2_session_mem_recv() returns %zd\n", rv);
  /* Always send pending frames in nghttp2 session, because
     nghttp2_session_mem_recv() may queue new frame */
  rv = nghttp2_session_send(httpc->h2);
  if(rv != 0) {
    *err = CURLE_SEND_ERROR;
    return 0;
  }
  if(len != httpc->len) {
    return len - conn->proto.httpc.len;
  }
  /* If stream is closed, return 0 to signal the http routine to close
     the connection */
  if(httpc->closed) {
    return 0;
  }
  *err = CURLE_AGAIN;
  return -1;
}

<<<<<<< HEAD
/* return number of received (decrypted) bytes */
=======
/* Index where :authority header field will appear in request header
   field list. */
#define AUTHORITY_DST_IDX 3

#define HEADER_OVERFLOW(x) \
  (x.namelen > (uint16_t)-1 || x.valuelen > (uint16_t)-1 - x.namelen)

/*
 * Check header memory for the token "trailers".
 * Parse the tokens as separated by comma and surrounded by whitespace.
 * Returns TRUE if found or FALSE if not.
 */
static bool contains_trailers(const char *p, size_t len)
{
  const char *end = p + len;
  for(;;) {
    for(; p != end && (*p == ' ' || *p == '\t'); ++p)
      ;
    if(p == end || (size_t)(end - p) < sizeof("trailers") - 1)
      return FALSE;
    if(strncasecompare("trailers", p, sizeof("trailers") - 1)) {
      p += sizeof("trailers") - 1;
      for(; p != end && (*p == ' ' || *p == '\t'); ++p)
        ;
      if(p == end || *p == ',')
        return TRUE;
    }
    /* skip to next token */
    for(; p != end && *p != ','; ++p)
      ;
    if(p == end)
      return FALSE;
    ++p;
  }
}

typedef enum {
  /* Send header to server */
  HEADERINST_FORWARD,
  /* Don't send header to server */
  HEADERINST_IGNORE,
  /* Discard header, and replace it with "te: trailers" */
  HEADERINST_TE_TRAILERS
} header_instruction;

/* Decides how to treat given header field. */
static header_instruction inspect_header(const char *name, size_t namelen,
                                         const char *value, size_t valuelen) {
  switch(namelen) {
  case 2:
    if(!strncasecompare("te", name, namelen))
      return HEADERINST_FORWARD;

    return contains_trailers(value, valuelen) ?
           HEADERINST_TE_TRAILERS : HEADERINST_IGNORE;
  case 7:
    return strncasecompare("upgrade", name, namelen) ?
           HEADERINST_IGNORE : HEADERINST_FORWARD;
  case 10:
    return (strncasecompare("connection", name, namelen) ||
            strncasecompare("keep-alive", name, namelen)) ?
           HEADERINST_IGNORE : HEADERINST_FORWARD;
  case 16:
    return strncasecompare("proxy-connection", name, namelen) ?
           HEADERINST_IGNORE : HEADERINST_FORWARD;
  case 17:
    return strncasecompare("transfer-encoding", name, namelen) ?
           HEADERINST_IGNORE : HEADERINST_FORWARD;
  default:
    return HEADERINST_FORWARD;
  }
}

>>>>>>> origin/tomato-shibby-RT-AC
static ssize_t http2_send(struct connectdata *conn, int sockindex,
                          const void *mem, size_t len, CURLcode *err)
{
  /*
   * BIG TODO: Currently, we send request in this function, but this
   * function is also used to send request body. It would be nice to
   * add dedicated function for request.
   */
  int rv;
  struct http_conn *httpc = &conn->proto.httpc;
  nghttp2_nv *nva;
  size_t nheader;
  size_t i;
<<<<<<< HEAD
  char *hdbuf = (char*)mem;
  char *end;
=======
  size_t authority_idx;
  char *hdbuf = (char *)mem;
  char *end, *line_end;
>>>>>>> origin/tomato-shibby-RT-AC
  nghttp2_data_provider data_prd;
  int32_t stream_id;

  (void)sockindex;

  infof(conn->data, "http2_send len=%zu\n", len);

<<<<<<< HEAD
  if(httpc->stream_id != -1) {
=======
  if(stream->stream_id != -1) {
    if(stream->close_handled) {
      infof(conn->data, "stream %d closed\n", stream->stream_id);
      *err = CURLE_HTTP2_STREAM;
      return -1;
    }
    else if(stream->closed) {
      return http2_handle_stream_close(conn, conn->data, stream, err);
    }
>>>>>>> origin/tomato-shibby-RT-AC
    /* If stream_id != -1, we have dispatched request HEADERS, and now
       are going to send or sending request body in DATA frame */
    httpc->upload_mem = mem;
    httpc->upload_len = len;
    nghttp2_session_resume_data(httpc->h2, httpc->stream_id);
    rv = nghttp2_session_send(httpc->h2);
    if(nghttp2_is_fatal(rv)) {
      *err = CURLE_SEND_ERROR;
      return -1;
    }
    return len - httpc->upload_len;
  }

  /* Calculate number of headers contained in [mem, mem + len) */
  /* Here, we assume the curl http code generate *correct* HTTP header
     field block */
  nheader = 0;
  for(i = 0; i < len; ++i) {
    if(hdbuf[i] == 0x0a) {
      ++nheader;
    }
  }
  /* We counted additional 2 \n in the first and last line. We need 3
     new headers: :method, :path and :scheme. Therefore we need one
     more space. */
  nheader += 1;
  nva = malloc(sizeof(nghttp2_nv) * nheader);
  if(nva == NULL) {
    *err = CURLE_OUT_OF_MEMORY;
    return -1;
  }
  /* Extract :method, :path from request line */
  end = strchr(hdbuf, ' ');
  nva[0].name = (unsigned char *)":method";
  nva[0].namelen = (uint16_t)strlen((char *)nva[0].name);
  nva[0].value = (unsigned char *)hdbuf;
  nva[0].valuelen = (uint16_t)(end - hdbuf);
  nva[0].flags = NGHTTP2_NV_FLAG_NONE;

  hdbuf = end + 1;

  end = strchr(hdbuf, ' ');
  nva[1].name = (unsigned char *)":path";
  nva[1].namelen = (uint16_t)strlen((char *)nva[1].name);
  nva[1].value = (unsigned char *)hdbuf;
  nva[1].valuelen = (uint16_t)(end - hdbuf);
  nva[1].flags = NGHTTP2_NV_FLAG_NONE;

  nva[2].name = (unsigned char *)":scheme";
  nva[2].namelen = (uint16_t)strlen((char *)nva[2].name);
  if(conn->handler->flags & PROTOPT_SSL)
    nva[2].value = (unsigned char *)"https";
  else
    nva[2].value = (unsigned char *)"http";
  nva[2].valuelen = (uint16_t)strlen((char *)nva[2].value);
  nva[2].flags = NGHTTP2_NV_FLAG_NONE;
<<<<<<< HEAD
=======
  if(HEADER_OVERFLOW(nva[2])) {
    failf(conn->data, "Failed sending HTTP request: Header overflow");
    goto fail;
  }

  authority_idx = 0;
  i = 3;
  while(i < nheader) {
    size_t hlen;

    hdbuf = line_end + 2;

    line_end = strstr(hdbuf, "\r\n");
    if(line_end == hdbuf)
      goto fail;
>>>>>>> origin/tomato-shibby-RT-AC

  hdbuf = strchr(hdbuf, 0x0a);
  ++hdbuf;

<<<<<<< HEAD
  for(i = 3; i < nheader; ++i) {
    end = strchr(hdbuf, ':');
    assert(end);
    if(end - hdbuf == 4 && Curl_raw_nequal("host", hdbuf, 4)) {
=======
    for(end = hdbuf; end < line_end && *end != ':'; ++end)
      ;
    if(end == hdbuf || end == line_end)
      goto fail;
    hlen = end - hdbuf;

    if(hlen == 4 && strncasecompare("host", hdbuf, 4)) {
      authority_idx = i;
>>>>>>> origin/tomato-shibby-RT-AC
      nva[i].name = (unsigned char *)":authority";
      nva[i].namelen = (uint16_t)strlen((char *)nva[i].name);
    }
    else {
      nva[i].name = (unsigned char *)hdbuf;
      nva[i].namelen = (uint16_t)(end - hdbuf);
    }
    hdbuf = end + 1;
<<<<<<< HEAD
    for(; *hdbuf == ' '; ++hdbuf);
    end = strchr(hdbuf, 0x0d);
    assert(end);
    nva[i].value = (unsigned char *)hdbuf;
    nva[i].valuelen = (uint16_t)(end - hdbuf);
    nva[i].flags = NGHTTP2_NV_FLAG_NONE;

    hdbuf = end + 2;
    /* Inspect Content-Length header field and retrieve the request
       entity length so that we can set END_STREAM to the last DATA
       frame. */
    if(nva[i].namelen == 14 &&
       Curl_raw_nequal("content-length", (char*)nva[i].name, 14)) {
      size_t j;
      for(j = 0; j < nva[i].valuelen; ++j) {
        httpc->upload_left *= 10;
        httpc->upload_left += nva[i].value[j] - '0';
      }
      infof(conn->data, "request content-length=%zu\n", httpc->upload_left);
=======
    while(*hdbuf == ' ' || *hdbuf == '\t')
      ++hdbuf;
    end = line_end;

    switch(inspect_header((const char *)nva[i].name, nva[i].namelen, hdbuf,
                          end - hdbuf)) {
    case HEADERINST_IGNORE:
      /* skip header fields prohibited by HTTP/2 specification. */
      --nheader;
      continue;
    case HEADERINST_TE_TRAILERS:
      nva[i].value = (uint8_t*)"trailers";
      nva[i].valuelen = sizeof("trailers") - 1;
      break;
    default:
      nva[i].value = (unsigned char *)hdbuf;
      nva[i].valuelen = (size_t)(end - hdbuf);
    }

    nva[i].flags = NGHTTP2_NV_FLAG_NONE;
    if(HEADER_OVERFLOW(nva[i])) {
      failf(conn->data, "Failed sending HTTP request: Header overflow");
      goto fail;
    }
    ++i;
  }

  /* :authority must come before non-pseudo header fields */
  if(authority_idx != 0 && authority_idx != AUTHORITY_DST_IDX) {
    nghttp2_nv authority = nva[authority_idx];
    for(i = authority_idx; i > AUTHORITY_DST_IDX; --i) {
      nva[i] = nva[i - 1];
    }
    nva[i] = authority;
  }

  /* Warn stream may be rejected if cumulative length of headers is too large.
     It appears nghttp2 will not send a header frame larger than 64KB. */
#define MAX_ACC 60000  /* <64KB to account for some overhead */
  {
    size_t acc = 0;

    for(i = 0; i < nheader; ++i) {
      acc += nva[i].namelen + nva[i].valuelen;

      DEBUGF(infof(conn->data, "h2 header: %.*s:%.*s\n",
                   nva[i].namelen, nva[i].name,
                   nva[i].valuelen, nva[i].value));
    }

    if(acc > MAX_ACC) {
      infof(conn->data, "http2_send: Warning: The cumulative length of all "
            "headers exceeds %zu bytes and that could cause the "
            "stream to be rejected.\n", MAX_ACC);
>>>>>>> origin/tomato-shibby-RT-AC
    }
  }

  switch(conn->data->set.httpreq) {
  case HTTPREQ_POST:
  case HTTPREQ_POST_FORM:
  case HTTPREQ_PUT:
    if(conn->data->state.infilesize != -1)
      stream->upload_left = conn->data->state.infilesize;
    else
      /* data sending without specifying the data amount up front */
      stream->upload_left = -1; /* unknown, but not zero */

    data_prd.read_callback = data_source_read_callback;
    data_prd.source.ptr = NULL;
    stream_id = nghttp2_submit_request(httpc->h2, NULL, nva, nheader,
                                       &data_prd, NULL);
    break;
  default:
    stream_id = nghttp2_submit_request(httpc->h2, NULL, nva, nheader,
                                       NULL, NULL);
  }

  Curl_safefree(nva);

  if(stream_id < 0) {
    *err = CURLE_SEND_ERROR;
    return -1;
  }

  httpc->stream_id = stream_id;

  rv = nghttp2_session_send(httpc->h2);

  if(rv != 0) {
    *err = CURLE_SEND_ERROR;
    return -1;
  }

  if(httpc->stream_id != -1) {
    /* If whole HEADERS frame was sent off to the underlying socket,
       the nghttp2 library calls data_source_read_callback. But only
       it found that no data available, so it deferred the DATA
       transmission. Which means that nghttp2_session_want_write()
       returns 0 on http2_perform_getsock(), which results that no
       writable socket check is performed. To workaround this, we
       issue nghttp2_session_resume_data() here to bring back DATA
       transmission from deferred state. */
    nghttp2_session_resume_data(httpc->h2, httpc->stream_id);
  }

  return len;
}

void Curl_http2_setup(struct connectdata *conn)
{
  struct http_conn *httpc = &conn->proto.httpc;
  if(conn->handler->flags & PROTOPT_SSL)
    conn->handler = &Curl_handler_http2_ssl;
  else
    conn->handler = &Curl_handler_http2;

  infof(conn->data, "Using HTTP2\n");
  httpc->bodystarted = FALSE;
  httpc->closed = FALSE;
  httpc->header_recvbuf = Curl_add_buffer_init();
  httpc->nread_header_recvbuf = 0;
  httpc->data = NULL;
  httpc->datalen = 0;
  httpc->upload_left = 0;
  httpc->upload_mem = NULL;
  httpc->upload_len = 0;
  httpc->stream_id = -1;

  conn->httpversion = 20;

<<<<<<< HEAD
  /* Put place holder for status line */
  Curl_add_buffer(httpc->header_recvbuf, "HTTP/2.0 200\r\n", 14);
=======
  return CURLE_OK;
>>>>>>> origin/tomato-shibby-RT-AC
}

int Curl_http2_switched(struct connectdata *conn)
{
  /* TODO: May get CURLE_AGAIN */
  CURLcode rc;
  struct http_conn *httpc = &conn->proto.httpc;
  int rv;
<<<<<<< HEAD
=======
  ssize_t nproc;
  struct Curl_easy *data = conn->data;
  struct HTTP *stream = conn->data->req.protop;

  result = Curl_http2_setup(conn);
  if(result)
    return result;
>>>>>>> origin/tomato-shibby-RT-AC

  httpc->recv_underlying = (recving)conn->recv[FIRSTSOCKET];
  httpc->send_underlying = (sending)conn->send[FIRSTSOCKET];
  conn->recv[FIRSTSOCKET] = http2_recv;
  conn->send[FIRSTSOCKET] = http2_send;

  rv = (int) ((Curl_send*)httpc->send_underlying)
    (conn, FIRSTSOCKET,
     NGHTTP2_CLIENT_CONNECTION_PREFACE,
     NGHTTP2_CLIENT_CONNECTION_PREFACE_LEN,
     &rc);
  assert(rv == 24);
  if(conn->data->req.upgr101 == UPGR101_RECEIVED) {
    /* stream 1 is opened implicitly on upgrade */
    httpc->stream_id = 1;
    /* queue SETTINGS frame (again) */
    rv = nghttp2_session_upgrade(httpc->h2, httpc->binsettings,
                                 httpc->binlen, NULL);
    if(rv != 0) {
      failf(conn->data, "nghttp2_session_upgrade() failed: %s(%d)",
            nghttp2_strerror(rv), rv);
      return -1;
    }
  }
  else {
    populate_settings(conn, httpc);

    /* stream ID is unknown at this point */
<<<<<<< HEAD
    httpc->stream_id = -1;
    rv = nghttp2_submit_settings(httpc->h2, NGHTTP2_FLAG_NONE, NULL, 0);
=======
    stream->stream_id = -1;
    rv = nghttp2_submit_settings(httpc->h2, NGHTTP2_FLAG_NONE,
                                 httpc->local_settings,
                                 httpc->local_settings_num);
>>>>>>> origin/tomato-shibby-RT-AC
    if(rv != 0) {
      failf(conn->data, "nghttp2_submit_settings() failed: %s(%d)",
            nghttp2_strerror(rv), rv);
      return -1;
    }
  }
<<<<<<< HEAD
  return 0;
=======

#ifdef NGHTTP2_HAS_SET_LOCAL_WINDOW_SIZE
  rv = nghttp2_session_set_local_window_size(httpc->h2, NGHTTP2_FLAG_NONE, 0,
                                             HTTP2_HUGE_WINDOW_SIZE);
  if(rv != 0) {
    failf(data, "nghttp2_session_set_local_window_size() failed: %s(%d)",
          nghttp2_strerror(rv), rv);
    return CURLE_HTTP2;
  }
#endif

  /* we are going to copy mem to httpc->inbuf.  This is required since
     mem is part of buffer pointed by stream->mem, and callbacks
     called by nghttp2_session_mem_recv() will write stream specific
     data into stream->mem, overwriting data already there. */
  if(H2_BUFSIZE < nread) {
    failf(data, "connection buffer size is too small to store data following "
                "HTTP Upgrade response header: buflen=%zu, datalen=%zu",
          H2_BUFSIZE, nread);
    return CURLE_HTTP2;
  }

  infof(conn->data, "Copying HTTP/2 data in stream buffer to connection buffer"
                    " after upgrade: len=%zu\n",
        nread);

  if(nread)
    memcpy(httpc->inbuf, mem, nread);
  httpc->inbuflen = nread;

  nproc = nghttp2_session_mem_recv(httpc->h2, (const uint8_t *)httpc->inbuf,
                                   httpc->inbuflen);

  if(nghttp2_is_fatal((int)nproc)) {
    failf(data, "nghttp2_session_mem_recv() failed: %s(%d)",
          nghttp2_strerror((int)nproc), (int)nproc);
    return CURLE_HTTP2;
  }

  DEBUGF(infof(data, "nghttp2_session_mem_recv() returns %zd\n", nproc));

  if((ssize_t)nread == nproc) {
    httpc->inbuflen = 0;
    httpc->nread_inbuf = 0;
  }
  else {
    httpc->nread_inbuf += nproc;
  }

  /* Try to send some frames since we may read SETTINGS already. */
  rv = h2_session_send(data, httpc->h2);

  if(rv != 0) {
    failf(data, "nghttp2_session_send() failed: %s(%d)",
          nghttp2_strerror(rv), rv);
    return CURLE_HTTP2;
  }

  if(should_close_session(httpc)) {
    DEBUGF(infof(data,
                 "nghttp2_session_send(): nothing to do in this session\n"));
    return CURLE_HTTP2;
  }

  return CURLE_OK;
}

void Curl_http2_add_child(struct Curl_easy *parent, struct Curl_easy *child,
                          bool exclusive)
{
  struct Curl_http2_dep **tail;
  struct Curl_http2_dep *dep = calloc(1, sizeof(struct Curl_http2_dep));
  dep->data = child;

  if(parent->set.stream_dependents && exclusive) {
    struct Curl_http2_dep *node = parent->set.stream_dependents;
    while(node) {
      node->data->set.stream_depends_on = child;
      node = node->next;
    }

    tail = &child->set.stream_dependents;
    while(*tail)
      tail = &(*tail)->next;

    DEBUGASSERT(!*tail);
    *tail = parent->set.stream_dependents;
    parent->set.stream_dependents = 0;
  }

  tail = &parent->set.stream_dependents;
  while(*tail) {
    (*tail)->data->set.stream_depends_e = FALSE;
    tail = &(*tail)->next;
  }

  DEBUGASSERT(!*tail);
  *tail = dep;

  child->set.stream_depends_on = parent;
  child->set.stream_depends_e = exclusive;
}

void Curl_http2_remove_child(struct Curl_easy *parent, struct Curl_easy *child)
{
  struct Curl_http2_dep *last = 0;
  struct Curl_http2_dep *data = parent->set.stream_dependents;
  DEBUGASSERT(child->set.stream_depends_on == parent);

  while(data && data->data != child) {
    last = data;
    data = data->next;
  }

  DEBUGASSERT(data);

  if(data) {
    if(last) {
      last->next = data->next;
    }
    else {
      parent->set.stream_dependents = data->next;
    }
    free(data);
  }

  child->set.stream_depends_on = 0;
  child->set.stream_depends_e = FALSE;
}

void Curl_http2_cleanup_dependencies(struct Curl_easy *data)
{
  while(data->set.stream_dependents) {
    struct Curl_easy *tmp = data->set.stream_dependents->data;
    Curl_http2_remove_child(data, tmp);
    if(data->set.stream_depends_on)
      Curl_http2_add_child(data->set.stream_depends_on, tmp, FALSE);
  }

  if(data->set.stream_depends_on)
    Curl_http2_remove_child(data->set.stream_depends_on, data);
}

#else /* !USE_NGHTTP2 */

/* Satisfy external references even if http2 is not compiled in. */

#define CURL_DISABLE_TYPECHECK
#include <curl/curl.h>

char *curl_pushheader_bynum(struct curl_pushheaders *h, size_t num)
{
  (void) h;
  (void) num;
  return NULL;
}

char *curl_pushheader_byname(struct curl_pushheaders *h, const char *header)
{
  (void) h;
  (void) header;
  return NULL;
>>>>>>> origin/tomato-shibby-RT-AC
}

#endif
