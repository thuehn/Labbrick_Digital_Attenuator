#ifndef _CONTROL_H_
#define _CONTROL_H_

#include <stdio.h>

char errmsg[64];

void get_serial_and_name(int device_count, char *device_name);
char * get_device_data(unsigned int current_devices);
int set_ramp(int id);
int set_ramp(int id, struct user_data *ud);
int set_attenuation(int id);
int set_attenuation(int id,struct user_data *ud);
int set_triangle(int id);
int set_triangle(int id, struct user_data *ud);
void print_dev_info(int id);

#endif
