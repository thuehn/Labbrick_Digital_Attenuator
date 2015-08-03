#ifndef _INPUT_H_
#define _INPUT_H_

#define TIME_MICROS(step_time) (step_time)
#define TIME_MILLIS(step_time) (step_time * 1000)
#define TIME_SECONDS(step_time) (step_time * 1000000)

struct user_data
{
	unsigned long atime;
	unsigned int attenuation;
	unsigned int start_att;
	unsigned int end_att;
	unsigned int ramp;
	unsigned int sine;
	unsigned int triangle;
	unsigned int ramp_steps;
	unsigned int cont;
	unsigned int step_time;
	unsigned int simple;
	unsigned int file;
	unsigned int info;
	unsigned int runs;
	unsigned int ms;
	unsigned int us;
	unsigned int log;
	char *path;
	char *logfile;
};

int read_file(char *path, int id);
char * get_entry(char* line, int entry);
int get_parameters(int argc, char *argv[]);
void print_userdata(void);
void clear_userdata(void);

#endif
