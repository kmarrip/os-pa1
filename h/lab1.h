#define AGESCHED 1 
#define LINUXSCHED 2

#include <proc.h>
int currentClass;

void setschedclass(int sched_class){
    currentClass = sched_class;
}

int getschedclass(){
    return currentClass;
}

void calculateEpoch(){
    struct pentry *p;
    int i;
    for(i=0;i<NPROC;i++){
        p = &proctab[i];
        // we are resetting the goodness, quantum and counter values
        

        if(p -> pstate == PRFREE) continue;
        // 1. For a process that has never executed or has exhausted its time quantum in the previous epoch, 
        // its new quantum value is set to its process priority (i.e., quantum = priority).

        // if a process has utilized all of its quantum
        if(p->counter == 0){
            p -> quantum = p -> pprio;
        }
        // if a process has counter equal to quantum
        else if (p->counter == p->quantum){
            p -> quantum = p -> pprio;
        }
        
        // 2. or a process that did not get to use up its previously assigned quantum, we allow part of the unused quantum to be carried over to the new epoch. 
        // Suppose for each process, a variable counter describes how many ticks are left from its quantum, then at the beginning of the next epoch, quantum = floor(counter/2) + priority.

        // the process is either starting from scratch or it has some 
        else {
            p -> quantum = p -> pprio + (p->counter)/2;
        }
        p -> counter = p -> quantum;

        // if the process has used up its quantum, its goodness is set to 0
        // but here since we are resetting the values, goodness is equal to priority + counter;
        // we need to update the values of goodness everytime there is a reschedule.
        p -> goodness = p -> pprio + p -> counter;
    }
}