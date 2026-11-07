/*
 * Project: N|Watch
 * Author: Zak Kemble, contact@zakkemble.co.uk
 * Copyright: (C) 2013 by Zak Kemble
 * License: GNU GPL v3 (see License.txt)
 * Web: http://blog.zakkemble.co.uk/diy-digital-wristwatch/
 */

#ifndef APP_INPUT_H_
#define APP_INPUT_H_

#include <stdbool.h>
#include <stdint.h>
#include "portmacro.h"

void InputTask(void *pvParameters);

enum dev{
	IR , Rotary ,MPU6050
};

enum direction{
	LEFT , NONE , RIGHT
};

struct game1_data{
	uint32_t dev;
	uint32_t dir;
	uint32_t steps;
	TickType_t timestamp;
	
};


#endif /* APP_INPUT_H_ */
