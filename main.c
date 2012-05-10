#include <stdio.h>
#include "rts.h"

#include "time.h"

void fn(void *pmsg, void *pn) 
{
	char *msg  = (char *) pmsg;
//	t_sched *e = (int *) pn;

//	t_time t = get_time(e);

	printf("%s\n", msg);
}

void event(t_sched *e, double t, char *msg)
{
	sched_event_at(e, t, (void *) callback(fn, (void *) msg));		
}


int main() {
	t_time t0 = time_init(1, 0, 10);

	t_sched *q = sched_init(t0, 1000, eval_pid_list, NULL);

	event(q, 0.5, "bow");
	event(q, 1.1, "low");
	event(q, 1.3, "gow");
	event(q, 2.5, "hey-hey-hey");
	
	int n = 30;	
	while (n--) {
		t_time t = sched_time(q);
		printf("\n%d, %d : ", t.seconds, t.samples);

		if (n==25) {event(q, 1.5, "wow");}

		sched_tick(q);
	}
	printf("\n");


	sched_free(&q);

	printf("hello\n");
	return 0;
}
