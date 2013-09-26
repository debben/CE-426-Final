#include "Text.h"
#include "Display.h"


void screamAndDie();

//mailboxes
os_mbx_declare(serialChar, 1);

/*
	Method Name: initTexts
	Parameters: none
	Purpose: setup the mailbox. Needed a function hook for the main file.
	returns: none
*/
void initTexts(void){
	//mbx init
	os_mbx_init(&serialChar, 160);
}

/*
	Method Name: addMessage
	Parameters: unsigned char length,	the message length
				char* text,				the Message Body
				struct Time rxTime, 	the time the message came in
	Purpose: adds a new message struct to the linked list
	returns: none
*/
void addMessage(unsigned char length, char* text, struct Time rxTime)
{
	struct TextMessage* newMessage;
	unsigned char i;
	
	//use memory pools so that a free list is maintained and memory wont fragment
	newMessage = (struct TextMessage*)_alloc_box((void*)LINKED_LIST);

	//something went wrong. crash
	if(newMessage == NULL)
	{
		screamAndDie();
	}

	//assemble and add the message
	tailMessage->next = newMessage;
	newMessage->prev = tailMessage;
	newMessage->rxTime = rxTime;
	newMessage->next = NULL;
	newMessage->length = length;
	
	//copy the message from input pointer to the newly minted message.
	for(i = 0; i < length; i++)
	{
		newMessage->text[i] = text[i];
	}
	
	//update global pointers	
	tailMessage = newMessage;
	messageCount++;
}

/*
	Method Name: deleteMessage
	Parameters: struct TextMessage* toDelete, the message you want "done-in"
	Purpose: Deletes a given message from the linked list
	returns: The new message that's in this one's place
*/
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
	//In order to help update the display, return a pointer to the next most 
	//recent message (if available). If both next and prev are null, that means 
	//the entire list has been deleted. It is the responsibility of the calling 
	//code not to delete the list head.
	if(toDelete->next != NULL)
	{
		retval = toDelete->next;
	}
	else
	{
		retval = toDelete->prev;
	}
	
	//return the space to the pool so memory if not fragmented
	if(_free_box((void*)LINKED_LIST,(void*)toDelete) != 0)
	{
		screamAndDie();
	}
	
	return retval;
}

/*
	Method Name: resetList
	Parameters: none
	Purpose: Initializes the memory pool and global head/tail pointers
	returns: none
*/
void resetList(void)
{
	//intilize memory pool
	if(_init_box((void*)LINKED_LIST, BLOCK_SIZE*MESSAGE_LIMIT, BLOCK_SIZE) != 0)
	{
		screamAndDie(); //crash on fail
	}
	
	//tailMessage = (struct TextMessage*)LIST_HEAD;
	tailMessage = (struct TextMessage*)_alloc_box((void*)LINKED_LIST);
	if(tailMessage == NULL)
	{
		screamAndDie(); //crash on fail.
	}
	
	headMessage = tailMessage;
	messageCount = 0;
}

/*
	Method Name: taskSerial
	Parameters: none
	Purpose: handles recieving chars from the ISR
	returns: none
*/
__task void taskSerial(void){
 	char* latestChar;
 	unsigned char index = 0;
 	char str[160];
	OS_RESULT err;
	
	for(;;)
	{
		//wait for the chars
		err = os_mbx_wait(&serialChar, (void**)&latestChar, 0xFFFF);

		//if theres newline or 160
 		if((latestChar[0] == '\r') || (index >= 160))
		{
			//Move to next message
			err = os_mut_wait(&mutTime,0xFFFF);
			err = os_mut_wait(&mut_msg_ptr,0xFFFF);
			err = os_mut_wait(&mut_msg_offset, 0xFFFF);
			
			addMessage(index, str, time);
			
			//special handling of the "no messages" string
			if(messageCount == 1)
			{
				displayedMessage = tailMessage;
				msgOffset = 0; //Reset the message scroll
				//tell the display the message pointer changed
				os_evt_set(DIS_EVT_MSG_PTR, disTaskId);
			}
			else
			{
				//tell the display to show "NEW!"
				os_evt_set(DIS_EVT_SET_NOTIF, disTaskId);
			}
			
			os_mut_release(&mutTime);
			os_mut_release(&mut_msg_ptr);
			os_mut_release(&mut_msg_offset);
			
			index = 0;
		}
		else
		{
			//assemble the string.
			str[index++] = latestChar[0];
		}
	}
}

/*
	Method Name: screamAndDie
	Parameters: none
	Purpose: all purpose error tracking
	returns: none
*/
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