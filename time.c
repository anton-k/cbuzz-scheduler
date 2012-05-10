#include <stdio.h>
#include "time.h"

#include <assert.h>

/*------------------------------------------------*/
/* time functions */

t_time time_init(int seconds, int samples, int rate)
{
	t_time res = { seconds, samples, rate };
	return res;
}

void time_print(t_time t)
{
	printf("(time:\n\tseconds: %d\n\tsamples: %d\n\trate: %d)\n", 
		t.seconds, t.samples, t.rate);
}

t_time time_zero(int rate)
{
	return time_init(0, 0, rate);
}

int time_lt(t_time a, t_time b) 
{
	if (a.seconds < b.seconds)
		return 1;

	if ((a.seconds == b.seconds) && (a.samples < b.samples))
		return 1;

	return 0;
}

int time_lteq(t_time a, t_time b) 
{
	if (a.seconds < b.seconds)
		return 1;

	if ((a.seconds == b.seconds) && (a.samples <= b.samples))
		return 1;

	return 0;
}


void time_succ(t_time *t) 
{
	(t->samples)++;
	if (t->samples >= t->rate) {
		t->samples = 0;
		(t->seconds) ++;	
	}		
}

void time_pred(t_time *t) 
{
	(t->samples)--;
	if (t->samples < 0) {
		t->samples = t->rate - 1;
		(t->seconds) --;	

		if (t->seconds < 0) { *t = time_zero(t->rate); }
	}		
}

t_time time_add(t_time a, t_time b)
{
assert(a.rate == b.rate);
	int samples = a.samples + b.samples;
	int seconds = a.seconds + b.seconds;
	int rate    = a.rate;	

	return (
		(samples < rate) 
			? time_init(seconds, samples, rate)
			: time_init(seconds+1, samples-rate, rate)
	);
}

t_time time_sub(t_time a, t_time b) 
{
assert(a.rate == b.rate);
	int rate = a.rate;

	if (a.seconds < b.seconds)
		return (time_zero(rate));	 

	if (a.seconds == b.seconds && a.samples < b.samples)	
		return (time_zero(rate));

	int seconds = a.seconds - b.seconds;

	if (a.samples < b.samples) {
		int samples = rate - (b.samples - a.samples);
		return (time_init(seconds-1, samples, rate));
	}

	int samples = a.samples - b.samples;
	
	return (time_init(seconds, samples, rate));
}


double time_to_double(t_time t)
{
	return ((double)t.seconds + (double)t.samples / (double)(t.rate) );
}

t_time time_from_double(int rate, double d)
{
	if (d < 0)	return time_zero(rate);

	int seconds = (int)d;
	int samples = (int)(0.5 + (double)rate * (d - (double)seconds) );

	if (samples == rate) {
		samples = 0;	
		seconds++;
	}		

	t_time res = { seconds, samples, rate };
	return res;	
}

int time_to_int(t_time t) 
{
	return (t.seconds * t.rate + t.samples);
}


