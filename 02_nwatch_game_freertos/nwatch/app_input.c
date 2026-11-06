/*
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

//#include "draw.h"
#include "resources.h"

#include "driver_ir_receiver.h"
#include "driver_rotary_encoder.h"
#include "driver_mpu6050.h" 
//#include "driver_dht11.h"



static QueueHandle_t g_xQueuePlatform; /* 挡球板队列 */
static QueueHandle_t g_xQueueIR;
static QueueHandle_t g_xQueueRotary; /* 编码器队列*/
static QueueHandle_t g_xQueueMPU6050;

static QueueSetHandle_t g_xQueueSetInput; /* 输入设备的队列集 */


static void ProcessIRData(void)
{
		struct ir_data idata ;
		struct input_data input;
		xQueueReceive(g_xQueueIR,&idata,0);
	
		if (idata.val == IR_KEY_LEFT)
		{
			input.dev = idata.dev;
			input.val = UPT_MOVE_LEFT;
		}
		else if (idata.val == IR_KEY_RIGHT)
		{
			input.dev = idata.dev;
			input.val = UPT_MOVE_RIGHT;
		}
		else if (idata.val == IR_KEY_REPEAT)
		{
			/* 保持不变 */;
		}
		else
		{
			input.dev = idata.dev;
			input.val = UPT_MOVE_NONE;
		}
		
		xQueueSend(g_xQueuePlatform, &input, 0);
	 
}

static void ProcessRotaryData(void)
{
	struct rotary_data rdata;
	struct input_data idata;
	int left;
	int i, cnt;
	
	/* 读旋转编码器队列 */
	xQueueReceive(g_xQueueRotary, &rdata, 0);
			
	/* 处理数据 */
	/* 判断速度: 负数表示向左转动, 正数表示向右转动 */
	if (rdata.speed < 0)
	{
		left = 1;
		rdata.speed = 0 - rdata.speed;
	}
	else
	{
		left = 0;
	}
	
	//cnt = rdata.speed / 10;
	//if (!cnt)
	//	cnt = 1;
	if (rdata.speed > 100)
		cnt = 4;
	else if (rdata.speed > 50)
		cnt = 2;
	else
		cnt = 1;
			
	/* 写挡球板队列 */
	idata.dev = 1;
	idata.val = left ? UPT_MOVE_LEFT : UPT_MOVE_RIGHT;
	for (i = 0; i < cnt; i++)
	{
		xQueueSend(g_xQueuePlatform, &idata, 0);
	}
	
}

static void ProcessMPU6050Data(void)
{
	struct mpu6050_data mdata;
	struct input_data idata;
	
	/* 读旋转编码器队列 */
	xQueueReceive(g_xQueueMPU6050, &mdata, 0);
			
	/* 处理数据 */
	/* 判断角度, 大于90度表示往左移动挡球板, 小于90度表示往右移动挡球板 */
	if (mdata.angle_x > 90)
	{
		idata.val = UPT_MOVE_LEFT;
	}
	else if(mdata.angle_x < 90)
	{
		idata.val = UPT_MOVE_RIGHT;
	}
	else
	{
		idata.val = UPT_MOVE_NONE;
	}
	
	/* 写挡球板队列 */
	idata.dev = 2;
	xQueueSend(g_xQueuePlatform, &idata, 0);
}

//static void DHT11_Fucn( TimerHandle_t xTimer )
//{
//	int hum, temp;
//	int ret;
//	char buff[16];
//	//读取数据
//
//
//	vTaskSuspendAll();
//	ret = DHT11_Read(&hum, &temp);	
//	xTaskResumeAll();
//	
//	//显示数据
//	
//	if(ret == 0)
//	{
//			sprintf(buff, "%dC,%d%%", temp, hum);
//			draw_string(buff, false, 40, 0);
//	}
//	else
//	{
//		draw_string("err      ", false, 40, 0);
//	}
//}
//
void InputTask(void *pvParameters)
{
	QueueSetMemberHandle_t xReadyQueue;
	
	/* 创建队列:平台任务从里面读到红外数据,旋转编码器数据... */
	
	g_xQueuePlatform = (QueueHandle_t)pvParameters;
  configASSERT(g_xQueuePlatform != NULL);
	
	g_xQueueSetInput = xQueueCreateSet(IR_QUEUE_LEN + ROTARY_QUEUE_LEN + MPU6050_QUEUE_LEN);
  
	g_xQueueIR = GetQueueIR();
	g_xQueueRotary = GetQueueRotary();
	g_xQueueMPU6050 = GetQueueMPU6050();
	
	xQueueAddToSet( g_xQueueIR,g_xQueueSetInput);
	xQueueAddToSet( g_xQueueRotary,g_xQueueSetInput);
	xQueueAddToSet( g_xQueueMPU6050,g_xQueueSetInput);
	
	while (1)
	{
		/* 读队列集, 得到有数据的队列句柄 */
		xReadyQueue = xQueueSelectFromSet(g_xQueueSetInput, portMAX_DELAY);
		
		if (xReadyQueue)
		{
			/* 读队列句柄得到数据,处理数据 */
			if (xReadyQueue == g_xQueueIR)
			{
				ProcessIRData();
			}
			else if (xReadyQueue == g_xQueueRotary)
			{
				ProcessRotaryData();
			}
			else if (xReadyQueue == g_xQueueMPU6050)
			{
				ProcessMPU6050Data();
			}
		}
	}
	

}

