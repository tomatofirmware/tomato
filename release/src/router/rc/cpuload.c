#include "rc.h"
#include <shared.h>
#include <string.h>
#include <wlutils.h>

#define	FILE_HISTORY	"/www/ext/cpuload.bin"
#define	FILE_TEMP		"/www/ext/cpuload.tmp"
#define	FILE_LIVE		"/www/ext/cpulast.bin"

#define	LEN_COMMAND		128							// Length: Command to be executed
#define	LEN_RESULT		128							// Length: Result from command
#define LEN_PARSED		64							// Length: Parsed Outputs
#define	LEN_OUTPUT		128							// Length: Output String (Time + CPU data)


// CPULoad: Store CPU Load information (history in /www/ext/cpuload.bin, last entry in /www/ext/cpulast.bin)
void usagehelp_cpuload()
{
	fprintf(stderr, "usage: cpuload interval minValidTime history chkInterval\n");
	fprintf(stderr, "\nArguments: \n");
	fprintf(stderr, "   interval       Update Interval (to store CPU load), in seconds\n");
	fprintf(stderr, "   minValidTime   Minimum Valid Time for entries (to allow NTP before starting to record), in seconds\n");
	fprintf(stderr, "   history        History (duration) to maintain CPU load data, in hours\n");
	fprintf(stderr, "   chkInterval    History check interval, in minutes. 0 to disable checking.\n");
	fprintf(stderr, "\nNote: Checking is blocking - so no corruption will occur, but CPU Load samples may be missed.\n");
	fprintf(stderr, "\n");
 	exit(EXIT_FAILURE);
}


static unsigned int write_header()
{
	FILE *f;

	if((f = fopen(FILE_HISTORY, "w"))==NULL) {
		syslog(LOG_ERR, "Error Opening History File");		
		return FALSE;
	}
	fprintf(f, "DateTime,1 min,5 min,10 min,Last PID\n");
	fclose(f);
	return TRUE;
}

static unsigned int write_data(unsigned long epochTime, char *cpuload)
{
	FILE *f;
	char outputstr[LEN_OUTPUT];

	// Build Output String -> HighCharts / HighStock uses Epoch Time in milliseconds, so multiply by 1000
	// -> Except here, keep Epoch Time in seconds, to avoid having to use unsigned long long everwhere!
	sprintf(outputstr, "%llu,%s", 1000*(unsigned long long)epochTime, cpuload);

	// Append String to History File
	if((f = fopen(FILE_HISTORY, "a"))==NULL) {
		syslog(LOG_ERR, "Error Opening History File");		
		return FALSE;
	}
	fprintf(f, "%s", outputstr);		
	fclose(f);

	// Write String to Live File
	if((f = fopen(FILE_LIVE, "w"))==NULL) {
		syslog(LOG_ERR, "Error Opening Live File");		
		return FALSE;
	}
	fprintf(f, "%s", outputstr);		
	fclose(f);

	return TRUE;
}

static unsigned long shell_cmd(char *command, char *cmdresult)
{
	FILE *mypipe;

	// Set up pipe for command read, and execute command
	//syslog(LOG_INFO, "Shell Command: |%s|", command);
	if ((mypipe = popen(command,"r")) == NULL) {
		syslog(LOG_ERR, "Could not open pipe for output.");
 		return FALSE;
	}

	// Grab data from process execution
	cmdresult[0] = '0';
	fgets(cmdresult, LEN_RESULT , mypipe);

	// And close pipe
	if (pclose(mypipe) == (int)-1) {
		//syslog below disabled, as seems to fill syslog with errors -> but no real error (data is fine)
		// -> Perhaps a timing issue for waitPID / wait4? Ignore, not really an issue!
		//syslog(LOG_ERR, "Failed to close command stream.");
 		return FALSE;
	}

 	return TRUE;
}


int cpuload_main(int argc, char *argv[])
{
	unsigned long interval;
	unsigned long minValidTime;
	unsigned long history;
	unsigned long chkInterval;
	unsigned long chkLastTime;

	char command[LEN_COMMAND];
	char cmdresult[LEN_RESULT];
	unsigned long epochTime;

	// Check for correct number of arguments
	if (argc != 5)
		usagehelp_cpuload();
	
	// Fork new process, run in the background (daemon)
	if (fork() != 0) return 0;
	setsid();
	signal(SIGCHLD, chld_reap);
	
	// Extract the parameters from the command line, convert to desired units
	interval = atof(argv[1]);			// Loop (sleep) interval, in seconds
	minValidTime = atof(argv[2]);		// Minimum time for valid entry, in seconds
	history = 3600*atof(argv[3]);		// History to maintain, convert hours to seconds
	chkInterval = 60*atof(argv[4]);		// History check interval, convert minutes to seconds

	// Write Header Row to History File (only at program startup, i.e. done once only)
	write_header();

	// Initialize History File Check Time (-1 means never checked, will force check on next run)
	chkLastTime = -1;

	// Loop Through, getting data and writing to file at specified interval
	while(1){
		// Get date (Epoch / Unix time, in seconds)
		shell_cmd("date +%s", cmdresult);
		//syslog(LOG_INFO, "Command Result (date): |%s|", cmdresult);
		epochTime = atol(cmdresult);

		// Check if epochTime is valid (above minimum threshold)
		if (epochTime >= minValidTime) {

			// Get cpuload data (3 Float for load, 1 UInt for Last PID)
			shell_cmd("cat /proc/loadavg | sed 's/ \\{1,\\}/,/g' | cut -f 1-3,5 -d,", cmdresult);
			//syslog(LOG_INFO, "Command Result (cpuload): |%s|", cmdresult);

			// Write string to target files
			write_data(epochTime, cmdresult);

			// And finally, see if it's time for a History File check (to remove data older than the specified limit)
			if (epochTime >= (chkLastTime + chkInterval)) {
				// Update Last Check Time
				chkLastTime = epochTime;
				syslog(LOG_INFO, "Executing History Check, Epoch Time: %ld", epochTime);
				// Execute Shell Commands to Complete Check
				sprintf(command, "awk '(NR==1) || (($1/1000) >= %ld)' %s > %s", epochTime - history, FILE_HISTORY, FILE_TEMP);
				shell_cmd(command, cmdresult);
				sprintf(command, "mv %s %s", FILE_TEMP, FILE_HISTORY);
				shell_cmd(command, cmdresult);
			}
		} else
			syslog(LOG_INFO, "Reported Epoch Time < Threshold, ignored! Time = %ld sec", epochTime);

		// And sleep, until next sample
		sleep(interval);
	}
}

