#include "Time.h"
#include "Display.h"
//increment minutes task
__task void taskIncMin(void) {
		OS_RESULT err;
		for(;;){
			err = os_sem_wait(&semIncMin, 0xFFFF);
			err = os_mut_wait(&mutTime, 0xFFFF);
			//modify the value
			time.min++;
			if(time.min >= 60){
				time.min = 0;				
				os_sem_send(&semIncHour);
			}
			os_mut_release(&mutTime);
			
			//display the updated time
			os_evt_set(DIS_EVT_TIME, disTaskId);
		}
}

//increment hour task
__task void taskIncHour(void){
	OS_RESULT err;
	for(;;){
			err = os_sem_wait(&semIncHour, 0xFFFF);
			err = os_mut_wait(&mutTime, 0xFFFF);
			//modify the value
			time.hours++;
			if(time.hours > 12){
				time.hours = 1;								
			}
			os_mut_release(&mutTime);
			
			//display the updated time
			os_evt_set(DIS_EVT_TIME, disTaskId);
	}
}

__task void taskIncSec(void){
	OS_RESULT err;
	for(;;){	
			err = os_sem_wait(&semIncSec, 0xFFFF);
			err = os_mut_wait(&mutTime, 0xFFFF);
			//modify the value
			time.sec++;
			if(time.sec > 59){
				time.sec = 0;
				os_sem_send(&semIncMin);
			}
			os_mut_release(&mutTime);
			
			//display the updated time
			os_evt_set(DIS_EVT_TIME, disTaskId);
	}
}



