/*
 * proxy.c
 *
 * Copyright (C) 2013 Hyzoom
 *
 */
#include <stdlib.h>
#include <rc.h>
#include <shutils.h>
#include <utils.h>
#include <syslog.h>
#include <sys/stat.h>

void start_proxy(void)
{

// make sure its really stop
    stop_proxy();

    printf( "Proxy server has started.\n");
    return;

}

void stop_proxy(void)
{
    printf( "Proxy server has stopped.\n");
    return;
}
