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
display::display()
{
	status = displayClock;

	std::cout<<"creating display"<<std::endl;
	//frequency_thread = ThreadCreate(0, &ledPWM, NULL, NULL);
}

void display::setDisplayMode(){

}

void display::setSpeed(){

}

void display::activateDisplay(){
	status = displayValues;
}
