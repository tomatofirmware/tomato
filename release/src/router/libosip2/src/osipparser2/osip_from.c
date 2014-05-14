/*
  The oSIP library implements the Session Initiation Protocol (SIP -rfc3261-)
  Copyright (C) 2001-2012 Aymeric MOIZARD amoizard@antisip.com
  
  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.
  
  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.
  
  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include <osipparser2/internal.h>

#include <osipparser2/osip_port.h>
#include <osipparser2/osip_message.h>
#include <osipparser2/osip_parser.h>
#include "parser.h"

/* adds the from header to message.              */
/* INPUT : char *hvalue | value of header.    */
/* OUTPUT: osip_message_t *sip | structure to save results.  */
/* returns -1 on error. */
int
osip_message_set_from (osip_message_t * sip, const char *hvalue)
{
  int i;

  if (hvalue == NULL || hvalue[0] == '\0')
    return OSIP_SUCCESS;

  if (sip->from != NULL)
    return OSIP_SYNTAXERROR;
  i = osip_from_init (&(sip->from));
  if (i != 0)
    return i;
  sip->message_property = 2;
  i = osip_from_parse (sip->from, hvalue);
  if (i != 0) {
    osip_from_free (sip->from);
    sip->from = NULL;
    return i;
  }
  return OSIP_SUCCESS;
}


#ifndef MINISIZE
/* returns the from header.            */
/* INPUT : osip_message_t *sip | sip message.   */
/* returns null on error. */
osip_from_t *
osip_message_get_from (const osip_message_t * sip)
{
  return sip->from;
}
#endif

int
osip_from_init (osip_from_t ** from)
{
  *from = (osip_from_t *) osip_malloc (sizeof (osip_from_t));
  if (*from == NULL)
    return OSIP_NOMEM;
  (*from)->displayname = NULL;
  (*from)->url = NULL;

  osip_list_init (&(*from)->gen_params);

  return OSIP_SUCCESS;
}

/* deallocates a osip_from_t structure.  */
/* INPUT : osip_from_t *from | from. */
void
osip_from_free (osip_from_t * from)
{
  if (from == NULL)
    return;
  if (from->url != NULL) {
    osip_uri_free (from->url);
  }
  osip_free (from->displayname);

  osip_generic_param_freelist (&from->gen_params);

  osip_free (from);
}

/* parses the string as a from header.                   */
/* INPUT : const char *string | pointer to a from string.*/
/* OUTPUT: osip_from_t *from | structure to save results.     */
/* returns -1 on error. */
int
osip_from_parse (osip_from_t * from, const char *hvalue)
{
  const char *displayname;
  const char *url;
  const char *url_end;
  const char *gen_params;
  int i;

  if (from == NULL || hvalue == NULL)
    return OSIP_BADPARAMETER;
  /* How to parse:

     we'll place the pointers:
     displayname  =>  beginning of displayname
     url          =>  beginning of url
     url_end      =>  end       of url
     gen_params  =>  beginning of params

     examples:

     jack <sip:amoizard@antisip.com>;tag=34erZ
     ^     ^                ^ ^

     sip:amoizard@antisip.com;tag=34erZ
     ^                ^^      
   */

  displayname = strchr (hvalue, '"');

  url = strchr (hvalue, '<');
  if (url != NULL) {
    url_end = strchr (url, '>');
    if (url_end == NULL)
      return OSIP_SYNTAXERROR;
  }

  /* SIPit day2: this case was not supported
     first '"' is placed after '<' and after '>'
     <sip:ixion@62.254.248.117;method=INVITE>;description="OPEN";expires=28800
     if the fisrt quote is after '<' then
     this is not a quote for a displayname.
   */
  if (displayname != NULL) {
    if (displayname > url)
      displayname = NULL;
  }

  if ((displayname == NULL) && (url != NULL)) { /* displayname IS A '*token' (not a quoted-string) */
    if (hvalue != url) {        /* displayname exists */
      if (url - hvalue + 1 < 2)
        return OSIP_SYNTAXERROR;
      from->displayname = (char *) osip_malloc (url - hvalue + 1);
      if (from->displayname == NULL)
        return OSIP_NOMEM;
      osip_clrncpy (from->displayname, hvalue, url - hvalue);
    }
    url++;                      /* place pointer on the beginning of url */
  }
  else {
    if ((displayname != NULL) && (url != NULL)) {       /* displayname IS A quoted-string (not a '*token') */
      const char *first;
      const char *second = NULL;

      /* search for quotes */
      first = __osip_quote_find (hvalue);
      if (first == NULL)
        return OSIP_SYNTAXERROR;        /* missing quote */
      second = __osip_quote_find (first + 1);
      if (second == NULL)
        return OSIP_SYNTAXERROR;        /* missing quote */
      if ((first > url))
        return OSIP_SYNTAXERROR;

      if (second - first + 2 >= 2) {
        from->displayname = (char *) osip_malloc (second - first + 2);
        if (from->displayname == NULL)
          return OSIP_NOMEM;
        osip_strncpy (from->displayname, first, second - first + 1);
        /* osip_clrspace(from->displayname); *//*should we do that? */

        /* special case: "<sip:joe@big.org>" <sip:joe@really.big.com> */
      }                         /* else displayname is empty? */
      url = strchr (second + 1, '<');
      if (url == NULL)
        return OSIP_SYNTAXERROR;        /* '<' MUST exist */
      url++;
    }
    else
      url = hvalue;             /* field does not contains '<' and '>' */
  }

  /* DISPLAY-NAME SET   */
  /* START of URL KNOWN */

  url_end = strchr (url, '>');

  if (url_end == NULL) {        /* sip:amoizard@antisip.com;tag=023 *//* We are sure ';' is the delimiter for from-parameters */
    char *host = strchr (url, '@');

    if (host != NULL)
      gen_params = strchr (host, ';');
    else
      gen_params = strchr (url, ';');
    if (gen_params != NULL)
      url_end = gen_params - 1;
    else
      url_end = url + strlen (url);
  }
  else {                        /* jack <sip:amoizard@antisip.com;user=phone>;tag=azer */

    gen_params = strchr (url_end, ';');
    url_end--;                  /* place pointer on the beginning of url */
  }

  if (gen_params != NULL) {     /* now we are sure a param exist */
    i = __osip_generic_param_parseall (&from->gen_params, gen_params);
    if (i != 0) {
      return i;
    }
  }

  /* set the url */
  {
    char *tmp;

    if (url_end - url + 2 < 7)
      return OSIP_SYNTAXERROR;
    i = osip_uri_init (&(from->url));
    if (i != 0)
      return i;
    tmp = (char *) osip_malloc (url_end - url + 2);
    if (tmp == NULL)
      return OSIP_NOMEM;
    osip_strncpy (tmp, url, url_end - url + 1);
    i = osip_uri_parse (from->url, tmp);
    osip_free (tmp);
    if (i != 0)
      return i;
  }
  return OSIP_SUCCESS;
}


/* returns the from header as a string.  */
/* INPUT : osip_from_t *from | from header.   */
/* returns -1 on error. */
int
osip_from_to_str (const osip_from_t * from, char **dest)
{
  char *url;
  char *buf;
  int i;
  size_t len;

  *dest = NULL;
  if ((from == NULL) || (from->url == NULL))
    return OSIP_BADPARAMETER;

  i = osip_uri_to_str (from->url, &url);
  if (i != 0)
    return i;

  if (from->displayname == NULL)
    len = strlen (url) + 5;
  else
    len = strlen (url) + strlen (from->displayname) + 5;

  buf = (char *) osip_malloc (len);
  if (buf == NULL) {
    osip_free (url);
    return OSIP_NOMEM;
  }

  if (from->displayname != NULL)
    sprintf (buf, "%s <%s>", from->displayname, url);
  else
    /* from rfc2543bis-04: for authentication related issue!
       "The To and From header fields always include the < and >
       delimiters even if the display-name is empty." */
    sprintf (buf, "<%s>", url);
  osip_free (url);

  {
    int pos = 0;
    osip_generic_param_t *u_param;
    size_t plen;
    char *tmp;

    while (!osip_list_eol (&from->gen_params, pos)) {
      u_param = (osip_generic_param_t *) osip_list_get (&from->gen_params, pos);

      if (u_param->gvalue == NULL)
        plen = strlen (u_param->gname) + 2;
      else
        plen = strlen (u_param->gname) + strlen (u_param->gvalue) + 3;
      len = len + plen;
      buf = (char *) osip_realloc (buf, len);
      tmp = buf;
      tmp = tmp + strlen (tmp);
      if (u_param->gvalue == NULL)
        snprintf (tmp, len - (tmp - buf), ";%s", u_param->gname);
      else
        snprintf (tmp, len - (tmp - buf), ";%s=%s", u_param->gname, u_param->gvalue);
      pos++;
    }
  }
  *dest = buf;
  return OSIP_SUCCESS;
}

char *
osip_from_get_displayname (osip_from_t * from)
{
  if (from == NULL)
    return NULL;
  return from->displayname;
}

void
osip_from_set_displayname (osip_from_t * from, char *displayname)
{
  from->displayname = displayname;
}

osip_uri_t *
osip_from_get_url (osip_from_t * from)
{
  if (from == NULL)
    return NULL;
  return from->url;
}

void
osip_from_set_url (osip_from_t * from, osip_uri_t * url)
{
  from->url = url;
}

int
osip_from_param_get (osip_from_t * from, int pos, osip_generic_param_t ** fparam)
{
  *fparam = NULL;
  if (from == NULL)
    return OSIP_BADPARAMETER;
  if (osip_list_size (&from->gen_params) <= pos)
    return OSIP_UNDEFINED_ERROR;        /* does not exist */
  *fparam = (osip_generic_param_t *) osip_list_get (&from->gen_params, pos);
  return pos;
}

int
osip_from_clone (const osip_from_t * from, osip_from_t ** dest)
{
  int i;
  osip_from_t *fr;

  *dest = NULL;
  if (from == NULL)
    return OSIP_BADPARAMETER;

  i = osip_from_init (&fr);
  if (i != 0)                   /* allocation failed */
    return i;
  if (from->displayname != NULL) {
    fr->displayname = osip_strdup (from->displayname);
    if (fr->displayname == NULL) {
      osip_from_free (fr);
      return OSIP_NOMEM;
    }
  }

  if (from->url != NULL) {
    i = osip_uri_clone (from->url, &(fr->url));
    if (i != 0) {
      osip_from_free (fr);
      return i;
    }
  }

  i = osip_list_clone (&from->gen_params, &fr->gen_params, (int (*)(void *, void **)) &osip_generic_param_clone);
  if (i != 0) {
    osip_from_free (fr);
    return i;
  }
  *dest = fr;
  return OSIP_SUCCESS;
}

int
osip_from_compare (osip_from_t * from1, osip_from_t * from2)
{
  char *tag1;
  char *tag2;

  if (from1 == NULL || from2 == NULL)
    return OSIP_BADPARAMETER;
  if (from1->url == NULL || from2->url == NULL)
    return OSIP_BADPARAMETER;

  /* we could have a sip or sips url, but if string!=NULL,
     host part will be NULL. */
  if (from1->url->host == NULL && from2->url->host == NULL) {
    if (from1->url->string == NULL || from2->url->string == NULL)
      return OSIP_UNDEFINED_ERROR;
    if (0 == strcmp (from1->url->string, from2->url->string))
      return OSIP_SUCCESS;
  }
  if (from1->url->host == NULL || from2->url->host == NULL)
    return OSIP_UNDEFINED_ERROR;

  /* compare url including tag */
  if (0 != strcmp (from1->url->host, from2->url->host))
    return OSIP_UNDEFINED_ERROR;
  if (from1->url->username != NULL && from2->url->username != NULL)
    if (0 != strcmp (from1->url->username, from2->url->username))
      return OSIP_UNDEFINED_ERROR;

  tag1 = NULL;
  tag2 = NULL;
  {
    int pos = 0;
    osip_generic_param_t *u_param;

    while (!osip_list_eol (&from1->gen_params, pos)) {
      u_param = (osip_generic_param_t *) osip_list_get (&from1->gen_params, pos);
      if (0 == strncmp (u_param->gname, "tag", 3)) {
        tag1 = u_param->gvalue;
        break;
      }
      pos++;
    }
  }
  {
    int pos = 0;
    osip_generic_param_t *u_param;

    while (!osip_list_eol (&from2->gen_params, pos)) {
      u_param = (osip_generic_param_t *) osip_list_get (&from2->gen_params, pos);
      if (0 == strncmp (u_param->gname, "tag", 3)) {
        tag2 = u_param->gvalue;
        break;
      }
      pos++;
    }
  }

  /* sounds like a BUG!
     if tag2 exists and tag1 does not, then it will
     return OSIP_SUCCESS;
     in the first request, (INVITE) the To field does not
     contain any tag. The response contains one! and the
     response must match the request....
   */
  /* so we test the tags only when both exist! */
  if (tag1 != NULL && tag2 != NULL)
    if (0 != strcmp (tag1, tag2))
      return OSIP_UNDEFINED_ERROR;

  /* We could return a special case, when */
  /* only one tag exists?? */

  return OSIP_SUCCESS;          /* return code changed to 0 from release 0.6.1 */
}

int
__osip_generic_param_parseall (osip_list_t * gen_params, const char *params)
{
  char *pname;
  char *pvalue;

  const char *comma;
  const char *equal;

  /* find '=' wich is the separator for one param */
  /* find ';' wich is the separator for multiple params */

  equal = next_separator (params + 1, '=', ';');
  comma = strchr (params + 1, ';');

  while (comma != NULL) {

    if (equal == NULL) {
      equal = comma;
      pvalue = NULL;
    }
    else {
      const char *tmp;

      /* check for NULL param with an '=' character */
      tmp = equal + 1;
      for (; *tmp == '\t' || *tmp == ' '; tmp++) {
      }
      pvalue = NULL;
      if (*tmp != ',' && *tmp != '\0') {
        if (comma - equal < 2)
          return OSIP_SYNTAXERROR;
        pvalue = (char *) osip_malloc (comma - equal);
        if (pvalue == NULL)
          return OSIP_NOMEM;
        osip_strncpy (pvalue, equal + 1, comma - equal - 1);
      }
    }

    if (equal - params < 2) {
      osip_free (pvalue);
      return OSIP_SYNTAXERROR;
    }
    pname = (char *) osip_malloc (equal - params);
    if (pname == NULL) {
      osip_free (pvalue);
      return OSIP_NOMEM;
    }
    osip_strncpy (pname, params + 1, equal - params - 1);

    osip_generic_param_add (gen_params, pname, pvalue);

    params = comma;
    equal = next_separator (params + 1, '=', ';');
    comma = strchr (params + 1, ';');
  }

  /* this is the last header (comma==NULL) */
  comma = params + strlen (params);

  if (equal == NULL) {
    equal = comma;              /* at the end */
    pvalue = NULL;
    if (equal - params < 2) {
      osip_free (pvalue);
      return OSIP_SUCCESS;      /* empty comma? */
    }
  }
  else {
    const char *tmp;

    /* check for NULL param with an '=' character */
    tmp = equal + 1;
    for (; *tmp == '\t' || *tmp == ' '; tmp++) {
    }
    pvalue = NULL;
    if (*tmp != ',' && *tmp != '\0') {
      if (comma - equal < 2)
        return OSIP_SYNTAXERROR;
      pvalue = (char *) osip_malloc (comma - equal);
      if (pvalue == NULL)
        return OSIP_NOMEM;
      osip_strncpy (pvalue, equal + 1, comma - equal - 1);
    }
  }

  if (equal - params < 2) {
    osip_free (pvalue);
    return OSIP_SYNTAXERROR;
  }
  pname = (char *) osip_malloc (equal - params);
  if (pname == NULL) {
    osip_free (pvalue);
    return OSIP_NOMEM;
  }
  osip_strncpy (pname, params + 1, equal - params - 1);

  osip_generic_param_add (gen_params, pname, pvalue);

  return OSIP_SUCCESS;
}


void
osip_generic_param_set_value (osip_generic_param_t * fparam, char *value)
{
  fparam->gvalue = value;
}

char *
osip_generic_param_get_name (const osip_generic_param_t * fparam)
{
  if (fparam == NULL)
    return NULL;
  return fparam->gname;
}

void
osip_generic_param_set_name (osip_generic_param_t * fparam, char *name)
{
  fparam->gname = name;
}

char *
osip_generic_param_get_value (const osip_generic_param_t * fparam)
{
  if (fparam == NULL)
    return NULL;
  if (fparam->gname == NULL)
    return NULL;                /* name is mandatory */
  return fparam->gvalue;
}

int
osip_from_tag_match (osip_from_t * from1, osip_from_t * from2)
{
  osip_generic_param_t *tag_from1;
  osip_generic_param_t *tag_from2;

  if (from1 == NULL || from2 == NULL)
    return OSIP_BADPARAMETER;

  osip_from_param_get_byname (from1, "tag", &tag_from1);
  osip_from_param_get_byname (from2, "tag", &tag_from2);
  if (tag_from1 == NULL && tag_from2 == NULL)
    return OSIP_SUCCESS;
  if ((tag_from1 != NULL && tag_from2 == NULL)
      || (tag_from1 == NULL && tag_from2 != NULL))
    return OSIP_UNDEFINED_ERROR;
  if (tag_from1->gvalue == NULL || tag_from2->gvalue == NULL)
    return OSIP_UNDEFINED_ERROR;
  if (0 != strcmp (tag_from1->gvalue, tag_from2->gvalue))
    return OSIP_UNDEFINED_ERROR;
  return OSIP_SUCCESS;
}
