/*
    Copyright (C) 2008  Thomas Ries <tries@gmx.net>

    This file is part of Siproxd.

    Siproxd is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    Siproxd is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warrantry of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Siproxd; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA 
*/

/* must be defined before including <plugin.h> */
#define PLUGIN_NAME	plugin_demo

#include "config.h"

#include <string.h>

#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <osipparser2/osip_parser.h>

#include "siproxd.h"
#include "plugins.h"
#include "log.h"


static char const ident[]="$Id: plugin_demo.c 440 2010-01-07 11:30:27Z hb9xar $";

/* Plug-in identification */
static char name[]="plugin_demo";
static char desc[]="This is just a demo plugin without any purpose";

/* global configuration storage - required for config file location */
extern struct siproxd_config configuration;

/* plugin configuration storage */
static struct plugin_config {
   char *string;
} plugin_cfg;

/* Instructions for config parser */
static cfgopts_t plugin_cfg_opts[] = {
   { "plugin_demo_string",      TYP_STRING, &plugin_cfg.string,			{0, NULL} },
   {0, 0, 0}
};


/* 
 * Initialization.
 * Called once suring siproxd startup.
 */
int  PLUGIN_INIT(plugin_def_t *plugin_def) {
   /* API version number of siproxd that this plugin is built against.
    * This constant will change whenever changes to the API are made
    * that require adaptions in the plugin. */
   plugin_def->api_version=SIPROXD_API_VERSION;

   /* Name and descriptive text of the plugin */
   plugin_def->name=name;
   plugin_def->desc=desc;

   /* Execution mask - during what stages of SIP processing shall
    * the plugin be called. */
   plugin_def->exe_mask=PLUGIN_DETERMINE_TARGET|PLUGIN_PRE_PROXY;

   /* read the config file */
   if (read_config(configuration.configfile,
                   configuration.config_search,
                   plugin_cfg_opts, name) == STS_FAILURE) {
      ERROR("Plugin '%s': could not load config file", name);
      return STS_FAILURE;
   }

   INFO("plugin_demo is initialized");
   return STS_SUCCESS;
}

/*
 * Processing.
 * 
 */
int  PLUGIN_PROCESS(int stage, sip_ticket_t *ticket){
   /* stage contains the PLUGIN_* value - the stage of SIP processing. */
   INFO("plugin_demo: processing - stage %i",stage);
   return STS_SUCCESS;
}

/*
 * De-Initialization.
 * Called during shutdown of siproxd. Gives the plugin the chance
 * to clean up its mess (e.g. dynamic memory allocation, database
 * connections, whatever the plugin messes around with)
 */
int  PLUGIN_END(plugin_def_t *plugin_def){
   INFO("plugin_demo ends here");
   return STS_SUCCESS;
}

