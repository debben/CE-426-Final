#include "Text.h"
#include "Display.h"

void screamAndDie();

//mailboxes
os_mbx_declare(serialChar, 1);

void initTexts(void){
	//mbx init
	os_mbx_init(&serialChar, 160);
}

void addMessage(unsigned char length, char* text, struct Time rxTime)
{
	struct TextMessage* newMessage;
	unsigned char i;
	
	//newMessage = tailMessage + sizeof(struct TextMessage);
	newMessage = (struct TextMessage*)_alloc_box((void*)LINKED_LIST);
	if(newMessage == NULL)
	{
		screamAndDie();
	}
	tailMessage->next = newMessage;
	newMessage->prev = tailMessage;
	newMessage->rxTime = rxTime;
	newMessage->next = NULL;
	newMessage->length = length;
	
	for(i = 0; i < length; i++)
	{
		newMessage->text[i] = text[i];
	}
		
	tailMessage = newMessage;
	messageCount++;
}

struct TextMessage* deleteMessage(struct TextMessage* toDelete)
{
	struct TextMessage* retval;
	if(toDelete->prev != NULL)
	{
		//Delete a node from the list by linking its predecessor to its successor
		toDelete->prev->next = toDelete->next;
		toDelete->next->prev = toDelete->prev;
		if(toDelete == tailMessage)
		{
			tailMessage = toDelete->prev;
		}
		
		if(messageCount > 0)
		{
			messageCount--;
		}
	}
	//In order to help update the display, return a pointer to the next most recent message (if available)
	//If both next and prev are null, that means the entire list has been deleted. It is the responsibility
	//of the calling code not to delete the list head.
	if(toDelete->next != NULL)
	{
		retval = toDelete->next;
	}
	else
	{
		retval = toDelete->prev;
	}
	
	if(_free_box((void*)LINKED_LIST,(void*)toDelete) != 0)
	{
		screamAndDie();
	}
	
	return retval;
}

void resetList(void)
{
	if(_init_box((void*)LINKED_LIST, BLOCK_SIZE*MESSAGE_LIMIT, BLOCK_SIZE) != 0)
	{
		screamAndDie();
	}
	
	//tailMessage = (struct TextMessage*)LIST_HEAD;
	tailMessage = (struct TextMessage*)_alloc_box((void*)LINKED_LIST);
	if(tailMessage == NULL)
	{
		screamAndDie();
	}
	
	headMessage = tailMessage;
	messageCount = 0;
}


__task void taskSerial(void){
 	char* latestChar;
 	unsigned char index = 0;
 	char str[160];
	OS_RESULT err;
	
	for(;;)
	{
		err = os_mbx_wait(&serialChar, (void**)&latestChar, 0xFFFF);
// 		latestChar = (USART3->DR & 0x000000FF);
 		if((latestChar[0] == '\r') || (index >= 160))
		{
			//Move to next message
			err = os_mut_wait(&mutTime,0xFFFF);
			err = os_mut_wait(&mut_msg_ptr,0xFFFF);
			err = os_mut_wait(&mut_msg_offset, 0xFFFF);
			
			addMessage(index, str, time);
						

			
			if(messageCount == 1)
			{
				displayedMessage = tailMessage;
			  msgOffset = 0; //Reset the message scroll
				os_evt_set(DIS_EVT_MSG_PTR, disTaskId);
			}
			else
			{
				os_evt_set(DIS_EVT_SET_NOTIF, disTaskId);
			}
			
			os_mut_release(&mutTime);
			os_mut_release(&mut_msg_ptr);
			os_mut_release(&mut_msg_offset);
			
			index = 0;
		}
		else
		{
			str[index++] = latestChar[0];
		}
	}
}

void screamAndDie()
{
	static unsigned char failures = 0;
	failures++;
}


/*-------------------------------------------------------------------------------
 USART3_IRQHandler()
 This is triggered when serial data is received by the USART3 serial interface
 ------------------------------------------------------------------------------*/
void USART3_IRQHandler()
{
	static char newCharacter[60]; //60 character buffer
	static char index = 0;

	newCharacter[index] = SER_GetChar();
	isr_mbx_send(&serialChar, (void*)(newCharacter + index));
	index++;
	if(index >= 60)
		index = 0;
}