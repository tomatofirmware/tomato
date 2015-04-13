#include "rc.h"
#include <shared.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#define	LEN_COMMAND		128							// Length: Command to be executed
#define	LEN_RESULT		128							// Length: Result from command
#define LEN_PARSED		32							// Length: Parsed Outputs (IP, Host, Lease, Time Units)

#define	GET_IP			0x01						// ARP Data: Get IP Address
#define	GET_HOST		0x02						// ARP Data: Get Hostname
#define	GET_LEASE		0x04						// ARP Data: Get Lease (time)
#define	GET_UNITS		0x08						// ARP Data: Get Lease (units)

#define MIN(a, b) (((a) < (b)) ? (a) : (b))			// Macro: Maximum of inputs
#define MAX(a, b) (((a) > (b)) ? (a) : (b)) 		// Macro: Minimum of inputs

// Remote-ARP: Retrieve ARP information from remote machine (via SSH)
void usagehelp_remote_arp()
{
	fprintf(stderr, "usage: remote-arp [-ihlu] MAC server user password\n");
	fprintf(stderr, "\nOperations: \n");
	fprintf(stderr, "   -i        Return IP address from remote ARP data, based on MAC address\n");
	fprintf(stderr, "   -h        Return Hostname from remote ARP data, based on MAC address\n");
	fprintf(stderr, "   -l        Return Lease (time) from remote ARP data, based on MAC address\n");
	fprintf(stderr, "   -u        Return Lease (units) from remote ARP data, based on MAC address\n");
	fprintf(stderr, "\n");
 	exit(EXIT_FAILURE);
}

int oper_arp(char *mac, char *server, char *user, char *password, int mask) {
	char command[LEN_COMMAND];
	char cmdresult[LEN_RESULT];
	FILE *mypipe;
	char parse[4][LEN_PARSED];
	char cntout, addcomma;

	// Configure ARP Command
	sprintf(command, "export DROPBEAR_PASSWORD=%s; ssh -y -y %s@%s arp -a | grep -i %s 2>/dev/null", password, user, server, mac);

	// Set up pipe for command read, and execute command
	if ((mypipe = popen(command,"r")) == NULL) {
		fprintf(stderr, "remote-arp: Could not open pipe for output.\n");
 		return FALSE;
	}

	// Grab data from process execution, convert to number (long)
	cmdresult[0] = '0';
	fgets(cmdresult, LEN_RESULT , mypipe);

	// And close pipe
	if (pclose(mypipe) == (int)-1) {
		fprintf(stderr,"remote-arp: Failed to close command stream.\n");
 		return FALSE;
	}

	// Parse Outpur from (remote) ARP
	if (sscanf(cmdresult, "%s (%[^)]) %*s %*s %*s %*s %*s %*s %s %s", parse[1], parse[0], parse[2], parse[3]) == 4) {
		// Print out targeted ARP values
		addcomma = FALSE;
		for (cntout = 0; cntout <= 3; cntout++) {
			if ((mask & (0x01 << cntout)) != 0) {
				if (addcomma == TRUE)
					printf(",");
				addcomma = TRUE;
				printf("%s", parse[cntout]);
			}
		}
		printf("\n");
	} else
		printf("ARP parsing failed!\n");

 	return TRUE;
}


int remote_arp_main(int argc, char *argv[])
{
	// State variables are static only so the are initialized to zero (saves adding a bunch of code for this ... :-))
	static int opt, optmask;
	int argsneeded;
	
	// If no arguments, print help
	if (argc <= 1)
		usagehelp_remote_arp();
		
	else {
		// Get Command Line Options, set up flags / values as needed
		argsneeded = 0;
		while ((opt = getopt(argc, argv, "ihlu")) != -1) {
			switch (opt) {
				case 'i':
					optmask |= GET_IP;
					argsneeded = MAX(4, argsneeded);
					break;
				case 'h':
					optmask |= GET_HOST;
					argsneeded = MAX(4, argsneeded);
					break;
				case 'l':
					optmask |= GET_LEASE;
					argsneeded = MAX(4, argsneeded);
					break;
				case 'u':
					optmask |= GET_UNITS;
					argsneeded = MAX(4, argsneeded);
					break;
				default: /* '?' */
					usagehelp_remote_arp();
 					exit(EXIT_SUCCESS);
			}
		}

		// Check for correct number of required arguments
		if ((argc >= optind + argsneeded) && (optmask != 0)) {
			// Command line looks to be correct, so process it now!
			oper_arp(argv[optind], argv[optind+1], argv[optind+2], argv[optind+3], optmask);
		} else
			usagehelp_remote_arp();
						
	}
	
	// And Exit with success ...
 	exit(EXIT_SUCCESS);

}

