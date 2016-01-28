/* ###*B*###
 * ERIKA Enterprise - a tiny RTOS for small microcontrollers
 *
 * Copyright (C) 2002-2013  Evidence Srl
 *
 * This file is part of ERIKA Enterprise.
 *
 * ERIKA Enterprise is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation,
 * (with a special exception described below).
 *
 * Linking this code statically or dynamically with other modules is
 * making a combined work based on this code.  Thus, the terms and
 * conditions of the GNU General Public License cover the whole
 * combination.
 *
 * As a special exception, the copyright holders of this library give you
 * permission to link this code with independent modules to produce an
 * executable, regardless of the license terms of these independent
 * modules, and to copy and distribute the resulting executable under
 * terms of your choice, provided that you also meet, for each linked
 * independent module, the terms and conditions of the license of that
 * module.  An independent module is a module which is not derived from
 * or based on this library.  If you modify this code, you may extend
 * this exception to your version of the code, but you are not
 * obligated to do so.  If you do not wish to do so, delete this
 * exception statement from your version.
 *
 * ERIKA Enterprise is distributed in the hope that it will be
 * useful, but WITHOUT ANY WARRANTY; without even the implied warranty
 * of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License version 2 for more details.
 *
 * You should have received a copy of the GNU General Public License
 * version 2 along with ERIKA Enterprise; if not, write to the
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301 USA.
 * ###*E*### */

#include "ee.h"
#include "ee_irq.h"
#include <stdio.h>
#include "stm32f4xx_conf.h"
#include "stm32f4_discovery.h"
#include "stm32f4_discovery_lcd.h"
#include "stm32f4xx.h"
#include "types.h"

#include "STMPE811QTR.h"
#include "pictures.h"
#include "Widget.h"
#include "WidgetConfig.h"
#include "Touch.h"
#include "Event.h"
#include "StateMachine.h"
#include "lcd_add.h"
#include "fonts.h"
#include "debug.h"


Control sm_control;

int ran = 0;



/*
 * SysTick ISR2
 */
ISR2(systick_handler)
{
	/* count the interrupts, waking up expired alarms */
	CounterTick(myCounter);
}

/*
 * TASK LCD
 */
TASK(TaskLCD)
{
	unsigned int px, py;
	TPoint p;
	if (GetTouch_SC_Async(&px, &py)) {
		p.x = px;
		p.y = py;
		OnTouch(MyWatchScr, &p);
	}
}

void setTimeString(char *watchstr, uint8_T hours, uint8_T minutes, uint8_T seconds, uint8_T tenths, uint8_T mode)
{
	sprintf(watchstr, "%2d:%2d:%2d", hours, minutes, seconds);
}
/*
 * TASK Clock
 */
unsigned char IsUpdateTime()
{
	unsigned char res;
	static unsigned char oh=0, om=0, os=0;
	if (Dhours!=oh || Dminutes!=om || Dseconds!= os)
		res = 1;
	else
		res = 0;
	oh = Dhours;
	om = Dminutes;
	os = Dseconds;
	return res;
}

void UpdateTime()
{
	unsigned char watchstr[20];
	setTimeString(watchstr, Dhours, Dminutes, Dseconds, Dtenths, mode);

	LCD_SetTextColor(Blue);
	LCD_SetBackColor(Blue);
	//LCD_DrawFullRect(25, 100, 106, 32);

	/*	WPrint(&MyWatchScr[TIMESTR], watchstr); */
}

void UpdateMode(unsigned char om, unsigned char m)
{
	switch(om) {
	case 2:
		DrawOff(&MyWatchScr[BALARM]);
		break;
	case 1:
		DrawOff(&MyWatchScr[BTIMER]);
		break;
	case 0:
		DrawOff(&MyWatchScr[BTIME]);
		break;
	case 3:
		DrawOff(&MyWatchScr[BSWATCH]);
		break;
	}
	switch(m) {
	case 2:
		DrawOn(&MyWatchScr[BALARM]);
		break;
	case 1:
		DrawOn(&MyWatchScr[BTIMER]);
		break;
	case 0:
		DrawOn(&MyWatchScr[BTIME]);
		break;
	case 3:
		DrawOn(&MyWatchScr[BSWATCH]);
		break;
	}
}

void strencode2digit(char *str, int digit)
{
	str[2]=0;
	str[0]=digit/10+'0';
	str[1]=digit%10+'0';
}

void tenths2digit(char *str, int digit)
{
	str[1]=0;
	str[0]=digit%10+'0';
}

void drawdigits(char *tstr,const Widget *ws, int digits, int x, int y)
{
	strencode2digit(tstr, digits);
	LCD_SetTextColor(Black);
	LCD_SetBackColor(Black);
	LCD_DrawFullRect(x, y, 62, 48);
	WPrint(ws, tstr);
}

// For visualizing the correct digits in the mode

void handling_digits(char *tstr)
{
	if (mode == 2) {
		LCD_SetTextColor(Black);
		LCD_SetBackColor(Black);
		LCD_DrawFullRect(140, 80, 110, 56);
	}
	else {
		WPrint(&MyWatchScr[SEP1STR], ":");
		WPrint(&MyWatchScr[SEP2STR], ":");
		if (mode == 1 ) {
			LCD_SetTextColor(Black);
			LCD_SetBackColor(Black);
			LCD_DrawFullRect(210, 80, 40, 56);
			drawdigits(tstr,&MyWatchScr[SECSTR], 0, 150, 80);
		}
		if (mode == 3) {
			drawdigits(tstr,&MyWatchScr[SECSTR], 0, 150, 80);
			tenths2digit(tstr, 0);
			LCD_SetTextColor(Black);
			LCD_SetBackColor(Black);
			LCD_DrawFullRect(208, 80, 35, 48);
			WPrint(&MyWatchScr[DOT], ".");
			WPrint(&MyWatchScr[TTSSTR], tstr);
		}
		if (mode == 0) {
			LCD_SetTextColor(Black);
			LCD_SetBackColor(Black);
			LCD_DrawFullRect(210, 80, 40, 56);
		}
	}
}

// Updating digits after every step

void update_digits(char *tstr, unsigned char *oh, unsigned char *om,
		unsigned char *os, unsigned char *ot) {
	if (Dhours!=*oh) {
		drawdigits(tstr,&MyWatchScr[HRSSTR],(int)Dhours, 10, 80);
		*oh=Dhours;
		WPrint(&MyWatchScr[SEP1STR], ":");
	}

	if (Dminutes!=*om) {
		drawdigits(tstr,&MyWatchScr[MINSTR],(int)Dminutes, 80, 80);
		*om=Dminutes;
		if (mode!=1 && mode != 2)
			WPrint(&MyWatchScr[SEP2STR], ":");
	}
	if (Dseconds!= *os && mode!=2) {
		drawdigits(tstr,&MyWatchScr[SECSTR],(int)Dseconds, 150, 80);
		*os=Dseconds;
	}
	if (mode == 3 && Dtenths != *ot) {
		tenths2digit(tstr, (int)Dtenths);
		LCD_SetTextColor(Black);
		LCD_SetBackColor(Black);
		LCD_DrawFullRect(208, 80, 35, 48);
		WPrint(&MyWatchScr[DOT], ".");
		WPrint(&MyWatchScr[TTSSTR], tstr);
		*ot=Dtenths;
	}
}



void handling_eventsC()
{
	if (IsEvent(TIMEMODE))
		control_smDispatch(&sm_control, Btime);
	else if (IsEvent(TIMERMODE))
		control_smDispatch(&sm_control, Btimer);
	else if (IsEvent(ALARMMODE))
		control_smDispatch(&sm_control, Balarm);
	else if (IsEvent(SWATCHMODE))
		control_smDispatch(&sm_control, Bswatch);
	else if (IsEvent(PLUS))
		control_smDispatch(&sm_control, Bplus);
	else if (IsEvent(MINUS))
		control_smDispatch(&sm_control, Bminus);
	else if (IsEvent(START))
		control_smDispatch(&sm_control, Bstart);
	else if (IsEvent(STOP))
		control_smDispatch(&sm_control, Bstop);

}

TASK(TaskControl)
{
	int rand = 0, t = 0;
	unsigned char i;
	static int oldmode=8;
	static unsigned char oh=99, om=99, os=99, ot=99;
	char tstr[3];

	control_smDispatch(&sm_control, ptick);
	handling_eventsC();

	ClearEvents();

	update_digits(tstr, &oh, &om, &os, &ot);

	if (Alarm_signal) {
		if (!ran) {
			ran = 1;
			DrawOff(&MyWatchScr[17]);
		}
		else {
			ran = 0;
			DrawOff(&MyWatchScr[18]);
		}
	}
		else DrawOff(&MyWatchScr[18]);


	if (Timer_exp)
		WPrint(&MyWatchScr[14], "Timer Expired");
	else
		DrawOff(&MyWatchScr[19]);
	if (mode != oldmode) {
		handling_digits(tstr);
		UpdateMode(oldmode, mode);
		oldmode = mode;
	}
}


TASK(TaskTC)
{
	if (mutex == 0) {
		tenths = (tenths + 1) % 10;
		if (tenths == 0) {
			seconds = (seconds + 1) % 60;
			if (seconds == 0) {
				minutes = (minutes + 1) % 60;
				if (minutes == 0)
					hours = (hours + 1) % 24;
			}
		}
	}
}

TASK(TaskAlarmMNG)
{
	if (Dhours == Alarm_h && Dminutes == Alarm_m && Alarm_state == 1
			&& Dseconds == 0 && Dtenths == 0)
	{
		Alarm_state = 0;
		Alarm_signal = 1;
	}
}
/**
 * @brief  Inserts a delay time.
 * @param  nCount: specifies the delay time length.
 * @retval None
 */



/*
 * MAIN TASK
 */
int main(void)
{
	//GPIO_InitTypeDef GPIO_InitStructure;

	SystemInit();
	/*Initializes Erika related stuffs*/
	EE_system_init();

	/*Initialize systick */
	EE_systick_set_period(MILLISECONDS_TO_TICKS(1, SystemCoreClock));
	EE_systick_enable_int();
	EE_systick_start();

	/* Initializes LCD and touchscreen */
	IOE_Config();
	/* Initialize the LCD */
	STM32f4_Discovery_LCD_Init();


	control_smInit(&sm_control);


	InitTouch(-0.089471, 0.064819, -354, 6);
	/* Draw the background */
	DrawInit(MyWatchScr);
	LCD_SetTextColor(Black);
	LCD_SetBackColor(Black);
	LCD_DrawFullRect(10, 80, 240, 56);
	WPrint(&MyWatchScr[SEP1STR], ":");
	WPrint(&MyWatchScr[SEP2STR], ":");


	/* Program cyclic alarms which will fire after an initial offset,
	 * and after that periodically
	 * */
	SetRelAlarm(AlarmTaskLCD, 10, 150);
	SetRelAlarm(AlarmTaskControl, 10, 100);
	SetRelAlarm(AlarmTaskTC, 10, 100);
	SetRelAlarm(AlarmTaskMNG, 10, 100);
	/* Forever loop: background activities (if any) should go here */
	for (;;) { 
	}

}


