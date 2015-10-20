#include <stdio.h>
#include <unistd.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include "control.h"
#include "input.h"
#include "LDAhid.h"

#define _GNU_SOURCE
#define FALSE 0
#define TRUE !FALSE
#define STRING_LENGTH 12
#define SLEEP_TIME 1000000
#define MAX_USLEEP_TIME 1000000
#define LINE_LENGTH 256
#define TIME 1
#define ATT 2
#define RAMP 1
#define TRIANGLE 1
#define SIMPLE 0

struct user_data ud;

//TODO: currently only useable with one device at some points
//even if it is possible to use several devices they will be handled
//one after the other

/*
 * Get the model name and serial number of connected devices
 * @param device_count: number of devices connected
 * @param device_name: storage location for device name
 */
void
get_serial_and_name(int device_count, char *device_name)
{
	DEVID id;
	unsigned int serial;

	for (id = 1; id <= device_count; id++) {
		printf("Device %d has Modelname: ", id);
		fnLDA_GetModelName(id, device_name);
		serial = fnLDA_GetSerialNumber(id);
		printf("%s with serial number %d\n", device_name, serial);
	}
}

/*
 * use usleep in a safe way for systems not supporting more than 1,000,000
 * microseconds for usleep
 * @param usec: time to sleep
 * @return: returns 0 if time is up
 */
int
susleep(unsigned long usec)
{
	int res;
	unsigned long timeleft = usec;
	while (timeleft > MAX_USLEEP_TIME) {
		res = usleep(MAX_USLEEP_TIME);
		if (res != 0)
			return res;
		timeleft -= MAX_USLEEP_TIME;
	}
	return usleep(timeleft);
}

void
print_dev_info(int id)
{
	printf("Attenuation is set to: %.2f\n",
		(double)(fnLDA_GetAttenuation(id) / 4));
}

/*
 * check device status and return error state if something is wrong
 * @param current_device: id of current device
 * @return: returns success, or error message
 */
char *
get_device_data(unsigned int current_device)
{
	char *success = "Successfully checked device ";
	int status;

	status = fnLDA_GetAttenuation(current_device);
	if (status == INVALID_DEVID
	    || status == DEVICE_NOT_READY)
		return strncpy(errmsg ,fnLDA_perror(status),
		     strlen(errmsg));

	status = fnLDA_GetMinAttenuation(current_device);
	if (status == INVALID_DEVID
	    || status == DEVICE_NOT_READY)
		return strncpy(errmsg ,fnLDA_perror(status),
		     strlen(errmsg));

	status = fnLDA_GetMaxAttenuation(current_device);
	if (status == INVALID_DEVID
	    || status == DEVICE_NOT_READY)
		return strncpy(errmsg ,fnLDA_perror(status),
		     strlen(errmsg));

	status = fnLDA_GetIdleTime(current_device);
	if (status == INVALID_DEVID
	    || status == DEVICE_NOT_READY)
		return strncpy(errmsg ,fnLDA_perror(status),
		     strlen(errmsg));

	status = fnLDA_GetDwellTime(current_device);
	if (status == INVALID_DEVID
	    || status == DEVICE_NOT_READY)
		return strncpy(errmsg ,fnLDA_perror(status),
		     strlen(errmsg));

	status = fnLDA_GetAttenuationStep(current_device);
	if (status == INVALID_DEVID
	    || status == DEVICE_NOT_READY)
		return strncpy(errmsg ,fnLDA_perror(status),
		     strlen(errmsg));

	status = fnLDA_GetRF_On(current_device);
	if (status == INVALID_DEVID
	    || status == DEVICE_NOT_READY)
		return strncpy(errmsg ,fnLDA_perror(status),
		     strlen(errmsg));

	status = fnLDA_GetRampStart(current_device);
	if (status == INVALID_DEVID
	    || status == DEVICE_NOT_READY)
		return strncpy(errmsg ,fnLDA_perror(status),
		     strlen(errmsg));

	status = fnLDA_GetRampEnd(current_device);
	if (status == INVALID_DEVID
	    || status == DEVICE_NOT_READY)
		return strncpy(errmsg ,fnLDA_perror(status),
		     strlen(errmsg));
	return success;
}

/*
 * help function to display possible options and correct usage
 */
void
call_help(void)
{
	printf("-to show this overview use\n");
	printf("\t-h\n");
	printf("\r\n");

	printf("-set attenuation with\n");
	printf("\t-a <attenuation in dB>\n");
	printf("\r\n");

	printf("-set time for attenuation duration with\n");
	printf("\t-t <time in sec>\n");
	printf("\r\n");

	printf("you can use s, ms and us to set time units\n");
	printf("\ts -> seconds\n");
	printf("\tms -> milliseconds\n");
	printf("\tus -> microseconds\n");
	printf("\r\n");

	printf("-set attenuation form with\n");
	printf("\t-p <ramp|triangle>\n");
	printf("\r\n");

	printf("-set starting attenuation strength in dB with\n");
	printf("\t-start <dB>\n");
	printf("\r\n");

	printf("-set end attenuation strength in dB with\n");
	printf("\t-end <dB>\n");
	printf("\r\n");

	printf("-set time per step with\n");
	printf("\t-step_time <step_time>\n");
	printf("\r\n");

	printf("-to use a .csv file\n");
	printf("\t-f path/to/file\n");
	printf("csv file is expected to have time;attenuation format\n");
	printf("\r\n");

	printf("-log attenuation changes to a .csv file\n");
	printf("\t-l path/to/logfile\n");
	printf("\r\n");

	printf("repeat form, or file input until canceled by user\n");
	printf("\t-r\n");
	printf("\r\n");

	printf("repeat form, or file input for several times\n");
	printf("\t-rr <#runs>\n");
	printf("\r\n");

	return;
}

/*
 * check if attenuation is above, or below device limits
 * @param id: device id
 * @param ud: user data struct to check attenuation from
 * @param check: 
 */
void
check_att_limits(int id, struct user_data *ud, int check)
{
	/* check for simple case */
	if (check == 0) {
		if (ud->attenuation < fnLDA_GetMinAttenuation(id)) {
			printf("%.2f is below minimal attenuation of %.2f\n",
				(double)ud->attenuation / 4,
				(double)fnLDA_GetMinAttenuation(id) / 4);
			printf("attenuation has been set to %.2fdB\n",
				(double)fnLDA_GetMinAttenuation(id) / 4);
			fnLDA_SetAttenuation(id, fnLDA_GetMinAttenuation(id));
			log_attenuation( fnLDA_GetMinAttenuation(id) );
		} else if (ud->attenuation > fnLDA_GetMaxAttenuation(id)) {
			printf("%.2f is above maximal attenuation of %.2f\n",
				(double)ud->attenuation / 4,
				(double)fnLDA_GetMaxAttenuation(id) / 4);
			printf("attenuation has been set to %.2f\n",
				(double)fnLDA_GetMaxAttenuation(id) / 4);
			fnLDA_SetAttenuation(id, fnLDA_GetMaxAttenuation(id));
			log_attenuation(fnLDA_GetMaxAttenuation(id));
		} else {
			fnLDA_SetAttenuation(id, (ud->attenuation));
			log_attenuation( ud->attenuation );
			printf("set device to %.2fdB attenuation\n",
				(double)(fnLDA_GetAttenuation(id)) / 4);
		}
	}

	/* check for start and end attenuation */
	if (check == 1) {
		if (ud->start_att < fnLDA_GetMinAttenuation(id)) {
			printf("%.2f is below minimal attenuation of %.2f\n",
				(double)ud->start_att / 4,
				(double)fnLDA_GetMinAttenuation(id) / 4);
			printf("start attenuation has been set to %.2fdB\n",
				(double)fnLDA_GetMinAttenuation(id));
			ud->start_att = fnLDA_GetMinAttenuation(id);
		}
		if (ud->start_att > fnLDA_GetMaxAttenuation(id)) {
			printf("%.2f is above maximal attenuation of %.2f\n",
				(double)ud->start_att / 4, 
				(double)fnLDA_GetMaxAttenuation(id) / 4);
			printf("start attenuation has been set to %.2f\n",
				(double)fnLDA_GetMaxAttenuation(id) / 4);
			ud->start_att = fnLDA_GetMaxAttenuation(id);
		}
		if (ud->end_att < fnLDA_GetMinAttenuation(id)) {
			printf("%.2f is below minumal attenuation of %.2f\n",
				(double)ud->end_att / 4,
				(double)fnLDA_GetMinAttenuation(id) / 4);
			printf("final attenuation has been set to %.2fdB\n",
				(double)fnLDA_GetMinAttenuation(id) / 4);
			ud->end_att = fnLDA_GetMinAttenuation(id);
		}
		if (ud->end_att > fnLDA_GetMaxAttenuation(id)) {
			printf("%.2f is above maximal attenuation of %.2f\n",
				(double)ud->end_att,
				(double)fnLDA_GetMaxAttenuation(id));
			printf("final attenuation has been set to %.2f\n",
				(double)fnLDA_GetMinAttenuation(id) / 4);
			ud->end_att = fnLDA_GetMaxAttenuation(id);
		}
	}
}

/*
 * checks if attenutaion is outside of devices limits and sets
 * attenuation stepwise up or down to get a ramp like form
 * @param id: device id
 */
void
set_ramp_new(int id, struct user_data *ud)
{
	int i, cur_att;

	check_att_limits(id, ud, RAMP);

	if (ud->cont && (ud->start_att < ud->end_att)) {
		for(;;) {
			fnLDA_SetAttenuation(id, ud->start_att);
			log_attenuation( ud->start_att );
			for(i = 0; i <= (ud->end_att - ud->start_att); i++) {
				if (ud->us == 1)
					susleep(TIME_MICROS(ud->atime));
				else if(ud->ms == 1)
					susleep(TIME_MILLIS(ud->atime));
				else
					susleep(TIME_SECONDS(ud->atime));
				cur_att = fnLDA_GetAttenuation(id);
				//printf("cur_att %d\n", cur_att);
				fnLDA_SetAttenuation(id,
					cur_att + ud->ramp_steps);
				log_attenuation( cur_att + ud->ramp_steps );
			}
		}
	}
	else if (ud->cont && (ud->start_att > ud->end_att)) {
		for(;;) {
			fnLDA_SetAttenuation(id, ud->start_att);
			log_attenuation( ud->start_att );
			for(i = 0; i <= (ud->start_att - ud->end_att); i++) {
				if (ud->us == 1)
					susleep(TIME_MICROS(ud->atime));
				else if(ud->ms == 1)
					susleep(TIME_MILLIS(ud->atime));
				else
					susleep(TIME_SECONDS(ud->atime));
				cur_att = fnLDA_GetAttenuation(id);
				//printf("cur_att %d\n", cur_att);
				fnLDA_SetAttenuation(id,
					cur_att - ud->ramp_steps);
				log_attenuation( cur_att - ud->ramp_steps );
			}
		}
	}
	else if (ud->start_att < ud->end_att) {
		fnLDA_SetAttenuation(id, ud->start_att);
		log_attenuation( ud->start_att );
		for(i = 0; i <= (ud->end_att - ud->start_att); i++) {
			if (ud->us == 1)
				susleep(TIME_MICROS(ud->atime));
			else if(ud->ms == 1)
				susleep(TIME_MILLIS(ud->atime));
			else
				susleep(TIME_SECONDS(ud->atime));
			cur_att = fnLDA_GetAttenuation(id);
			//printf("cur_att %d\n", cur_att);
			fnLDA_SetAttenuation(id,
				cur_att + ud->ramp_steps);
			log_attenuation( cur_att + ud->ramp_steps );
		}
	}
	else if (ud->start_att > ud->end_att) {
		fnLDA_SetAttenuation(id, ud->start_att);
		log_attenuation( ud->start_att );
		for(i = 0; i <= (ud->start_att - ud->end_att); i++) {
			if (ud->us == 1)
				susleep(TIME_MICROS(ud->atime));
			else if(ud->ms == 1)
				susleep(TIME_MILLIS(ud->atime));
			else
				susleep(TIME_SECONDS(ud->atime));
			cur_att = fnLDA_GetAttenuation(id);
			//printf("cur_att %d\n", cur_att);
			fnLDA_SetAttenuation(id,
				cur_att - ud->ramp_steps);
			log_attenuation( cur_att - ud->ramp_steps );
		}
	}
}

/*
 * checks if attenutaion is outside of devices limits and sets
 * attenuation stepwise up or down to get a ramp like form
 * @param id: device id
 * @return: returns 0 on success
 */
int
set_ramp(int id)
{
	int i, cur_att;

	if (ud.start_att < fnLDA_GetMinAttenuation(id)) {
		printf("%.2f is below minimal attenuation of %.2f\n",
			(double)ud.start_att / 4,
			(double)fnLDA_GetMinAttenuation(id) / 4);
		printf("start attenuation has been set to %.2fdB\n",
			(double)fnLDA_GetMinAttenuation(id));
		ud.start_att = fnLDA_GetMinAttenuation(id);
	}
	if (ud.start_att > fnLDA_GetMaxAttenuation(id)) {
		printf("%.2f is above maximal attenuation of %.2f\n",
			(double)ud.start_att / 4, 
			(double)fnLDA_GetMaxAttenuation(id) / 4);
		printf("start attenuation has been set to %.2f\n",
			(double)fnLDA_GetMaxAttenuation(id) / 4);
		ud.start_att = fnLDA_GetMaxAttenuation(id);
	}
	if (ud.end_att < fnLDA_GetMinAttenuation(id)) {
		printf("%.2f is below minumal attenuation of %.2f\n",
			(double)ud.end_att / 4,
			(double)fnLDA_GetMinAttenuation(id) / 4);
		printf("final attenuation has been set to %.2fdB\n",
			(double)fnLDA_GetMinAttenuation(id) / 4);
		ud.end_att = fnLDA_GetMinAttenuation(id);
	}
	if (ud.end_att > fnLDA_GetMaxAttenuation(id)) {
		printf("%.2f is above maximal attenuation of %.2f\n",
			(double)ud.end_att,
			(double)fnLDA_GetMaxAttenuation(id));
		printf("final attenuation has been set to %.2f\n",
			(double)fnLDA_GetMinAttenuation(id) / 4);
		ud.end_att = fnLDA_GetMaxAttenuation(id);
	}

	if (ud.cont && (ud.start_att < ud.end_att)) {
		for(;;) {
			fnLDA_SetAttenuation(id, ud.start_att);
			log_attenuation( ud.start_att );
			for(i = 0; i <= (ud.end_att - ud.start_att); i++) {
				if (ud.us == 1)
					susleep(TIME_MICROS(ud.atime));
				else if(ud.ms == 1)
					susleep(TIME_MILLIS(ud.atime));
				else
					susleep(TIME_SECONDS(ud.atime));
				cur_att = fnLDA_GetAttenuation(id);
				//printf("cur_att %d\n", cur_att);
				fnLDA_SetAttenuation(id,
					cur_att + ud.ramp_steps);
				log_attenuation( cur_att + ud.ramp_steps );
			}
		}
	}
	else if (ud.cont && (ud.start_att > ud.end_att)) {
		for(;;) {
			fnLDA_SetAttenuation(id, ud.start_att);
			log_attenuation( ud.start_att );
			for(i = 0; i <= (ud.start_att - ud.end_att); i++) {
				if (ud.us == 1)
					susleep(TIME_MICROS(ud.atime));
				else if(ud.ms == 1)
					susleep(TIME_MILLIS(ud.atime));
				else
					susleep(TIME_SECONDS(ud.atime));
				cur_att = fnLDA_GetAttenuation(id);
				//printf("cur_att %d\n", cur_att);
				fnLDA_SetAttenuation(id,
					cur_att - ud.ramp_steps);
				log_attenuation( cur_att - ud.ramp_steps );
			}
		}
	}
	else if (ud.start_att < ud.end_att) {
		fnLDA_SetAttenuation(id, ud.start_att);
		log_attenuation( ud.start_att );
		for(i = 0; i <= (ud.end_att - ud.start_att); i++) {
			if (ud.us == 1)
				susleep(TIME_MICROS(ud.atime));
			else if(ud.ms == 1)
				susleep(TIME_MILLIS(ud.atime));
			else
				susleep(TIME_SECONDS(ud.atime));
			cur_att = fnLDA_GetAttenuation(id);
			//printf("cur_att %d\n", cur_att);
			fnLDA_SetAttenuation(id,
				cur_att + ud.ramp_steps);
			log_attenuation( cur_att + ud.ramp_steps );
		}
	}
	else if (ud.start_att > ud.end_att) {
		fnLDA_SetAttenuation(id, ud.start_att);
		log_attenuation( ud.start_att );
		for(i = 0; i <= (ud.start_att - ud.end_att); i++) {
			if (ud.us == 1)
				susleep(TIME_MICROS(ud.atime));
			else if(ud.ms == 1)
				susleep(TIME_MILLIS(ud.atime));
			else
				susleep(TIME_SECONDS(ud.atime));
			cur_att = fnLDA_GetAttenuation(id);
			//printf("cur_att %d\n", cur_att);
			fnLDA_SetAttenuation(id,
				cur_att - ud.ramp_steps);
			log_attenuation( cur_att - ud.ramp_steps );
		}
	}
}

/*
 * Sets attenuation to a level defined by user if
 * not above Max or below Min attenuation of the connected
 * attenuator. The Device will keep the attenuation for the
 * time given by the user or the standard sleeptime.
 * After the given time the attenuation is set to 0 again.
 * @param id: device id
 */
void
set_attenuation_new(int id, struct user_data *ud)
{
	check_att_limits(id, ud, SIMPLE);

	if (ud->us == 1)
		susleep(TIME_MICROS(ud->atime));
	else if(ud->ms == 1)
		susleep(TIME_MILLIS(ud->atime));
	else
		susleep(TIME_SECONDS(ud->atime));
}

/*
 * Sets attenuation to a level defined by user if
 * not above Max or below Min attenuation of the connected
 * attenuator. The Device will keep the attenuation for the
 * time given by the user or the standard sleeptime.
 * After the given time the attenuation is set to 0 again.
 * @param id: device id
 * @return: returns 0 on success
 */
int
set_attenuation(int id)
{
	if (ud.attenuation < fnLDA_GetMinAttenuation(id)) {
		printf("%.2f is below minimal attenuation of %.2f\n",
			(double)ud.attenuation / 4,
			(double)fnLDA_GetMinAttenuation(id) / 4);
		printf("attenuation has been set to %.2fdB\n",
			(double)fnLDA_GetMinAttenuation(id) / 4);
		fnLDA_SetAttenuation(id, fnLDA_GetMinAttenuation(id));
		log_attenuation( fnLDA_GetMinAttenuation(id) );
		if (ud.us == 1)
			susleep(TIME_MICROS(ud.atime));
		else if(ud.ms == 1)
			susleep(TIME_MILLIS(ud.atime));
		else
			susleep(TIME_SECONDS(ud.atime));
		return 1;
	}

	if (ud.attenuation > fnLDA_GetMaxAttenuation(id)) {
		printf("%.2f is above maximal attenuation of %.2f\n",
			(double)ud.attenuation / 4,
			(double)fnLDA_GetMaxAttenuation(id) / 4);
		printf("attenuation has been set to %.2f\n",
			(double)fnLDA_GetMaxAttenuation(id) / 4);
		fnLDA_SetAttenuation(id, fnLDA_GetMaxAttenuation(id));
		log_attenuation( fnLDA_GetMaxAttenuation(id) );

		if (ud.us == 1)
			susleep(TIME_MICROS(ud.atime));
		else if(ud.ms == 1)
			susleep(TIME_MILLIS(ud.atime));
		else
			susleep(TIME_SECONDS(ud.atime));
		return 1;
	}

	fnLDA_SetAttenuation(id, (ud.attenuation));
	log_attenuation( ud.attenuation );
	printf("set device to %.2fdB attenuation\n",
		(double)(fnLDA_GetAttenuation(id)) / 4);
	if (ud.us == 1)
		susleep(TIME_MICROS(ud.atime));
	else if(ud.ms == 1)
		susleep(TIME_MILLIS(ud.atime));
	else
		susleep(TIME_SECONDS(ud.atime));
	return 1;
}

/*
 * Set attenuation stepwise from start attenuation to end attenuation and
 * log it.
 * @param id: device id
 */
void
set_triangle_new(int id, struct user_data *ud)
{
	int i, cur_att;
	check_att_limits(id, ud, TRIANGLE);

	fnLDA_SetAttenuation(id, ud->start_att);
	log_attenuation( ud->start_att );
	if (ud->cont && (ud->start_att < ud->end_att)) {
		for(;;) {
			for (i = 1; i <= (ud->end_att - ud->start_att); i++) {
				if (ud->us == 1)
					susleep(TIME_MICROS(ud->atime));
				else if(ud->ms == 1)
					susleep(TIME_MILLIS(ud->atime));
				else
					susleep(TIME_SECONDS(ud->atime));
				cur_att = fnLDA_GetAttenuation(id);
				printf("attenuation set to %.2fdB\n",
					((double)cur_att) / 4);
				fnLDA_SetAttenuation(id,
					cur_att + ud->ramp_steps);
				log_attenuation( cur_att + ud->ramp_steps );
			}
			for (i = ud->end_att; i >
			     (ud->end_att - ud->start_att); i--) {
				if (ud->us == 1)
					susleep(TIME_MICROS(ud->atime));
				else if(ud->ms == 1)
					susleep(TIME_MILLIS(ud->atime));
				else
					susleep(TIME_SECONDS(ud->atime));
				cur_att = fnLDA_GetAttenuation(id);
				printf("attenuation set to %.2fdB\n",
					((double)cur_att) / 4);
				fnLDA_SetAttenuation(id,
					cur_att - ud->ramp_steps);
				log_attenuation( cur_att - ud->ramp_steps );
			}
			fnLDA_SetAttenuation(id, ud->start_att);
			log_attenuation( ud->start_att );
		}
	}
	if (ud->start_att < ud->end_att) {
		for (i = 1; i <= (ud->end_att - ud->start_att); i++) {
			if (ud->us == 1)
				susleep(TIME_MICROS(ud->atime));
			else if(ud->ms == 1)
				susleep(TIME_MILLIS(ud->atime));
			else
				susleep(TIME_SECONDS(ud->atime));
			cur_att = fnLDA_GetAttenuation(id);
			printf("attenuation set to %.2fdB\n",
				((double)cur_att) / 4);
			fnLDA_SetAttenuation(id, cur_att + ud->ramp_steps);
			log_attenuation( cur_att + ud->ramp_steps );
		}
		for (i = ud->end_att; i > (ud->end_att - ud->start_att); i--) {
			if (ud->us == 1)
				susleep(TIME_MICROS(ud->atime));
			else if(ud->ms == 1)
				susleep(TIME_MILLIS(ud->atime));
			else
				susleep(TIME_SECONDS(ud->atime));
			cur_att = fnLDA_GetAttenuation(id);
			printf("attenuation set to %.2fdB\n",
				((double)cur_att) / 4);
			fnLDA_SetAttenuation(id, cur_att - ud->ramp_steps);
			log_attenuation( cur_att - ud->ramp_steps );
		}
		fnLDA_SetAttenuation(id, ud->start_att);
		log_attenuation( ud->start_att );
	}
	if (ud->cont && (ud->start_att > ud->end_att)) {
		for(;;) {
			for (i = 0; i < (ud->start_att - ud->end_att); i++) {
				if (ud->us == 1)
					susleep(TIME_MICROS(ud->atime));
				else if(ud->ms == 1)
					susleep(TIME_MILLIS(ud->atime));
				else
					susleep(TIME_SECONDS(ud->atime));
				cur_att = fnLDA_GetAttenuation(id);
				printf("attenuation set to %.2fdB\n",
					((double)cur_att) / 4);
				fnLDA_SetAttenuation(id,
					cur_att - ud->ramp_steps);
				log_attenuation( cur_att - ud->ramp_steps );
			}
			for (i = 1; i <= (ud->start_att - ud->end_att); i++) {
				if (ud->us == 1)
					susleep(TIME_MICROS(ud->atime));
				else if(ud->ms == 1)
					susleep(TIME_MILLIS(ud->atime));
				else
					susleep(TIME_SECONDS(ud->atime));
				cur_att = fnLDA_GetAttenuation(id);
				printf("attenuation set to %.2fdB\n",
					((double)cur_att) / 4);
				fnLDA_SetAttenuation(id,
					cur_att + ud->ramp_steps);
				log_attenuation( cur_att + ud->ramp_steps );
			}
			fnLDA_SetAttenuation(id, ud->start_att);
			log_attenuation( ud->start_att );
		}
	}
	if (ud->start_att > ud->end_att) {
		for (i = 0; i < (ud->start_att - ud->end_att); i++) {
			if (ud->us == 1)
				susleep(TIME_MICROS(ud->atime));
			else if(ud->ms == 1)
				susleep(TIME_MILLIS(ud->atime));
			else
				susleep(TIME_SECONDS(ud->atime));
			cur_att = fnLDA_GetAttenuation(id);
			printf("attenuation set to %.2fdB\n",
				((double)cur_att) / 4);
			fnLDA_SetAttenuation(id, cur_att - ud->ramp_steps);
			log_attenuation( cur_att - ud->ramp_steps );
		}
		for (i = 1; i <= (ud->start_att - ud->end_att); i++) {
			if (ud->us == 1)
				susleep(TIME_MICROS(ud->atime));
			else if(ud->ms == 1)
				susleep(TIME_MILLIS(ud->atime));
			else
				susleep(TIME_SECONDS(ud->atime));
			cur_att = fnLDA_GetAttenuation(id);
			printf("attenuation set to %.2fdB\n",
				((double)cur_att) / 4);
			fnLDA_SetAttenuation(id, cur_att + ud->ramp_steps);
			log_attenuation( cur_att + ud->ramp_steps );
		}
		fnLDA_SetAttenuation(id, ud->start_att);
		log_attenuation( ud->start_att );
	}
}

/*
 * Set attenuation stepwise from start attenuation to end attenuation and
 * log it.
 * @param id: device id
 * @return: returns 0 on success
 */
int
set_triangle(int id)
{
	int i, cur_att;

	if (ud.start_att < fnLDA_GetMinAttenuation(id)) {
		printf("%.2f is below minimal attenuation of %.2fdB\n",
			((double)ud.start_att) / 4,
			((double)fnLDA_GetMinAttenuation(id)) / 4);
		printf("start attenuation has been set to %.2fdB\n",
			((double)fnLDA_GetMinAttenuation(id)) / 4);
		ud.start_att = fnLDA_GetMinAttenuation(id);
	}
	if (ud.start_att > fnLDA_GetMaxAttenuation(id)) {
		printf("%.2f is above maximal attenuation of %.2f\n",
			((double)ud.start_att) / 4,
			((double)fnLDA_GetMaxAttenuation(id)) / 4);
		printf("start attenuation has been set to %.2fdB\n",
			((double)fnLDA_GetMaxAttenuation(id)) / 4);
		ud.start_att = fnLDA_GetMaxAttenuation(id);
	}
	if (ud.end_att < fnLDA_GetMinAttenuation(id)) {
		printf("%.2f is below minimal attenuation of %.2f\n",
			((double)ud.end_att) / 4,
			((double)fnLDA_GetMinAttenuation(id)) / 4);
		printf("final attenuation has been set to %.2fdB\n",
			((double)fnLDA_GetMinAttenuation(id)) / 4);
		ud.end_att = fnLDA_GetMinAttenuation(id);
	}
	if (ud.end_att > fnLDA_GetMaxAttenuation(id)) {
		printf("%.2f is above maximal attenuation of %.2f\n",
			((double)ud.end_att / 4),
			((double)fnLDA_GetMaxAttenuation(id)) / 4);
		printf("final attenuation has been set to %.2f\n",
			(double) fnLDA_GetMaxAttenuation(id) / 4);
		ud.end_att = fnLDA_GetMaxAttenuation(id);
	}

	fnLDA_SetAttenuation(id, ud.start_att);
	log_attenuation( ud.start_att );
	if (ud.cont && (ud.start_att < ud.end_att)) {
		for(;;) {
			for (i = 1; i <= (ud.end_att - ud.start_att); i++) {
				if (ud.us == 1)
					susleep(TIME_MICROS(ud.atime));
				else if(ud.ms == 1)
					susleep(TIME_MILLIS(ud.atime));
				else
					susleep(TIME_SECONDS(ud.atime));
				cur_att = fnLDA_GetAttenuation(id);
				printf("attenuation set to %.2fdB\n",
					((double)cur_att) / 4);
				fnLDA_SetAttenuation(id,
					cur_att + ud.ramp_steps);
				log_attenuation( cur_att + ud.ramp_steps );
			}
			for (i = ud.end_att; i >
			     (ud.end_att - ud.start_att); i--) {
				if (ud.us == 1)
					susleep(TIME_MICROS(ud.atime));
				else if(ud.ms == 1)
					susleep(TIME_MILLIS(ud.atime));
				else
					susleep(TIME_SECONDS(ud.atime));
				cur_att = fnLDA_GetAttenuation(id);
				printf("attenuation set to %.2fdB\n",
					((double)cur_att) / 4);
				fnLDA_SetAttenuation(id,
					cur_att - ud.ramp_steps);
				log_attenuation( cur_att - ud.ramp_steps );
			}
			fnLDA_SetAttenuation(id, ud.start_att);
			log_attenuation( ud.start_att );
		}
	}
	if (ud.start_att < ud.end_att) {
		for (i = 1; i <= (ud.end_att - ud.start_att); i++) {
			if (ud.us == 1)
				susleep(TIME_MICROS(ud.atime));
			else if(ud.ms == 1)
				susleep(TIME_MILLIS(ud.atime));
			else
				susleep(TIME_SECONDS(ud.atime));
			cur_att = fnLDA_GetAttenuation(id);
			printf("attenuation set to %.2fdB\n",
				((double)cur_att) / 4);
			fnLDA_SetAttenuation(id, cur_att + ud.ramp_steps);
			log_attenuation( cur_att + ud.ramp_steps );
		}
		for (i = ud.end_att; i > (ud.end_att - ud.start_att); i--) {
			if (ud.us == 1)
				susleep(TIME_MICROS(ud.atime));
			else if(ud.ms == 1)
				susleep(TIME_MILLIS(ud.atime));
			else
				susleep(TIME_SECONDS(ud.atime));
			cur_att = fnLDA_GetAttenuation(id);
			printf("attenuation set to %.2fdB\n",
				((double)cur_att) / 4);
			fnLDA_SetAttenuation(id, cur_att - ud.ramp_steps);
			log_attenuation( cur_att - ud.ramp_steps );
		}
		fnLDA_SetAttenuation(id, ud.start_att);
		log_attenuation( ud.start_att );
	}
	if (ud.cont && (ud.start_att > ud.end_att)) {
		for(;;) {
			for (i = 0; i < (ud.start_att - ud.end_att); i++) {
				if (ud.us == 1)
					susleep(TIME_MICROS(ud.atime));
				else if(ud.ms == 1)
					susleep(TIME_MILLIS(ud.atime));
				else
					susleep(TIME_SECONDS(ud.atime));
				cur_att = fnLDA_GetAttenuation(id);
				printf("attenuation set to %.2fdB\n",
					((double)cur_att) / 4);
				fnLDA_SetAttenuation(id,
					cur_att - ud.ramp_steps);
				log_attenuation( cur_att - ud.ramp_steps );
			}
			for (i = 1; i <= (ud.start_att - ud.end_att); i++) {
				if (ud.us == 1)
					susleep(TIME_MICROS(ud.atime));
				else if(ud.ms == 1)
					susleep(TIME_MILLIS(ud.atime));
				else
					susleep(TIME_SECONDS(ud.atime));
				cur_att = fnLDA_GetAttenuation(id);
				printf("attenuation set to %.2fdB\n",
					((double)cur_att) / 4);
				fnLDA_SetAttenuation(id,
					cur_att + ud.ramp_steps);
				log_attenuation( cur_att + ud.ramp_steps );
			}
			fnLDA_SetAttenuation(id, ud.start_att);
			log_attenuation( ud.start_att );
		}
	}
	if (ud.start_att > ud.end_att) {
		for (i = 0; i < (ud.start_att - ud.end_att); i++) {
			if (ud.us == 1)
				susleep(TIME_MICROS(ud.atime));
			else if(ud.ms == 1)
				susleep(TIME_MILLIS(ud.atime));
			else
				susleep(TIME_SECONDS(ud.atime));
			cur_att = fnLDA_GetAttenuation(id);
			printf("attenuation set to %.2fdB\n",
				((double)cur_att) / 4);
			fnLDA_SetAttenuation(id, cur_att - ud.ramp_steps);
			log_attenuation( cur_att - ud.ramp_steps );
		}
		for (i = 1; i <= (ud.start_att - ud.end_att); i++) {
			if (ud.us == 1)
				susleep(TIME_MICROS(ud.atime));
			else if(ud.ms == 1)
				susleep(TIME_MILLIS(ud.atime));
			else
				susleep(TIME_SECONDS(ud.atime));
			cur_att = fnLDA_GetAttenuation(id);
			printf("attenuation set to %.2fdB\n",
				((double)cur_att) / 4);
			fnLDA_SetAttenuation(id, cur_att + ud.ramp_steps);
			log_attenuation( cur_att + ud.ramp_steps );
		}
		fnLDA_SetAttenuation(id, ud.start_att);
		log_attenuation( ud.start_att );
	}
}

//TODO: add function to show max/min att, stepsize and other device infos
int
main(int argc, char *argv[])
{
	int device_count = 0;
	int id, nr_active_devices, status, i;
	int parameter_status;
	DEVID working_devices[MAXDEVICES];
	char device_name[MAX_MODELNAME];
	char *tmp, *version;
	char message[64];
	int res;

	/* get the uid of caller */
	uid_t uid = geteuid();
	clear_userdata();
	if (uid != 0) {
		printf("This tool needs to be run as root to access USB ports\n");
		printf("Please run again as root\n");
		exit(1);
	}
	if (argc < 2) {
		printf("Usage: %s [options]\n", argv[0]);
		call_help();
		exit(1);
	}
	if ((strncmp(argv[1], "-h", strlen(argv[1]))) == 0) {
		call_help();
		exit(0);
	}
	if (!get_parameters(argc, argv)){
		printf("Usage: %s [options]\n", argv[0]);
		call_help();
		exit(1);
	}
	fnLDA_Init();
	version = fnLDA_LibVersion();
	fnLDA_SetTestMode(FALSE);

	//TODO: check in intervals if connected devices have been
	//exchanged or disconnected
	device_count = fnLDA_GetNumDevices();

	printf("you are using libversion %s\n", version);

	if (device_count == 0)
		printf("There is no attenuator connected\n");
	else if (device_count > 1)
		printf("There are %d attenuators connected\n", device_count);
	else
		printf("There is %d attenuator connected\n", device_count);

	get_serial_and_name(device_count, device_name);
	nr_active_devices = fnLDA_GetDevInfo(working_devices);
	printf("%d active devices found\n", nr_active_devices);


	for (id = 0; id < nr_active_devices; id++) {
		if ((strncmp(argv[1], "-i", strlen(argv[1]))) == 0)
			print_dev_info(id);
	}

	/*
	 * initiate devices
	 */
	for (id = 0; id < nr_active_devices; id++) {
		status = fnLDA_InitDevice(working_devices[id]);
		if (status != 0) {
			printf("initialising device %d failed\n",
				id + 1);
			continue;
		}
		printf("initialized device %d successfully\n", id + 1);
		if (ud.info != 1)
			printf("You can set attenuation steps in %.2fdB steps\n",
				(double)(fnLDA_GetDevResolution(id + 1)) / 4);
		else
			print_dev_info(id);
	}

	for(id = 0; id < nr_active_devices; id++) {
		strncpy(message, get_device_data(working_devices[id]),
				 strlen(message));
		if (strncmp(message,"Successfully checked device ",
			strlen(message)) == 0) {
			printf(message);
			printf("%d\n", id);
		} else {
			printf("check failed for device %d\n", id);
			printf("%s\n", message);
		}
	}
	print_userdata();

	/*
	 * Set device as specified by user
	 */
	for (id = 1; id <= nr_active_devices; id++) {
		/* TODO implement sine_function which will set ramp form
		  * in interval maybe with steps and set one step a
		  * second so it will be decided by step size and
		  * time how many curve intervals there will be */
		if (ud.simple == 1)
			set_attenuation(id);

		else if (ud.sine && ud.cont) {
			for(;;)
				printf("not implemented yet\n");
				//set_sine(id);
		}
		else if (ud.sine && (ud.runs >= 1))
			for(i = 0; i < ud.runs; i++)
				//set_sine(id);
				printf("not implemented yet\n");

		else if (ud.triangle && ud.cont) {
			for(;;)
				set_triangle(id);
		}
		else if (ud.triangle && ud.runs >= 1)
			for(i = 0; i < ud.runs; i++)
				set_triangle(id);

		else if (ud.ramp && ud.cont) {
			for(;;)
				set_ramp(id);
		}
		else if (ud.ramp && ud.runs >= 1)
			for(i = 0; i < ud.runs; i++)
				set_ramp(id);

		else if (ud.file && ud.cont) {
			res = 0;
			while (res == 0)
				res = read_file(ud.path, id);
		}
		else if (ud.file && ud.runs >= 1)
			res = 0;
			while (res == 0)
				res = read_file(ud.path, id);

		if (ud.atime != 0) {
			fnLDA_SetAttenuation(id, 0);
			log_attenuation( 0 );
		}
	}

	/*
	 * close any open device
	 */
	for (id = 0; id < nr_active_devices; id++) {
		status = fnLDA_CloseDevice(working_devices[id]);
		if (status != 0) {
			printf("shutting down device %d failed\n",
				id + 1);
			continue;
		}
		printf("shut down of device %d was successful\n", id + 1);
	}
	return 1;
}

