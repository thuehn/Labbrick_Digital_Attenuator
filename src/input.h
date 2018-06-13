#ifndef _INPUT_H_
#define _INPUT_H_

#define TIME_MICROS(step_time) (step_time)
#define TIME_MILLIS(step_time) (step_time * 1000)
#define TIME_SECONDS(step_time) (step_time * 1000000)
#define MAX_LENGTH 128

/* 
 * Variable attenuation steps according to Vaunix LDA Linux SDK
 * Attenuation setting in 0.25dB, so 4 steps = 1 dB
 * Attenuation setting in 0.05dB, so 20 steps = 1 dB
 * */
#define MULTIPLIER_STEP 20

struct user_data
{
	unsigned long atime;
	int attenuation;
	int start_att;
	int end_att;
	unsigned int ramp;
	unsigned int triangle;
	int ramp_steps;
	unsigned int cont;
	unsigned int simple;
	unsigned int file;
	unsigned int info;
	unsigned int runs;
	unsigned int ms;
	unsigned int us;
	unsigned int log;
	unsigned int quiet;
	unsigned int serial_number;
	char path[128];
	char logfile[128];
};

int read_file(char *patch, int id, struct user_data *ud);
char * get_entry(char* line, int entry);
int get_parameters(int argc, char *argv[], struct user_data *ud);
void print_userdata(struct user_data *ud);
void clear_userdata(struct user_data *ud);
int log_attenuation(unsigned int att, struct user_data *ud);

#endif

