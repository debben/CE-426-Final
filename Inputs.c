#include "Inputs.h"

/*
	Method Name: key_task
	Parameters: none
	Purpose: Task which gets signals about the hour and minute buttons
	returns: none
*/
__task void key_task(void){
	OS_RESULT err;
	uint32_t key_press = 0;

	for(;;){
		//wait for one of the two defined keys.
		err = os_evt_wait_or((KEY_WAKEUP | KEY_TAMPER), 0xFFFF);
		key_press = os_evt_get();

		//if presesd, send the appropriate semaphore.
		if(key_press != 0)
		{
			if((key_press & KEY_WAKEUP) != 0)
			{
				os_sem_send(&semIncHour);
			}
			else if((key_press & KEY_TAMPER) != 0)
			{
				os_sem_send(&semIncMin);
			}
		}
	}
}

/*
	Method Name: joy_task
	Parameters: none
	Purpose: Contains a state machine for user interraction. It's behavior 
			 varies on what state the system is in.
	returns: none
*/
__task void joy_task(void){
	OS_RESULT err;
	uint32_t joy_press = 0;
	unsigned char proceed = 0;
	
	enum Joy_Machine state = JOY_WAIT;

	for(;;){
		switch(state){ //nested switches, for the win
				case JOY_WAIT: //system isn't doing anything, so just wait					
					switch(joy_press){
						case JOY_UP:
						case JOY_DOWN:
							state = JOY_INC_OFFSET;
							break;							
						case JOY_LEFT:
						case JOY_RIGHT:
							state = JOY_INC_MESSAGE;
							break;
						case JOY_CENTER:
							state = JOY_DELETE_CONF;
							break;
						default:
							//wait for joystick
							err = os_evt_wait_or((JOY_LEFT | JOY_RIGHT | JOY_CENTER | JOY_UP | JOY_DOWN), 0xFFFF);
							joy_press = os_evt_get();
							break;
					}
					
					break;
				case JOY_INC_OFFSET:
					//get mutex for message text offset									  	
					err = os_mut_wait(&mut_msg_offset,0xFFFF);

					//change offset bassed on what was pressed
					if(joy_press == JOY_UP){
						msgOffset -= MESSAGE_OFFSET_INCREMENT;
						if(msgOffset <= 0) msgOffset = 0;
					}
					else{
						if(msgOffset + MESSAGE_OFFSET_INCREMENT < displayedMessage->length)
							msgOffset += MESSAGE_OFFSET_INCREMENT;						
					}
					//release offset
					os_mut_release(&mut_msg_offset);
					//send to the display task that the offset has changed
					os_evt_set(DIS_EVT_MSG_OFFSET, disTaskId);
					//reset the machine
				  	joy_press = 0;
					state = JOY_WAIT;
					break;
				case JOY_INC_MESSAGE:
					//get mutex for global message pointer
					err = os_mut_wait(&mut_msg_ptr, 0xFFFF);
					if(joy_press == JOY_RIGHT)
					{
						displayedMessage = ((displayedMessage->next != NULL) ? displayedMessage->next : displayedMessage);
						
						if(displayedMessage == tailMessage)
						{
							//tell the display to clear the "NEW!" notificaiton
							os_evt_set(DIS_EVT_CLR_NOTIF, disTaskId);
						}
					}
					else
					{
						if((messageCount > 0) && (displayedMessage->prev == headMessage))
						{
							//Don't move to the "No Messages" message if there are actually messages
						}
						else
						{
						  displayedMessage = ((displayedMessage->prev != NULL) ? displayedMessage->prev: displayedMessage);
						}
					}
					//release that mutex
					os_mut_release(&mut_msg_ptr);
				  	//tell the display we've switched messages
					os_evt_set(DIS_EVT_MSG_PTR, disTaskId);
					//reset machine
					joy_press = 0;
					state = JOY_WAIT;
					break;
				case JOY_DELETE_CONF:					
					//get mutex for global delete mode bool									
					err = os_mut_wait(&mut_msg_ptr, 0xFFFF);
					if(displayedMessage != headMessage)
						proceed = 1;
					else
						proceed = 0;
					
					os_mut_release(&mut_msg_ptr);
				
					if(proceed != 0)
					{
						os_evt_set(DIS_EVT_CONF, disTaskId);
						//wait here for further user input.
						err = os_evt_wait_or((JOY_LEFT | JOY_RIGHT), 0xFFFF);
						joy_press = os_evt_get();
						if(joy_press == JOY_LEFT){
							state = JOY_DELETE;
						}
						else{
							state = JOY_CLEAR;
						}
					}
					else
					{
						state = JOY_CLEAR;
					}
					
					break;
				case JOY_DELETE:
					//call method to delete message at current pointer and 
					//update the nodes around it.
					//needs to use mutex to safely update global message ptr.				  	
					err = os_mut_wait(&mut_msg_ptr, 0xFFFF);

					if(displayedMessage->next == tailMessage)
					{
						//tell the display to clear the delete box
						os_evt_set(DIS_EVT_CLR_NOTIF, disTaskId);
					}
					displayedMessage = deleteMessage(displayedMessage);
				
					os_mut_release(&mut_msg_ptr);
					//tell the display the message pointer changed
					os_evt_set(DIS_EVT_MSG_PTR, disTaskId);
					joy_press = 0;
					//reset the machine
					state = JOY_WAIT;
					break;
				  					
				case JOY_CLEAR:					
					//tell display task the show delete confirmation bool as changed.
					os_evt_set(DIS_EVT_MSG_PTR, disTaskId);
					joy_press=0;
					//reset the machine
					state = JOY_WAIT;
					break;
		}
	}
}
