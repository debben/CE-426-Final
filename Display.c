#include "Display.h"

/*
	Method Name: init_display
	Parameters: none
	Purpose: Paint the intial background colors and text of the display.
	returns: none
*/
void init_display(void){
	//Use a blue background with white text
  	GLCD_Clear(Blue);
  	GLCD_SetBackColor(Black);
	GLCD_ClearLn(0,1);
	GLCD_SetTextColor(LightGrey);
	GLCD_DisplayString(0, 6, 1, "12:00:00");
}

/*
	Method Name: drawDeleteConf
	Parameters: none
	Purpose: Paint the red delete dialog to the screen
	returns: none
*/
void drawDeleteConf(void)
{
	char line1[] = "Delete Message?";
	char line2[] = "Left = Yes";
	char line3[] = "Right = No";
	
	GLCD_SetBackColor(Red);
	GLCD_SetTextColor(Black);
	
	GLCD_ClearLn(3,1);
	GLCD_ClearLn(4,1);
	GLCD_ClearLn(5,1);
	GLCD_ClearLn(6,1);
	GLCD_DisplayString (4, 0, 1, line1);
	GLCD_DisplayString (5, 0, 1, line2);
	GLCD_DisplayString (6, 0, 1, line3);
}

/*
	Method Name: drawMessage
	Parameters: the scroll offset into the message buffer.
	Purpose: Paint the message text and the timestamp to the screen at the 
			 appropriate scroll offset. Handles making sure long words won't
			 straddle lines by padding with whitespace and newlines.
	returns: int, how far into the message text got displayed
*/
int drawMessage(int offset){
	
	struct TextMessage message;
	int x, y, lookahead, messageCursor;
	
	unsigned char string[8];
	OS_RESULT err;
	
	if(offset < 0){
		offset = 0;
	}
	
	
	//get the mutex for the message and copy it localy.
	err = os_mut_wait(&mut_msg_ptr,0xFFFF);				
	message = *displayedMessage;
	os_mut_release(&mut_msg_ptr);
	
	
	//set color
	GLCD_SetBackColor(Blue);
	GLCD_SetTextColor(White);
	
	//start the message at the offset parameter.
	messageCursor = offset;

	//check to see if there's any whitespace at where we've been asked to start
	//the message. Move up the offset to the first word if there is.
	while(messageCursor > 0 && message.text[messageCursor] != ' ') messageCursor--;
	if(messageCursor < offset){
		messageCursor++; //start not at the space, but at first char of the word
	}
	
	//start painting the lines of the message
	for(y = 0; y < 5; y++)
	{
		//if we're already through the message, just paint whitespace for the 
		//line and continue.
		if(messageCursor >= message.length) //already at end of message
		{
			GLCD_ClearLn(3+y,1);
			continue;
		}

		//for the line,
		for(x = 0; x < 20; x++)
		{
			lookahead = 0; 
			//count how many non-whitespace characters are in the buffer
			while(message.text[messageCursor+lookahead] != ' ' && (messageCursor+lookahead) < message.length) lookahead++;

			//if that word doesn't fit on the line, we'll move to the next line
			if(x + lookahead > 20){				
				//the word would be split accross lines, don't print it.
				while(x<20)GLCD_DisplayChar(3+y,x++,1, ' ');
				break;
			}
			else{
				//the word fits on the line, print it.
				while(lookahead > 0){
					GLCD_DisplayChar(3+y,x++,1, message.text[messageCursor++]);									
					lookahead --;
				}					
				//increment the message cursor by 1 just so we get the trailing whitespace.
				GLCD_DisplayChar(3+y,x,1, ' ');
				messageCursor++;
			}
			//if there is space at the end of the line but the next word doesnt
			//fit, just pad with whitespace.
			if(x < 20 && messageCursor >= message.length){
				//need to pad with whitespace.
				while(x<20)GLCD_DisplayChar(3+y,x++,1, ' ');
				break;
			}
		}		
	}
	
	//print the time at the bottom of the screen.
	GLCD_SetBackColor(Blue);
	GLCD_SetTextColor(White);
	sprintf(string, "%02u:%02u:%02u", message.rxTime.hours, message.rxTime.min, message.rxTime.sec);
	GLCD_DisplayString(9, 0, 1, string);
	
	return messageCursor;
}

/*
	Method Name: updateDisplay
	Parameters: none
	Purpose: Task which listens for event flags of what has changed and what 
			 should be re-painted.
	returns: none
*/
__task void updateDisplay(void){
	OS_RESULT err;
	struct Time mytime;
	unsigned char string[8];
	uint16_t event;
  	int offset = 0;
	
	for(;;){
		//wait only on the bits we have defined flags for
		err = os_evt_wait_or((DIS_EVT_TIME | DIS_EVT_MSG_PTR | DIS_EVT_MSG_OFFSET | DIS_EVT_CONF | DIS_EVT_SET_NOTIF | DIS_EVT_CLR_NOTIF),0xFFFF);
		event = os_evt_get();

		//bassed on the event (see.h for event definitions)
		switch(event){
			case DIS_EVT_TIME:
				err = os_mut_wait(&mutTime,0xFFFF);				
				//copy the data to local time		
				mytime = time;
				os_mut_release(&mutTime);
				//now display the data
				GLCD_SetBackColor(Black);
				GLCD_SetTextColor(LightGrey);
				sprintf(string, "%02u:%02u:%02u", mytime.hours, mytime.min, mytime.sec);
				GLCD_DisplayString(0, 6, 1, string);				
				break;
			case DIS_EVT_SET_NOTIF:
				GLCD_SetBackColor(Black);
				GLCD_SetTextColor(LightGrey);
				//paint a new message notification
				GLCD_DisplayString(0,0,1,"NEW!");
				break;
			case DIS_EVT_CLR_NOTIF:
				//clear the new message notification
				GLCD_SetBackColor(Black);
				GLCD_SetTextColor(LightGrey);
				GLCD_DisplayString(0,0,1,"    ");
				break;
			case DIS_EVT_MSG_PTR: //current message pointer changed
				//get the mutex for the message			
				err = os_mut_wait(&mut_msg_ptr,0xFFFF);
				offset = drawMessage(0);
				msgOffset = 0;
				os_mut_release(&mut_msg_ptr);				
				break;
			case DIS_EVT_MSG_OFFSET: //message offset changed					
				err = os_mut_wait(&mut_msg_offset,0xFFFF);
				offset = drawMessage(msgOffset);
				os_mut_release(&mut_msg_offset);
				break;
			case DIS_EVT_CONF: //delete pressed
				drawDeleteConf();					
				break;
		}

	}
}
