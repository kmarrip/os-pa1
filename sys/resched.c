/* resched.c  -  resched */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <q.h>

#define AGESCHED 1 
#define LINUXSCHED 2

unsigned long currSP; /* REAL sp of current process */
extern int ctxsw(int, int, int, int);
/*-----------------------------------------------------------------------
 * resched  --  reschedule processor to highest priority ready process
 *
 * Notes:	Upon entry, currpid gives current process id.
 *		Proctab[currpid].pstate gives correct NEXT state for
 *			current process if other than PRREADY.
 *------------------------------------------------------------------------
 */
int resched() {
  register struct pentry * optr; /* pointer to old process entry */
  register struct pentry * nptr; /* pointer to new process entry */

  /* no switch needed if current process priority higher than next*/
  if (getschedclass() == AGESCHED) {

	// this is where the default scheduler will work from
    if (((optr = & proctab[currpid]) -> pstate == PRCURR) &&
      (lastkey(rdytail) < optr -> pprio)) {
	      // only here we increase the priorities of all the other processes
        // This only happens when the proces being taken out has lower priority than the process currently running
        // when this happens we increase the priorities of the processes in the ready queue.
        int head = rdyhead; 
        while(head != rdytail){

          //ignoring the null process
          if(head != 0)
          q[(head)].qkey+=2;

          head = q[(head)].qnext;
        }
      	return (OK);
    }

    /* force context switch */

    if (optr -> pstate == PRCURR) {
      optr -> pstate = PRREADY;
      insert(currpid, rdyhead, optr -> pprio);
    }

    /* remove highest priority process at end of ready list */

    nptr = & proctab[(currpid = getlast(rdytail))];
    nptr -> pstate = PRCURR; /* mark it currently running	*/
    #ifdef RTCLOCK
    preempt = QUANTUM; /* reset preemption counter	*/
    #endif

    ctxsw((int) & optr -> pesp, (int) optr -> pirmask, (int) & nptr -> pesp, (int) nptr -> pirmask);

    /* The OLD process returns here when resumed. */
    return OK;


  }
  else if (getschedclass() == LINUXSCHED){



    
  }
   else {
    // this is the default schedule we begin with

    // this is where the default scheduler will work from
    if (((optr = & proctab[currpid]) -> pstate == PRCURR) &&
      (lastkey(rdytail) < optr -> pprio)) {
      return (OK);
    }

    /* force context switch */

    if (optr -> pstate == PRCURR) {
      optr -> pstate = PRREADY;
      insert(currpid, rdyhead, optr -> pprio);
    }

    /* remove highest priority process at end of ready list */

    nptr = & proctab[(currpid = getlast(rdytail))];
    nptr -> pstate = PRCURR; /* mark it currently running	*/
    #ifdef RTCLOCK
    preempt = QUANTUM; /* reset preemption counter	*/
    #endif

    ctxsw((int) & optr -> pesp, (int) optr -> pirmask, (int) & nptr -> pesp, (int) nptr -> pirmask);

    /* The OLD process returns here when resumed. */
    return OK;
  }
}