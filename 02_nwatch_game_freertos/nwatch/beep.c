/*na
 * Project: N|Watch
 * Author: Zak Kemble, contact@zakkemble.co.uk
 * Copyright: (C) 2013 by Zak Kemble
 * License: GNU GPL v3 (see License.txt)
 * Web: http://blog.zakkemble.co.uk/diy-digital-wristwatch/
 */
#include <stdlib.h>
#include <stdio.h>

#include "cmsis_os.h"
#include "FreeRTOS.h"                   // ARM.FreeRTOS::RTOS:Core
#include "task.h"                       // ARM.FreeRTOS::RTOS:Core
#include "event_groups.h"               // ARM.FreeRTOS::RTOS:Event Groups
#include "semphr.h"                     // ARM.FreeRTOS::RTOS:Core


#include "driver_passive_buzzer.h"

static TimerHandle_t g_TimeSound;

static void buzz_control( TimerHandle_t xTimer )
{
	PassiveBuzzer_Control(0);
}

void buzzer_init(void)
{
	//初始化蜂鸣器
		PassiveBuzzer_Init();
	//初始化软件定时器
  g_TimeSound = xTimerCreate( "GameSound", 
							200,
							pdFALSE,
							NULL,
							buzz_control );
	
}

//实现设置buzz函数设置周期和频率

void buzzer_buzz(int freq, int time_ms)
{
	//设置频率和占空比
	PassiveBuzzer_Set_Freq_Duty(freq, 50);
	//设置周期
	xTimerChangePeriod(  g_TimeSound , time_ms, 0 );
	
}

//