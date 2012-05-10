
#include "time.h"


typedef void t_sched;
typedef void *t_pid;

typedef struct _pid_list {
	t_pid val;
	struct _pid_list *next;
} t_pid_list;

t_pid_list *pid_list_init();
void pid_list_free(t_pid_list **);

typedef void (*t_callback)(t_pid_list *, void *user_data);

t_sched *sched_init(
	t_time present_span, int future_span,
	t_callback f, void *user_data);

void sched_free(t_sched **);

void sched_tick(t_sched *);
t_time sched_time(t_sched *);

void sched_event(t_sched *, t_time, t_pid);
void sched_event_at(t_sched *, double, t_pid);


/* -------------------------- */

typedef struct {
	void (*fn)(void *, void *);
	void *input;
} t_callback_event;

t_callback_event *callback(
	void (*fn)(void *, void *), void *user_data);


void eval_pid_list(t_pid_list *ps, void *user_data);

