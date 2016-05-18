#include <stdio.h>
#include <unistd.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <time.h>
#include <sys/time.h>
#include "input.h"
#include "control.h"
#include "LDAhid.h"

#define FALSE 0
#define TRUE !FALSE
#define STRING_LENGTH 12
#define SLEEP_TIME 100000
#define LINE_LENGTH 256
#define TIME 1
#define ATT 2

/*
 * returns token on given place in .csv file
 */
char*
get_entry(char* line, int entry)
{
	char* token;
	for (token = strtok(line, ","); token && *token;
	     token = strtok(NULL, ",\n")) {
		if (!--entry)
			return token;
	}
	return NULL;
}

/*
 * open .csv file and checks it for correct entries.
 * time is expected to be in the first entry followed by the
 * attenuation.
 * @param path: path to config file
 * @param id: device id
 * @param ud: user data struct
 * @return: 0 on success, 1 if not possible to open file
 */
int
read_file(char *path, int id,struct user_data *ud)
{
	FILE *fp;
	char *tmp;
	char line[LINE_LENGTH];

	fp = fopen(path, "r");

	if (fp == NULL) {
                printf(ERR "unable to open input file for reading: %s\n", path);
		return 1;
	}

	while (fgets(line, LINE_LENGTH, fp)) {
		tmp = strdup(line);
		ud->atime = atol(get_entry(tmp, TIME));
		tmp = strdup(line);
		ud->attenuation = (int)(atof(get_entry(tmp, ATT))* 4);
		set_attenuation(id, ud);
		free(tmp);
	}

	fclose(fp);

	if (!ud->cont && (ud->runs >= 1)) {
		ud->runs -= 1;
		if (ud->runs == 0)
			return 1;
		else
			return 0;
	}

	return 0;
}

/*
 * log the current change of attenuation to a file including
 * a timestamp. Always append the file by default.
 * <timestamp>,<attenuation>
 * @param att: attenuation in db
 * @param ud: user data struct
 * @return: return 0 on success, 1 if no log, 2 if logfile couldn't be opened
 */
int
log_attenuation(unsigned int att, struct user_data *ud)
{

	if (ud->log != 1)
		return 1;

	FILE *fp;

        fp = fopen(ud->logfile, "a");
        if (fp == NULL) {
                printf(ERR "unable to open logfile for writing: %s\n", ud->logfile);
                return 2;
        }

	double real_att = (double) att / 4;
        struct timespec ts;
        clock_gettime (CLOCK_REALTIME, &ts);
        fprintf (fp, "%u.%09u,", (unsigned int) ts.tv_sec, (unsigned int) ts.tv_nsec);
        fprintf(fp, "%.2f\n", real_att);

        fclose( fp );

	return 0;
}

/*
 * gets the command line parameters and sets userdata parameters
 * @param argc: argument count
 * @param argv: argument array to read from
 * @param ud: user data struct
 * @return: 1 on success, 0 on error
 */
int
get_parameters(int argc, char *argv[], struct user_data *ud)
{
	int i, quiet;

	quiet = check_quiet(argc, argv);

	for (i = 1; i < argc; i++) {
		printf(WARN "argv[%d]: %s\n",i, argv[i]);
		if (strncmp(argv[i], "-a", strlen(argv[i])) == 0) {
			ud->simple = 1;
			if ((i + 1) < argc)
				ud->attenuation = (int)(atof(argv[i + 1]) * 4);
			else {
				printf(ERR "you set the -a switch, but missed to enter an attenuation\n");
				return 0;
			}
		} else if (strncmp(argv[i], "-i", strlen(argv[i])) == 0) {
			ud->info = 1;
		} else if (strncmp(argv[i], "-t", strlen(argv[i])) == 0) {
			if ((i + 1) < argc)
				ud->atime = atol(argv[i + 1]);
			else {
				printf(ERR "You missed to set a time\n");
				return 0;
			}
		} else if (strncmp(argv[i], "-step", strlen(argv[i])) == 0) {
			if ((i + 1) < argc)
				ud->ramp_steps = (int)(atof(argv[i + 1]) * 4);
			else {
				printf(WARN "no attenuation steps set\n");
				printf(WARN "Step size will be set to device minimum\n");
				//TODO: set steps du resolution if none is set
			}
		} else if (strncmp(argv[i], "-start", strlen(argv[i])) == 0) {
			if ((i + 1) < argc)
				ud->start_att = (int)(atof(argv[i + 1]) * 4);
			else {
				printf(ERR "no start attenuation set\n");
				return 0;
			}
		} else if (strncmp(argv[i], "-end", strlen(argv[i])) == 0) {
			if ((i + 1) < argc)
				ud->end_att = (int)(atof(argv[i + 1]) * 4);
			else {
				printf(ERR "no end attenuation set\n");
				return 0;
			}
		} else if (strncmp(argv[i],"-f", strlen(argv[i])) == 0) {
			if ((i + 1) < argc) {
				strncpy(ud->path, argv[i + 1], MAX_LENGTH - 1);
				ud->path[MAX_LENGTH - 1] = '\0';
				ud->file = 1;
			}
			else {
				printf(ERR "no file specified\n");
				return 0;
			}
		} else if (strncmp(argv[i], "-q", strlen(argv[i])) == 0) {
			ud->quiet = 1;
		/* set time unit us/ms/s */
		} else if (strncmp(argv[i],"s", strlen(argv[i])) == 0) {
				ud->ms = 0;
				ud->us = 0;
				if (!quiet)
					printf(INFO "time in seconds\n");
		} else if (strncmp(argv[i],"ms", strlen(argv[i])) == 0) {
			ud->ms = 1;
			if (!quiet)
				printf(INFO "time in milliseconds\n");
		} else if (strncmp(argv[i],"us", strlen(argv[i])) == 0) {
			ud->us = 1;
			if (!quiet)
				printf(INFO "time in useconds\n");
		} else if (strncmp(argv[i], "-l", strlen(argv[i])) == 0) {
			if ((i + 1) < argc) {
				strncpy(ud->logfile, argv[i + 1], MAX_LENGTH - 1);
				ud->logfile[MAX_LENGTH - 1] = '\0';
				ud->log = 1;
				if (!quiet)
					printf(INFO "logging to file: %s\n", ud->logfile);
			} else {
				printf(ERR "please specify a logfile filename\n");
				return 0;
			}
		} else if (strncmp(argv[i], "-ramp\0", strlen(argv[i]) + 1) == 0) {
				ud->ramp = 1;
		} else if (strncmp(argv[i],"-triangle", strlen(argv[i])) == 0) {
				ud->triangle = 1;
		}else if (strncmp(argv[i], "-r\0", strlen(argv[i]) + 1) == 0) {
			ud->cont = 1;
		} else if (strncmp(argv[i], "-rr", strlen(argv[i])) == 0) {
			if ((i + 1) < argc) {
				ud->runs = atoi(argv[i+1]);
			} else {
				ud->runs = 1;
			}
		}
	}
	return 1;
}

/*
 * reset user data
 * @param ud: user data struct
 */
void
clear_userdata(struct user_data *ud)
{
	ud->atime = 1;
	ud->us = 0;
	ud->ms = 0;
	ud->attenuation = 0;
	ud->start_att = 0;
	ud->end_att = 0;
	ud->ramp = 0;
	ud->triangle = 0;
	ud->ramp_steps = 1;
	ud->cont = 0;
	ud->simple = 0;
	ud->file = 0;
	ud->info = 0;
	ud->runs = 1;
	ud->log = 0;
	ud->quiet=0;
	memset(ud->path, '\0', sizeof(ud->path));
	memset(ud->logfile, '\0', sizeof(ud->logfile));
}

