/*
 * gaeproxy.c
 *
 * Copyright (C) 2013 bwq518, Hyzoom
 *
 */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <syslog.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <fcntl.h>
#include <rc.h>
#include <shutils.h>
#include <utils.h>

#ifndef TRUE
#define TRUE    1
#endif

#ifndef FALSE
#define FALSE   0
#endif

#ifndef MAXPATH
#define MAXPATH 256
#endif
#define MAXLINELEN  5120

int           GetPrivateProfileString (char * Section,char * Entry,char * Default,char * Buffer,unsigned int Size,char * Filename);
unsigned int  GetPrivateProfileUInt (char * Section, char * Entry, unsigned int  Default, char * Filename);
unsigned int  GetPrivateProfileUIntx (char * Section, char * Entry, unsigned int  Default, char * Filename);
unsigned long GetPrivateProfileULong (char * Section, char * Entry, unsigned long Default, char * Filename);
unsigned long GetPrivateProfileULongx (char * Section, char * Entry, unsigned long Default, char * Filename);
int           GetPrivateProfileInt (char * Section, char * Entry, int   Default, char * Filename);
long          GetPrivateProfileLong (char * Section, char * Entry, long  Default, char * Filename);

int  WritePrivateProfileString (char * Section, char * Entry, char * String, char * Filename);
int  WritePrivateProfileUInt (char * Section, char * Entry, unsigned int  value, char * Filename);
int  WritePrivateProfileUIntx (char * Section, char * Entry, unsigned int  value, char * Filename);
int  WritePrivateProfileULong (char * Section, char * Entry, unsigned long value, char * Filename);
int  WritePrivateProfileULongx (char * Section, char * Entry, unsigned long value, char * Filename);
int  WritePrivateProfileInt (char * Section, char * Entry, int   value, char * Filename);
int  WritePrivateProfileLong (char * Section, char * Entry, long  value, char * Filename);

#define GetProfileString(a1,a2,a3,a4,a5,a6) GetPrivateProfileString(a1,a2,a3,a4,a5,a6)
#define GetProfileUInt(a1,a2,a3,a4) GetPrivateProfileUInt(a1,a2,a3,a4)
#define GetProfileUIntx(a1,a2,a3,a4) GetPrivateProfileUIntx(a1,a2,a3,a4)
#define GetProfileULong(a1,a2,a3,a4) GetPrivateProfileULong(a1,a2,a3,a4)
#define GetProfileInt(a1,a2,a3,a4) GetPrivateProfileInt(a1,a2,a3,a4)
#define GetProfileLong(a1,a2,a3,a4) GetPrivateProfileLong(a1,a2,a3,a4)

#define WriteProfileString(a1,a2,a3,a4) WritePrivateProfileString(a1,a2,a3,a4)
#define WriteProfileUInt(a1,a2,a3,a4) WritePrivateProfileUInt(a1,a2,a3,a4)
#define WriteProfileUIntx(a1,a2,a3,a4) WritePrivateProfileUIntx(a1,a2,a3,a4)
#define WriteProfileULong(a1,a2,a3,a4) WritePrivateProfileULong(a1,a2,a3,a4)
#define WriteProfileULongx(a1,a2,a3,a4) WritePrivateProfileULongx(a1,a2,a3,a4)
#define WriteProfileInt(a1,a2,a3,a4) WritePrivateProfileInt(a1,a2,a3,a4)
#define WriteProfileLong(a1,a2,a3,a4) WritePrivateProfileLong(a1,a2,a3,a4)

#ifndef DEBUG
#define PDEBUG(fmt, args...) \
    do{}while(0)
#else
#define PDEBUG(fmt, args...) \
    fprintf(stderr, "[%s:%d]"fmt, __func__, __LINE__, ##args)
#endif
#ifndef ERROR
#define PERROR(err) \
    do{}while(0)
#else
#define PERROR(err) \
    fprintf(stderr, "[%s:%d]\n", __func__, __LINE__); \
    perror(err)
#endif
#define BUFFER_SIZE 1024
/*
 * 判断是否是目录
 * @ 是目录返回1，是普通文件返回0，出错返回-1
 * */
int IsDir(const char *path)
{
	struct stat buf;
	if (stat(path, &buf)==-1)
	{
		PERROR("stat");
		PDEBUG("path = %s\n", path);
		return -1;
	}
	return S_ISDIR(buf.st_mode);
}
/*
 * 创建目录
 * @ 可以创建多级目录，失败返回-1
 * */
int CreateDir(const char *path)
{
	char pathname[256];
	strcpy(pathname, path);
	int i, len=strlen(pathname);
	if (pathname[len-1]!='/')
	{
		strcat(pathname, "/");
		len++;
	}
	for (i=0; i<len; i++)
	{
		if ((pathname[i]=='/') && (i > 0))
		{
			pathname[i]=0;
			trimstr(pathname);
			if ((strlen(pathname)>0) && access(pathname, F_OK)) //判断路径是否存在
			{
				//不存在则创建
				if (mkdir(pathname, 0755)==-1)
				{
					PERROR("mkdir");
					return -1;
				}
			}
			pathname[i]='/';
		}
	}
	PDEBUG("Success to create dir:%s\n",pathname);
	return 0;
}
/*
 * 拷贝文件
 * @ @dstpath -- 可以是文件名也可以是目录名
 * */
int FileCopy(const char *srcpath, const char *dstpath)
{
	int srcfd, dstfd, file_len, ret=1;
	char buffer[BUFFER_SIZE];
	char dstfn[256];
	struct stat src_stat;

	if (access(srcpath, R_OK))
	{
		PDEBUG("不能拷贝不存在或不可读文件: %s\n", srcpath);
		return -1;
	}
	if (stat (srcpath, &src_stat) != 0)
	{
		PDEBUG("无法取得源文件的属性: %s\n", srcpath);
		return -1;
	}
	strcpy(dstfn, dstpath);
	//如果@dstpath存在且是目录则在其后加上srcpath的文件名
	if (access(dstpath, F_OK)==0 && IsDir(dstpath)==1)
	{
		if (dstfn[strlen(dstfn)-1]!='/')
		{
			strcat(dstfn, "/");
		}
		if (strchr(srcpath, '/'))
		{
			strcat(dstfn, strrchr(srcpath, '/'));
		}
		else
		{
			strcat(dstfn, srcpath);
		}
	}
	srcfd=open(srcpath, O_RDONLY);
	/* 创建目标文件，若存在，则长度清零，忽略现有内容 */
	dstfd=open(dstfn, O_WRONLY|O_CREAT|O_TRUNC, src_stat.st_mode);
	if (srcfd==-1 || dstfd==-1)
	{
		if (srcfd!=-1)
		{
			close(srcfd);
		}
		PERROR("open");
		return -1;
	}

	file_len= lseek(srcfd, 0L, SEEK_END);
	lseek(srcfd, 0L, SEEK_SET);
	while(ret)
	{
		ret= read(srcfd, buffer, BUFFER_SIZE);
		if (ret==-1)
		{
			PERROR("read");
			close(srcfd);
			close(dstfd);
			return -1;
		}
		write(dstfd, buffer, ret);
		file_len-=ret;
		bzero(buffer, BUFFER_SIZE);
	}
	close(srcfd);
	close(dstfd);
	if (ret)
	{
		PDEBUG("文件: %s, 没有拷贝完\n", srcpath);
		return -1;
	}
	return 0;
}
/*
 * 目录拷贝
 * */
int DirCopy(const char *srcpath, const char *dstpath)
{
	int ret;
	DIR * dir;
	struct dirent *ptr;

	if (!(ret=IsDir(srcpath))) //如果@srcpath 是文件，直接进行文件拷贝
	{
		FileCopy(srcpath, dstpath);
		return 0;
	}
	else if(ret!=1)  //目录或文件不存在
	{
		return -1;
	}
	dir = opendir(srcpath);
	if ( dir == NULL )
	{
		PDEBUG("Failed to open dir:%s\n", srcpath);
		return -1;
	}
	CreateDir(dstpath);
	while((ptr=readdir(dir))!=NULL)
	{
		char frompath_r[256];
		char topath_r[256];

		bzero(frompath_r, 256);
		bzero(topath_r, 256);
		strcpy(frompath_r, srcpath);
		strcpy(topath_r, dstpath);
		if (frompath_r[strlen(frompath_r)-1]!='/')
		{
			strcat(frompath_r, "/");
		}
		if (topath_r[strlen(topath_r)-1]!='/')
		{
			strcat(topath_r, "/");
		}
		strcat(frompath_r, ptr->d_name);
		strcat(topath_r, ptr->d_name);
		if ((ret=IsDir(frompath_r))==1)
		{
			if (strcmp(strrchr(frompath_r, '/'), "/.")==0
			        || strcmp(strrchr(frompath_r, '/'), "/..")==0)
			{
				PDEBUG(". or ..目录不用复制\n");
			}
			else
			{
				DirCopy(frompath_r, topath_r);
			}
		}
		else if (ret!=-1)
		{
			FileCopy(frompath_r, topath_r);
		}
	}
	closedir(dir);
	return 0;
}
/*****************************************************************************/
/*       Service: start_gaeproxy, stop_gaeproxy                              */
/*****************************************************************************/
void start_gaeproxy(void)
{
	char f_proxy_ini[256] = "/tmp/wp_internal/local/proxy.ini";
	char f_home_local[256], f_home_server[256], f_startup[256], f_autoupload[256], f_python[128]="/usr/bin/python";
	char stmp1[2048], stmp2[1024], stmp3[512], file[256];
	int bin_flag; // 0 - internal, 1 - optware, 2 - custom, 3 - download
	FILE *fp;

	// make sure its really stop
	stop_gaeproxy();

	if( !nvram_match( "gaeproxy_enable", "1" ) ) return;
	if (nvram_match( "gaeproxy_binary", "internal" ) )
	{
		strcpy(stmp1,"/tmp/wp_internal/local/startup.py");
		bin_flag = 0;
	}
	else if (nvram_match( "gaeproxy_binary", "optware" ) )
	{
		strcpy(stmp1, "/mnt/opt/wallproxy/local/startup.py");
		bin_flag = 1;
	}
	else if (nvram_match( "gaeproxy_binary", "custom" ) )
	{
		strcpy(stmp1, nvram_safe_get( "gaeproxy_binary_custom" ));
		bin_flag = 2;
	}
	else if (nvram_match( "gaeproxy_binary", "download" ) )
	{
		strcpy(stmp1, "/tmp/wp_download/local/startup.py");
		bin_flag = 3;
	}
	else // invalid
	{
		return;
	}

	splitpath(stmp1, f_home_local, file);
	f_home_local[strlen(f_home_local) -1] = 0;
	splitpath(f_home_local, f_home_server, file);
	strcat(f_home_server, "server");
	PDEBUG("f_home_local:|%s|   f_home_server:|%s|\n",f_home_local, f_home_server);

	if (bin_flag == 0) //internal
	{
		strcpy(f_proxy_ini, "/tmp/wp_internal/local/proxy.ini");
		// Copy variable files to /tmp/gaeproxy
		CreateDir("/tmp/wp_internal/local");
		if (!f_exists( f_proxy_ini ))
		{
			DirCopy("/usr/lib/python2.7/site-packages/gaeproxy/", "/tmp/wp_internal/");
		}
		strcpy(f_home_local,"/tmp/wp_internal/local");
		strcpy(f_home_server,"/tmp/wp_internal/server");
		strcpy(f_autoupload, "/tmp/wp_internal/server/autoupload.py");
	}
	else if (bin_flag == 3) //download from URL
	{
		if (!f_exists( "/tmp/wp_download/local/proxy.ini" )) //if already downloaded, skip to download again
		{
			if( !( fp = fopen( "/tmp/wp_download.sh", "w" ) ) )
			{
				perror( "/tmp/wp_download.sh" );
				return;
			}
			sprintf(stmp1,nvram_safe_get("gaeproxy_binary_download"));
			trimstr(stmp1);
			if (strlen(stmp1) == 0) strcpy(stmp1,"http://www.hyzoom.com/tomato/wallproxy.zip");
			fprintf(fp, "#!/bin/sh\n" );
			fprintf(fp, "## Utility for downloading WallProxy source from URL\n");
			fprintf(fp, "## Copyright(C) Hyzoom bwq518, 2013. All rights reserved.\n\n" );
			fprintf(fp, "wget -q %s -O /tmp/wallproxy.zip\n",stmp1);
			fprintf(fp, "cd /tmp\n");
			fprintf(fp, "unzip -q -o wallproxy.zip\n");
			fprintf(fp, "rm -f wallproxy.zip\n");
			fprintf(fp, "rm -rf wp_download\n");
			fprintf(fp, "mv -f wallproxy-master wp_download\n");
			// Process font-family of css
			sprintf(stmp1,"%s/src.zip/web/css/codemirror.css",f_home_local);
			fprintf(fp, "sed -i \"s/monospace;/monospace,微软雅黑;/g\" %s\n", stmp1);
			sprintf(stmp1,"%s/src.zip/web/css/jqm-docs.css",f_home_local);
			fprintf(fp, "sed -i \"s/sans-serif}/sans-serif,微软雅黑}/g\" %s\n", stmp1);
			fprintf(fp, "sed -i \"s/sans-serif;/sans-serif,微软雅黑;/g\" %s\n", stmp1);
			sprintf(stmp1,"%s/src.zip/web/css/themes/default/jquery.mobile-1.1.1.css",f_home_local);
			fprintf(fp, "sed -i \"s/sans-serif}/sans-serif,微软雅黑}/g\" %s\n", stmp1);
			fprintf(fp, "sed -i \"s/sans-serif;/sans-serif,微软雅黑;/g\" %s\n", stmp1);
			fclose( fp );
			chmod( "/tmp/wp_download.sh", 0755 );
			eval("/tmp/wp_download.sh");
		}
		strcpy(f_proxy_ini, "/tmp/wp_download/local/proxy.ini"); // if not exists proxy.ini in downloaded package
		if (!f_exists( f_proxy_ini ))
		{
			// Use the internal proxy.ini as default
			FileCopy("/usr/lib/python2.7/site-packages/gaeproxy/local/proxy.ini", f_proxy_ini);
		}
		strcpy(f_home_local,"/tmp/wp_download/local");
		strcpy(f_home_server,"/tmp/wp_download/server");
		strcpy(f_autoupload, "/tmp/wp_download/server/autoupload.py");
		DirCopy("/usr/lib/python2.7/site-packages/gaeproxy/local/cert/", "/tmp/wp_download/local/cert/");
	}
	else // bin_flag == 1 or 2
	{
		sprintf(f_proxy_ini, "%s/proxy.ini", f_home_local);
		sprintf(f_autoupload, "%s/autoupload.py", f_home_server);
		if (!f_exists( f_proxy_ini ))
		{
			FileCopy("/usr/lib/python2.7/site-packages/gaeproxy/local/proxy.ini", f_proxy_ini);
		}
		sprintf(stmp1, "%s/cert/", f_home_local);
		DirCopy("/usr/lib/python2.7/site-packages/gaeproxy/local/cert/", stmp1);
	}
	// Modify proxy.ini according to nvram
	strcpy(stmp1,nvram_safe_get("gaeproxy_listen_ip"));
	WritePrivateProfileString("listen","ip",stmp1,f_proxy_ini);
	strcpy(stmp1,nvram_safe_get("gaeproxy_listen_port"));
	WritePrivateProfileString("listen","port",stmp1,f_proxy_ini);
	strcpy(stmp1,nvram_safe_get("gaeproxy_listen_web_username"));
	trimstr(stmp1);
	if (strlen(stmp1) == 0) WritePrivateProfileString("listen","web_username",NULL,f_proxy_ini);
	else WritePrivateProfileString("listen","web_username",stmp1,f_proxy_ini);
	strcpy(stmp1,nvram_safe_get("gaeproxy_listen_web_password"));
	trimstr(stmp1);
	if (strlen(stmp1) == 0) WritePrivateProfileString("listen","web_password",NULL,f_proxy_ini);
	else WritePrivateProfileString("listen","web_password",stmp1,f_proxy_ini);
	strcpy(stmp1,nvram_safe_get("gaeproxy_listen_username"));
	trimstr(stmp1);
	if (strlen(stmp1) == 0) WritePrivateProfileString("listen","username",NULL,f_proxy_ini);
	else WritePrivateProfileString("listen","username",stmp1,f_proxy_ini);
	strcpy(stmp1,nvram_safe_get("gaeproxy_listen_password"));
	trimstr(stmp1);
	if (strlen(stmp1) == 0) WritePrivateProfileString("listen","password",NULL,f_proxy_ini);
	else WritePrivateProfileString("listen","password",stmp1,f_proxy_ini);

	strcpy(stmp1,nvram_safe_get("gaeproxy_pac_enable"));
	WritePrivateProfileString("pac","enable",stmp1,f_proxy_ini);
	strcpy(stmp1,nvram_safe_get("gaeproxy_pac_file"));
	if (!nvram_match("gaeproxy_pac_file_enable","1")) WritePrivateProfileString("pac","file",NULL,f_proxy_ini);
	else WritePrivateProfileString("pac","file",stmp1,f_proxy_ini);
	strcpy(stmp1,nvram_safe_get("gaeproxy_pac_https_mode"));
	WritePrivateProfileString("pac","https_mode",stmp1,f_proxy_ini);

	strcpy(stmp1,nvram_safe_get("gaeproxy_gae_enable"));
	WritePrivateProfileString("gae","enable",stmp1,f_proxy_ini);
	strcpy(stmp1,nvram_safe_get("gaeproxy_gae_appid"));
	WritePrivateProfileString("gae","appid",stmp1,f_proxy_ini);
	strcpy(stmp1,nvram_safe_get("gaeproxy_gae_password"));
	WritePrivateProfileString("gae","password",stmp1,f_proxy_ini);
	strcpy(stmp1,nvram_safe_get("gaeproxy_gae_listen"));
	WritePrivateProfileString("gae","listen",stmp1,f_proxy_ini);
	strcpy(stmp1,nvram_safe_get("gaeproxy_gae_profile"));
	WritePrivateProfileString("gae","profile",stmp1,f_proxy_ini);
	strcpy(stmp1,nvram_safe_get("gaeproxy_gae_max_threads"));
	WritePrivateProfileString("gae","max_threads",stmp1,f_proxy_ini);
	strcpy(stmp1,nvram_safe_get("gaeproxy_gae_fetch_mode"));
	WritePrivateProfileString("gae","fetch_mode",stmp1,f_proxy_ini);

	strcpy(stmp1,nvram_safe_get("gaeproxy_hosts_enable"));
	WritePrivateProfileString("hosts","enable",stmp1,f_proxy_ini);
	strcpy(stmp1,nvram_safe_get("gaeproxy_hosts_dns"));
	WritePrivateProfileString("hosts","dns",stmp1,f_proxy_ini);
	strcpy(stmp1,nvram_safe_get("gaeproxy_hosts_resolve"));
	WritePrivateProfileString("hosts","resolve",stmp1,f_proxy_ini);

	strcpy(stmp1,nvram_safe_get("gaeproxy_autorange_enable"));
	WritePrivateProfileString("autorange","enable",stmp1,f_proxy_ini);
	strcpy(stmp1,nvram_safe_get("gaeproxy_autorange_maxsize"));
	WritePrivateProfileString("autorange","maxsize",stmp1,f_proxy_ini);
	strcpy(stmp1,nvram_safe_get("gaeproxy_autorange_waitsize"));
	WritePrivateProfileString("autorange","waitsize",stmp1,f_proxy_ini);
	strcpy(stmp1,nvram_safe_get("gaeproxy_autorange_bufsize"));
	WritePrivateProfileString("autorange","bufsize",stmp1,f_proxy_ini);

	strcpy(stmp1,nvram_safe_get("gaeproxy_proxy_enable"));
	WritePrivateProfileString("proxy","enable",stmp1,f_proxy_ini);
	strcpy(stmp1,nvram_safe_get("gaeproxy_proxy_host"));
	WritePrivateProfileString("proxy","host",stmp1,f_proxy_ini);
	strcpy(stmp1,nvram_safe_get("gaeproxy_proxy_port"));
	WritePrivateProfileString("proxy","port",stmp1,f_proxy_ini);
	strcpy(stmp1,nvram_safe_get("gaeproxy_proxy_username"));
	WritePrivateProfileString("proxy","username",stmp1,f_proxy_ini);
	strcpy(stmp1,nvram_safe_get("gaeproxy_proxy_password"));
	WritePrivateProfileString("proxy","password",stmp1,f_proxy_ini);

	strcpy(stmp1,nvram_safe_get("gaeproxy_paas_enable"));
	WritePrivateProfileString("paas","enable",stmp1,f_proxy_ini);
	strcpy(stmp1,nvram_safe_get("gaeproxy_paas_password"));
	WritePrivateProfileString("paas","password",stmp1,f_proxy_ini);
	strcpy(stmp1,nvram_safe_get("gaeproxy_paas_listen"));
	WritePrivateProfileString("paas","listen",stmp1,f_proxy_ini);
	strcpy(stmp1,nvram_safe_get("gaeproxy_paas_fetch_server"));
	WritePrivateProfileString("paas","fetch_server",stmp1,f_proxy_ini);
	strcpy(stmp1,nvram_safe_get("gaeproxy_paas_proxy"));
	trimstr(stmp1);
	if (strlen(stmp1) == 0) WritePrivateProfileString("paas","proxy",NULL,f_proxy_ini);
	else WritePrivateProfileString("paas","proxy",stmp1,f_proxy_ini);
	strcpy(stmp1,nvram_safe_get("gaeproxy_useragent_enable"));
	WritePrivateProfileString("useragent","enable",stmp1,f_proxy_ini);
	strcpy(stmp1,nvram_safe_get("gaeproxy_useragent_match"));
	WritePrivateProfileString("useragent","match",stmp1,f_proxy_ini);
	strcpy(stmp1,nvram_safe_get("gaeproxy_useragent_rules"));
	WritePrivateProfileString("useragent","rules",stmp1,f_proxy_ini);
	strcpy(stmp1,nvram_safe_get("gaeproxy_useragent_string"));
	WritePrivateProfileString("useragent","string",stmp1,f_proxy_ini);

	GetPrivateProfileString("rulelist","PROXY *:8086","", stmp1, 2048, f_proxy_ini);
	WritePrivateProfileString("rulelist","PROXY *:8086",NULL, f_proxy_ini);
	if(nvram_match("gaeproxy_enable","1"))
	{
		strcpy(stmp3, nvram_safe_get("gaeproxy_listen_port"));
		trimstr(stmp3);
		sprintf(stmp2,"PROXY *:%s",stmp3);
		WritePrivateProfileString("rulelist",stmp2, stmp1, f_proxy_ini);
	}
	GetPrivateProfileString("rulelist","PROXY *:8087;DIRECT","", stmp1, 2048, f_proxy_ini);
	WritePrivateProfileString("rulelist","PROXY *:8087;DIRECT",NULL, f_proxy_ini);
	if(nvram_match("gaeproxy_gae_enable","1"))
	{
		strcpy(stmp3, nvram_safe_get("gaeproxy_gae_listen"));
		trimstr(stmp3);
		sprintf(stmp2,"PROXY *:%s;DIRECT",stmp3);
		WritePrivateProfileString("rulelist",stmp2, stmp1, f_proxy_ini);
	}

	GetPrivateProfileString("rulelist","PROXY *:8088;DIRECT","", stmp1, 2048, f_proxy_ini);
	WritePrivateProfileString("rulelist","PROXY *:8088;DIRECT",NULL, f_proxy_ini);
	if(nvram_match("gaeproxy_paas_enable","1"))
	{
		strcpy(stmp3, nvram_safe_get("gaeproxy_paas_listen"));
		trimstr(stmp3);
		sprintf(stmp2,"PROXY *:%s;DIRECT",stmp3);
		WritePrivateProfileString("rulelist",stmp2, stmp1, f_proxy_ini);
	}
	// Modify end.

	sprintf(f_startup, "%s/%s", f_home_local, "startup.py");
	PDEBUG("Exec: %s %s\n", f_python, f_startup);

	//start file
	if( !( fp = fopen( "/tmp/start_gaeproxy.sh", "w" ) ) )
	{
		perror( "/tmp/start_gaeproxy.sh" );
		return;
	}
	fprintf( fp, "#!/bin/sh\n\n" );
	/*************** 已经在 firewall.c 中打开端口了
	    fprintf( fp, "del_iptables()\n");
	    fprintf( fp, "{\n");
	    fprintf( fp, "    local port\n");
	    fprintf( fp, "    port=$1\n");
	    fprintf( fp, "    TOTALRULES=`iptables -L INPUT --line-number | grep tcp | grep dpt:$port | wc -l`\n");
	    fprintf( fp, "    while [ \"$TOTALRULES\" -gt 0 ]; do\n");
	    fprintf( fp, "        ALLLINENUM=`iptables -L INPUT --line-number | grep tcp | grep dpt:$port | awk '{print $1; }'`\n");
	    fprintf( fp, "        DELLINENUM=`echo $ALLLINENUM | awk '{print $1; }'`\n");
	    fprintf( fp, "        iptables -D INPUT $DELLINENUM\n");
	    fprintf( fp, "        TOTALRULES=`iptables -L INPUT --line-number | grep tcp | grep dpt:$port | wc -l`\n");
	    fprintf( fp, "    done\n");
	    fprintf( fp, "}\n");
	    fprintf( fp, "add_iptables()\n");
	    fprintf( fp, "{\n");
	    fprintf( fp, "    local port\n");
	    fprintf( fp, "    port=$1\n");
	    fprintf( fp, "    del_iptables $port\n");
	    fprintf( fp, "    iptables -I INPUT -p tcp --dport $port -j ACCEPT\n");
	    fprintf( fp, "}\n\n");
	********************/

	fprintf( fp, "sleep %s\n", nvram_safe_get( "gaeproxy_sleep" ));
	fprintf( fp, "nohup %s %s >/dev/null 2>&1 &\n",f_python, f_startup);

	/*************** 已经在 firewall.c 中打开端口了
	    fprintf( fp, "add_iptables %s\n",nvram_safe_get("gaeproxy_listen_port"));
	    if(nvram_match("gaeproxy_gae_enable","1"))
	        fprintf( fp, "add_iptables %s\n",nvram_safe_get("gaeproxy_gae_listen"));
	    if(nvram_match("gaeproxy_paas_enable","1"))
	        fprintf( fp, "add_iptables %s\n",nvram_safe_get("gaeproxy_paas_listen"));
	***************/
	fprintf( fp, "/usr/bin/wpcheck addcru\n");
	fprintf( fp, "sleep 5\n");
	fprintf( fp, "logger \"gaeproxy successfully started\" \n");
	fclose( fp );

	chmod( "/tmp/start_gaeproxy.sh", 0755 );
	xstart( "/tmp/start_gaeproxy.sh" );

	//autoupload to GAE server shell script
	if (nvram_match("gaeproxy_autoupload_enable","1"))
	{
		int i, j;
		char s_appid[64];

		if( !( fp = fopen( "/tmp/upload_to_gae.sh", "w" ) ) )
		{
			perror( "/tmp/upload_to_gae.sh" );
			return;
		}
		fprintf(fp, "#!/bin/sh\n" );
		fprintf(fp, "## Utility for auto uploading source to GAE server\n");
		fprintf(fp, "## Copyright(C) Hyzoom bwq518, 2013. All rights reserved.\n\n" );
		if (bin_flag == 0) //internal
		{
			// download wallproxy server package from URL
			sprintf(stmp1,nvram_safe_get("gaeproxy_binary_download"));
			trimstr(stmp1);
			if (strlen(stmp1) == 0) strcpy(stmp1,"http://www.hyzoom.com/tomato/wallproxy.zip");
			fprintf(fp, "rm -rf /tmp/wp_internal/server\n");
			fprintf(fp, "wget -q %s -O /tmp/wallproxy.zip\n",stmp1);
			fprintf(fp, "cd /tmp\n");
			fprintf(fp, "unzip -q -o wallproxy.zip\n");
			fprintf(fp, "mv -f wallproxy-master/server wp_internal/\n");
			fprintf(fp, "rm -rf wallproxy-master\n");
			fprintf(fp, "rm -f wallproxy.zip\n\n");
		}
		fprintf(fp, "echo \"autoupload.py is : %s\"\n",f_autoupload);
		fprintf(fp, "echo \"wallproxy server is : %s\"\n",f_home_server);
		fprintf(fp, "cp -f /usr/lib/python2.7/site-packages/gaeproxy/server/autoupload.py %s\n",f_autoupload);
		strcpy(stmp1, nvram_safe_get("gaeproxy_gae_appid"));
		trimstr(stmp1);
		bzero(s_appid, 64);
		stmp3[0] = '\0';
		j = 0;
		for (i = 0; i < strlen(stmp1); i ++ )
		{
			if (stmp1[i] == '|')
			{
				s_appid[j] = '\0';
				trimstr(s_appid);
				if (strlen(s_appid)>0)
				{
					strcat(stmp3,"'");
					strcat(stmp3,s_appid);
					strcat(stmp3,"', ");
				}
				j = 0;
				bzero(s_appid, 64);
			}
			else
			{
				s_appid[j] = stmp1[i];
				j ++;
			}
		}
		/* 处理最后一个 */
		s_appid[j] = '\0';
		trimstr(s_appid);
		if (strlen(s_appid)>0)
		{
			strcat(stmp3,"'");
			strcat(stmp3,s_appid);
			strcat(stmp3,"'");
		}
		trimstr(stmp3);
		i = strlen(stmp3);
		if (stmp3[i - 1] == ',') stmp3[i-1] = '\0';
		PDEBUG("APPIDs:%s\n",stmp3);
		fprintf(fp, "AUTOUPLOAD_ENABLE=`nvram get gaeproxy_autoupload_enable`\n");
		fprintf(fp, "if [ $AUTOUPLOAD_ENABLE -ne \"1\" ];then\n");
		fprintf(fp, "   echo \"Autoupload is disabled. exit.\"\n");
		fprintf(fp, "   exit 1;\n");
		fprintf(fp, "fi\n");
		fprintf(fp, "AUTOUPLOAD_GMAIL=`nvram get gaeproxy_autoupload_gmail`\n");
		fprintf(fp, "if [ X$AUTOUPLOAD_GMAIL = \"X\" ];then\n");
		fprintf(fp, "   echo \"Gmail address is NULL. exit.\"\n");
		fprintf(fp, "   exit 2;\n");
		fprintf(fp, "fi\n");
		fprintf(fp, "AUTOUPLOAD_PASSWD=`nvram get gaeproxy_autoupload_password`\n");
		fprintf(fp, "if [ X$AUTOUPLOAD_PASSWD = \"X\" ];then\n");
		fprintf(fp, "   echo \"Password of gmail is NULL. exit.\"\n");
		fprintf(fp, "   exit 3;\n");
		fprintf(fp, "fi\n");
		fprintf(fp, "sed -i \"s/'app1', 'app2', 'app3'/%s/g\" %s\n", stmp3, f_autoupload);
		fprintf(fp, "sed -i \"s/yyyy@gmail.com/%s/g\" %s\n", nvram_safe_get("gaeproxy_autoupload_gmail"), f_autoupload);
		fprintf(fp, "sed -i \"s/xxxxxxxx/%s/g\" %s\n", nvram_safe_get("gaeproxy_autoupload_password"), f_autoupload);
		int ii,jj;
		char fhs_tmp[128];
		jj = 0;
		for (ii = 0; ii < strlen(f_home_server); ii ++)
		{
			if (	f_home_server[ii] == '/' ||
			        f_home_server[ii] == '*' ||
			        f_home_server[ii] == '[' ||
			        f_home_server[ii] == ']' ||
			        f_home_server[ii] == '&' )
			{
				fhs_tmp[jj] = '\\';
				jj ++;
				fhs_tmp[jj] = f_home_server[ii];
				jj ++;
			}
			else
			{
				fhs_tmp[jj] = f_home_server[ii];
				jj ++;
			}
		}
		fhs_tmp[jj] = '\0';
		fprintf( fp, "sed -i \"s/'\\/usr\\/lib\\/python2.7\\/site-packages\\/gaeproxy\\/server'/'%s'/g\" %s\n", fhs_tmp, f_autoupload);
		fprintf( fp, "nohup %s %s/autoupload.py >/www/ext/autoupload.txt 2>&1 &\n", f_python, f_home_server);
		fprintf( fp, "\nsleep 5\nnvram set gaeproxy_autoupload_enable=0\nnvram commit\n\n");
		fclose( fp );
		chmod( "/tmp/upload_to_gae.sh", 0755 );
		xstart( "/tmp/upload_to_gae.sh" );
	}
	return;
}

void stop_gaeproxy(void)
{
	FILE *fp;

	//stop file
	if( !( fp = fopen( "/tmp/stop_gaeproxy.sh", "w" ) ) )
	{
		perror( "/tmp/stop_gaeproxy.sh" );
		return;
	}
	fprintf( fp, "#!/bin/sh\n\n" );
	fprintf( fp, "killall -KILL python\n");
	fprintf( fp, "/usr/bin/wpcheck addcru\n");
	fprintf( fp, "logger \"gaeproxy successfully stoped\" \n");
	fprintf( fp, "sleep 2\n");

	fclose( fp );
	chmod( "/tmp/stop_gaeproxy.sh", 0755 );
	xstart( "/tmp/stop_gaeproxy.sh" );
	return;
}
/*****************************************************************************/
/*****************************************************************************/
static char * strupr( str)
char * str;
{
	char *p;

	p = str;

	while(*p != '\0')
	{
		*p = toupper(*p);
		p ++;
	}

	return str;
}

static char * strlwr( str)
char *str;
{
	char *p;

	p = str;

	while(*p != '\0')
	{
		*p = tolower(*p);
		p ++;
	}

	return str;
}

static int CRLF2LF(src, dest)
char * src, *dest;
{
	FILE * fps,*fpd;
	char c[2];

	if(!(fps=fopen(src,"rb")))
	{
		fprintf( stderr,"Open source file error.\n");
		return 0;
	}
	if(!(fpd=fopen(dest,"wb")))
	{
		fprintf( stderr,"Open dest. file error.\n");
		fclose(fps);
		return 0;
	}
	while((c[0] = fgetc(fps))!=EOF)
	{
		if(c[0] == '\r')
		{
			c[1] = fgetc(fps);
			if(c[1] == '\n')
				fputc(c[1],fpd);
			else
			{
				fputc(c[0],fpd);
				fputc(c[1],fpd);
			}
		}
		else
			fputc(c[0],fpd);
	}
	fclose(fps);
	fclose(fpd);
	return TRUE;
}

static int LF2CRLF(src, dest)
char * src, * dest;
{

	FILE * fps,*fpd;
	char c, c0;
	int i = 0;

	if(!(fps=fopen(src,"rb")))
	{
		fprintf( stderr,"Open source file error.\n");
		return 0;
	}
	if(!(fpd=fopen(dest,"wb")))
	{
		fprintf( stderr,"Open dest. file error.\n");
		fclose(fps);
		return 0;
	}
	c0 = ' ';
	while((c = fgetc(fps))!=EOF)
	{
		if(c == '\n')
		{
			if ((i == 0 ) || (c0 != '\r')) fputc('\r',fpd);
			fputc(c,fpd);
		}
		else
		{
			fputc(c,fpd);
			c0 = c;
		}
		i ++;
	}
	fclose(fps);
	fclose(fpd);
	return TRUE;
}
static int ReadOneLine (buf, bsize, fp)
char * buf;
unsigned int bsize;
FILE * fp;
{
	/* The end of one line must be : 0D 0A      */
	/* Return :  x -- OK                        */
	/*          -1 -- EOF or ERROR              */

	unsigned int        i;
	unsigned char       lend, c;

	bzero(buf, bsize);
	if (bsize == 0) return 0;
	if (feof(fp))   return -1;

	for (i = 0, lend = 0, bsize --; i < bsize; i++)
	{
		buf[i] = 0;
		if (feof(fp)) return -1;
		if (fread(&c, 1, 1, fp) != 1) return -1;
		if (c == 0x1a)
		{
			if (i == 0) return -1;      /* EOF of text file */
			else
			{
				lend = 1;
				break;
			}
		}
		if (c == 13)
		{
			fread(&c, 1, 1, fp);        /* bypass 0x0A */
			lend = 1;
			break;
		}
		else buf[i] = c;
	}
	buf[i] = 0;
	if (!lend)
	{
		do fread(&c, 1, 1, fp);
		while ((c != 10) && !feof(fp));
	}
	return (i);
}

/* ************************************************************************* */
static long fpSeek;
static char fnName[MAXPATH];
static int   FindPrivateProfileEntry(Section, EntryLine,Size,Filename)
char* Section;
char* EntryLine;
unsigned int Size;
char* Filename;
{
	FILE        *fp;
	char        tmp[MAXLINELEN];
	int         retc;

	strcpy(fnName, Filename);
	fpSeek = -1;	/* set fpSeek error*/

	fp = fopen(Filename, "rb");
	if (fp == NULL) return -1;

	strcpy(tmp, "[");
	strcat(tmp, Section);
	strcat(tmp, "]");
	strupr(tmp);

	for (;;)
	{
		retc = ReadOneLine(EntryLine, Size, fp);
		if (retc == -1)
		{
			fclose(fp);
			return -1;
		}
		if (retc ==  0) continue;

		strupr(EntryLine);
		if (!strcmp(tmp, EntryLine)) break;
	}

	retc = ReadOneLine(EntryLine, Size, fp);
	if (retc == -1)
	{
		fclose(fp);
		return -1;
	}
	if (retc)
	{
		if (*EntryLine == '[')
		{
			fclose(fp);
			return -1;
		}
	}
	fpSeek = ftell(fp);
	fclose(fp);
	return retc;
}
static int   NextPrivateProfileEntry( EntryLine,Size)
char* EntryLine;
unsigned int Size;
{
	FILE        *fp;
	int         retc;

	fp = fopen(fnName, "rb");
	if (fp == NULL) return -1;

	retc = fseek(fp, fpSeek, SEEK_SET);
	fpSeek = -1;	/* set fpSeek error */

	if (retc)
	{
		fclose(fp);
		return -1;
	}
	retc = ReadOneLine(EntryLine, Size, fp);
	if (retc == -1)
	{
		fclose(fp);
		return -1;
	}
	if (retc)
	{
		if (*EntryLine == '[')
		{
			fclose(fp);
			return -1;
		}
	}
	fpSeek = ftell(fp);
	fclose(fp);
	return retc;
}
/* ************************************************************************* */
int   GetPrivateProfileString
(Section,Entry,Default, Buffer,Size,Filename)
char* Section;
char* Entry;
char* Default;
char* Buffer;
unsigned int Size;
char* Filename;
{
	FILE        *fp;
	char        tmp[MAXLINELEN], buf[MAXLINELEN], *val;
	int         retc;
	char        path[MAXLINELEN];

	tmpnam(path);

	LF2CRLF(Filename,path);
	fp = fopen(path, "rb");
	if (fp == NULL) goto DFLT1;

	strcpy(tmp, "[");
	strcat(tmp, Section);
	strcat(tmp, "]");
	strupr(tmp);
	for (;;)
	{
		retc = ReadOneLine(buf, MAXLINELEN-1, fp);
		if (retc == -1) goto DFLT;
		if (retc ==  0) continue;

		trimstr(buf);
		strupr(buf);
		if (!strcmp(tmp, buf)) break;
	}

	strcpy(tmp, Entry);
	strupr(tmp);

	for (;;)
	{
		retc = ReadOneLine(buf, MAXLINELEN-1, fp);
		if (retc ==  0) continue;
		if (retc == -1)     /* EOF */
		{
DFLT:
			fclose(fp);
DFLT1:
			if (Default == NULL)
			{
				*Buffer = 0;
				unlink(path);
				return 0;
			}
			if (strlen(Default) < Size)
			{
				strcpy(Buffer, Default);

				unlink(path);
				return strlen(Default);
			}
			else
			{
				strncpy(Buffer, Default, Size - 1);
				Buffer[Size - 1] = 0;
				unlink(path);
				return (Size - 1);
			}
		}
		trimstr(buf);
		if (*buf == ';') continue;      /* description  line */

		val = strchr(buf, '=');
		if (val == NULL)
		{
			trimstr(buf);
			strupr(buf);
			if (!strcmp(tmp, buf)) goto DFLT;
			continue;
		}
		*val = 0;
		val ++;
		trimstr(buf);
		strupr(buf);
		if (strcmp(tmp, buf)) continue;

		if (!(*val)) goto DFLT;
		fclose(fp);
		trimstr(val);
		if (strlen(val) < Size)
		{
			strcpy(Buffer, val);
			unlink(path);
			return strlen(val);
		}
		else
		{
			strncpy(Buffer, val, Size - 1);
			Buffer[Size - 1] = 0;
			unlink(path);
			return (Size - 1);
		}
	}
}
unsigned int  GetPrivateProfileUInt
(Section, Entry, Default, Filename)
char* Section;
char* Entry;
unsigned int Default;
char* Filename;
{
	char        tmp[MAXLINELEN];
	int         retc;
	unsigned int        v;

	if (!GetPrivateProfileString(Section, Entry, 0, tmp, MAXLINELEN-1, Filename))
		return Default;
	retc = sscanf(tmp, "%u", &v);
	if (retc == 0) return Default;
	return v;
}
unsigned int  GetPrivateProfileUIntx
(Section, Entry, Default, Filename)
char* Section;
char* Entry;
unsigned int Default;
char* Filename;
{
	char        tmp[MAXLINELEN];
	int         retc;
	unsigned int        v;

	if (!GetPrivateProfileString(Section, Entry, 0, tmp, MAXLINELEN-1, Filename))
		return Default;
	retc = sscanf(tmp, "%x", &v);
	if (retc == 0) return Default;
	return v;
}
unsigned long GetPrivateProfileULong
(Section, Entry, Default, Filename)
char* Section;
char* Entry;
unsigned long Default;
char* Filename;
{
	char        tmp[MAXLINELEN];
	int         retc;
	unsigned long       v;

	if (!GetPrivateProfileString(Section, Entry, 0, tmp, MAXLINELEN-1, Filename))
		return Default;
	retc = sscanf(tmp, "%lu", &v);
	if (retc == 0) return Default;
	return v;
}
unsigned long GetPrivateProfileULongx
(Section, Entry, Default, Filename)
char* Section;
char* Entry;
unsigned long Default;
char* Filename;
{
	char        tmp[MAXLINELEN];
	int         retc;
	unsigned long       v;

	if (!GetPrivateProfileString(Section, Entry, 0, tmp, MAXLINELEN-1, Filename))
		return Default;
	retc = sscanf(tmp, "%lX", &v);
	if (retc == 0) return Default;
	return v;
}
int   GetPrivateProfileInt
(Section, Entry, Default, Filename)
char* Section;
char* Entry;
int   Default;
char* Filename;
{
	char        tmp[MAXLINELEN];
	int         retc;
	int         v;

	if (!GetPrivateProfileString(Section, Entry, 0, tmp, MAXLINELEN-1, Filename))
		return Default;
	retc = sscanf(tmp, "%d", &v);
	if (retc == 0) return Default;
	return v;
}
long  GetPrivateProfileLong
(Section, Entry, Default, Filename)
char* Section;
char* Entry;
long  Default;
char* Filename;
{
	char        tmp[MAXLINELEN];
	int         retc;
	long        v;

	if (!GetPrivateProfileString(Section, Entry, 0, tmp, MAXLINELEN-1, Filename))
		return Default;
	retc = sscanf(tmp, "%ld", &v);
	if (retc == 0) return Default;
	return v;
}

/* ************************************************************************* */
int  WritePrivateProfileString
(Section, Entry, String, Filename)
char* Section;
char* Entry;
char* String;
char* Filename;
{
	FILE        *fps, *fpd;
	char        path[MAXLINELEN];
	char        sec[MAXLINELEN], ent[MAXLINELEN], buf[MAXLINELEN], buf1[MAXLINELEN], *val;
	int        retc, CRLF;

	tmpnam(path);
	fpd=fopen(Filename,"rw+");
	if (!fpd)
	{
		fps = fopen(Filename, "wb");
		if (fps == NULL) return FALSE;
		if (Entry == NULL)      /* delete section */
		{
			fclose(fps);
			return TRUE;
		}
		fprintf(fps, "[%s]\r\n", Section);
		if (String == NULL)     /*delete entry */
		{
			fclose(fps);
			goto OUT_0;
		}
		fprintf(fps, "%s=%s\r\n\r\n", Entry, String);
		fclose(fps);
OUT_0:
		CRLF2LF(Filename,path);
		/*
		sprintf(sec,"mv -f %s %s",path,Filename);
		system(sec);
		*/
		if(FileCopy(path,Filename) != 0)
		{
			fprintf( stderr,"Duplicate file error.\n");
			retc = FALSE;
		}

		unlink(path);

		return TRUE;
	}
	else fclose(fpd);

	if(!LF2CRLF(Filename,path))
	{
		fprintf( stderr,"LF->CR LF failure.\n");
		unlink(path);
		return FALSE;
	}
	fps = fopen(path, "rb");
	if (fps == NULL) return FALSE;
	fpd = fopen(Filename, "wb");
	if (fpd == NULL)
	{
		fclose(fps);
		return FALSE;
	}

	strcpy(sec, "[");
	strcat(sec, Section);
	strcat(sec, "]");
	strupr(sec);
	if (Entry == NULL)  /* delete Section */
	{
		retc = FALSE;
		for (;;)
		{
			retc = ReadOneLine(buf, MAXLINELEN-1, fps);
			if (retc == -1) goto OUT1;

			trimstr(buf);
			strcpy(buf1, buf);
			strupr(buf);
			if (!strcmp(sec, buf)) break;
			fprintf(fpd, "%s\r\n", buf1);
		}
		retc = TRUE;
		for (;;)
		{
			retc = ReadOneLine(buf, MAXLINELEN-1, fps);
			trimstr(buf);
			if (retc == -1) goto OUT1;
			if (*buf == '[') break;
		}
		for (;;)
		{
			fprintf(fpd, "%s\r\n", buf);
			retc = ReadOneLine(buf, MAXLINELEN-1, fps);
			trimstr(buf);
			if (retc == -1) break;
		}
OUT1:
		/*
			*buf =0x1a;
		        fwrite(buf, 1, 1, fpd);
		*/
		fclose(fps);
		fclose(fpd);
		if(!CRLF2LF(Filename,path))
		{
			fprintf( stderr,"CR LF-> LF failure.\n");
			retc = FALSE;
			unlink(path);
			return retc;
		}
		/*
		sprintf(sec,"mv -f %s %s",path,Filename);
		system(sec);
		*/
		if(FileCopy(path,Filename) != 0)
		{
			fprintf( stderr,"Duplicate file error.\n");
			retc = FALSE;
		}

		unlink(path);
		return retc;
	}

	strcpy(ent, Entry);
	strupr(ent);
	CRLF = FALSE;
	for (;;)
	{
		retc = ReadOneLine(buf, MAXLINELEN-1, fps);
		trimstr(buf);
		if (retc == -1)
		{
			if (CRLF == FALSE) fprintf(fpd, "\r\n");
			strcpy(sec, "[");
			strcat(sec, Section);
			strcat(sec, "]");
			fprintf(fpd, "%s\r\n", sec);
			if (String != NULL) fprintf(fpd, "%s=%s\r\n\r\n", Entry, String);
			else                fprintf(fpd, "\r\n");
			goto OUT2;
		}
		if (*buf == 0) CRLF = TRUE;
		else           CRLF = FALSE;
		fprintf(fpd, "%s\r\n", buf);
		strupr(buf);
		if (!strcmp(sec, buf)) break;
	}
	CRLF = FALSE;
	for (;;)
	{
		retc = ReadOneLine(buf, MAXLINELEN-1, fps);
		trimstr(buf);
		if (retc == -1)
		{
			if (CRLF == TRUE) fseek(fpd, -2L, SEEK_CUR);
			if (String != NULL) fprintf(fpd, "%s=%s\r\n\r\n", Entry, String);
			else                fprintf(fpd, "\r\n");
			goto OUT2;
		}
		if (*buf == '[')
		{
			if (CRLF == TRUE) fseek(fpd, -2L, SEEK_CUR);
			if (String != NULL) fprintf(fpd, "%s=%s\r\n\r\n", Entry, String);
			else                fprintf(fpd, "\r\n");
			fprintf(fpd, "%s\r\n", buf);
			break;
		}

		if (*buf == 0) CRLF = TRUE;
		else           CRLF = FALSE;
		strcpy(buf1, buf);
		val = strchr(buf, '=');
		if (val != NULL) *val = 0;
		trimstr(buf);
		trimstr(buf1);
		strupr(buf);
		if (strcmp(ent, buf)) fprintf(fpd, "%s\r\n", buf1);
		else
		{
			if (String != NULL) fprintf(fpd, "%s=%s\r\n", Entry, String);
			break;
		}
	}
	for (;;)
	{
		retc = ReadOneLine(buf, MAXLINELEN-1, fps);
		trimstr(buf);
		if (retc == -1) goto OUT2;
		fprintf(fpd, "%s\r\n", buf);
	}
OUT2:
	/*
	    *buf = 0x1a;
	    fwrite(buf, 1, 1, fpd);
	*/
	fclose(fps);
	fclose(fpd);

	if(!CRLF2LF(Filename,path))
	{
		fprintf( stderr,"CR LF-> LF failure.\n");
		retc = FALSE;
		unlink(path);
		return retc;
	}
	/*
	sprintf(sec,"mv -f %s %s",path,Filename);
	system(sec);
	*/

	if(FileCopy(path,Filename) != 0)
	{
		fprintf( stderr,"Duplicate file error.\n");
		retc = FALSE;
	}
	unlink(path);

	return TRUE;
}
int  WritePrivateProfileUInt
(Section, Entry, value, Filename)
char* Section;
char* Entry;
unsigned int  value;
char* Filename;
{
	char        tmp[128];
	sprintf(tmp, "%u", value);
	return WritePrivateProfileString(Section, Entry, tmp, Filename);
}
int  WritePrivateProfileUIntx
(Section, Entry, value, Filename)
char* Section;
char* Entry;
unsigned int  value;
char* Filename;
{
	char        tmp[128];
	sprintf(tmp, "%X", value);
	return WritePrivateProfileString(Section, Entry, tmp, Filename);
}
int  WritePrivateProfileULong
(Section, Entry, value, Filename)
char* Section;
char* Entry;
unsigned long  value;
char* Filename;
{
	char        tmp[128];
	sprintf(tmp, "%lu", value);
	return WritePrivateProfileString(Section, Entry, tmp, Filename);
}
int  WritePrivateProfileULongx
(Section, Entry, value, Filename)
char* Section;
char* Entry;
unsigned long  value;
char* Filename;
{
	char        tmp[128];
	sprintf(tmp, "%lX", value);
	return WritePrivateProfileString(Section, Entry, tmp, Filename);
}
int  WritePrivateProfileInt
(Section, Entry, value, Filename)
char* Section;
char* Entry;
int   value;
char* Filename;
{
	char        tmp[128];
	sprintf(tmp, "%d", value);
	return WritePrivateProfileString(Section, Entry, tmp, Filename);
}
int  WritePrivateProfileLong
(Section, Entry, value, Filename)
char* Section;
char* Entry;
long  value;
char* Filename;
{
	char        tmp[128];
	sprintf(tmp, "%ld", value);
	return WritePrivateProfileString(Section, Entry, tmp, Filename);
}

