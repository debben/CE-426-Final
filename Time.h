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

extern OS_SEM semIncMin;
//OS_SEM semDisplay;
extern OS_SEM semIncHour;
extern OS_SEM semIncSec;
//OS_SEM semKeyPress;

//task id'd
//OS_TID keyTaskId;
//OS_TID joyTaskId;
extern OS_TID disTaskId;

//Mutex definitions
extern OS_MUT mutTime;
//OS_MUT mut_msg_ptr;
//OS_MUT mut_msg_offset;


#endif