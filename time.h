#ifndef _TIME_h
#define _TIME_h

typedef struct {
	int seconds;
	int samples;
	int rate;
} t_time;

t_time time_init(int seconds, int samples, int rate);
t_time time_zero(int rate);
void time_succ(t_time *t);
t_time time_add(t_time a, t_time b);
t_time time_sub(t_time a, t_time b);
double time_to_double(t_time a);
int time_to_int(t_time a);
t_time time_from_double(int rate, double d);
void time_print(t_time t);

int time_lt(t_time a, t_time b);
int time_lteq(t_time a, t_time b);
#endif
