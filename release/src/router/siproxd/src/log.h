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

/* $Id: log.h 369 2008-01-19 16:07:14Z hb9xar $ */

#include <stdarg.h>

#define DBCLASS_BABBLE	0x00000001	/* babble (like entering/leaving fnc)*/
#define DBCLASS_NET	0x00000002	/* network			     */
#define DBCLASS_SIP	0x00000004	/* SIP manipulations		     */
#define DBCLASS_REG	0x00000008	/* Client registration		     */
#define DBCLASS_NOSPEC	0x00000010	/* non specified class		     */
#define DBCLASS_PROXY	0x00000020	/* proxy			     */
#define DBCLASS_DNS	0x00000040	/* DNS stuff			     */
#define DBCLASS_NETTRAF	0x00000080	/* network traffic		     */
#define DBCLASS_CONFIG	0x00000100	/* configuration		     */
#define DBCLASS_RTP	0x00000200	/* RTP proxy			     */
#define DBCLASS_ACCESS	0x00000400	/* Access list evaluation	     */
#define DBCLASS_AUTH	0x00000800	/* Authentication		     */
#define DBCLASS_PLUGIN	0x00001000	/* Plugins			     */
#define DBCLASS_RTPBABL	0x00002000	/* RTP babble			     */
#define DBCLASS_ALL	0xffffffff	/* All classes			     */

void log_init(void);
void log_end(void);

void log_set_pattern(unsigned int pattern);
unsigned int  log_get_pattern(void);
void log_set_stderr(int tostdout);
void log_set_silence(int level);
void log_set_listen_port(int port);
void log_tcp_listen(void);
void log_tcp_connect(void);

#if __GNUC__ > 2 || (__GNUC__ == 2 && __GNUC_MINOR__ > 4)
#  define GNUC_PRINTF(format_idx, arg_idx) \
     __attribute__((__format__ (__printf__, format_idx, arg_idx)))
#else
#  define GNUC_PRINTF(format_idx, arg_idx)
#endif

#undef DEBUG
#define DEBUG(F...) log_debug(1,__FILE__, __LINE__,F)

#define DEBUGC(C,F...) log_debug(C,__FILE__, __LINE__,F)
void log_debug(unsigned int class, char *file, int line, const char *format, ...) GNUC_PRINTF(4, 5);

#define ERROR(F...) log_error(__FILE__, __LINE__,F)
void log_error(char *file, int line, const char *format, ...) GNUC_PRINTF(3, 4);

#define WARN(F...) log_warn(__FILE__, __LINE__,F)
void log_warn(char *file, int line, const char *format, ...) GNUC_PRINTF(3, 4);

#define INFO(F...) log_info(__FILE__, __LINE__,F)
void log_info(char *file, int line, const char *format, ...) GNUC_PRINTF(3, 4);

/* tobedone: dump a buffer */
#define DUMP_BUFFER(C,F,L) log_dump_buffer(C,__FILE__, __LINE__,F,L)
void log_dump_buffer(unsigned int class, char *file, int line,
                     char *buffer, int length);
