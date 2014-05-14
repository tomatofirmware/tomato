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
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <limits.h>
#include <time.h>
#include <signal.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <net/if.h>
#include <sys/ioctl.h>

#ifdef HAVE_GETIFADDRS
# include <ifaddrs.h>
#endif

#ifdef _SOLARIS2
# include <sys/sockio.h>
#endif

#include <sys/types.h>
#include <pwd.h>

#include <osipparser2/osip_parser.h>

#include "siproxd.h"
#include "log.h"

static char const ident[]="$Id: utils.c 481 2011-06-12 17:53:48Z hb9xar $";

/* configuration storage */
extern struct siproxd_config configuration;

extern int h_errno;


/*
 * resolve a hostname and return in_addr
 * handles its own little DNS cache.
 *
 * RETURNS
 *	STS_SUCCESS on success
 *	STS_FAILURE on failure
 */
int get_ip_by_host(char *hostname, struct in_addr *addr) {
   int i, j, k, idx;
   time_t t1, t2;
   struct hostent *hostentry;
#if defined(HAVE_GETHOSTBYNAME_R)
   struct hostent result_buffer;
   char tmp[GETHOSTBYNAME_BUFLEN];
#endif
   int error;
   static struct {
      time_t expires_timestamp;	/* time of expiration */
      struct in_addr addr;	/* IP address or 0.0.0.0 if a bad entry */
      char   error_count;	/* counts failed resolution attempts */
      char   bad_entry;		/* != 0 if resolving failed */
      char hostname[HOSTNAME_SIZE+1];
   } dns_cache[DNS_CACHE_SIZE];
   static int cache_initialized=0;

   if (hostname == NULL) {
      ERROR("get_ip_by_host: NULL hostname requested");
      return STS_FAILURE;
   }

   if (addr == NULL) {
      ERROR("get_ip_by_host: NULL in_addr passed");
      return STS_FAILURE;
   }

   /* first time: initialize DNS cache */
   if (cache_initialized == 0) {
      DEBUGC(DBCLASS_DNS, "initializing DNS cache (%i entries)", DNS_CACHE_SIZE);
      memset(dns_cache, 0, sizeof(dns_cache));
      cache_initialized=1;
   }

   time(&t1);
   /* clean expired entries */
   for (i=0; i<DNS_CACHE_SIZE; i++) {
      if (dns_cache[i].hostname[0]=='\0') continue;
      if ( (dns_cache[i].expires_timestamp) < t1 ) {
         DEBUGC(DBCLASS_DNS, "cleaning DNS cache (entry %i)", i);
         memset (&dns_cache[i], 0, sizeof(dns_cache[0]));
      }
   }

   /*
    * search requested entry in cache
    */
   idx=0;
   for (i=0; i<DNS_CACHE_SIZE; i++) {
      if (dns_cache[i].hostname[0]=='\0') continue; /* empty */
      if (strcasecmp(hostname, dns_cache[i].hostname) == 0) { /* match */
         memcpy(addr, &dns_cache[i].addr, sizeof(struct in_addr));
         if (dns_cache[i].bad_entry) {
            DEBUGC(DBCLASS_DNS, "DNS lookup - blacklisted from cache: %s",
                   hostname);
            return STS_FAILURE;
         }
         if (dns_cache[i].error_count > 0) {
            DEBUGC(DBCLASS_DNS, "DNS lookup - previous resolution failed: %s"
                   ", attempt %i", hostname, dns_cache[i].error_count);
            idx=i;
            break;
         }
         DEBUGC(DBCLASS_DNS, "DNS lookup - from cache: %s -> %s",
                hostname, utils_inet_ntoa(*addr));
         return STS_SUCCESS;
      }
   }
   
   /* I did not find it in cache, so I have to resolve it */
   error = 0;

   /* need to deal with reentrant versions of gethostbyname_r()
    * as we may use threads... */
#if defined(HAVE_GETHOSTBYNAME_R)

   /* gethostbyname_r() with 3 arguments (e.g. osf/1) */
   #if defined(HAVE_FUNC_GETHOSTBYNAME_R_3)
   gethostbyname_r(hostname,		/* the FQDN */
		   &result_buffer,	/* the result buffer */ 
		   &hostentry
		   );
   if (hostentry == NULL) error = h_errno;

   /* gethostbyname_r() with 5 arguments (e.g. solaris, linux libc5) */
   #elif defined(HAVE_FUNC_GETHOSTBYNAME_R_5)
   hostentry = gethostbyname_r(hostname,        /* the FQDN */
			       &result_buffer,  /* the result buffer */
			       tmp,
			       GETHOSTBYNAME_BUFLEN,
			       &error);

   /* gethostbyname_r() with 6 arguments (e.g. linux glibc) */
   #elif defined(HAVE_FUNC_GETHOSTBYNAME_R_6)
   gethostbyname_r(hostname,        /* the FQDN */
		   &result_buffer,  /* the result buffer */
		   tmp,
		   GETHOSTBYNAME_BUFLEN,
		   &hostentry,
		   &error);
   #else
      #error "gethostbyname_r() with 3, 5 or 6 arguments supported only"
   #endif   
#elif defined(HAVE_GETHOSTBYNAME)
   hostentry=gethostbyname(hostname);
   if (hostentry == NULL) error = h_errno;
#else
   #error "need gethostbyname() or gethostbyname_r()"
#endif
   /* Here I have 'hostentry' and 'error' */


   if (hostentry==NULL) {
      /*
       * Some errors just tell us that there was no IP resolvable.
       * From the manpage:
       *   HOST_NOT_FOUND
       *      The specified host is unknown.
       *   NO_ADDRESS or NO_DATA
       *      The requested name is valid but does not have an IP
       *      address.
       */
      if ((error == HOST_NOT_FOUND) ||
          (error == NO_ADDRESS) ||
          (error == NO_DATA)) {
#ifdef HAVE_HSTRERROR
         DEBUGC(DBCLASS_DNS, "gethostbyname(%s) failed: h_errno=%i [%s]",
                hostname, h_errno, hstrerror(error));
#else
         DEBUGC(DBCLASS_DNS, "gethostbyname(%s) failed: h_errno=%i",
                hostname, error);
#endif
      } else {
#ifdef HAVE_HSTRERROR
         ERROR("gethostbyname(%s) failed: h_errno=%i [%s]",
               hostname, h_errno, hstrerror(h_errno));
#else
         ERROR("gethostbyname(%s) failed: h_errno=%i",hostname, h_errno);
#endif
      }
   }

   if (hostentry) {
      memcpy(addr, hostentry->h_addr, sizeof(struct in_addr));
      DEBUGC(DBCLASS_DNS, "DNS lookup - resolved: %s -> %s",
             hostname, utils_inet_ntoa(*addr));
   }

   /* if we already have the entry, skip finding a new empty one */
   if (idx == 0) {
      /*
       * find an empty slot in the cache
       */
      j=0;
      k=0;
      t1=INT_MAX;
      t2=INT_MAX;
      for (i=0; i<DNS_CACHE_SIZE; i++) {
         if (dns_cache[i].hostname[0]=='\0') break;
         if ((dns_cache[i].expires_timestamp < t1) &&
             (dns_cache[i].bad_entry == 0)) {
            /* remember oldest good entry */
            t1=dns_cache[i].expires_timestamp;
            j=i;
         } else 
         if (dns_cache[i].expires_timestamp < t2) {
            /* remember oldest bad entry */
            t2=dns_cache[i].expires_timestamp;
            k=i;
         }
      }
      /* if no empty slot found, victimize oldest one.
       * Give preference to the oldest "bad" entry if 
       * one exists */
      if (i >= DNS_CACHE_SIZE) {
         if (k > 0) i=k;
         else       i=j;
      }
      idx=i;
      memset(&dns_cache[idx], 0, sizeof(dns_cache[0]));
   }

   /*
    * store the result in the cache
    */
   DEBUGC(DBCLASS_DNS, "DNS lookup - store into cache, entry %i)", idx);
   strncpy(dns_cache[idx].hostname, hostname, HOSTNAME_SIZE);
   dns_cache[idx].expires_timestamp = time(NULL) + DNS_GOOD_AGE;
   if (hostentry) {
      memcpy(&dns_cache[idx].addr, addr, sizeof(struct in_addr));
      dns_cache[idx].error_count = 0;
      dns_cache[idx].bad_entry = 0;
   } else {
      dns_cache[idx].error_count++;
      DEBUGC(DBCLASS_DNS, "DNS lookup - errcnt=%i", dns_cache[idx].error_count);
      if (dns_cache[idx].error_count >= DNS_ATTEMPTS) {
         DEBUGC(DBCLASS_DNS, "DNS lookup - blacklisting entry");
         dns_cache[idx].expires_timestamp = time(NULL) + DNS_BAD_AGE;
         dns_cache[idx].bad_entry = 1;
      }
      return STS_FAILURE;
   }
   return STS_SUCCESS;
}


/*
 * Secure enviroment:
 * If running as root, put myself into a chroot jail and
 * change UID/GID to user as requested in config file
 */
void secure_enviroment (void) {
   int sts;
   struct passwd *passwd=NULL;

   DEBUGC(DBCLASS_CONFIG,"running w/uid=%i, euid=%i, gid=%i, egid=%i",
          (int)getuid(), (int)geteuid(), (int)getgid(), (int)getegid());

   if ((getuid()==0) || (geteuid()==0)) {
      /*
       * preparation - after chrooting there will be NOTHING more around
       */
      if (configuration.user) passwd=getpwnam(configuration.user);

      /*
       * change root directory into chroot jail
       */
      if (configuration.chrootjail) {
         /* !!!
          * Before chrooting I must at least once trigger the resolver
          * as it loads some dynamic libraries. Once chrootet
          * these libraries will *not* be found and gethostbyname()
          * calls will simply fail (return NULL pointer and h_errno=0).
          * Also (at least for FreeBSD) syslog() needs to be called
          * before chroot()ing - this is done in main() by an INFO().
          * Took me a while to figure THIS one out
          */
         struct in_addr dummy;
         get_ip_by_host("localhost", &dummy);
         DEBUGC(DBCLASS_CONFIG,"chrooting to %s",
                configuration.chrootjail);
         sts = chroot(configuration.chrootjail);
	 if (sts != 0) DEBUGC(DBCLASS_CONFIG,"chroot(%s) failed: %s",
	                      configuration.chrootjail, strerror(errno));
         sts=chdir("/");
      }

      /*
       * change user ID and group ID 
       */
      if (passwd) {
         DEBUGC(DBCLASS_CONFIG,"changing uid/gid to %s",
                configuration.user);
         sts = setgid(passwd->pw_gid);
         DEBUGC(DBCLASS_CONFIG,"changed gid to %i - %s",
	        (int)passwd->pw_gid, (sts==0)?"Ok":"Failed");

         sts = setegid(passwd->pw_gid);
         DEBUGC(DBCLASS_CONFIG,"changed egid to %i - %s",
	        (int)passwd->pw_gid, (sts==0)?"Ok":"Failed");

         sts = seteuid(passwd->pw_uid);
         DEBUGC(DBCLASS_CONFIG,"changed euid to %i - %s",
	        (int)passwd->pw_uid, (sts==0)?"Ok":"Failed");
      }
   }
}


/*
 * get_interface_ip:
 * fetches own IP address by interface INBOUND/OUTBOUND
 * takes into account a possible outbound_host setting.
 *
 * STS_SUCCESS on returning a valid IP and interface is UP
 * STS_FAILURE if interface is DOWN or other problem
 */
int  get_interface_ip(int interface, struct in_addr *retaddr) {
   int sts=STS_FAILURE;

   if ((interface == IF_OUTBOUND) && 
              (configuration.outbound_host) &&
              (strcmp(configuration.outbound_host, "")!=0)) {
      DEBUGC(DBCLASS_DNS, "fetching outbound IP by HOSTNAME");
      if (retaddr) {
         sts = get_ip_by_host(configuration.outbound_host, retaddr);
      } else {
         sts = STS_SUCCESS;
      }

   } else  {
      sts = get_interface_real_ip(interface, retaddr);
   }

   return sts;
}


/*
 * get_interface_real_ip:
 * fetches the real IP address of my interface INBOUND/OUTBOUND
 *
 * STS_SUCCESS on returning a valid IP and interface is UP
 * STS_FAILURE if interface is DOWN or other problem
 */
int  get_interface_real_ip(int interface, struct in_addr *retaddr) {
   int sts=STS_FAILURE;
   char *tmp=NULL;

      if (interface == IF_INBOUND) {
         tmp = configuration.inbound_if;
      } else if (interface == IF_OUTBOUND) {
         tmp = configuration.outbound_if;
      }

   if (tmp && (strcmp(tmp, "")!=0)) {
      DEBUGC(DBCLASS_DNS, "fetching interface IP by INTERFACE [%i]", interface);
      sts = get_ip_by_ifname(tmp, retaddr);
      if (sts != STS_SUCCESS) {
         ERROR("can't find interface %s - configuration error?", tmp);
      }

   } else {
      ERROR("Don't know what interface to look for - configuration error?");
   }

   return sts;
}


/*
 * get_ip_by_ifname:
 * fetches own IP address by its interface name
 *
 * STS_SUCCESS on returning a valid IP and interface is UP
 * STS_FAILURE if interface is DOWN or other problem
 */
int get_ip_by_ifname(char *ifname, struct in_addr *retaddr) {
   struct in_addr ifaddr; /* resulting IP */
   int i, j;
   int ifflags=0, isup=0;
   time_t t;
   static struct {
      time_t timestamp;
      struct in_addr ifaddr;	/* IP */
      int isup;			/* interface is UP */
      char ifname[IFNAME_SIZE+1];
   } ifaddr_cache[IFADR_CACHE_SIZE];
   static int cache_initialized=0;
#ifdef HAVE_GETIFADDRS
   struct ifaddrs *ifa;
   struct ifaddrs *ifa_list;
#else
   struct ifreq ifr;
   struct sockaddr_in *sin = (struct sockaddr_in *)&ifr.ifr_addr;
   int sockfd;
#endif

   if (ifname == NULL) {
      WARN("get_ip_by_ifname: got NULL ifname passed - please check config"
           "file ('if_inbound' and 'if_outbound')");
      return STS_FAILURE;
   }

   /* first time: initialize ifaddr cache */
   if (cache_initialized == 0) {
      DEBUGC(DBCLASS_DNS, "initializing ifaddr cache (%i entries)", 
             IFADR_CACHE_SIZE);
      memset(ifaddr_cache, 0, sizeof(ifaddr_cache));
      cache_initialized=1;
   }

   if (retaddr) memset(retaddr, 0, sizeof(struct in_addr));

   time(&t);
   /* clean expired entries */
   for (i=0; i<IFADR_CACHE_SIZE; i++) {
      if (ifaddr_cache[i].ifname[0]=='\0') continue;
      if ( (ifaddr_cache[i].timestamp+IFADR_MAX_AGE) < t ) {
         DEBUGC(DBCLASS_DNS, "cleaning ifaddr cache (entry %i)", i);
         memset (&ifaddr_cache[i], 0, sizeof(ifaddr_cache[0]));
      }
   }

   /*
    * search requested entry in cache
    */
   for (i=0; i<IFADR_CACHE_SIZE; i++) {
      if (ifaddr_cache[i].ifname[0]=='\0') continue;
      if (strcmp(ifname, ifaddr_cache[i].ifname) == 0) { /* match */
         if (retaddr) memcpy(retaddr, &ifaddr_cache[i].ifaddr,
                             sizeof(struct in_addr));
         DEBUGC(DBCLASS_DNS, "ifaddr lookup - from cache: %s -> %s %s",
	        ifname, utils_inet_ntoa(ifaddr_cache[i].ifaddr),
                (ifaddr_cache[i].isup)? "UP":"DOWN");
         return (ifaddr_cache[i].isup)? STS_SUCCESS: STS_FAILURE;
      } /* if */
   } /* for i */

   /* not found in cache, go and get it */

#ifdef HAVE_GETIFADDRS
   if (getifaddrs(&ifa_list)) {
      ERROR("Error in getifaddrs: %s",strerror(errno));
      return STS_FAILURE;
   }

   i=0; /* use "found" marker */
   for (ifa = ifa_list; ifa != NULL; ifa = ifa->ifa_next) {
      DEBUGC(DBCLASS_BABBLE,"getifaddrs - %s / %s, ifa_addr=%p, addrfamily=%i",
             ifname, ifa->ifa_name,ifa->ifa_addr,
             (ifa->ifa_addr)?ifa->ifa_addr->sa_family:-1);

      if (ifa && ifa->ifa_name && ifa->ifa_addr && 
          ifa->ifa_addr->sa_family == AF_INET &&
          strcmp(ifa->ifa_name, ifname) == 0) {
         /* found the entry */
         i=1;
         memcpy(&ifaddr, &((struct sockaddr_in*)ifa->ifa_addr)->sin_addr, sizeof(struct in_addr));
         ifflags=ifa->ifa_flags;
         DEBUGC(DBCLASS_BABBLE,"getifaddrs - MATCH, sin_addr=%s",
                utils_inet_ntoa(ifaddr));
         break;
      }
   }
   freeifaddrs(ifa_list);

   if (i==0) {
      DEBUGC(DBCLASS_DNS,"Interface %s not found.", ifname);
      return STS_FAILURE;
   }

#else
   memset(&ifr, 0, sizeof(ifr));

   if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
      ERROR("Error in socket: %s",strerror(errno));
      return STS_FAILURE;
   }

/*&&&*/DEBUGC(DBCLASS_BABBLE,"&&&6 ifname=0x%p",ifname);
   strcpy(ifr.ifr_name, ifname);
   sin->sin_family = AF_INET;

   /* get interface flags */
   if(ioctl(sockfd, SIOCGIFFLAGS, &ifr) != 0) {
      ERROR("Error in ioctl SIOCGIFFLAGS: %s [%s]",
            strerror(errno), ifname);
      close(sockfd);
      return STS_FAILURE;
   } 
   ifflags=ifr.ifr_flags;
/*&&&*/DEBUGC(DBCLASS_BABBLE,"&&&7 ifname=0x%p",ifname);

   /* get address */
   if(ioctl(sockfd, SIOCGIFADDR, &ifr) != 0) {
      ERROR("Error in ioctl SIOCGIFADDR: %s (interface %s)",
      strerror(errno), ifname);
      close(sockfd);
      return STS_FAILURE;
   }

/*&&&*/DEBUGC(DBCLASS_BABBLE,"&&&8 ifname=0x%p",ifname);
   memcpy(&ifaddr, &sin->sin_addr, sizeof(struct in_addr));
/*&&&*/DEBUGC(DBCLASS_BABBLE,"&&&9 ifname=0x%p",ifname);
   close(sockfd);
#endif

   if (ifflags & IFF_UP) isup=1;
   else isup=0;

   DEBUGC(DBCLASS_DNS, "get_ip_by_ifname: if %s has IP:%s (flags=%x) %s",
          ifname, utils_inet_ntoa(ifaddr), ifflags,
          (isup)? "UP":"DOWN");

   /*
    *find an empty slot in the cache
    */
   j=0;
   for (i=0; i<IFADR_CACHE_SIZE; i++) {
      if (ifaddr_cache[i].ifname[0]=='\0') break;
      if (ifaddr_cache[i].timestamp < t) {
         /* remember oldest entry */
         t=ifaddr_cache[i].timestamp;
	 j=i;
      }
   }
   /* if no empty slot found, take oldest one */
   if (i >= IFADR_CACHE_SIZE) i=j;

   /*
    * store the result in the cache
    */
   DEBUGC(DBCLASS_DNS, "ifname lookup - store into cache, entry %i)", i);
   memset(&ifaddr_cache[i], 0, sizeof(ifaddr_cache[0]));
   strncpy(ifaddr_cache[i].ifname, ifname, IFNAME_SIZE);
   ifaddr_cache[i].timestamp=t;
   memcpy(&ifaddr_cache[i].ifaddr, &ifaddr, sizeof(struct in_addr));
   ifaddr_cache[i].isup=isup;

   if (retaddr) memcpy(retaddr, &ifaddr, sizeof(struct in_addr));

   return (isup)? STS_SUCCESS : STS_FAILURE;
}


/*
 * utils_inet_ntoa:
 * implements an inet_ntoa()
 *
 * Returns pointer to a STATIC character string.
 * NOte: BE AWARE OF THE STATIC NATURE of the string! Never pass it as
 * calling argument to a function and use it immediately or strcpy()
 * it into a buffer.
 * !! Any subsequent call to this function will DESTROY the previous
 * !! value - and may result in very strange effects like magically
 * !! changing variable value (that has been passed to a function)
 * Been there, seen that, so TAKE CARE!
 */
char *utils_inet_ntoa(struct in_addr in) {
#if defined(HAVE_INET_NTOP)
   static char string[INET_ADDRSTRLEN];
   if ((inet_ntop(AF_INET, &in, string, INET_ADDRSTRLEN)) == NULL) {
      ERROR("inet_ntop() failed: %s",strerror(errno));
      string[0]='\0';
   }
   return string;
#elif defined(HAVE_INET_NTOA)
   return inet_ntoa(in);
#else
#error "need inet_ntop() or inet_ntoa()"
#endif
}


/*
 * utils_inet_aton:
 * implements an inet_aton()
 *
 * converts the string in *cp and stores it into inp
 * Returns != 0 on success
 */
int  utils_inet_aton(const char *cp, struct in_addr *inp) {
#if defined(HAVE_INET_PTON)
   return inet_pton (AF_INET, cp, inp);
#elif defined(HAVE_INET_ATON)
   return inet_aton(cp, inp);
#else
#error "need inet_pton() or inet_aton()"
#endif
}

/*
 * Create the PID file
 */
int createpidfile(char *pidfilename) {
   FILE *f = NULL;
   int sts;
   DEBUGC(DBCLASS_CONFIG,"creating PID file [%s]", pidfilename);
   sts=unlink(pidfilename);
   if ((sts==0) || (errno == ENOENT)) {
      if ((f=fopen(pidfilename, "w"))) {
         fprintf(f,"%i\n",(int)getpid());
         fclose(f);
      } else {
         WARN("couldn't create new PID file: %s", strerror(errno));
         return STS_FAILURE;
      }
   } else {
      WARN("couldn't delete old PID file: %s", strerror(errno));
      return STS_FAILURE;
   }
   return STS_SUCCESS;
}


/*
 * compare_client_id:
 * Compares two client_id_t structures. If both have the Contact item
 * defined (not NULL), then compare it and return.
 * If one (or both) do NOT have the contact item defined, then
 * fall back on comparing the from_ip (IP address).
 *
 * returns:
 * STS_SUCCESS on match
 * STS_FAILURE on no match
 */
int  compare_client_id(client_id_t cid1, client_id_t cid2) {

   /* Prio 1: Contact - if present in both structures */
   if ((cid1.idstring[0] != '\0') && (cid2.idstring[0] != '\0')) {
      if (strncmp(cid1.idstring, cid2.idstring, CLIENT_ID_SIZE) == 0) {
         DEBUGC(DBCLASS_BABBLE, "compare_client_id: contact match [%s]",
                cid1.idstring);
         return STS_SUCCESS;
      }
      DEBUGC(DBCLASS_BABBLE, "compare_client_id: contact NO match [%s<->%s]",
             cid1.idstring, cid2.idstring);
      return STS_FAILURE;
   }

   /* Prio 2: IP (always present) - fallback, if no ID string present. */
   if (memcmp(&cid1.from_ip, &cid2.from_ip, sizeof(struct in_addr)) == 0) {
      DEBUGC(DBCLASS_BABBLE, "compare_client_id: IP match [%s]",
             utils_inet_ntoa(cid1.from_ip));
      return STS_SUCCESS;
   }

   DEBUGC(DBCLASS_BABBLE, "compare_client_id: no match");
   return STS_FAILURE;
}
