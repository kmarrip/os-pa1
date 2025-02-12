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
  
    optr = &proctab[currpid];
    
    // preempt value will show how much time is still left for this process, that is about to be reschuled
    // optr -> counter, preempt - optr-> coutner will tell you how many ticks have been used up by the process
    // everytime the process is rescheduled, the goodness value decreases by the time it has used up

    // goodness is based on counter and priority, but changes made to priority while epoch is in progress is ignored
    // the only chnage that can be made to goodness is via the time slice
    // kprintf("current state %d, process priority %d, currpid %d , current goodness is %d, decrease by %d\n",optr -> pstate, optr -> pprio, currpid,optr->goodness ,preempt - optr-> counter);
    optr -> goodness+= preempt - optr->counter;
    optr -> counter = preempt;
    
    // also as the basecase to makesure the NULLPROC is not taken into consideration
    if(currpid == NULLPROC){
      optr -> goodness = 0;
      optr -> counter = 0;
    }
    // During each epoch, runnable processes are scheduled according to their goodness.
    // For processes that have used up their quantum, their goodness value is 0

    if(optr -> counter <= 0){
      optr -> goodness = 0;
      optr -> counter = 0;
    }


    // first lets iterate through the ready queue and find the next best process based on the goodness
    int head = q[rdyhead].qnext;
    int nextProcess = 0;
    int maxGoodness = 0;
    
    while(head != rdytail && head < NPROC){
      if(proctab[head].goodness > maxGoodness){
        maxGoodness =  proctab[head].goodness;
        nextProcess = head;
      }
      head = q[head].qnext;
    }

    // optr -> pstate will always be PRCURR, should be otherwise nothing makes sense


    // now based on the iteration we come up with different scenarios
    // 1. if the currpid is better than everything in the ready queue, dont really do anything
    if(optr-> pstate == PRCURR && optr-> goodness > 0 && optr->goodness > maxGoodness){
      preempt = optr -> counter; // update the prempt to the counter variable
      return OK;
    }

    // 2. if there is a better process in the ready queue which can be scheduled, or if the current process's counter has become zero
    // pick the best process and context switch
    if(( optr->goodness < maxGoodness || optr -> counter == 0 || optr -> pstate != PRCURR) && maxGoodness > 0){
      if(optr->pstate == PRCURR){
          optr -> pstate = PRREADY;
          insert(currpid, rdyhead, optr -> pprio);
      }


      // now the make the bette?r process the currpid and make it running
      // just use the functions implemented in q.h
      currpid = dequeue(nextProcess);
      nptr = &proctab[currpid];
			nptr->pstate = PRCURR;
      preempt = nptr -> counter;

      ctxsw((int) & optr -> pesp, (int) optr -> pirmask, (int) & nptr -> pesp, (int) nptr -> pirmask);
      return OK;
    }

    // 3. if the currpid is done with its counter and the next process in ready queue is useless,
    // then its time for us to start a new epoch
    if(( optr -> counter == 0 || optr-> pstate != PRCURR ) && maxGoodness == 0){
      calculateEpoch();
      // now the counter values have changed after restarting the epoch.
      preempt = optr -> counter;

      if(currpid == NULLPROC)
      return OK;


      // updates current process's state and put it back into the ready queue
      if(optr -> pstate == PRCURR){
          optr->pstate = PRREADY;
			    insert(currpid,rdyhead,optr->pprio);
      }
      // since the epoch is now starting new, dequeue the null process and restart the game
      // its assumed that the null proc will not be large
      currpid = dequeue(NULLPROC);
      nptr = &proctab[currpid];
      nptr -> pstate = PRCURR;

      #ifdef RTCLOCK
          preempt = QUANTUM;
      #endif

      ctxsw((int) & optr -> pesp, (int) optr -> pirmask, (int) & nptr -> pesp, (int) nptr -> pirmask);
      return OK;
    }
    return OK;

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