/*
    Copyright (C) 2011  Thomas Ries <tries@gmx.net>

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

/* $Id: siproxd.h 482 2011-06-12 18:45:17Z hb9xar $ */

typedef struct {
    void            *next;
    osip_call_id_t  *call_id;
    time_t          ts;
} redirected_cache_element_t;

/* return STS_ codes */
int add_to_redirected_cache(redirected_cache_element_t *redirected_cache,
                            sip_ticket_t *ticket);
int is_in_redirected_cache( redirected_cache_element_t *redirected_cache, 
                            sip_ticket_t *ticket);
int expire_redirected_cache(redirected_cache_element_t *redirected_cache);
