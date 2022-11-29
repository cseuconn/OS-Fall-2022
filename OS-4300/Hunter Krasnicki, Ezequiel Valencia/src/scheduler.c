/*
 * Scheduler.
 *
 * The default scheduler is very simple, just a round-robin run queue.
 * You'll want to improve it.
 */

#include <types.h>
#include <lib.h>
#include <scheduler.h>
#include <thread.h>
#include <machine/spl.h>
#include <queue.h>

/*
 *  Scheduler data
 */

// Queues of runnable threads by priority
static struct queue *lowqueue, *medqueue, *highqueue;

/*
 * Setup function
 */
void
scheduler_bootstrap(void)
{
  //Init all three queues
	lowqueue = q_create(32);
  medqueue = q_create(32);
  highqueue = q_create(32);
  //Panic if any queue cannot be created
	if (lowqueue == NULL || medqueue == NULL || highqueue == NULL) {
		panic("scheduler: Could not create a priority run queue\n");
	}
}

/*
 * Ensure space for handling at least NTHREADS threads.
 * This is done only to ensure that make_runnable() does not fail -
 * if you change the scheduler to not require space outside the 
 * thread structure, for instance, this function can reasonably
 * do nothing.
 */
int
scheduler_preallocate(int nthreads)
{
	assert(curspl>0);
  
  //Attempt to preallocate each queue, error if unable to
  if(q_preallocate(lowqueue, nthreads) || q_preallocate(medqueue, nthreads) || q_preallocate(highqueue, nthreads)){
    return 1;
  }
	return 0;

 /* {
	assert(curspl>0);
	return q_preallocate(runqueue, nthreads);
}
*/
}

/*
 * This is called during panic shutdown to dispose of threads other
 * than the one invoking panic. We drop them on the floor instead of
 * cleaning them up properly; since we're about to go down it doesn't
 * really matter, and freeing everything might cause further panics.
 */
void
scheduler_killall(void)
{
	assert(curspl>0);

  //Drop threads from each queue
  while(!q_empty(lowqueue)) {
    struct thread *t = q_remhead(lowqueue);
    kprintf("scheduler: Dropping thread %s.\n", t->t_name);
  }
  while(!q_empty(medqueue)) {
    struct thread *t = q_remhead(medqueue);
    kprintf("scheduler: Dropping thread %s.\n", t->t_name);
  }
  while(!q_empty(highqueue)) {
    struct thread *t = q_remhead(highqueue);
    kprintf("scheduler: Dropping thread %s.\n", t->t_name);
  }
  /*
  //Old RR implementation
	while (!q_empty(runqueue)) {
		struct thread *t = q_remhead(runqueue);
		kprintf("scheduler: Dropping thread %s.\n", t->t_name);
	}
*/
}

/*
 * Cleanup function.
 *
 * The queue objects to being destroyed if it's got stuff in it.
 * Use scheduler_killall to make sure this is the case. During
 * ordinary shutdown, normally it should be.
 */
void
scheduler_shutdown(void)
{
	scheduler_killall();

	assert(curspl>0);
  //Destroy each queue
  q_destroy(lowqueue);
	q_destroy(medqueue);
  q_destroy(highqueue);

  //Set each queue pointer to NULL
  lowqueue = NULL;
  medqueue = NULL;
  highqueue = NULL;
}

/*
 * Actual scheduler. Returns the next thread to run.  Calls cpu_idle()
 * if there's nothing ready. (Note: cpu_idle must be called in a loop
 * until something's ready - it doesn't know whether the things that
 * wake it up are going to make a thread runnable or not.) 
 */
struct thread *
scheduler(void)
{
    // meant to be called with interrupts off
    assert(curspl>0);
    
    while (q_empty(lowqueue) && q_empty(medqueue) && q_empty(highqueue)) {
        cpu_idle();
    }

  //Return runnable threads in order of high->med->low priority
  if(!q_empty(highqueue)){
      //kprintf("High Queue");
      //print_run_queue(highqueue);
    return q_remhead(highqueue);
  }
  else if (!q_empty(medqueue)){
     //kprintf("Middle Queue");
     //print_run_queue(medqueue);
    return q_remhead(medqueue);
  }
  else{
    //kprintf("Low Queue");
    //print_run_queue(lowqueue);
    return q_remhead(lowqueue);
  }
    // You can actually uncomment this to see what the scheduler's
    // doing - even this deep inside thread code, the console
    // still works. However, the amount of text printed is
    // prohibitive.
    // 
    //print_run_queue();
    
    //return q_remhead(runqueue);
}

/* 
 * Make a thread runnable.
 * With the base scheduler, just add it to the end of the run queue.
 */
int
make_runnable(struct thread *t)
{
	// meant to be called with interrupts off
	assert(curspl>0);
  
  //Check thread priority and assign thread to proper queue
  switch(t->priority){
    case 0:
      return q_addtail(lowqueue, t);

    case 1:
      return q_addtail(medqueue, t);

    case 2:
      return q_addtail(highqueue, t);
  }
	//return q_addtail(runqueue, t);
}

/*
 * Debugging function to dump the run queue.
 */



void
print_run_queue(struct queue * q)
{
	// Turn interrupts off so the whole list prints atomically.
	int spl = splhigh();

	int i,k=0;
	i = q_getstart(q);
	
	while (i!=q_getend(q)) {
		struct thread *t = q_getguy(q, i);
		kprintf("  %2d: %s %p\n", k, t->t_name, t->t_sleepaddr);
		i=(i+1)%q_getsize(q);
		k++;
	}
	
	splx(spl);
	return;
}
