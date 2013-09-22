#include "Display.h"

/*
	Initialize the LCD
*/
void init_display(void){
	//Use a blue background with white text
  GLCD_Clear(Blue);
  GLCD_SetBackColor(Black);
	GLCD_ClearLn(0,1);
  GLCD_SetTextColor(LightGrey);
	GLCD_DisplayString(0, 6, 1, "12:00:00");
}

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

int drawMessage(int offset){
	
	struct TextMessage message;
	int x, y, lookahead, messageCursor;
	
	unsigned char string[8];
	OS_RESULT err;
	
	if(offset < 0){
		offset = 0;
	}
	
	
	//get the mutex for the message
	err = os_mut_wait(&mut_msg_ptr,0xFFFF);				
	message = *displayedMessage;
	os_mut_release(&mut_msg_ptr);
	
	
	//set color
	GLCD_SetBackColor(Blue);
	GLCD_SetTextColor(White);
	
	messageCursor = offset;
	while(messageCursor > 0 && message.text[messageCursor] != ' ') messageCursor--;
	if(messageCursor < offset){
		messageCursor++; //start not at the space, but at first char of the word
	}
	
	for(y = 0; y < 5; y++)
	{
		if(messageCursor >= message.length) //already at end of message
		{
			GLCD_ClearLn(3+y,1);
			continue;
		}
		for(x = 0; x < 20; x++)
		{
			lookahead = 0; 
			while(message.text[messageCursor+lookahead] != ' ' && (messageCursor+lookahead) < message.length) lookahead++;
			if(x + lookahead > 20){
				
				//speical case for if the word just strait up is > 18
				
				//the word would be split accross lines, don't print it.
				while(x<20)GLCD_DisplayChar(3+y,x++,1, ' ');
				break;
			}
			else{
				while(lookahead > 0){
					GLCD_DisplayChar(3+y,x++,1, message.text[messageCursor++]);									
					lookahead --;
				}					
				//increment the message cursor by 1 just so we get the trailing whitespace.
				GLCD_DisplayChar(3+y,x,1, ' ');
				messageCursor++;
			}
			if(x < 18 && messageCursor >= message.length){
				//need to pad with whitespace.
				while(x<20)GLCD_DisplayChar(3+y,x++,1, ' ');
				break;
			}
		}		
	}
	
	/*//now update the scroll bar.
	for(y = 1; y < 10; y++){
		GLCD_SetBackColor(DarkGrey);
		GLCD_DisplayChar(y,19,1,176);
	}
	GLCD_SetTextColor(White);
	GLCD_DisplayChar(1+(8*messageCursor/160),19,1,219);*/

	GLCD_SetBackColor(Blue);
	GLCD_SetTextColor(White);
	sprintf(string, "%02u:%02u:%02u", message.rxTime.hours, message.rxTime.min, message.rxTime.sec);
	GLCD_DisplayString(9, 0, 1, string);
	
	return messageCursor;
}

//update display task
__task void updateDisplay(void){
	OS_RESULT err;
	struct Time mytime;
	unsigned char string[8];
	uint16_t event;
  int offset = 0;
	
	for(;;){			
		err = os_evt_wait_or((DIS_EVT_TIME | DIS_EVT_MSG_PTR | DIS_EVT_MSG_OFFSET | DIS_EVT_CONF | DIS_EVT_SET_NOTIF | DIS_EVT_CLR_NOTIF),0xFFFF);
		event = os_evt_get();
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
				GLCD_DisplayString(0,0,1,"NEW!");
				break;
			case DIS_EVT_CLR_NOTIF:
				GLCD_SetBackColor(Black);
				GLCD_SetTextColor(LightGrey);
				GLCD_DisplayString(0,0,1,"    ");
				break;
			case DIS_EVT_MSG_PTR:								
				err = os_mut_wait(&mut_msg_ptr,0xFFFF);
				offset = drawMessage(0);
				msgOffset = 0;
				os_mut_release(&mut_msg_ptr);				
				break;
			case DIS_EVT_MSG_OFFSET:					
					//init_display();
					err = os_mut_wait(&mut_msg_offset,0xFFFF);
					offset = drawMessage(msgOffset);
					os_mut_release(&mut_msg_offset);
				break;
			case DIS_EVT_CONF:
				drawDeleteConf();
					//offset -= MESSAGE_OFFSET_INCREMENT;
					//offset = drawMessage(offset);
				break;
		}

	}
}
