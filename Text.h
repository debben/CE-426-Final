#ifndef __TEXT_H
#define __TEXT_H

#include "Time.h"
#include <stdint.h>
#include "board_files/sram.h"
#include "board_files/Serial.h"

#define LIST_HEAD ((struct TextMessage*)mySRAM_BASE)
#define LINKED_LIST mySRAM_BASE
#define BLOCK_SIZE sizeof(struct TextMessage)
#define MESSAGE_LIMIT 2000

//contains data structures for text messaging
struct TextMessage {
	struct Time rxTime;
	struct TextMessage* prev; //Link to previous message
	struct TextMessage* next; //Link to next message
	char text[160];
	uint8_t length;
};

void initTexts(void);
void resetList(void);
struct TextMessage* deleteMessage(struct TextMessage* toDelete);
void addMessage(unsigned char length, char* text, struct Time rxTime);
__task void taskSerial(void);
void USART3_IRQHandler();



extern struct TextMessage* tailMessage;
extern struct TextMessage* headMessage;
extern struct TextMessage* displayedMessage;
extern uint16_t messageCount;
extern int msgOffset;
extern OS_MUT mutTime;
extern OS_MUT mut_msg_ptr;
extern OS_MUT mut_msg_offset;
#endif