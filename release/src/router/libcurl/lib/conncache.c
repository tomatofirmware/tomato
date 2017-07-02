/***************************************************************************
 *                                  _   _ ____  _
 *  Project                     ___| | | |  _ \| |
 *                             / __| | | | |_) | |
 *                            | (__| |_| |  _ <| |___
 *                             \___|\___/|_| \_\_____|
 *
 * Copyright (C) 2012, Linus Nielsen Feltzing, <linus@haxx.se>
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

#include "curl_setup.h"

#include <curl/curl.h>

#include "urldata.h"
#include "url.h"
#include "progress.h"
#include "multiif.h"
#include "sendf.h"
<<<<<<< HEAD
#include "rawstr.h"
#include "bundles.h"
=======
>>>>>>> origin/tomato-shibby-RT-AC
#include "conncache.h"

#include "curl_memory.h"
/* The last #include file should be: */
#include "memdebug.h"

<<<<<<< HEAD
static void free_bundle_hash_entry(void *freethis)
=======
static void conn_llist_dtor(void *user, void *element)
{
  struct connectdata *data = element;
  (void)user;

  data->bundle = NULL;
}

static CURLcode bundle_create(struct Curl_easy *data,
                              struct connectbundle **cb_ptr)
{
  (void)data;
  DEBUGASSERT(*cb_ptr == NULL);
  *cb_ptr = malloc(sizeof(struct connectbundle));
  if(!*cb_ptr)
    return CURLE_OUT_OF_MEMORY;

  (*cb_ptr)->num_connections = 0;
  (*cb_ptr)->multiuse = BUNDLE_UNKNOWN;

  (*cb_ptr)->conn_list = Curl_llist_alloc((curl_llist_dtor) conn_llist_dtor);
  if(!(*cb_ptr)->conn_list) {
    Curl_safefree(*cb_ptr);
    return CURLE_OUT_OF_MEMORY;
  }
  return CURLE_OK;
}

static void bundle_destroy(struct connectbundle *cb_ptr)
>>>>>>> origin/tomato-shibby-RT-AC
{
  struct connectbundle *b = (struct connectbundle *) freethis;

  Curl_bundle_destroy(b);
}

struct conncache *Curl_conncache_init(int size)
{
  struct conncache *connc;

  connc = calloc(1, sizeof(struct conncache));
  if(!connc)
    return NULL;

  connc->hash = Curl_hash_alloc(size, Curl_hash_str,
                                Curl_str_key_compare, free_bundle_hash_entry);

  if(!connc->hash) {
    free(connc);
    return NULL;
  }

  return connc;
}

void Curl_conncache_destroy(struct conncache *connc)
{
<<<<<<< HEAD
  if(connc) {
    Curl_hash_destroy(connc->hash);
    connc->hash = NULL;
    free(connc);
  }
=======
  if(connc)
    Curl_hash_destroy(&connc->hash);
}

/* returns an allocated key to find a bundle for this connection */
static char *hashkey(struct connectdata *conn)
{
  const char *hostname;

  if(conn->bits.socksproxy)
    hostname = conn->socks_proxy.host.name;
  else if(conn->bits.httpproxy)
    hostname = conn->http_proxy.host.name;
  else if(conn->bits.conn_to_host)
    hostname = conn->conn_to_host.name;
  else
    hostname = conn->host.name;

  return aprintf("%s:%ld", hostname, conn->port);
>>>>>>> origin/tomato-shibby-RT-AC
}

struct connectbundle *Curl_conncache_find_bundle(struct conncache *connc,
                                                 char *hostname)
{
  struct connectbundle *bundle = NULL;

  if(connc)
    bundle = Curl_hash_pick(connc->hash, hostname, strlen(hostname)+1);

  return bundle;
}

static bool conncache_add_bundle(struct conncache *connc,
                                 char *hostname,
                                 struct connectbundle *bundle)
{
  void *p;

  p = Curl_hash_add(connc->hash, hostname, strlen(hostname)+1, bundle);

  return p?TRUE:FALSE;
}

static void conncache_remove_bundle(struct conncache *connc,
                                    struct connectbundle *bundle)
{
  struct curl_hash_iterator iter;
  struct curl_hash_element *he;

  if(!connc)
    return;

  Curl_hash_start_iterate(connc->hash, &iter);

  he = Curl_hash_next_element(&iter);
  while(he) {
    if(he->ptr == bundle) {
      /* The bundle is destroyed by the hash destructor function,
         free_bundle_hash_entry() */
      Curl_hash_delete(connc->hash, he->key, he->key_len);
      return;
    }

    he = Curl_hash_next_element(&iter);
  }
}

CURLcode Curl_conncache_add_conn(struct conncache *connc,
                                 struct connectdata *conn)
{
  CURLcode result;
  struct connectbundle *bundle;
  struct connectbundle *new_bundle = NULL;
  struct Curl_easy *data = conn->data;

  bundle = Curl_conncache_find_bundle(data->state.conn_cache,
                                      conn->host.name);
  if(!bundle) {
    result = Curl_bundle_create(data, &new_bundle);
    if(result != CURLE_OK)
      return result;

    if(!conncache_add_bundle(data->state.conn_cache,
                             conn->host.name, new_bundle)) {
      Curl_bundle_destroy(new_bundle);
      return CURLE_OUT_OF_MEMORY;
    }
    bundle = new_bundle;
  }

  result = Curl_bundle_add_conn(bundle, conn);
  if(result != CURLE_OK) {
    if(new_bundle)
      conncache_remove_bundle(data->state.conn_cache, new_bundle);
    return result;
  }

  connc->num_connections++;

  return CURLE_OK;
}

void Curl_conncache_remove_conn(struct conncache *connc,
                                struct connectdata *conn)
{
  struct connectbundle *bundle = conn->bundle;

  /* The bundle pointer can be NULL, since this function can be called
     due to a failed connection attempt, before being added to a bundle */
  if(bundle) {
    Curl_bundle_remove_conn(bundle, conn);
    if(bundle->num_connections == 0) {
      conncache_remove_bundle(connc, bundle);
    }

    if(connc) {
      connc->num_connections--;

      DEBUGF(infof(conn->data, "The cache now contains %d members\n",
                   connc->num_connections));
    }
  }
}

/* This function iterates the entire connection cache and calls the
   function func() with the connection pointer as the first argument
   and the supplied 'param' argument as the other,

   Return 0 from func() to continue the loop, return 1 to abort it.
 */
void Curl_conncache_foreach(struct conncache *connc,
                            void *param,
                            int (*func)(struct connectdata *conn, void *param))
{
  struct curl_hash_iterator iter;
  struct curl_llist_element *curr;
  struct curl_hash_element *he;

  if(!connc)
    return;

  Curl_hash_start_iterate(connc->hash, &iter);

  he = Curl_hash_next_element(&iter);
  while(he) {
    struct connectbundle *bundle;
    struct connectdata *conn;

    bundle = he->ptr;

    curr = bundle->conn_list->head;
    while(curr) {
      /* Yes, we need to update curr before calling func(), because func()
         might decide to remove the connection */
      conn = curr->ptr;
      curr = curr->next;

      if(1 == func(conn, param))
        return;
    }

    he = Curl_hash_next_element(&iter);
  }
}

/* Return the first connection found in the cache. Used when closing all
   connections */
struct connectdata *
Curl_conncache_find_first_connection(struct conncache *connc)
{
  struct curl_hash_iterator iter;
  struct curl_llist_element *curr;
  struct curl_hash_element *he;
  struct connectbundle *bundle;

  Curl_hash_start_iterate(connc->hash, &iter);

  he = Curl_hash_next_element(&iter);
  while(he) {
    bundle = he->ptr;

    curr = bundle->conn_list->head;
    if(curr) {
      return curr->ptr;
    }

    he = Curl_hash_next_element(&iter);
  }

  return NULL;
}


#if 0
/* Useful for debugging the connection cache */
void Curl_conncache_print(struct conncache *connc)
{
  struct curl_hash_iterator iter;
  struct curl_llist_element *curr;
  struct curl_hash_element *he;

  if(!connc)
    return;

  fprintf(stderr, "=Bundle cache=\n");

  Curl_hash_start_iterate(connc->hash, &iter);

  he = Curl_hash_next_element(&iter);
  while(he) {
    struct connectbundle *bundle;
    struct connectdata *conn;

    bundle = he->ptr;

    fprintf(stderr, "%s -", he->key);
    curr = bundle->conn_list->head;
    while(curr) {
      conn = curr->ptr;

      fprintf(stderr, " [%p %d]", (void *)conn, conn->inuse);
      curr = curr->next;
    }
    fprintf(stderr, "\n");

    he = Curl_hash_next_element(&iter);
  }
}
#endif
