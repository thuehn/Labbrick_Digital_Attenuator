#include <stdio.h>
#include <unistd.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
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
#define SINGLE_DEV 0
#define SINGLE_DEV_ID 1

/* pthread struct */
struct thread_arguments {
	char *path;
	int id;
};

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

/*
 * print out device information of attenuator
 * @param id: attenuator id
 */
void
print_dev_info(int id)
{
	if ((fnLDA_GetAttenuation(id) / 4) < 0)
		printf("Attenuation is set to 0dB\n");
	else
		printf("Attenuation is set to: %.2fdB\n",
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
	char *success = "Successfully checked device\n";
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

	printf("\tyou can use s, ms and us to set time units\n");
	printf("\t\ts -> seconds\n");
	printf("\t\tms -> milliseconds\n");
	printf("\t\tus -> microseconds\n");
	printf("\r\n");

	printf("-set attenuation form with\n");
	printf("\t-p <-ramp|-triangle>\n");
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

	printf("-repeat form, or file input until canceled by user\n");
	printf("\t-r\n");
	printf("\r\n");

	printf("-repeat form, or file input for several times\n");
	printf("\t-rr <#runs>\n");
	printf("\r\n");

	printf("-to use more than one connected attenuator use\n");
	printf("\t-md <config_file1> <config_file2> ...\n");
	printf("\r\n");
	printf("\t every connected and detected attenuator will be used\n");
	printf("\t if there are more config files than attenuators detected\n");
	printf("\t the remaining files will be discarded\n");
	printf("\r\n");

	return;
}

/*
 * check if attenuation is above, or below device limits
 * @param id: device id
 * @param ud: user data struct to check attenuation from
 * @param check: 0 if only check for atttenuation boundaries
 * 		 1 if to check for start and end attenuation as well
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
			log_attenuation(fnLDA_GetMinAttenuation(id), ud);
		} else if (ud->attenuation > fnLDA_GetMaxAttenuation(id)) {
			printf("%.2f is above maximal attenuation of %.2f\n",
				(double)ud->attenuation / 4,
				(double)fnLDA_GetMaxAttenuation(id) / 4);
			printf("attenuation has been set to %.2f\n",
				(double)fnLDA_GetMaxAttenuation(id) / 4);
			fnLDA_SetAttenuation(id, fnLDA_GetMaxAttenuation(id));
			log_attenuation(fnLDA_GetMaxAttenuation(id), ud);
		} else {
			fnLDA_SetAttenuation(id, (ud->attenuation));
			log_attenuation(ud->attenuation, ud);
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
 * @param ud: user data struct
 */
void
set_ramp(int id, struct user_data *ud)
{
	int i, cur_att;

	check_att_limits(id, ud, RAMP);

	if (ud->cont && (ud->start_att < ud->end_att)) {
		for(;;) {
			fnLDA_SetAttenuation(id, ud->start_att);
			log_attenuation(ud->start_att, ud);
			for(i = 0; i <= ((ud->end_att - ud->start_att) /
					 ud->ramp_steps); i++) {
				if (ud->us == 1)
					susleep(TIME_MICROS(ud->atime));
				else if(ud->ms == 1)
					susleep(TIME_MILLIS(ud->atime));
				else
					susleep(TIME_SECONDS(ud->atime));
				cur_att = fnLDA_GetAttenuation(id);
				fnLDA_SetAttenuation(id,
					cur_att + ud->ramp_steps);
				log_attenuation(cur_att + ud->ramp_steps, ud);
			}
		}
	}
	else if (ud->cont && (ud->start_att > ud->end_att)) {
		for(;;) {
			fnLDA_SetAttenuation(id, ud->start_att);
			log_attenuation(ud->start_att, ud);
			for(i = 0; i <= ((ud->start_att - ud->end_att) /
					 ud->ramp_steps); i++) {
				if (ud->us == 1)
					susleep(TIME_MICROS(ud->atime));
				else if(ud->ms == 1)
					susleep(TIME_MILLIS(ud->atime));
				else
					susleep(TIME_SECONDS(ud->atime));
				cur_att = fnLDA_GetAttenuation(id);
				fnLDA_SetAttenuation(id,
					cur_att - ud->ramp_steps);
				log_attenuation(cur_att - ud->ramp_steps, ud);
			}
		}
	}
	else if (ud->start_att < ud->end_att) {
		fnLDA_SetAttenuation(id, ud->start_att);
		log_attenuation(ud->start_att, ud);
		for(i = 0; i <= ((ud->end_att - ud->start_att) /
				 ud->ramp_steps); i++) {
			if (ud->us == 1)
				susleep(TIME_MICROS(ud->atime));
			else if(ud->ms == 1)
				susleep(TIME_MILLIS(ud->atime));
			else
				susleep(TIME_SECONDS(ud->atime));
			cur_att = fnLDA_GetAttenuation(id);
			fnLDA_SetAttenuation(id,
				cur_att + ud->ramp_steps);
			log_attenuation(cur_att + ud->ramp_steps, ud);
		}
	}
	else if (ud->start_att > ud->end_att) {
		fnLDA_SetAttenuation(id, ud->start_att);
		log_attenuation(ud->start_att, ud);
		for(i = 0; i <= ((ud->start_att - ud->end_att) /
				 ud->ramp_steps); i++) {
			if (ud->us == 1)
				susleep(TIME_MICROS(ud->atime));
			else if(ud->ms == 1)
				susleep(TIME_MILLIS(ud->atime));
			else
				susleep(TIME_SECONDS(ud->atime));
			cur_att = fnLDA_GetAttenuation(id);
			fnLDA_SetAttenuation(id,
				cur_att - ud->ramp_steps);
			log_attenuation(cur_att - ud->ramp_steps, ud);
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
 * @param ud: user data struct
 */
void
set_attenuation(int id, struct user_data *ud)
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
 * Set attenuation stepwise from start attenuation to end attenuation and
 * log it.
 * @param id: device id
 * @param ud: user data struct
 */
void
set_triangle(int id, struct user_data *ud)
{
	int i, cur_att, nr_steps;
	check_att_limits(id, ud, TRIANGLE);

	if (ud->start_att < ud->end_att) {
		nr_steps = (ud->end_att - ud->start_att) / ud->ramp_steps;
	} else if (ud->start_att > ud->end_att) {
		nr_steps= (ud->start_att - ud->end_att) / ud->ramp_steps;
	} else {
		printf("start attenuation equals end attenuation");
		return;
	}

	fnLDA_SetAttenuation(id, ud->start_att);
	log_attenuation(ud->start_att, ud);
	if (ud->cont && (ud->start_att < ud->end_att)) {
		for(;;) {
			for (i = 1; i <= nr_steps; i++) {
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
				log_attenuation(cur_att + ud->ramp_steps, ud);
			}
			for (i = (ud->end_att / ud->ramp_steps); i > nr_steps; i--) {
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
				log_attenuation(cur_att - ud->ramp_steps, ud);
			}
			fnLDA_SetAttenuation(id, ud->start_att);
			log_attenuation(ud->start_att, ud);
		}
	}
	if (ud->start_att < ud->end_att) {
		for (i = 1; i <= nr_steps; i++) {
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
			log_attenuation(cur_att + ud->ramp_steps, ud);
		}
		for (i = (ud->end_att / ud->ramp_steps); i >= nr_steps; i--) {
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
			log_attenuation(cur_att - ud->ramp_steps, ud);
		}
		fnLDA_SetAttenuation(id, ud->start_att);
		log_attenuation(ud->start_att, ud);
	}
	if (ud->cont && (ud->start_att > ud->end_att)) {
		for(;;) {
			for (i = 0; i < nr_steps; i++) {
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
				log_attenuation(cur_att - ud->ramp_steps, ud);
			}
			for (i = 1; i <= nr_steps; i++) {
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
				log_attenuation(cur_att + ud->ramp_steps, ud);
			}
			fnLDA_SetAttenuation(id, ud->start_att);
			log_attenuation(ud->start_att, ud);
		}
	}
	if (ud->start_att > ud->end_att) {
		for (i = 0; i < nr_steps; i++) {
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
			log_attenuation(cur_att - ud->ramp_steps, ud);
		}
		for (i = 1; i <= nr_steps; i++) {
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
			log_attenuation(cur_att + ud->ramp_steps, ud);
		}
		fnLDA_SetAttenuation(id, ud->start_att);
		log_attenuation(ud->start_att, ud);
	}
}

/*
 * allocate memory for user data struct
 * return: allocated user data struct address
 */
struct user_data *
allocate_user_data(void)
{
	struct user_data *ud = malloc(sizeof(struct user_data));

	if (ud == NULL) {
		printf("could not allocate memory for user data\n");
		exit(1);
	}
	return ud;
}

/*
 * Set device as specified by user
 * @param ud: user data struct
 */
void
set_data(struct user_data *ud)
{
	int i, res;

	if (ud->simple == 1) {
		set_attenuation(SINGLE_DEV_ID, ud);
	} else if (ud->triangle && ud->cont) {
		for(;;)
			set_triangle(SINGLE_DEV_ID, ud);
	} else if (ud->triangle && ud->runs >= 1) {
		for(i = 0; i < ud->runs; i++)
			set_triangle(SINGLE_DEV_ID, ud);
	} else if (ud->ramp && ud->cont) {
		for(;;)
			set_ramp(SINGLE_DEV_ID, ud);
	} else if (ud->ramp && ud->runs >= 1) {
		for(i = 0; i < ud->runs; i++)
			set_ramp(SINGLE_DEV_ID, ud);
	} else if (ud->file && ud->cont) {
		res = 0;
		while (res == 0)
			res = read_file(ud->path,SINGLE_DEV_ID, ud);
	} else if (ud->file && ud->runs > 1) {
		res = 0;
		while (res == 0)
			res = read_file(ud->path, SINGLE_DEV_ID, ud);
	} else if (ud->file && (ud->cont == 0)) {
		read_file(ud->path, SINGLE_DEV_ID, ud);
	}

	if (ud->atime != 0) {
		fnLDA_SetAttenuation(SINGLE_DEV_ID, 0);
		log_attenuation(0, ud);
	}
}

/*
 * check if the user wants to use multiple attenuators
 * @param argc: argument count
 * @param *argv: arguments passed to the program
 * return returns 1 on multiple devices, else 0
 */
int
check_multi_device(char *argv[])
{
	if (strncmp(argv[1], "-md", strlen(argv[1])) == 0)
		return 1;
	else
		return 0;
}

/*
 * get instructions for attenuator from file and start it
 * @param arguments: pthread argument struct
 */
void *
start_device(void *arguments)
{
	struct thread_arguments *args = arguments;
	struct user_data *ud = allocate_user_data();
	printf("size of user_data %ld\n",sizeof(struct user_data));
	clear_userdata(ud);
	read_file(args->path, args->id, ud);
	free(ud);
	pthread_exit((void *)args->id);
}

/*
 * close any open devices
 * @param nr_active_devices: number of active devices
 * @param working_devices: array of active devices
 */
void
close_device(int nr_active_devices, DEVID *working_devices)
{
	int id, status = 0;

	for (id = 1; id <= nr_active_devices; id++) {
		status = fnLDA_CloseDevice(working_devices[id - 1]);
		if (status != 0) {
			printf("shutting down device %d failed\n",
				id);
			printf("%d\n", nr_active_devices);
		}
		else
			printf("shut down of device %d was successful\n", id);
	}
}

/*
 * start thread for each active device
 * @param argc: argument count
 * @param argv: arguments given by the user
 */
void
handle_multi_dev(int argc, char *argv[])
{
	struct thread_arguments args;
	pthread_t threads[MAXDEVICES];
	int device_count = 0;
	int id, nr_active_devices, file_count, ret, state;
	DEVID working_devices[MAXDEVICES];
	char message[64];
	char device_name[MAX_MODELNAME];
	void *status;

	device_count = fnLDA_GetNumDevices();

	if (device_count == 0)
		printf("There is no attenuator connected\n");
	else if (device_count > 1)
		printf("There are %d attenuators connected\n", device_count);
	else
		printf("There is %d attenuator connected\n", device_count);

	get_serial_and_name(device_count, device_name);
	nr_active_devices = fnLDA_GetDevInfo(working_devices);
	printf("%d active devices found\n", nr_active_devices);

	for (id = 0; id <= nr_active_devices; id++) {
		if ((strncmp(argv[1], "-i", strlen(argv[1]))) == 0)
			print_dev_info(id);
	}

	/*
	 * initiate devices
	 */
	for (id = 0; id < nr_active_devices; id++) {
		state = fnLDA_InitDevice(working_devices[id]);
		if (state != 0) {
			printf("initialising device %d failed\n",
				id);
			continue;
		}
		printf("initialized device %d successfully\n", id);
	}

	/*
	 * check devices
	 */
	for(id = 0; id < nr_active_devices; id++) {
		strncpy(message, get_device_data(working_devices[id]),
			sizeof(message));
		if (!strncmp(message,"Successfully checked device\n",
		    strlen(message)) == 0) {
			printf("check failed for device %d\n", id);
			printf("%s\n", message);
		}
	}

	/* check number of available files */
	if ((argc - 2) < nr_active_devices)
		file_count = argc - 2;
	else
		file_count = nr_active_devices;

	for (id = 0; id < file_count; id++) {
		args.path = argv[id + 2];
		args.id = id + 1;

		ret = pthread_create(&threads[id], NULL, start_device,
		    (void *)&args);

		if (ret)
			printf("Failed to create thread! Error Code: %d\n",ret);
	}

	for (id = 0; id < file_count; id++) {
		ret = pthread_join(threads[id], &status);

		if (ret)
			printf("Failed to join thread! Error Code: %d\n", ret);
	}
	
	close_device(nr_active_devices, working_devices);
	return;
}

/*
 * handle single connected device
 * @param ud: user data struct
 * @param argc: argument count
 * @param argv: arguments given by the user
 * @param working_devices: array of active devices
 * return: 0 on success, 1 on error
 */
int
handle_single_dev(struct user_data *ud, int argc, char *argv[], DEVID *working_devices)
{
	int status;
	char message[64];

	clear_userdata(ud);

	if (!get_parameters(argc, argv, ud)){
		printf("Usage: %s [options]\n", argv[0]);
		call_help();
		exit(1);
	}

	print_dev_info(SINGLE_DEV);
	if (ud->info)
		print_userdata(ud);
	status = fnLDA_InitDevice(working_devices[SINGLE_DEV]);
	if (status != 0) {
		printf("initialising device 1 failed\n");
		return 0;
	}
	else
		printf("initialized device %d successfully\n", SINGLE_DEV_ID);
	if (ud->info)
		printf("You can set attenuation steps in %.2fdB steps\n",
			(double)(fnLDA_GetDevResolution(SINGLE_DEV_ID)) / 4);

	strncpy(message, get_device_data(working_devices[SINGLE_DEV]),
		sizeof(message));
	if (strncmp(message,"Successfully checked device\n",
	    strlen(message)) == 0) {
		printf("%s",message);
	} else {
		printf("check failed for the device\n");
		printf("%s\n", message);
		return 0;
	}

	set_data(ud);
	close_device(SINGLE_DEV_ID, working_devices);
	return 1;
}

/*
 * returns 0 on success, 1 on error
 */
int
main(int argc, char *argv[])
{
	int device_count = 0;
	int id, nr_active_devices;
	DEVID working_devices[MAXDEVICES];
	char device_name[MAX_MODELNAME];
	char *version;

	/* get the uid of caller */
	uid_t uid = geteuid();
	fnLDA_Init();
	version = fnLDA_LibVersion();
	printf("you are using libversion %s\n", version);

	fnLDA_SetTestMode(FALSE);

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
	if (check_multi_device(argv)){
		printf("multidevice support enabled\n");
		handle_multi_dev(argc, argv);
		exit(0);
	}

	struct user_data *ud = allocate_user_data();
	device_count = fnLDA_GetNumDevices();

	if (device_count == 0)
		printf("There is no attenuator connected\n");
	else if (device_count > 1)
		printf("There are %d attenuators connected\n", device_count);
	else
		printf("There is %d attenuator connected\n", device_count);

	get_serial_and_name(device_count, device_name);
	nr_active_devices = fnLDA_GetDevInfo(working_devices);
	printf("%d active devices found\n", nr_active_devices);

	if ((strncmp(argv[1], "-i", strlen(argv[1]))) == 0) {
		for (id = 0; id < nr_active_devices; id++)
			print_dev_info(id);
	}

	handle_single_dev(ud, argc, argv, working_devices);
	free(ud);
	return 0;
}

