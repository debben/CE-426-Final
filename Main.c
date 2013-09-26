/*----------------------------------------------------------------------------
 * Name:    Main.c
 * Authors: Noah Gagnon, Don Ebben
 * Purpose: The main file of our SMS text messaging final project
 * Note(s): All globals are defined here. 
 *----------------------------------------------------------------------------*/

#include <stm32f2xx.h>                  /* STM32F2xx Definitions              */
#include "Display.h"
#include "Text.h"
#include "Time.h"
#include "Inputs.h"
#include "board_files/LED.h"



#include <RTL.h> /* access to all OS items */
#include <stdio.h>


uint32_t LEDOn, LEDOff; 
uint32_t keys;
uint16_t messageCount;
uint16_t messageIndex;
unsigned char last;

//our 3 global scope variables.
struct TextMessage* tailMessage;
struct TextMessage* headMessage;
struct TextMessage* displayedMessage;
int msgOffset;

//example message
struct TextMessage test;
struct Time time;					//global time

/*
Real-time things.
*/

//semaphores
OS_SEM semIncMin;
OS_SEM semDisplay;
OS_SEM semIncHour;
OS_SEM semIncSec;
OS_SEM semKeyPress;

//task id'd
OS_TID keyTaskId;
OS_TID joyTaskId;
OS_TID disTaskId;

//Mutex definitions
OS_MUT mutTime;
OS_MUT mut_msg_ptr;
OS_MUT mut_msg_offset;


/*
	Method Name: clock_task
	Parameters: none
	Purpose: provides ticks for the counter task and checks joystick and 
			 keys periodicly.
	returns: none
*/
__task void clock_task(void){
	  uint32_t ticks;
	  uint32_t last_keys = 0;
	  uint32_t last_joy = 0;
	  uint32_t value = 0;
	
		static unsigned char sec;
		for(;;){
			os_dly_wait(10);
			ticks++;
			if(ticks >= 10) {
				ticks = 0;
				//inc seconds if 1000 ms pass
				os_sem_send(&semIncSec);
			}

			//always send the value of the keys masked into the event flag.
			value = KBD_GetKeys();
			os_evt_set((uint16_t)((value ^ last_keys) & ~value),keyTaskId);
			last_keys = value;

			//send the joystick press always, using clever bitwise ops to send
			//only when a press is detected.			
			value = JOY_GetKeys();
			os_evt_set((uint16_t)((value ^ last_joy) & ~value),joyTaskId);
			last_joy = value;			
	}
		
}

/*
	Method Name: initTask
	Parameters: none
	Purpose: sets up other tasks and RTL constructs
	returns: none
*/
__task void initTask(void){
	
	//sem init
	os_sem_init(&semIncMin, 0);
	os_sem_init(&semKeyPress, 0);
	os_sem_init(&semIncHour, 0);
	os_sem_init(&semIncSec, 0);
	
	//setup the mailbox for text message chars
	initTexts();
		
	//mutex init
	os_mut_init(&mutTime);
	os_mut_init(&mut_msg_ptr);
	os_mut_init(&mut_msg_offset);
	
	//task init
	os_tsk_create(taskSerial, 0x82);
	os_tsk_create(clock_task, 0x90);
	os_tsk_create(taskIncHour, 0x7F);
	os_tsk_create(taskIncMin, 0x80);
	os_tsk_create(taskIncSec, 0x81);
	disTaskId = os_tsk_create(updateDisplay, 0x10);
	keyTaskId = os_tsk_create(key_task, 0x7E);
	joyTaskId = os_tsk_create(joy_task, 0x7D);
	
	//display stuff...
	os_evt_set(DIS_EVT_MSG_PTR, disTaskId);
	
	//end
	os_tsk_delete_self();
}


/*
	Method Name: main
	Parameters: none
	Purpose: setup drivers and other hardware related items before starting RTL
	returns: none
*/
int main (void) {
	
	//SysTick_Config(SystemCoreClock/1000);      /* Generate interrupt each 100 ms  */

	//Initialize the required I/O device libraries
	JOY_Init();
	LED_Init();
	SER_Init();
	KBD_Init();
	SRAM_Init();
	GLCD_Init();     // LCD Initialization
	
	time.min = 0;
	time.hours = 12;
	time.sec = 0;
	
	//serial and serial IRQ
	USART3->CR1 |= (1<<5); //Enable the "data received" interrupt for USART3
	NVIC_EnableIRQ(USART3_IRQn); //Enable interrupts for USART3
	NVIC_SetPriority (USART3_IRQn, (1<<__NVIC_PRIO_BITS) - 1);
	
	resetList();

	//Here we are manually setting the head node to display a 
	//"No Messages" message if we are not storing anything else
	memcpy(&headMessage->text,"No Messages",11);
	headMessage->length = 11;
	headMessage->prev = NULL;
	headMessage->next = NULL;
	headMessage->rxTime.hours = 0;
	headMessage->rxTime.min = 0;
	headMessage->rxTime.sec = 0;
	displayedMessage = headMessage;
	
	
	//Initialize the LCD	
	init_display();
	
	os_sys_init_prio(initTask,0xFE);
  
}





