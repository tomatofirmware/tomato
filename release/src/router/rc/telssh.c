/*

	Tomato Firmware
	Copyright (C) 2006-2009 Jonathan Zarate

*/

#include "rc.h"


void create_passwd(void)
{
	char s[512];
	char *p;
	char salt[32];
	FILE *f;
	mode_t m;

	strcpy(salt, "$1$");
	f_read("/dev/urandom", s, 6);
	base64_encode(s, salt + 3, 6);
	salt[3 + 8] = 0;
	p = salt;
	while (*p) {
		if (*p == '+') *p = '.';
		++p;
	}
	if (((p = nvram_get("http_passwd")) == NULL) || (*p == 0)) p = "admin";

	m = umask(0777);
	if ((f = fopen("/etc/shadow", "w")) != NULL) {
		p = crypt(p, salt);
		fprintf(f, "root:%s:0:0:99999:7:0:0:\n"
				   "nobody:*:0:0:99999:7:0:0:\n", p);
#if TOMATO_SL
		// todo		zzz
		fprintf(f, "admin:*:0:0:99999:7:0:0:\n");
#endif
		fclose(f);
	}
	umask(m);
	chmod("/etc/shadow", 0600);

	f_write_string("/etc/passwd",
		"root:x:0:0:root:/root:/bin/sh\n"
#if TOMATO_SL
		// todo		zzz
		"admin:x:100:100:nas:/dev/null:/dev/null\n"
#endif
		"nobody:x:65534:65534:nobody:/dev/null:/dev/null\n",
		0, 0644);

	f_write_string("/etc/gshadow",
		"root:*:0:\n"
#if TOMATO_SL
		"nas:*:100:\n"
#endif
		"nobody:*:65534:\n",
		0, 0600);
	f_write_string("/etc/group",
		"root:x:0:\n"
#if TOMATO_SL
		"nas:x:100:\n"
#endif
		"nobody:x:65534:\n",
		0, 0644);
}

void start_sshd(void)
{
	static const char *hkfn = "/etc/dropbear/dropbear_rsa_host_key";

	mkdir("/etc/dropbear", 0700);
	mkdir("/root/.ssh", 0700);

	f_write_string("/root/.ssh/authorized_keys", nvram_safe_get("sshd_authkeys"), 0, 0700);

	unlink(hkfn);

	if (!nvram_get_file("sshd_hostkey", hkfn, 2048)) {
		eval("dropbearkey", "-t", "rsa", "-f", (char *)hkfn);
		if (nvram_set_file("sshd_hostkey", hkfn, 2048)) {
			nvram_commit_x();
		}
	}

/*
	xstart("dropbear", "-a", "-p", nvram_safe_get("sshd_port"), nvram_get_int("sshd_pass") ? "" : "-s");
*/

	char *argv[9];
	int argc;
	char *p;

	argv[0] = "dropbear";
	argv[1] = "-p";
	argv[2] = nvram_safe_get("sshd_port");
	argc = 3;

	if (!nvram_get_int("sshd_pass")) argv[argc++] = "-s";

	if (nvram_get_int("sshd_forwarding")) argv[argc++] = "-a";

	if (((p = nvram_get("sshd_rwb")) != NULL) && (*p)) {
		argv[argc++] = "-W";
		argv[argc++] = p;
	}

	argv[argc] = NULL;
	_eval(argv, NULL, 0, NULL);
}

void stop_sshd(void)
{
	killall("dropbear", SIGTERM);
}

void start_telnetd(void)
{
	xstart("telnetd", "-p", nvram_safe_get("telnetd_port"));
}

void stop_telnetd(void)
{
	killall("telnetd", SIGTERM);
}
