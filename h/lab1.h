#define AGESCHED 1 
#define LINUXSCHED 2

int currentClass;

void setschedclass(int sched_class){
    currentClass = sched_class;
}

int getschedclass(){
    return currentClass;
}