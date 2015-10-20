#ifndef _CONTROL_H_
#define _CONTROL_H_

#include <stdio.h>

char errmsg[64];

void get_serial_and_name(int device_count, char *device_name);
char * get_device_data(unsigned int current_devices);
int set_ramp(int id);
int set_attenuation(unsigned int id);
int set_triangle(unsigned int id);
void print_dev_info(int id);

#endif
