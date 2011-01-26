#ifdef SWILL

#include "totd.h"

static char stats_start[] = "<HTML>\n<BODY>\n";
static char stats_end[] = "</BODY></HTML>\n";

void print_stats(FILE *f) {
        char astr[MAX_DNAME];
	G_List *gl;
	int i;

        fprintf(f,"%s\n",stats_start);
        fprintf(f,"<h1>Totd DNS-proxy Status</h1>\n<p></p>\n");

        fprintf(f,"<p></p>\n<h2>Forwarders:</h2>\n");
	fprintf (f, "<ul>\n");
        for (gl = T.Fwd_list->next; gl->list_data; gl = gl->next) {
                Fwd *fwd = (Fwd *) (gl->list_data);
		if (gl == T.current_fwd)
			fprintf (f, "<li><b>hostname: %s port: %d</b> (active)</li>\n", 
                		 sprint_inet(fwd->sa, astr), fwd->port);
		else
			fprintf (f, "<li>hostname: %s port: %d</li>\n", 
                		 sprint_inet(fwd->sa, astr), fwd->port);
	}
	fprintf (f, "</ul>\n");

        fprintf(f,"<p></p>\n<h2>Configured Prefixes:</h2>\n");
	fprintf (f, "<ul>\n");
	for (i = 0; i < T.prefixnum; i++) {
		if (i == T.current_prefix)
			fprintf (f, "<li><b>%s</b> (active)</li>\n",
				 inet_ntop(AF_INET6, &T.prefix[i], astr, 
				 	   sizeof (struct sockaddr_in6)));
		else
			fprintf (f, "<li>%s</li>\n",
				 inet_ntop(AF_INET6, &T.prefix[i], astr, 
					   sizeof (struct sockaddr_in6)));
	}
	fprintf (f, "</ul>\n");

        fprintf(f,"%s\n",stats_end);
        return;
}

void add_prefix(FILE *f) {
	const int cmpsiz = sizeof(struct in6_addr) - sizeof(struct in_addr);
	char prefix[TOTPREFIXLEN + 1];
	char *pref;
	int i;

        fprintf(f,"<HTML>\n<BODY>\n");
  	if (!swill_getargs("s(prefix)",&pref)) {
		fprintf(f, "<h1>No prefix</h1>\n");
        	fprintf(f,"</BODY>\n</HTML>");
		return;
	}

        if (inet_pton (AF_INET6, pref, (void *)&prefix) != 1) {
                syslog (LOG_DEBUG, "Not a valid prefix: %s", pref);
		fprintf(f, "<h1>No valid prefix</h1>\n");
        	fprintf(f,"</BODY>\n</HTML>");
		return;
	}
        for (i = 0; i < T.prefixnum; i++) {
                if (!memcmp(prefix, &T.prefix[i], cmpsiz)) {
                	syslog (LOG_DEBUG, "Duplicate prefix: %s", pref);
			fprintf(f, "<h1>Duplicate prefix, not added</h1>\n");
        		fprintf(f,"</BODY>\n</HTML>");
			return;
		}
	}

	if (conv_trick_conf(pref)) {
		fprintf(f, "<h1>Can not add prefix: %s</h1>\n", pref);
		syslog (LOG_INFO, "can not add prefix %d: %s", T.prefixnum, pref);

	} else {
		fprintf(f, "<h1>Added prefix %d: %s</h1>\n", T.prefixnum, pref);
		syslog (LOG_INFO, "prefix %d added: %s", T.prefixnum, pref);
	}
        fprintf(f,"</BODY>\n</HTML>");
	return;
}

void del_prefix(FILE *f) {
	const int cmpsiz = sizeof(struct in6_addr) - sizeof(struct in_addr);
	char prefix[TOTPREFIXLEN + 1];
        char *pref;
	int i;

        fprintf(f,"<HTML>\n<BODY>\n");
        if (!swill_getargs("s(prefix)",&pref)) {
		fprintf(f, "<h1>No prefix</h1>\n");
        	fprintf(f,"</BODY>\n</HTML>");
		return;
	}

        if (inet_pton (AF_INET6, pref, (void *)&prefix) != 1) {
                syslog (LOG_DEBUG, "Not a valid prefix: %s", pref);
		fprintf(f, "<h1>No valid prefix</h1>\n");
        	fprintf(f,"</BODY>\n</HTML>");
		return;
	}

        for (i = 0; i < T.prefixnum; i++) {
                if (!memcmp(prefix, &T.prefix[i], cmpsiz)) {
			int j;

			for (j=i+1; j < MAXPREFIXES; j++) {
				memcpy(&T.prefix[j-1], &T.prefix[j], TOTPREFIXLEN);
			}

			fprintf(f, "<h1>Deleted prefix %d: %s</h1>\n", T.prefixnum, pref);
			syslog (LOG_INFO, "prefix %d deleted: %s", T.prefixnum, pref);
		}
        }
	if (i == T.prefixnum) {
		fprintf(f, "<h1>No such prefix: %s</h1>\n", pref);
		syslog (LOG_INFO, "No such prefix to delete: %s", pref);
	}
	else T.prefixnum--;

        fprintf(f,"</BODY>\n</HTML>");
	return;
}

#endif
