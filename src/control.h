#ifndef _CONTROL_H_
#define _CONTROL_H_

#include <stdio.h>
#include "input.h"

#define ERR "\x1B[31m" "[ERROR]: " "\x1B[0m"
#define WARN "\x1B[33m" "[WARNING]: " "\x1B[0m"
#define INFO "\x1B[32m" "[INFO]: " "\x1B[0m"

char errmsg[64];

void get_serial_and_name(int device_count, char *device_name);
char * get_device_data(unsigned int current_devices);
int set_ramp(int id, struct user_data *ud);
void set_attenuation(int id,struct user_data *ud);
int set_triangle(int id, struct user_data *ud);
void print_dev_info(int id);
int check_multi_device(char *argv[]);
int check_quiet(int argc, char *argv[]);

#endif

