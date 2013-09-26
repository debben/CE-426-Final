#ifndef __TIME_H
#define __TIME_H

#include <RTL.h>

//contains data structures for timing
struct Time {
	unsigned char min;
	unsigned char hours;
	unsigned char sec;
};

__task void taskIncSec(void);
__task void taskIncHour(void);
__task void taskIncMin(void);


extern struct Time time;					//global time

//semaphores
extern OS_SEM semIncMin;
extern OS_SEM semIncHour;
extern OS_SEM semIncSec;


//task id'd
extern OS_TID disTaskId;
extern OS_MUT mutTime;



#endif