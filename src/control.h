#ifndef _CONTROL_H_
#define _CONTROL_H_

#include <stdio.h>

char errmsg[64];



int get_serial_and_name(int device_count, unsigned int serial,
	char *device_name);
char * get_device_data(unsigned int *working_devices, int nr_active_devices);
int set_ramp(int id);
int set_attenuation(unsigned int id);
int set_triangle(unsigned int id);
int print_dev_info(int id);

#endif
