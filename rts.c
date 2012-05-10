#include "sglib.h"

#include "pqueue.h"
#include <assert.h>
#include "time.h"

#include <stdio.h>

typedef void *t_pid;

typedef struct _event_list {
	t_time t;
	t_pid val;
	struct _event_list *next;
} t_event_list;

t_event_list *event_list_init();
void event_list_free(t_event_list **);

typedef struct _pid_list {
	t_pid val;
	struct _pid_list *next;
} t_pid_list;

t_pid_list *pid_list_init();
void pid_list_free(t_pid_list **);






typedef struct {
	int pos;
	t_pid_list **val; /* array of pid lists */
	int length;
	t_time dur;
} t_buffer;

t_buffer *buffer_init(t_time t);
void buffer_free(t_buffer **);

t_pid_list *get_buffer_value(t_buffer *a);
void set_buffer_value(t_buffer *a, t_pid_list *v);
void buffer_tick(t_buffer *);
int is_present_event(t_buffer *buf, t_time t);
void buffer_insert(t_buffer *, t_time t, t_pid pid);

void buffer_clear_current(t_buffer *);

typedef void (*t_callback)(t_pid_list *, void *user_data);

typedef struct {
	const t_time *current_time;
		
	t_event_list *pending;
	pqueue_t *storage;
} t_future;


t_future *future_init(size_t queue_size, 
	t_time *current_time, t_event_list *pending);

void future_free(t_future **);

void future_insert(t_event_list **q, t_time t, t_pid pid);

typedef struct {
	double pri; /* special for pqueue lib */
	t_time t;
	t_pid val;
	size_t pos;
} t_pri_event;

t_pri_event *pri_event_init(t_time t, t_pid val);
void *pri_event_free(t_pri_event**);

typedef struct {	
	t_time t;

	t_buffer *buffer;
	t_event_list *pending;

	t_callback callback;
	void *user_data;

	t_future *future;	
}  t_sched;


/*
typedef struct {
	int rate;
	int current;
	t_pid  val;	
} t_repeat;

typedef struct {
	t_pid_list exec;
	t_repeat_list *units;
} t_timer;
*/

/* ----------------------------- */
/* main API */

t_sched *sched_init(
	t_time present_span, int future_span,
	t_callback f, void *user_data);

void sched_free(t_sched **);

void sched_tick(t_sched *);
t_time sched_time(t_sched *);

void sched_event(t_sched *, t_time, t_pid);
void sched_event_at(t_sched *, double, t_pid);

/* --------------------------  */

typedef struct {
	void (*fn)(void *, void *);
	void *input;
} t_callback_event;


t_callback_event *callback(
	void (*fn)(void *, void *), void *input)
{
	t_callback_event *res = 
		(t_callback_event *) malloc(sizeof(t_callback_event));

	res->fn = fn;
	res->input = input;

	return res;
}

void eval_pid_list(t_pid_list *ps, void *global_user_data)
{	
	SGLIB_LIST_MAP_ON_ELEMENTS(t_pid_list, ps, ll, next, {
		t_callback_event *q = (t_callback_event *) ll->val;
		(q->fn)(q->input, global_user_data);
		free(q);
	})
}


/* sched tick */

t_sched *sched_init(
	t_time present_span, int future_span,
	t_callback callback, void *user_data)
{
	t_sched *res = (t_sched *) malloc(sizeof(t_sched));
	
	res->t = time_zero(present_span.rate);
	res->buffer = buffer_init(present_span);
	res->pending = event_list_init();
	
	res->future = future_init(future_span, &(res->t), res->pending);
	res->callback = callback;	
	res->user_data = user_data;

	return res;	
}


void sched_free(t_sched **in)
{
	buffer_free(&((*in)->buffer));
	event_list_free(&((*in)->pending));
	future_free(&((*in)->future));

	free(*in);
	*in = NULL;
}

t_time sched_time(t_sched *e) 
{
	return (e->t);
}

void schedule_pending_events(t_event_list **, t_future *);
void fetch_future_events(t_sched *);

void sched_tick(t_sched *q) 
{
	/* process current events */
	t_pid_list *es = get_buffer_value(q->buffer);
	if (es) {
		q->callback(es, q->user_data);
		buffer_clear_current(q->buffer);		
	}

	buffer_tick(q->buffer);

	/* process future events */	
	schedule_pending_events(&(q->pending), q->future);
	fetch_future_events(q);

	/* update time */	
	time_succ(&(q->t));
}


/* here time t is relative 
	it starts from current time (q->t)

	when we are looking for a place in present
	we are using relative time, but when
	we are planning for the future we are
	using absolute time (time_add(q->t, t))	
*/
void sched_event(t_sched *q, t_time t, t_pid pid)
{
	if (is_present_event(q->buffer, t)) 
		buffer_insert(q->buffer, t, pid);	
	else
		future_insert(&(q->pending), time_add(q->t, t), pid);	
}

void sched_event_at(t_sched *q, double t, t_pid pid)
{
	sched_event(q, time_from_double((q->t).rate, t), pid);
}

/*----------------------------------------------------*/
/* pri_event */
/* priority functions */

static int pri_event_cmp(double a, double b)
{
	return (a > b);
}


static double
get_pri(void *a)
{
	return (double) ((t_pri_event *) a)->pri;
}


static void
set_pri(void *a, double pri)
{
	((t_pri_event *) a)->pri = pri;
}


static size_t
get_pos(void *a)
{
	return ((t_pri_event *) a)->pos;
}


static void
set_pos(void *a, size_t pos)
{
	((t_pri_event *) a)->pos = pos;
}
	
t_pri_event *pri_event_init(t_time t, t_pid p)
{
	t_pri_event* res = (t_pri_event *) malloc(sizeof(t_pri_event));
	
	res->pri = time_to_double(t);
	res->t = t;
	res->val = p;

	return res;
}


/*------------------------------------------------*/
/* buffer functions */

t_buffer *buffer_init(t_time dur)
{
	t_buffer *res = (t_buffer *) malloc(sizeof(t_buffer));
	
	int n = time_to_int(dur);
	res->val = (t_pid_list **) malloc(n * sizeof(t_pid_list*));

	int i;
	for (i=0; i<n; i++)	
		res->val[i] = NULL;

	res->length = n;
	res->pos = 0;
	res->dur = dur;

	return res;
}

void buffer_free(t_buffer **pbuf)
{
	t_buffer *buf = *pbuf;

	int i;
	for (i=0; i < buf->length; i++)
		pid_list_free(&(buf->val[i]));

	free(buf->val);
	free(buf);
	buf = NULL;
}


int is_present_event(t_buffer *buf, t_time t)
{
	return time_lt(t, buf->dur);
}

t_pid_list *get_buffer_value(t_buffer *a)
{
	return (a->val[a->pos]);
}

void set_buffer_value(t_buffer *a, t_pid_list *v)
{
	a->val[a->pos] = v;
}

void buffer_tick(t_buffer *a)
{
	(a->pos)++;
	if (a->pos == a->length)
		a->pos = 0;
}

void buffer_insert(t_buffer *buf, t_time t, t_pid pid)
{
	int id = (buf->pos + t.seconds * t.rate + t.samples) % buf->length;
assert(id < buf->length);

	
	t_pid_list *elem = (t_pid_list *) malloc(sizeof(t_pid_list));
	elem->val = pid;

	SGLIB_LIST_ADD(t_pid_list, buf->val[id], elem, next);
}

void buffer_clear_current(t_buffer *buf)
{
	t_pid_list *as = buf->val[buf->pos];
	SGLIB_LIST_MAP_ON_ELEMENTS(t_pid_list, as, ll, next, {
		free(ll);
	});
	buf->val[buf->pos] = NULL;
}


/*------------------------------------------------*/
/* future functions */

t_future *future_init(size_t queue_size, 
	t_time *current_time, t_event_list *pending)
{
	t_future *res = (t_future *) malloc(sizeof(t_future));
	
	res->current_time  = current_time;
	res->pending = pending;

	res->storage = pqueue_init(queue_size, 
		pri_event_cmp, get_pri, set_pri, get_pos, set_pos);

	return res;
}

void future_free(t_future **pin)
{
	t_future *in = *pin;

	pqueue_free(in->storage);
	free(in);
	in = NULL;
}

void schedule_pending_events(t_event_list **ptr_es, t_future *q)
{
	t_event_list *es = *ptr_es;
	t_event_list *p;	
	while (es) {
		pqueue_insert(q->storage, pri_event_init(es->t, es->val));
		p = es;
		es = es->next;
		free(p);
	}

	*ptr_es = NULL;
}

int next_event(t_future *fut, t_time *t, t_pid *pid) 
{
	t_pri_event *e = (t_pri_event *) pqueue_peek(fut->storage);

	if (e == NULL) { return 0; }	
	
	/* substract absolute event time from current time
		(all events are sent with relative time)
	 */
	*t = time_sub(e->t, *(fut->current_time));
	*pid = e->val;

	return 1;
}

void remove_future_event(t_future *fut)
{
	pqueue_pop(fut->storage);
}


void fetch_future_events(t_sched *q)
{
	t_future *fut = q->future;

	t_time t;
	t_pid pid;

	while (1) {	
		if (!next_event(fut, &t, &pid)) 		break;		
		if (!is_present_event(q->buffer, t)) 	break;
		
		time_pred(&t); 
			/* event is scheduled at the end of the tick
			so we substract one tick as if it was scheduled
			before the tick */ 

		sched_event(q, t, pid);	
		remove_future_event(fut);
	}
}


void future_insert(t_event_list **q, t_time t, t_pid pid)
{
	t_event_list *elem = (t_event_list *) malloc(sizeof(t_event_list));
	elem->t = t;
	elem->val = pid;

	SGLIB_LIST_ADD(t_event_list, *q, elem, next);
}

/* ---------------------------------------------------- */
/* special lists */

/* t_pid_list */

t_pid_list *pid_list_init()
{	
	return NULL;	
}

void pid_list_free(t_pid_list **pin)
{
	t_pid_list *in = *pin;

	if (in==NULL) { return; }
	
	SGLIB_LIST_MAP_ON_ELEMENTS(t_pid_list, in, ll, next, {
		free(ll); });
	free(in);	
	in = NULL;
}

/* t_event_list */


t_event_list *event_list_init()
{
	return NULL;
}

void event_list_free(t_event_list **pin)
{
	t_event_list *in = *pin;

	if (in==NULL) { return; }
	SGLIB_LIST_MAP_ON_ELEMENTS(t_event_list, in, ll, next, {
		free(ll); });
	free(in);	
	in = NULL;
}



