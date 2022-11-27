#include <types.h>
#include <lib.h>
#include <machine/spl.h>
#include <thread.h>
#include <clock.h>

//Including curthread to allow for priority resets
//#include <curthread.h>
/* 
 * The address of lbolt has thread_wakeup called on it once a second.
 */
int lbolt;

static int lbolt_counter;

//Create a reset timer to keep track of when we should reset thread priority
//static int reset_timer;

/*
 * This is called HZ times a second by the timer device setup.
 */

void
hardclock(void)
{
	/*
	 * Collect statistics here as desired.
	 */

	lbolt_counter++;
	if (lbolt_counter >= HZ) {
		lbolt_counter = 0;
		thread_wakeup(&lbolt);
	}

	thread_yield();
  
  /*
  if (reset_timer >= (HZ * 1000)){
    //Reset priority to 1 after some time has elapsed
    curthread->priority = 1;
  }
*/
}

/*
 * Suspend execution for n seconds.
 */
void
clocksleep(int num_secs)
{
	int s;

	s = splhigh();
	while (num_secs > 0) {
		thread_sleep(&lbolt);
		num_secs--;
	}
	splx(s);
}
