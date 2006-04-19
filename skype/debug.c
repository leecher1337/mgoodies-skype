#include "debug.h"

#ifdef _DEBUG
#include <windows.h>
#include <stdio.h>
#include <time.h>
#include <string.h>

char logfile[MAX_PATH]="skype_log.txt";

void init_debug(void) {
	char *p;
		
	ZeroMemory(logfile, sizeof(logfile));
	GetModuleFileName(NULL, logfile, sizeof(logfile));
	p=logfile+strlen(logfile);
	while (*p!='\\' && p>logfile) p--;
	if (p>logfile) {
		p[1]=0;
		strncat(logfile, "skype_log.txt", sizeof(logfile));
	}
}

void log_write(char *prfx, char *text) {
	FILE *stream;
	time_t lt;
	char *ct;

	stream=fopen(logfile, "a");
	time(&lt);
	ct=ctime(&lt);
	ct[strlen(ct)-1]=0;
	fprintf(stream, "%s   %s %s\n", ct, prfx, text);
	fclose(stream);
}
void log_long(char *prfx, long text) {
	FILE *stream;
	time_t lt;
	char *ct;

	stream=fopen(logfile, "a");
	time(&lt);
	ct=ctime(&lt);
	ct[strlen(ct)-1]=0;
	fprintf(stream, "%s   %s %d\n", ct, prfx, text);
	fclose(stream);
}
#endif