#include "rc.h"
#include <shared.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#define	LEN_OPERATION	32							// Length: Operation ("mode")
#define	LEN_COMMAND		256							// Length: Command to be executed
#define	LEN_RESULT		64							// Length: Result from command
#define LEN_DNSMASQ		256							// Length: dnsmasq lease entry
#define LEN_MACADDR		19							// Length: MAC Address (with trailing \0)
#define	FILE_STAT		"/tmp/dnsmasq.stat"			// Filename: Stat for remote dnsmasq.leases file
#define	FILE_LEASES		"/tmp/dnsmasq.leases"		// Filename: Local dnsmasq.leases file (copied from remote)

#define	GET_IP			1							// DNS Lease: Get IP Address
#define	GET_HOST		2							// DNS Lease: Get IP Address

#define MIN(a, b) (((a) < (b)) ? (a) : (b))			// Macro: Maximum of inputs
#define MAX(a, b) (((a) > (b)) ? (a) : (b)) 		// Macro: Minimum of inputs

void usagehelp()
{
	fprintf(stderr, "usage: remote-leases [-cuaq] [-d N] [-i MAC] [-h MAC] server filename user password\n");
	fprintf(stderr, "\nOperations: \n");
	fprintf(stderr, "   -c        Check to see if remote file has changed\n");
	fprintf(stderr, "   -u        Update local file (forced), remote to local\n");
	fprintf(stderr, "   -a        Auto-Update ... check if remote file has changed, update remote to local if needed\n");
	fprintf(stderr, "   -d N      Daemon (monitor for changes ,interval N seconds), update local copy when file changes\n");
	fprintf(stderr, "   -i MAC    IP address mode, return IP address from local file, based on MAC address\n");
	fprintf(stderr, "   -h MAC    Hostname mode, return hostname from local file, based on MAC address\n");
	fprintf(stderr, "   -q        Quiet Mode ... perform action, no output shown\n");
	fprintf(stderr, "\n");
	exit(EXIT_FAILURE);
}

int oper_check(char *server, char *filename, char *user, char *password, int multioper, int quiet) {
	char operation[LEN_OPERATION];
	char command[LEN_COMMAND];
	char cmdresult[LEN_RESULT];
	FILE *mypipe, *statfile;
	long newstat, oldstat;
	int newexists;
	
	// Configure Check Operation
	strncpy(operation, "Check", LEN_OPERATION);
	if ((multioper == TRUE) && (quiet == FALSE))
		printf("   %s: Result = ", operation);
	sprintf(command, "export DROPBEAR_PASSWORD=%s; ssh -y -y %s@%s stat -c %%Y %s 2>/dev/null", password, user, server, filename);
		
	// Set up pipe for command read, and execute command
	if ((mypipe = popen(command,"r")) == NULL) {
		fprintf(stderr, "%s: Could not open pipe for output.\n", operation);
		return FALSE;
	}

	// Grab data from process execution, convert to number (long)
	cmdresult[0] = '0';
	fgets(cmdresult, LEN_RESULT , mypipe);
	newstat = atol(cmdresult);

	// And close pipe
	if (pclose(mypipe) == (int)-1) {
		fprintf(stderr,"%s: Failed to close command stream.\n", operation);
		return FALSE;
	}

	// Compare result from operation to data in statfile (and write back if needed)
	if ((statfile = fopen(FILE_STAT, "r+")) == NULL) {
		// File likely doesn't exist, so try to create new file
		if ((statfile = fopen(FILE_STAT, "w+")) == NULL) {	
			fprintf(stderr,"%s: Failed to open statfile.\n", operation);
			return FALSE;
		}
	}

	// Get data from statfile, convert to number (long)
	cmdresult[0] = '0';
	fgets(cmdresult, LEN_RESULT , statfile);
	oldstat = atol(cmdresult);
	
	// Check for New File on Remote system, update statfile if needed
	if (newstat == oldstat)
		newexists = FALSE;
	else {
		newexists = TRUE;
		if (fseek(statfile, (long)0, SEEK_SET) < 0) {
			fprintf(stderr,"%s: Failed to reposition pointer in statfile.\n", operation);
			return FALSE;
		}
		if (fprintf(statfile, "%ld", newstat) < 0) {
			fprintf(stderr,"%s: Failed to write to statfile.\n", operation);
			return FALSE;
 		}
	}

	// And close statfile
	if (fclose(statfile) != (int)NULL) {
		fprintf(stderr,"%s: Failed to close statfile.\n", operation);
		return FALSE;
	}

	// Print result, return status
	if (quiet == FALSE)
		printf("%s\n", newexists ? "New File Exists" : "New File does not Exist");
	return newexists;
}

int oper_update(char *server, char *filename, char *user, char *password, int multioper, int quiet) {
	char operation[LEN_OPERATION];
	char command[LEN_COMMAND];
	int cmdresult;
	
	// Configure Update Operation
	strncpy(operation, "Update", LEN_OPERATION);
	if ((multioper == TRUE) && (quiet == FALSE))
		printf("   %s: Result = ", operation);
	sprintf(command, "export DROPBEAR_PASSWORD=%s; ssh -y -y %s@%s cat %s > %s 2>/dev/null", password, user, server, filename, FILE_LEASES);
			
	// Execute Command, print results (cmdresult = -1 if fork (to execute command) fails, 0 for successful command execution)
	cmdresult = system(command);
	if (cmdresult == (int)NULL)
		cmdresult = TRUE;
	else
		cmdresult = FALSE;
		
	if (quiet == FALSE)
		printf("Local Lease file %s\n", cmdresult ? "Updated" : "Not Updated");
	return cmdresult;
}

int oper_auto(char *server, char *filename, char *user, char *password, int multioper, int quiet) {
	char operation[LEN_OPERATION];
	int cmdresult;
	
	// Configure Auto-Update Operation
	strncpy(operation, "Auto-Update", LEN_OPERATION);
	if ((multioper == TRUE) && (quiet == FALSE))
		printf("   %s: Result = ", operation);

	// Perform Check, see if Update is needed
	cmdresult = oper_check(server, filename, user, password, multioper, quiet);

	// Perform Update, based on Check above
	if (cmdresult == TRUE) {
		// Execute Update, print results
		cmdresult = oper_update(server, filename, user, password, multioper, TRUE);
	} else {
		// Do not Execute Update
		cmdresult = FALSE;
	}

	// And print results
	if (quiet == FALSE)
		printf("Local Lease File %s\n", cmdresult ? "Updated" : "Not Updated");
	return cmdresult;
}

int oper_daemon(char *server, char *filename, char *user, char *password, int multioper, int quiet, int interval) {
	char operation[LEN_OPERATION];
	int cmdresult;
	int childPID;
	
	// Make sure interval is valid / acceptable
	if (interval >= 1) {

		// Configure Daemon Operation
		strncpy(operation, "Daemon", LEN_OPERATION);

		// Fork Process
		childPID = fork();
		
		// And Handle Parent and Child processes
		if (childPID) {
			// Parent Process - inform complete, and exit)
			if ((multioper == TRUE) && (quiet == FALSE))
				printf("   %s: Result = ", operation);
			if (quiet == FALSE)
				printf("Daemon started, interval = %d seconds. Exiting parent process.\n", interval);
			return TRUE;
		} else {
			// Child Process - loop through (infinite loop), auto-check with defined interval (and if not quiet, echo output for debugging)
			while (1) {
				if ((multioper == TRUE) && (quiet == FALSE))
					printf("   %s: Result = ", operation);

				// Perform Check, see if Update is needed
				cmdresult = oper_check(server, filename, user, password, multioper, TRUE);

				// Perform Update, based on Check above
				if (cmdresult == TRUE) {
					// Execute Update, print results
					cmdresult = oper_update(server, filename, user, password, multioper, TRUE);
				} else {
					// Do not Execute Update
					cmdresult = FALSE;
				}

				// Print results, and sleep until next update ...
				if (quiet == FALSE)
					printf("Daemon, %s\n", cmdresult ? "Updated" : "Not Updated");
				sleep(interval);
			}
		}

	} else
		return FALSE;
}

int oper_iphost(int multioper, int quiet, char *macaddr, int getmode) {
	char operation[LEN_OPERATION];
	int cmdresult;
	FILE *leasefile;
	char dnslease[LEN_DNSMASQ];
	char *maccheck;
	char *ipfound, *hostfound;
	
	// Configure IP Address Operation
	strncpy(operation, "IP Address", LEN_OPERATION);
	if ((multioper == TRUE) && (quiet == FALSE))
		printf("   %s: Result = ", operation);
			
	// Open dnsmasq lease table (file)
	if ((leasefile = fopen(FILE_LEASES, "r")) == NULL) {
		fprintf(stderr,"%s: Failed to open lease file.\n", operation);
		return FALSE;
	}

	// Search file for target MAC Address
	cmdresult = FALSE;
	ipfound[0] = 0;
	while (fgets(dnslease, LEN_DNSMASQ , leasefile) != 0) {
		strtok(dnslease, " ");
		maccheck = strtok(NULL, " ");
		if (strcmp(maccheck, macaddr) == 0) {
			ipfound = strtok(NULL, " ");
			hostfound = strtok(NULL, " ");
			cmdresult = TRUE;
			break;
		}
	}

	// And close dnsmasq lease table (file)
	if (fclose(leasefile) != (int)NULL) {
		fprintf(stderr,"%s: Failed to close lease file.\n", operation);
		return FALSE;
	}
	
	// Print results
	if (quiet == FALSE) {
		if (cmdresult == TRUE) {
			if (getmode == GET_IP)
				printf("%s\n", ipfound);
			if (getmode == GET_HOST)
				printf("%s\n", hostfound);
		}
	}
	return cmdresult;
}

void formatMAC(char *macaddr_in, char *macaddr_out) {
    char *inptr, *outptr;
    int outcnt;
    
    // Reformat MAC Address (to match the dnsmasq lease table -> lower case, with colons)
    inptr = macaddr_in;
    outptr = macaddr_out;
    outcnt = 1;
    
    // Loop over input string (MAC address)
    while(*inptr != (char)NULL) {
    	// Check if this should be a colon (every third output character ... so modulo 3 is equal to 0)
    	if ((outcnt++ % 3) == 0) {
    		// Output should be a colon, insert one if needed
			if (*inptr != ':')
				*outptr++ = ':';
			else
				*outptr++ = *inptr++;
    	} else
    		// Not position for a colon, just copy over (lower case) and track
			*outptr++ = tolower(*inptr++);
	}
	// And add null termination to string (output MAC address)
	*outptr = 0;
}


int remote_leases_main(int argc, char *argv[])
{
	// State variables are static only so the are initialized to zero (saves adding a bunch of code for this ... :-))
	static int opt, optcnt;
	static int optchk, optupd, optauto, optdaemon, optip, opthost, optquiet;
	static int interval;
	char macaddr[LEN_RESULT], macaddr_fmt[LEN_RESULT];
	int argsneeded;
	
	// If no arguments, print help
	if (argc <= 1)
		usagehelp();
		
	else {
		// Get Command Line Options, set up flags / values as needed
		argsneeded = 0;
		while ((opt = getopt(argc, argv, "cuaqd:i:h:")) != -1) {
			switch (opt) {
				case 'c':
					optchk = TRUE;
					optcnt++;
					argsneeded = MAX(4, argsneeded);
					break;
				case 'u':
					optupd = TRUE;
					optcnt++;
					argsneeded = MAX(4, argsneeded);
					break;
				case 'a':
					optauto = TRUE;
					optcnt++;
					argsneeded = MAX(4, argsneeded);
					break;
				case 'd':
					optdaemon = TRUE;
					interval = atoi(optarg);
					optcnt++;
					argsneeded = MAX(4, argsneeded);
					break;
				case 'i':
					optip = TRUE;
					strncpy(macaddr, optarg, LEN_RESULT);
					formatMAC(macaddr, macaddr_fmt);
					optcnt++;
					argsneeded = MAX(0, argsneeded);
					break;
				case 'h':
					opthost = TRUE;
					strncpy(macaddr, optarg, LEN_RESULT);
					formatMAC(macaddr, macaddr_fmt);
					optcnt++;
					argsneeded = MAX(0, argsneeded);
					break;
				case 'q':
					optquiet = TRUE;
					optcnt++;
					break;
				default: /* '?' */
					usagehelp();
			}
		}

		// Check for correct number of required arguments
		if (argc >= optind + argsneeded) {
			// Command line looks to be correct, so process it now! Multiple commands allowed, process in the order below ...
			if ((optcnt > 1) && (optquiet == FALSE)) 
				printf("Multiple Operations Requested ...\n");
			if (optchk == TRUE)
				oper_check(argv[optind], argv[optind+1], argv[optind+2], argv[optind+3], optcnt > 1, optquiet);
			if (optupd == TRUE)
				oper_update(argv[optind], argv[optind+1], argv[optind+2], argv[optind+3], optcnt > 1, optquiet);
			if (optauto == TRUE)
				oper_auto(argv[optind], argv[optind+1], argv[optind+2], argv[optind+3], optcnt > 1, optquiet);
			if (optdaemon == TRUE)
				oper_daemon(argv[optind], argv[optind+1], argv[optind+2], argv[optind+3], optcnt > 1, optquiet, interval);
			if (optip == TRUE)
				oper_iphost(optcnt > 1, optquiet, macaddr_fmt, GET_IP);
			if (opthost == TRUE)
				oper_iphost(optcnt > 1, optquiet, macaddr_fmt, GET_HOST);
		} else
			usagehelp();
						
	}
	
	// And Exit with success ...
	exit(EXIT_SUCCESS);

}

