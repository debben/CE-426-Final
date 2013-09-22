//this file only exists to help with linking
#include <RTL.h>

// extern uint32_t LEDOn, LEDOff; 
// extern uint32_t keys;
// extern uint16_t messageCount;
// extern uint16_t messageIndex;
// extern unsigned char last;

// //our 3 global scope variables.
// extern struct TextMessage* tailMessage;


// //example message
// extern struct TextMessage test;


// /*
// Real-time things.
// */

// //semaphores
// extern OS_SEM semIncMin;
// extern OS_SEM semDisplay;
// extern OS_SEM semIncHour;
// extern OS_SEM semIncSec;
// extern OS_SEM semKeyPress;

// //task id'd
// extern OS_TID keyTaskId;
// extern OS_TID joyTaskId;
// extern OS_TID disTaskId;

//Mutex definitions
//extern OS_MUT mutTime;
extern OS_MUT mut_msg_ptr;
extern OS_MUT mut_msg_offset;
