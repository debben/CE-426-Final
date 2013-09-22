#ifndef __INPUTS_H
#define __INPUTS_H

#include <stdint.h>
#include "board_files/JOY.h"
#include <RTL.h>
#include "Display.h"
#include "Time.h"

#define KEY_WAKEUP 1
#define KEY_TAMPER 2

//redefine JOY constants here because they're flipped.

#define JOY_UP    (1 << 0)
#define JOY_DOWN   (1 << 1)
#define JOY_RIGHT      (1 << 3)
#define JOY_LEFT    (1 << 4)

#define MESSAGE_OFFSET_INCREMENT 20

enum Joy_Machine
{
	JOY_WAIT = 0,
	JOY_INC_MESSAGE,
	JOY_INC_OFFSET,
	JOY_DELETE_CONF,
	JOY_DELETE,
	JOY_CLEAR
	
};

__task void key_task(void);
__task void joy_task(void);

extern OS_TID disTaskId;
extern struct TextMessage* tailMessage;
extern struct TextMessage* displayedMessage;
extern uint16_t messageCount;
extern OS_SEM semIncMin;

extern OS_SEM semIncHour;
//OS_SEM semIncSec;

#endif