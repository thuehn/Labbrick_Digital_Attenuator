#include <stdio.h>
#include <unistd.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <time.h>
#include <sys/time.h>
#include "input.h"
#include "LDAhid.h"

#define FALSE 0
#define TRUE !FALSE
#define STRING_LENGTH 12
#define SLEEP_TIME 100000
#define LINE_LENGTH 256
#define TIME 1
#define ATT 2

struct user_data ud;

/*
 * returns token on given place in .csv file
 */
char*
get_entry(char* line, int entry)
{
	char* token;
	for (token = strtok(line, ";"); token && *token;
	     token = strtok(NULL, ";\n")) {
		if (!--entry)
			return token;
	}
	return NULL;
}

/*
 * follows the path to a given .csv file
 * and checks it for correct entries.
 * time is expected to be in the first entry followed by the
 * attenuation.
 */
int
read_file(char *path, int id)
{
	//TODO: check if more then two entries in file

	int i = 0;
	int isDigit = 0;
	FILE *fp;
	char *tmp, *endptr;
	char line[LINE_LENGTH];
	char atime[250];
	char att[6];

	fp = fopen(path, "r");

	if (fp == NULL) {
                printf("unable to open input file for reading: %s\n", path);
		return -1;
	}

	while (fgets(line, LINE_LENGTH, fp)) {
		tmp = strdup(line);
		ud.atime = atol(get_entry(tmp, TIME));
		tmp = strdup(line);
		ud.attenuation = (int)(atof(get_entry(tmp, ATT))* 4);
		set_attenuation(id);
		free(tmp);
	}

	fclose(fp);
	return 0;
}

/*
 * log the current change of attenuation to a file including
 * a timestamp. Always append the file by default.
 * <timestamp>,<attenuation>
 */
int
log_attenuation(unsigned int att)
{

	if (ud.log != 1)
		return 1;

	FILE *fp;

        fp = fopen(ud.logfile, "a");
        if (fp == NULL) {
                printf("unable to open logfile for writing: %s\n", ud.logfile);
                return 2;
        }

	double real_att = (double) att / 4;
        struct timespec ts;
        clock_gettime (CLOCK_REALTIME, &ts);
        fprintf (fp, "%u.%09u,", (unsigned int) ts.tv_sec, (unsigned int) ts.tv_nsec);
        fprintf(fp, "%.2f\n", real_att); // attenuation

        fclose( fp );

	return 0;
}

/*
 * gets the command line parameters and sets userdata parameters
 */
int
get_parameters(int argc, char *argv[])
{
	int i;
	//TODO: let user set ramp, triangle and/or sine after each other

	//TODO: check for invalid input

	for (i = 1; i < argc; i++) {
		if (strncmp(argv[i], "-a", strlen(argv[i])) == 0) {
			ud.simple = 1;
			if ((i + 1) < argc)
				ud.attenuation = (int)(atof(argv[i + 1]) * 4);
			else
				printf("you set the -a switch, but missed to enter an attenuation\n");
		}

		else if (strncmp(argv[i], "-i", strlen(argv[i])) == 0)
			ud.info = 1;

		else if (strncmp(argv[i], "-t", strlen(argv[i])) == 0)
			if ((i + 1) < argc)
				ud.atime = atol(argv[i + 1]);
			else {
				printf("You missed to set a time\n");
				return 0;
			}

		else if (strncmp(argv[i], "-step", strlen(argv[i])) == 0)
			if ((i + 1) < argc)
				ud.ramp_steps = (int)(atof(argv[i + 1]) * 4);
			else {
				printf("no attenuation steps set\n");
				printf("Step size will be set to device minimum\n");
				//TODO: set steps du resolution if none is set
			}

		else if (strncmp(argv[i], "-step_time", strlen(argv[i])) == 0)
			if ((i + 1) < argc)
				ud.step_time = atoi(argv[i + 1]);
			else{
				printf("no steptime set\n");
				printf("steptime set to: %d\n", ud.step_time);
			}

		else if (strncmp(argv[i], "-start", strlen(argv[i])) == 0)
			if ((i + 1) < argc)
				ud.start_att = (int)(atof(argv[i + 1]) * 4);
			else {
				printf("no start attenuation set\n");
				return 0;
			}


		else if (strncmp(argv[i], "-end", strlen(argv[i])) == 0)
			if ((i + 1) < argc)
				ud.end_att = (int)(atof(argv[i + 1]) * 4);
			else {
				printf("no end attenuation set\n");
				return 0;
			}

		else if (strncmp(argv[i],"-f", strlen(argv[i])) == 0) {
			if ((i + 1) < argc) {
				ud.path = argv[i + 1];
				ud.file = 1;
			}
			else {
				printf("no file specified\n");
			}
		}

		else if (strncmp(argv[i],"s", strlen(argv[i])) == 0) {
				ud.ms = 0;
				ud.us = 0;
		}

		else if (strncmp(argv[i],"ms", strlen(argv[i])) == 0) {
			ud.ms = 1;
		}

		else if (strncmp(argv[i],"us", strlen(argv[i])) == 0) {
			ud.us = 1;
		}
		else if (strncmp(argv[i], "-l", strlen(argv[i])) == 0) {
			if ((i + 1) < argc) {
				ud.logfile = argv[i + 1];
				ud.log = 1;
				printf("logging to file: %s", ud.logfile);
			} else
				printf("please specify a logfile filename\n");
		}

		else if(strncmp(argv[i], "-p", strlen(argv[i])) == 0) {
			if (strncmp(argv[i + 1], "-ramp",
			    strlen(argv[i + 1])) == 0)
				ud.ramp = 1;
			else if (strncmp(argv[i + 1], "-sine",
			    strlen(argv[i + 1])) == 0)
				ud.sine = 1;
			else if (strncmp(argv[i + 1],"-triangle",
			    strlen(argv[i + 1])) == 0)
				ud.triangle = 1;
		}

		else if (strncmp(argv[i], "-r", strlen(argv[i])) == 0) {
			ud.cont = 1;
		}

		else if (strncmp(argv[i], "-rr", strlen(argv[i])) == 0) {
			if ((i + 1) < argc)
				ud.runs = atoi(argv[i+1]);
			else {
				ud.runs = 1;
			}
		}
	}
	return 1;
}

void
print_userdata(void)
{
	char* tu;
	if (ud.us == 1)
		strncpy(tu ,"mikroseconds\0", 20);
	else if(ud.ms == 1)
		strncpy(tu ,"milliseconds\0", 20);
	else
		strncpy(tu ,"seconds\0", 20);

	if (ud.simple == 1) {
		printf("attenuation set to %.2fdB\n", (double)ud.attenuation / 4);
		if (ud.atime != 0)
			printf("time for attenuation set to %ld %s \n", ud.atime, tu);
	}
	if (ud.ramp == 1) {
		printf("attenuation set to ramp\n");
		printf("ramp steps set to %.2fdB\n", (double)ud.ramp_steps / 4);
		printf("start attenuation set to %.2fdB\n", (double)ud.start_att / 4);
		printf("end attenuation set to %.2fdB\n", (double)ud.end_att / 4);
		printf("time per step set to %d %s\n", ud.step_time, tu);
	}
	if (ud.triangle == 1) {
		printf("attenuation form set to both sided ramp\n");
		printf("ramp steps set to %.2fdB\n", (double)ud.ramp_steps / 4);
		printf("start attenuation set to %.2fdB\n", (double)ud.start_att / 4);
		printf("maximal attenuation set to %.2fdB\n", (double)ud.end_att / 4);
		printf("time per step set to %d %s\n", ud.step_time, tu);
	}
	if (ud.cont == 1)
		printf("continous behavior is set\n");
		if (ud.runs > 1)
			printf("for %d runs\n", ud.runs);
	if (ud.sine == 1)
		printf("attenuation set to sine\n");
}

void
clear_userdata(void)
{
	ud.atime = 0;
	ud.attenuation = 0;
	ud.start_att = 0;
	ud.end_att = 0;
	ud.ramp = 0;
	ud.sine = 0;
	ud.triangle = 0;
	ud.ramp_steps = 1;
	ud.cont = 0;
	ud.step_time = SLEEP_TIME;
	ud.simple = 0;
	ud.file = 0;
	ud.info = 0;
	ud.runs = 1;
	ud.path = "";
	ud.log = 0;
	ud.logfile = "";
}
