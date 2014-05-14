/*
    Copyright (C) 2002-2008  Thomas Ries <tries@gmx.net>

    This file is part of Siproxd.
    
    Siproxd is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.
    
    Siproxd is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
    
    You should have received a copy of the GNU General Public License
    along with Siproxd; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA 
*/

#include "config.h"

#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include <sys/time.h>

#include <netinet/in.h>

#include <osipparser2/osip_parser.h>
#include <osipparser2/osip_md5.h>

#include "digcalc.h"

#include "siproxd.h"
#include "log.h"

static char const ident[]="$Id: auth.c 392 2008-07-22 11:17:49Z hb9xar $";

/* configuration storage */
extern struct siproxd_config configuration;

/* Global File instance on pw file */
extern FILE *siproxd_passwordfile;

/* local protorypes */
static char *auth_generate_nonce(void);
static int auth_check(osip_proxy_authorization_t *proxy_auth);
static char *auth_getpwd(char *username);

/*
 * perform proxy authentication
 *
 * RETURNS
 *	STS_SUCCESS : authentication ok / not needed
 *	STS_FAILURE : authentication failed
 *	STS_NEEDAUTH: authentication needed
 */
int authenticate_proxy(osip_message_t *sipmsg) {
   osip_proxy_authorization_t *proxy_auth=NULL;
   
   /* required by config? */
   if (configuration.proxy_auth_realm == NULL) {
      return STS_SUCCESS;
   }
   
   /* supplied by UA? */
   osip_message_get_proxy_authorization(sipmsg, 0, &proxy_auth);
   if (proxy_auth == NULL) {
      DEBUGC(DBCLASS_AUTH,"proxy-auth required, not supplied by UA");
      return STS_NEED_AUTH;
   }

   /* verify supplied authentication */
   if (auth_check(proxy_auth) == STS_SUCCESS) {
      DEBUGC(DBCLASS_AUTH,"proxy-auth succeeded");
      return STS_SUCCESS;
   }

   /* authentication failed */
   WARN("authenticate_proxy failed");
   return STS_FAILURE;
}

/*
 * includes proxy authentication header in SIP message
 *
 * RETURNS
 *	STS_SUCCESS
 *	STS_FAILURE
 */
int auth_include_authrq(osip_message_t *sipmsg) {
   osip_proxy_authenticate_t *p_auth;
   char *realm=NULL;

   if (osip_proxy_authenticate_init(&p_auth) != 0) {
      ERROR("proxy_authenticate_init failed");
      return STS_FAILURE;
   }

   osip_proxy_authenticate_set_auth_type(p_auth, osip_strdup("Digest"));
   osip_proxy_authenticate_set_nonce(p_auth, osip_strdup(auth_generate_nonce()));
   realm=osip_malloc(strlen(configuration.proxy_auth_realm)+3); /* add 2x" and \0 */
   if (realm) {
      sprintf(realm,"\"%s\"",configuration.proxy_auth_realm);
      osip_proxy_authenticate_set_realm(p_auth, realm);
   } else {
      ERROR("unable to malloc() %ld bytes for authentication realm",
            (long)strlen(configuration.proxy_auth_realm)+3);
      return STS_FAILURE;
   }

   osip_list_add (&(sipmsg->proxy_authenticates), p_auth, -1);

   DEBUGC(DBCLASS_AUTH,"added authentication header");

   return STS_SUCCESS;
}

/*
 * generates a nonce string
 *
 * RETURNS nonce string
 */
static char *auth_generate_nonce() {
   static char nonce[40];
   struct timeval tv;
   
   gettimeofday (&tv, NULL);

/* yeah, I know... should be a better algorithm */
/* enclose it in double quotes, as libosip does *not* do it (2.0.6) */
   sprintf(nonce, "\"%8.8lx%8.8lx%8.8x%8.8x\"",
           (long)tv.tv_sec, (long)tv.tv_usec, rand(), rand() );

   DEBUGC(DBCLASS_AUTH,"created nonce=\"%s\"",nonce);
   return nonce;
}


/*
 * verify the supplied authentication information from UA
 *
 * RETURNS
 *	STS_SUCCESS if succeeded
 *	STS_FAILURE if failed
 */
static int auth_check(osip_proxy_authorization_t *proxy_auth) {
   char *password=NULL;
   int sts;

   HASHHEX HA1;
   HASHHEX HA2 = "";
   HASHHEX Lcl_Response;
 
   char *Username   = NULL;
   char *Realm      = NULL;
   char *Nonce      = NULL;
   char *CNonce     = NULL;
   char *NonceCount = NULL;
   char *Qpop	    = NULL;
   char *Uri	    = NULL;
   char *Response   = NULL;

   /* if item exists, allocate& copy string without quotes */
   if (proxy_auth->username)
      Username=osip_strdup_without_quote(proxy_auth->username);

   if (proxy_auth->realm)
      Realm=osip_strdup_without_quote(proxy_auth->realm);

   if (proxy_auth->nonce)
      Nonce=osip_strdup_without_quote(proxy_auth->nonce);

   if (proxy_auth->cnonce)
      CNonce=osip_strdup_without_quote(proxy_auth->cnonce);

   if (proxy_auth->nonce_count)
      NonceCount=osip_strdup_without_quote(proxy_auth->nonce_count);

   if (proxy_auth->message_qop)
      Qpop=osip_strdup_without_quote(proxy_auth->message_qop);

   if (proxy_auth->uri) 
      Uri=osip_strdup_without_quote(proxy_auth->uri);

   if (proxy_auth->response)
      Response=osip_strdup_without_quote(proxy_auth->response);

   /* get password */
   if (configuration.proxy_auth_pwfile) {
      /* check in passwd file */
      password=auth_getpwd(Username);
   } else if (configuration.proxy_auth_passwd) {
      /* get password from configuration */
      password=configuration.proxy_auth_passwd;
   }

   if (password == NULL) {
      DEBUGC(DBCLASS_AUTH,"user [%s] not in password file!",
             (Username)?Username:"*NULL*");
      return STS_FAILURE;
   }

   DEBUGC(DBCLASS_BABBLE," username=\"%s\"",Username  );
   DEBUGC(DBCLASS_BABBLE," realm   =\"%s\"",Realm     );
   DEBUGC(DBCLASS_BABBLE," nonce   =\"%s\"",Nonce     );
   DEBUGC(DBCLASS_BABBLE," cnonce  =\"%s\"",CNonce    );
   DEBUGC(DBCLASS_BABBLE," nonce_nc=\"%s\"",NonceCount);
   DEBUGC(DBCLASS_BABBLE," qpop    =\"%s\"",Qpop      );
   DEBUGC(DBCLASS_BABBLE," uri     =\"%s\"",Uri	    );
   DEBUGC(DBCLASS_BABBLE," response=\"%s\"",Response  );

   /* calculate the MD5 digest (heavily inspired from linphone code) */
   DigestCalcHA1("MD5", Username, Realm, password, Nonce, CNonce, HA1);
   DigestCalcResponse(HA1, Nonce, NonceCount, CNonce, Qpop,
		      "REGISTER", Uri, HA2, Lcl_Response);

   DEBUGC(DBCLASS_BABBLE,"calculated Response=\"%s\"", Lcl_Response);

   if (strcmp((char*)Lcl_Response, Response)==0) {
      DEBUGC(DBCLASS_AUTH,"Authentication succeeded");
      sts = STS_SUCCESS;
   } else {
      DEBUGC(DBCLASS_AUTH,"Authentication failed");
      sts = STS_FAILURE;
   }

   /* free allocated memory from above */
   if (Username)   free(Username);
   if (Realm)      free(Realm);
   if (Nonce)      free(Nonce);
   if (CNonce)     free(CNonce);
   if (NonceCount) free(NonceCount);
   if (Qpop)       free(Qpop);
   if (Uri)        free(Uri);
   if (Response)   free(Response);

   return sts;
}


/*
 * lookup in the password file and return
 * the user specific password for 'username'
 *
 * RETURNS
 *	password for user or NULL if not found
 */
static char *auth_getpwd(char *username) {
   typedef struct {
      char username[USERNAME_SIZE];
      char password[PASSWORD_SIZE];
   } auth_cache_t;

   char buff[128];
   int i;
   static auth_cache_t *auth_cache=NULL;
   void *tmpptr;
   static int auth_cache_size=0;
   static int auth_cache_count=0;

   if (auth_cache==NULL) {
      DEBUGC(DBCLASS_AUTH,"initialize password cache");

      /* config file not found or unable to open for read */
      if (siproxd_passwordfile==NULL) {
         ERROR ("could not open password file: %s", strerror(errno));
         return NULL;
      }
      
      rewind(siproxd_passwordfile);

      while (fgets(buff,sizeof(buff),siproxd_passwordfile) != NULL) {
         /* life insurance */
         buff[sizeof(buff)-1]='\0';

	 /* strip newline if present */
	 if (buff[strlen(buff)-1]=='\n') buff[strlen(buff)-1]='\0';

	 /* strip emty lines */
	 if (strlen(buff) == 0) continue;

	 /* strip comments and line with only whitespaces */
	 for (i=0;i<strlen(buff);i++) {
            if ((buff[i] == ' ') || (buff[i] == '\t')) continue;
            if (buff[i] =='#') i=strlen(buff);
            break;
	 }
	 if (i == strlen(buff)) continue;

         /* allocate space whenever needed */
	 if (auth_cache_count >= auth_cache_size) {
	    auth_cache_size+=10;
	    tmpptr=realloc(auth_cache, auth_cache_size*sizeof(auth_cache_t));
            if (tmpptr != NULL) {
               auth_cache= (auth_cache_t *)tmpptr;
	    } else {
               ERROR("realloc failed! this is not good");
	       auth_cache_size-=10;
	       return NULL;
	    }
         } /* cnt > size */

         i=sscanf(buff,"%s %s",auth_cache[auth_cache_count].username,
	                       auth_cache[auth_cache_count].password);
         /* if I got username & passwd, make it valid and increment counter */
         if (i == 2) auth_cache_count++;
      }

   } /* initialize cache */

   /* search cache for user */
   DEBUGC(DBCLASS_AUTH,"searching password entry for user %s",username);
   for (i=0; i< auth_cache_count;i++) {
      if (strcmp(username, auth_cache[i].username)==0) {
         DEBUGC(DBCLASS_AUTH,"found password entry for user %s",username);
         return auth_cache[i].password;
      }
   }

   DEBUGC(DBCLASS_AUTH,"no password entry found for user %s",username);
   return NULL;
}


/*-------------------------------------------------------------------------
  -------------------------------------------------------------------------
  The routines below have been taken from linphone
  (osipua/src/authentication.c)
  -------------------------------------------------------------------------
  -------------------------------------------------------------------------*/

void CvtHex(
	    IN HASH Bin,
	    OUT HASHHEX Hex
	    )
{
  unsigned short i;
  unsigned char j;
  
  for (i = 0; i < HASHLEN; i++) {
    j = (Bin[i] >> 4) & 0xf;
    if (j <= 9)
      Hex[i*2] = (j + '0');
    else
      Hex[i*2] = (j + 'a' - 10);
    j = Bin[i] & 0xf;
    if (j <= 9)
      Hex[i*2+1] = (j + '0');
    else
      Hex[i*2+1] = (j + 'a' - 10);
  };
  Hex[HASHHEXLEN] = '\0';
}

/* calculate H(A1) as per spec */
void DigestCalcHA1(
		   IN char * pszAlg,
		   IN char * pszUserName,
		   IN char * pszRealm,
		   IN char * pszPassword,
		   IN char * pszNonce,
		   IN char * pszCNonce,
		   OUT HASHHEX SessionKey
		   )
{
  osip_MD5_CTX Md5Ctx;
  HASH HA1;
  
  osip_MD5Init(&Md5Ctx);
  if (pszUserName) osip_MD5Update(&Md5Ctx, (unsigned char*)pszUserName, 
                                  strlen(pszUserName));
  osip_MD5Update(&Md5Ctx, (unsigned char*)":", 1);
  if (pszRealm)    osip_MD5Update(&Md5Ctx, (unsigned char*)pszRealm,
                                  strlen(pszRealm));
  osip_MD5Update(&Md5Ctx, (unsigned char*)":", 1);
  if (pszPassword) osip_MD5Update(&Md5Ctx, (unsigned char*)pszPassword,
                                  strlen(pszPassword));
  osip_MD5Final(HA1, &Md5Ctx);

  if ((pszAlg!=NULL) && (osip_strcasecmp(pszAlg, "md5-sess") == 0)) {
    osip_MD5Init(&Md5Ctx);
    osip_MD5Update(&Md5Ctx, HA1, HASHLEN);
    osip_MD5Update(&Md5Ctx, (unsigned char*)":", 1);
    if (pszNonce)  osip_MD5Update(&Md5Ctx, (unsigned char*)pszNonce,
                                  strlen(pszNonce));
    osip_MD5Update(&Md5Ctx, (unsigned char*)":", 1);
    if (pszCNonce) osip_MD5Update(&Md5Ctx, (unsigned char*)pszCNonce,
                                  strlen(pszCNonce));
    osip_MD5Final(HA1, &Md5Ctx);
  };
  CvtHex(HA1, SessionKey);
}

/* calculate request-digest/response-digest as per HTTP Digest spec */
void DigestCalcResponse(
			IN HASHHEX HA1,         /* H(A1) */
			IN char * pszNonce,     /* nonce from server */
			IN char * pszNonceCount,  /* 8 hex digits */
			IN char * pszCNonce,    /* client nonce */
			IN char * pszQop,       /* qop-value: "", "auth", "auth-int" */
			IN char * pszMethod,    /* method from the request */
			IN char * pszDigestUri, /* requested URL */
			IN HASHHEX HEntity,     /* H(entity body) if qop="auth-int" */
			OUT HASHHEX Response    /* request-digest or response-digest */
			)
{
  osip_MD5_CTX Md5Ctx;
  HASH HA2;
  HASH RespHash;
  HASHHEX HA2Hex;
  
  /* calculate H(A2) */
  osip_MD5Init(&Md5Ctx);
  if (pszMethod)    osip_MD5Update(&Md5Ctx, (unsigned char*)pszMethod,
                                   strlen(pszMethod));
  osip_MD5Update(&Md5Ctx, (unsigned char*)":", 1);
  if (pszDigestUri) osip_MD5Update(&Md5Ctx, (unsigned char*)pszDigestUri,
                                   strlen(pszDigestUri));
  
  if (pszQop!=NULL) {
      goto auth_withqop;
  };
  
  /* auth_withoutqop: */
  osip_MD5Final(HA2, &Md5Ctx);
  CvtHex(HA2, HA2Hex);

  /* calculate response */
  osip_MD5Init(&Md5Ctx);
  osip_MD5Update(&Md5Ctx, HA1, HASHHEXLEN);
  osip_MD5Update(&Md5Ctx, (unsigned char*)":", 1);
  if (pszNonce)    osip_MD5Update(&Md5Ctx, (unsigned char*)pszNonce, strlen(pszNonce));
  osip_MD5Update(&Md5Ctx, (unsigned char*)":", 1);

  goto end;

 auth_withqop:

  osip_MD5Update(&Md5Ctx, (unsigned char*)":", 1);
  osip_MD5Update(&Md5Ctx, HEntity, HASHHEXLEN);
  osip_MD5Final(HA2, &Md5Ctx);
  CvtHex(HA2, HA2Hex);

  /* calculate response */
  osip_MD5Init(&Md5Ctx);
  osip_MD5Update(&Md5Ctx, HA1, HASHHEXLEN);
  osip_MD5Update(&Md5Ctx, (unsigned char*)":", 1);
  if (pszNonce)    osip_MD5Update(&Md5Ctx, (unsigned char*)pszNonce, strlen(pszNonce));
  osip_MD5Update(&Md5Ctx, (unsigned char*)":", 1);
  if (pszNonceCount)osip_MD5Update(&Md5Ctx, (unsigned char*)pszNonceCount, strlen(pszNonceCount));
  osip_MD5Update(&Md5Ctx, (unsigned char*)":", 1);
  if (pszCNonce)   osip_MD5Update(&Md5Ctx, (unsigned char*)pszCNonce, strlen(pszCNonce));
  osip_MD5Update(&Md5Ctx, (unsigned char*)":", 1);
  if (pszQop)      osip_MD5Update(&Md5Ctx, (unsigned char*)pszQop, strlen(pszQop));
  osip_MD5Update(&Md5Ctx, (unsigned char*)":", 1);

 end:
  osip_MD5Update(&Md5Ctx, HA2Hex, HASHHEXLEN);
  osip_MD5Final(RespHash, &Md5Ctx);
  CvtHex(RespHash, Response);
}


