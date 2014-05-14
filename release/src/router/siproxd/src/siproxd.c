/*
    Copyright (C) 2002-2009  Thomas Ries <tries@gmx.net>

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
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#ifdef  HAVE_GETOPT_H
#include <getopt.h>
#endif

#include <osipparser2/osip_parser.h>

#include "siproxd.h"
#include "plugins.h"
#include "log.h"

static char const ident[]="$Id: siproxd.c 482 2011-06-12 18:45:17Z hb9xar $";

/* configuration storage */
struct siproxd_config configuration;
/* instructions for config parser */
static cfgopts_t main_cfg_opts[] = {
   { "debug_level",         TYP_INT4,   &configuration.debuglevel,		{0, NULL} },
   { "debug_port",          TYP_INT4,   &configuration.debugport,		{0, NULL} },
   { "sip_listen_port",     TYP_INT4,   &configuration.sip_listen_port,		{SIP_PORT, NULL} },
   { "daemonize",           TYP_INT4,   &configuration.daemonize,		{0, NULL} },
   { "silence_log",         TYP_INT4,   &configuration.silence_log,		{1, NULL} },
   { "if_inbound",          TYP_STRING, &configuration.inbound_if,		{0, NULL} },
   { "if_outbound",         TYP_STRING, &configuration.outbound_if,		{0, NULL} },
   { "host_outbound",       TYP_STRING, &configuration.outbound_host,		{0, NULL} },
   { "rtp_port_low",        TYP_INT4,   &configuration.rtp_port_low,		{0, NULL} },
   { "rtp_port_high",       TYP_INT4,   &configuration.rtp_port_high,		{0, NULL} },
   { "rtp_timeout",         TYP_INT4,   &configuration.rtp_timeout,		{0, NULL} },
   { "rtp_proxy_enable",    TYP_INT4,   &configuration.rtp_proxy_enable,	{1, NULL} },
   { "rtp_dscp",            TYP_INT4,   &configuration.rtp_dscp,		{0, NULL} },
   { "rtp_input_dejitter",  TYP_INT4,   &configuration.rtp_input_dejitter,	{0, NULL} },
   { "rtp_output_dejitter", TYP_INT4,   &configuration.rtp_output_dejitter,	{0, NULL} },
   { "user",                TYP_STRING, &configuration.user,			{0, NULL} },
   { "chrootjail",          TYP_STRING, &configuration.chrootjail,		{0, NULL} },
   { "hosts_allow_reg",     TYP_STRING, &configuration.hosts_allow_reg,		{0, NULL} },
   { "hosts_allow_sip",     TYP_STRING, &configuration.hosts_allow_sip,		{0, NULL} },
   { "hosts_deny_sip",      TYP_STRING, &configuration.hosts_deny_sip,		{0, NULL} },
   { "proxy_auth_realm",    TYP_STRING, &configuration.proxy_auth_realm,	{0, NULL} },
   { "proxy_auth_passwd",   TYP_STRING, &configuration.proxy_auth_passwd,	{0, NULL} },
   { "proxy_auth_pwfile",   TYP_STRING, &configuration.proxy_auth_pwfile,	{0, NULL} },
   { "mask_host",           TYP_STRINGA,&configuration.mask_host,		{0, NULL} },
   { "masked_host",         TYP_STRINGA,&configuration.masked_host,		{0, NULL} },
   { "outbound_proxy_host", TYP_STRING, &configuration.outbound_proxy_host,	{0, NULL} },
   { "outbound_proxy_port", TYP_INT4,   &configuration.outbound_proxy_port,	{0, NULL} },
   { "outbound_domain_name",TYP_STRINGA,&configuration.outbound_proxy_domain_name,{0, NULL} },
   { "outbound_domain_host",TYP_STRINGA,&configuration.outbound_proxy_domain_host,{0, NULL} },
   { "outbound_domain_port",TYP_STRINGA,&configuration.outbound_proxy_domain_port,{0, NULL} },
   { "registration_file",   TYP_STRING, &configuration.registrationfile,	{0, NULL} },
   { "pid_file",            TYP_STRING, &configuration.pid_file,		{0, NULL} },
   { "default_expires",     TYP_INT4,   &configuration.default_expires,		{DEFAULT_EXPIRES, NULL} },
   { "autosave_registrations",TYP_INT4, &configuration.autosave_registrations,	{0, NULL} },
   { "ua_string",           TYP_STRING, &configuration.ua_string,		{0, NULL} },
   { "use_rport",           TYP_INT4,   &configuration.use_rport,		{0, NULL} },
   { "obscure_loops",       TYP_INT4,   &configuration.obscure_loops,		{0, NULL} },
   { "plugindir",           TYP_STRING, &configuration.plugin_dir,		{0, NULL} },
   { "load_plugin",         TYP_STRINGA,&configuration.load_plugin,		{0, NULL} },
   { "sip_dscp",            TYP_INT4,   &configuration.sip_dscp,		{0, NULL} },
   { "tcp_timeout",         TYP_INT4,   &configuration.tcp_timeout,		{TCP_IDLE_TO, NULL} },
   { "tcp_connect_timeout", TYP_INT4,   &configuration.tcp_connect_timeout,	{TCP_CONNECT_TO, NULL} },
   { "tcp_keepalive",       TYP_INT4,   &configuration.tcp_keepalive,		{0, NULL} },
   { "thread_stack_size",   TYP_INT4,   &configuration.thread_stack_size,	{0, NULL} },
   {0, 0, 0}
};

/* Global File instance on pw file */
FILE *siproxd_passwordfile;

/* -h help option text */
static const char str_helpmsg[] =
PACKAGE "-" VERSION "-" BUILDSTR " (c) 2002-2011 Thomas Ries\n"
"\nUsage: siproxd [options]\n\n"
"options:\n"
#ifdef  HAVE_GETOPT_LONG
"       -h, --help                 help\n"
"       -d, --debug <pattern>      set debug-pattern\n"
"       -c, --config <cfgfile>     use the specified config file\n"
"       -p, --pid-file <pidfile>   create pid file <pidfile>\n"
#else
"       -h              help\n"
"       -d <pattern>    set debug-pattern\n"
"       -c <cfgfile>    use the specified config file\n"
"       -p <pidfile>    create pid file <pidfile>\n"
#endif
"";



/*
 * module local data
 */
static  int dmalloc_dump=0;
static  int exit_program=0;

/*
 * local prototypes
 */
static void sighandler(int sig);


int main (int argc, char *argv[]) 
{
   int sts;
   int i;
   size_t buflen;
   int access;
   char buff[BUFFER_SIZE];
   sip_ticket_t ticket;

   extern char *optarg;         /* Defined in libc getopt and unistd.h */
   int ch1;
   
   char configfile[64]="siproxd";       /* basename of configfile */
   int  config_search=1;                /* search the config file */
   int  cmdline_debuglevel=0;
   char *pidfilename=NULL;
   struct sigaction act;

   log_init();

   log_set_stderr(1);

/*
 * setup signal handlers
 */
   act.sa_handler=sighandler;
   sigemptyset(&act.sa_mask);
   act.sa_flags=SA_RESTART;
   if (sigaction(SIGTERM, &act, NULL)) {
      ERROR("Failed to install SIGTERM handler");
   }
   if (sigaction(SIGINT, &act, NULL)) {
      ERROR("Failed to install SIGINT handler");
   }
   if (sigaction(SIGUSR2, &act, NULL)) {
      ERROR("Failed to install SIGUSR2 handler");
   }
   if (sigaction(SIGPIPE, &act, NULL)) {
      ERROR("Failed to install SIGPIPE handler");
   }


/*
 * prepare default configuration
 */
   memset (&configuration, 0, sizeof(configuration));
   log_set_pattern(0);

/*
 * parse command line
 */
{
#ifdef  HAVE_GETOPT_LONG
   int option_index = 0;
   static struct option long_options[] = {
      {"help", no_argument, NULL, 'h'},
      {"config", required_argument, NULL, 'c'},
      {"debug", required_argument, NULL, 'd'},
      {"pid-file", required_argument, NULL,'p'},
      {0,0,0,0}
   };

    while ((ch1 = getopt_long(argc, argv, "hc:d:p:",
                  long_options, &option_index)) != -1) {
#else   /* ! HAVE_GETOPT_LONG */
    while ((ch1 = getopt(argc, argv, "hc:d:p:")) != -1) {
#endif
      switch (ch1) {
      case 'h': /* help */
         DEBUGC(DBCLASS_CONFIG,"option: help");
         fprintf(stderr,str_helpmsg);
         exit(0);
         break;

      case 'c': /* load config file */
         DEBUGC(DBCLASS_CONFIG,"option: config file=%s",optarg);
         i=sizeof(configfile)-1;
         strncpy(configfile,optarg,i);
         configfile[i]='\0';
         config_search=0;
         break; 

      case 'd': /* set debug level */
         DEBUGC(DBCLASS_CONFIG,"option: set debug level: %s",optarg);
         cmdline_debuglevel=atoi(optarg);
         log_set_pattern(cmdline_debuglevel);
         break;

      case 'p':
         pidfilename = optarg;
         break;

      default:
         DEBUGC(DBCLASS_CONFIG,"no command line options");
         break; 
      }
   }
}

/*
 * Init stuff
 */
   INFO(PACKAGE"-"VERSION"-"BUILDSTR" "UNAME" starting up");

   /* read the config file */
   if (read_config(configfile, config_search, main_cfg_opts, "") == STS_FAILURE) {
      exit(1);
   }
   /* remember where config file is located, so the plugins
    * can load it as well - if required */
   configuration.configfile = strdup(configfile);
   configuration.config_search = config_search;

   /* if a debug level > 0 has been given on the commandline use its
      value and not what is in the config file */
   if (cmdline_debuglevel != 0) {
      configuration.debuglevel=cmdline_debuglevel;
   }

/*
 * open a the pwfile instance, so we still have access after
 * we possibly have chroot()ed to somewhere.
 */
   if (configuration.proxy_auth_pwfile) {
      siproxd_passwordfile = fopen(configuration.proxy_auth_pwfile, "r");
   } else {
      siproxd_passwordfile = NULL;
   }

   /* set debug level as desired */
   log_set_pattern(configuration.debuglevel);
   log_set_listen_port(configuration.debugport);


   /* daemonize if requested to */
   if (configuration.daemonize) {
      DEBUGC(DBCLASS_CONFIG,"daemonizing");
      if (fork()!=0) exit(0);
      setsid();
      if (fork()!=0) exit(0);

      log_set_stderr(0);

      /* detach STDIN/OUT/ERR file */
      i=open("/dev/null", O_RDWR);
      close(STDIN_FILENO);
      close(STDOUT_FILENO);
      close(STDERR_FILENO);
      if (dup2(i, STDIN_FILENO) <0) {
         WARN("detaching STDIN failed: %s",strerror(errno));
      }
      if (dup2(i, STDOUT_FILENO) <0) {
         WARN("detaching STDOUT failed: %s",strerror(errno));
      }
      if (dup2(i, STDERR_FILENO) <0) {
         WARN("detaching STDERR failed: %s",strerror(errno));
      }
      close(i);

      INFO("daemonized, pid=%i", getpid());
   }

   /* load and initialize the plugins */
   sts=load_plugins();
   /* if error, abort siproxd */
   if (sts != STS_SUCCESS) {
      ERROR("Error while loading and initializing plug-ins - aborting");
      exit(1);
   }

   /* prepare for creating PID file */
   if (pidfilename == NULL) pidfilename = configuration.pid_file;

   /* If going to dive into a chroot jail, create a PID file outside
    * the jail, too. However, it will be owned by root and not deleted
    * on process termination... */
   if (configuration.chrootjail && ((getuid()==0) || (geteuid()==0))) {
      if (pidfilename) createpidfile(pidfilename);
   }

   /* change user and group IDs */
   secure_enviroment();

   /* write PID file of main thread as changed siproxd user and
    * possibly into the chroot jail file tree  */
   if (pidfilename) createpidfile(pidfilename);

   /* initialize the RTP proxy */
   sts=rtpproxy_init();
   if (sts != STS_SUCCESS) {
      ERROR("unable to initialize RTP proxy - aborting"); 
      exit(1);
   }

   /* init the oSIP parser */
   parser_init();

   /* listen for incoming messages */
   sts=sipsock_listen();
   if (sts == STS_FAILURE) {
      /* failure to allocate SIP socket... */
      ERROR("unable to bind to SIP listening socket - aborting"); 
      exit(1);
   }

   /* initialize the registration facility */
   register_init();

/*
 * silence the log - if so required...
 */
   log_set_silence(configuration.silence_log);

   INFO(PACKAGE"-"VERSION"-"BUILDSTR" "UNAME" started");

/*****************************
 * Main loop
 *****************************/
   while (!exit_program) {

      while ((sts = sipsock_waitfordata(buff, sizeof(buff)-1,
                                    &ticket.from, &ticket.protocol)) <=0 ) {

         /* allow exit, even if there is no activity... */
         if (exit_program) goto exit_prg;

         if (sts < 0) {
            /* got no input, here by timeout. do aging */
            register_agemap();

            /* TCP log: check for a connection */
            log_tcp_connect();

            /* dump memory stats if requested to do so */
            if (dmalloc_dump) {
               dmalloc_dump=0;
#ifdef DMALLOC
               INFO("SIGUSR2 - DMALLOC statistics is dumped");
               dmalloc_log_stats();
               dmalloc_log_unfreed();
#else
               INFO("SIGUSR2 - DMALLOC support is not compiled in");
#endif
            } /* if dmalloc */

            /* Timer activation of plugins */
            sts = call_plugins(PLUGIN_TIMER, NULL);

         } /* if sts < 0 */

      } /* while sts */

      /*
       * got input, process
       */
      buflen = (size_t)sts;
      DEBUGC(DBCLASS_BABBLE,"received %zd bytes of data", buflen);
      ticket.direction=0;
      buff[buflen]='\0';

      /* pointers in ticket to raw message */
      ticket.raw_buffer=buff;
      ticket.raw_buffer_len=buflen;

      /* Call Plugins for stage: PLUGIN_PROCESS_RAW */
      sts = call_plugins(PLUGIN_PROCESS_RAW, &ticket);
      if (sts == STS_FALSE) continue;

      /*
       * evaluate the access lists (IP based filter)
       */
      access=accesslist_check(ticket.from);
      if (access == 0) {
         DEBUGC(DBCLASS_ACCESS,"access for this packet was denied");
         continue; /* there are no resources to free */
      }

      /*
       * integrity checks
       */
      sts=security_check_raw(buff, buflen);
      if (sts != STS_SUCCESS) {
         DEBUGC(DBCLASS_SIP,"security check (raw) failed");
         continue; /* there are no resources to free */
      }

      /*
       * Hacks to fix-up some broken headers
       */
      sts=sip_fixup_asterisk(buff, &buflen);

      /*
       * init sip_msg
       */
      sts=osip_message_init(&ticket.sipmsg);
      ticket.sipmsg->message=NULL;
      if (sts != 0) {
         ERROR("osip_message_init() failed, sts=%i... this is not good", sts);
         continue; /* skip, there are no resources to free */
      }

      /*
       * RFC 3261, Section 16.3 step 1
       * Proxy Behavior - Request Validation - Reasonable Syntax
       * (parse the received message)
       */
      sts=sip_message_parse(ticket.sipmsg, buff, buflen);
      if (sts != 0) {
         ERROR("sip_message_parse() failed, sts=%i... this is not good", sts);
         DUMP_BUFFER(-1, buff, buflen);
         goto end_loop; /* skip and free resources */
      }

      /*
       * integrity checks - parsed buffer
       */
      sts=security_check_sip(&ticket);
      if (sts != STS_SUCCESS) {
         ERROR("security_check_sip() failed, sts=%i... this is not good", sts);
         DUMP_BUFFER(-1, buff, buflen);
         goto end_loop; /* skip and free resources */
      }

      /*
       * RFC 3261, Section 16.3 step 2
       * Proxy Behavior - Request Validation - URI scheme
       * (check request URI and refuse with 416 if not understood)
       */
      /* NOT IMPLEMENTED */

      /* Call Plugins for stage: PLUGIN_VALIDATE */
      sts = call_plugins(PLUGIN_VALIDATE, &ticket);
      if (sts == STS_FALSE) goto end_loop;

      /*
       * RFC 3261, Section 16.3 step 3
       * Proxy Behavior - Request Validation - Max-Forwards check
       * (check Max-Forwards header and refuse with 483 if too many hops)
       */
      {
         osip_header_t *max_forwards;
         int forwards_count = DEFAULT_MAXFWD;

         osip_message_get_max_forwards(ticket.sipmsg, 0, &max_forwards);
         if (max_forwards && max_forwards->hvalue) {
            forwards_count = atoi(max_forwards->hvalue);
            if ((forwards_count<0)||
                (forwards_count>255)) forwards_count=DEFAULT_MAXFWD;
         }

         DEBUGC(DBCLASS_PROXY,"checking Max-Forwards (=%i)",forwards_count);
         if (forwards_count <= 0) {
            DEBUGC(DBCLASS_SIP, "Forward count reached 0 -> 483 response");
            sip_gen_response(&ticket, 483 /*Too many hops*/);
            goto end_loop; /* skip and free resources */
         }
      }

      /*
       * RFC 3261, Section 16.3 step 4
       * Proxy Behavior - Request Validation - Loop Detection check
       * (check for loop and return 482 if a loop is detected)
       */
      if (check_vialoop(&ticket) == STS_TRUE) {
         /* make sure we don't end up in endless loop when detecting
          * an loop in an "loop detected" message - brrr */
         if (MSG_IS_RESPONSE(ticket.sipmsg) && 
             MSG_TEST_CODE(ticket.sipmsg, 482)) {
            DEBUGC(DBCLASS_SIP,"loop in loop-response detected, ignoring");
         } else {
            DEBUGC(DBCLASS_SIP,"via loop detected, ignoring request");
            sip_gen_response(&ticket, 482 /*Loop detected*/);
         }
         goto end_loop; /* skip and free resources */
      }

      /*
       * RFC 3261, Section 16.3 step 5
       * Proxy Behavior - Request Validation - Proxy-Require check
       * (check Proxy-Require header and return 420 if unsupported option)
       */
      /* NOT IMPLEMENTED */

      /*
       * RFC 3261, Section 16.5
       * Proxy Behavior - Determining Request Targets
       */
      /* NOT IMPLEMENTED */

      DEBUGC(DBCLASS_SIP,"received SIP type %s:%s",
             (MSG_IS_REQUEST(ticket.sipmsg))? "REQ" : "RES",
             (MSG_IS_REQUEST(ticket.sipmsg) ?
                ((ticket.sipmsg->sip_method)?
                   ticket.sipmsg->sip_method : "NULL") :
                ((ticket.sipmsg->reason_phrase) ? 
                   ticket.sipmsg->reason_phrase : "NULL")));

      /*********************************
       * Call Plugins for stage: PLUGIN_DETERMINE_TARGET
       * The message did pass all the
       * tests above and is now ready
       * to be proxied.
       * Feed to the plugins. If a plugin decides
       * to end processing and terminate the ongoing
       * dialog (STS_SIP_SENT), then just free
       * the allocated resources.
       *********************************/
      sts = call_plugins(PLUGIN_DETERMINE_TARGET, &ticket);
      if (sts == STS_SIP_SENT) goto end_loop;


      /*********************************
       * finally proxy the message.
       * This includes the masquerading
       * of the local UA and starting/
       * stopping the RTP proxy for this
       * call
       *********************************/

      /*
       * if a REQ REGISTER, check if it is directed to myself,
       * or am I just the outbound proxy but no registrar.
       * - If I'm the registrar, register & generate answer
       * - If I'm just the outbound proxy, register, rewrite & forward
       */
      if (MSG_IS_REGISTER(ticket.sipmsg) && 
          MSG_IS_REQUEST(ticket.sipmsg)) {
         if (access & ACCESSCTL_REG) {
            osip_uri_t *url;
            struct in_addr addr1, addr2, addr3;
            int dest_port;

            url = osip_message_get_uri(ticket.sipmsg);
            dest_port= (url->port)?atoi(url->port):SIP_PORT;
            if ((dest_port <=0) || (dest_port >65535)) dest_port=SIP_PORT;

            if ( (get_ip_by_host(url->host, &addr1) == STS_SUCCESS) &&
                 (get_interface_ip(IF_INBOUND,&addr2) == STS_SUCCESS) &&
                 (get_interface_ip(IF_OUTBOUND,&addr3) == STS_SUCCESS)) {

               if ((configuration.sip_listen_port == dest_port) &&
                   ((memcmp(&addr1, &addr2, sizeof(addr1)) == 0) ||
                    (memcmp(&addr1, &addr3, sizeof(addr1)) == 0))) {
                  /* I'm the registrar, send response myself */
                  sts = register_client(&ticket, 0);
                  sts = register_response(&ticket, sts);
               } else {
                  /* I'm just the outbound proxy */
                  DEBUGC(DBCLASS_SIP,"proxying REGISTER request to:%s",
                         url->host);
                  sts = register_client(&ticket, 1);
                  if (sts == STS_SUCCESS) {
                     sts = proxy_request(&ticket);
                  }
               }
            } else {
               sip_gen_response(&ticket, 408 /*request timeout*/);
            }
         } else {
            WARN("non-authorized registration attempt from %s",
                 utils_inet_ntoa(ticket.from.sin_addr));
         }

      /*
       * check if outbound interface is UP.
       * If not, send back error to UA and
       * skip any proxying attempt
       */
      } else if (get_interface_ip(IF_OUTBOUND,NULL) !=
                 STS_SUCCESS) {
         DEBUGC(DBCLASS_SIP, "got a %s to proxy, but outbound interface "
                "is down", (MSG_IS_REQUEST(ticket.sipmsg))? "REQ" : "RES");

         if (MSG_IS_REQUEST(ticket.sipmsg))
            sip_gen_response(&ticket, 408 /*request timeout*/);
      
      /*
       * MSG is a request, add current via entry,
       * do a lookup in the URLMAP table and
       * send to the final destination
       */
      } else if (MSG_IS_REQUEST(ticket.sipmsg)) {
         if (access & ACCESSCTL_SIP) {
            sts = proxy_request(&ticket);
         } else {
            INFO("non-authorized request received from %s",
                 utils_inet_ntoa(ticket.from.sin_addr));
         }

      /*
       * MSG is a response, remove current via and
       * send to the next VIA in chain
       */
      } else if (MSG_IS_RESPONSE(ticket.sipmsg)) {
         if (access & ACCESSCTL_SIP) {
            sts = proxy_response(&ticket);
         } else {
            INFO("non-authorized response received from %s",
                 utils_inet_ntoa(ticket.from.sin_addr));
         }
         
      /*
       * unsupported message
       */
      } else {
         ERROR("received unsupported SIP type %s %s",
               (MSG_IS_REQUEST(ticket.sipmsg))? "REQ" : "RES",
               ticket.sipmsg->sip_method);
      }

      /*********************************
       * Done with proxying. Message
       * has been sent to its destination.
       *********************************/
/*
 * free the SIP message buffers
 */
      end_loop:
      osip_message_free(ticket.sipmsg);

   } /* while TRUE */
   exit_prg:

   /* save current known SIP registrations */
   register_save();
   INFO("properly terminating siproxd");

   /* remove PID file */
   if (pidfilename) {
      DEBUGC(DBCLASS_CONFIG,"deleting PID file [%s]", pidfilename);
      sts=unlink(pidfilename);
      if (sts != 0) {
         WARN("couldn't delete old PID file: %s", strerror(errno));
      }
   }

   /* unload the plugins */
   unload_plugins();

   /* END */
   log_end();
   return 0;
} /* main */

/*
 * Signal handler
 *
 * this one is called asynchronously whevener a registered
 * signal is applied. Just set a flag and don't do any funny
 * things here.
 */
static void sighandler(int sig) {
   if (sig==SIGTERM) exit_program=1;
   if (sig==SIGINT)  exit_program=1;
   if (sig==SIGUSR2) dmalloc_dump=1;
   return;
}
