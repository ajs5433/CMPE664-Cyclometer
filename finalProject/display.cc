/*
 * magneticSensorAnalyzer.cpp
 *
 *  Created on: May 9, 2017
 *      Author: ajs5433
 */

#include "display.h"

/*
void *ledPWM(void *object)
{

	return NULL;
}
*/
display::display()
{

}

void display::init(int dtime)
{
		std::cout<<"Display Connected!"<<std::endl;

		displayTime = dtime;
		displayMode = displayClockTime;
		Display1 = 0;
		Display2 = 0;
		Display3 = 0;
		Display4 = 0;

		pthread_create(&updateDisplays, NULL, updateDisplays_threadCreate, (void *)this);
}


void * display::updateDisplays_threadCreate(void *thread){
	/*display *self = static_cast<display *>(thread);
	self->updateDisplayRun();*/

	return ((display*)thread)->updateDisplayRun( );
}

/*
void * display:displayMain_threadCreate(void *object)
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
*/
void display::displaySpeedOn(){
	displayMode = displaySpeed;
}

void display::displaySpeedOff(){
	displayMode = displayClockTime;
}/*
void myThreads::updateDisplayRun()
{
	while(true)
	{
		std::cout<<"this is a test"<<std::endl;
		sleep(1);
	}
}



*/
