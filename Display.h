#ifndef __DISPLAY_H
#define __DISPLAY_H

#include "board_files/GLCD.h"
#include "Text.h"
#include <RTL.h>

#define FONT_HEIGHT	24
#define FONT_WIDTH	16

#define TOP_OFFSET	2
#define LEFT_OFFSET 2

//make globals visible
extern struct TextMessage* displayedMessage;
extern int msgOffset;
extern OS_MUT mutTime;
extern OS_MUT mut_msg_ptr;
extern OS_MUT mut_msg_offset;
extern struct Time time;


enum Display_Events
{
	DIS_EVT_TIME = 1,
	DIS_EVT_MSG_PTR = 2,
	DIS_EVT_MSG_OFFSET = 4,
	DIS_EVT_CONF = 8,
	DIS_EVT_SET_NOTIF = 16,
	DIS_EVT_CLR_NOTIF = 32
} ;

//utility methods
void init_display(void);
void drawDeleteConf(void);
int drawMessage(int offset);

//task definitions
__task void updateDisplay(void);
#endif