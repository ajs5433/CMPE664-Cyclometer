/*
 * magneticSensorAnalyzer.cpp
 *
 *  Created on: May 9, 2017
 *      Author: ajs5433
 */

#include <iostream>
#include <sys/neutrino.h>
#include "display.h"
/*
void *ledPWM(void *object)
{

	return NULL;
}
*/
display::display(int temp)
{
	std::cout<<"Display Connected!"<<std::endl;

	displayTime = temp;
	displayMode = displayClockTime;
	Display1 = 0;
	Display2 = 0;
	Display3 = 0;
	Display4 = 0;

	updateDisplays = ThreadCreate(0, &updateDisplays_thread, NULL, NULL);
	displayMain = ThreadCreate(0, &displayMain_thread, NULL, NULL);

}

void * display::updateDisplays_thread(void *object){

	return NULL;
}

void * display::displayMain_thread(void *object)
{
	while(true)
	{
		while(displayMode == displaySpeed)
		{

		}
		while(displayMode == displayClockTime)
		{

		}
	}

	return NULL;
}

void display::displaySpeedOn(){
	displayMode = displaySpeed;
}

void display::displaySpeedOff(){
	displayMode = displayClockTime;
}



