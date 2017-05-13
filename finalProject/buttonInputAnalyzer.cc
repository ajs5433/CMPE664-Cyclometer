/*
 * magneticSensorAnalyzer.cpp
 *
 *  Created on: May 9, 2017
 *      Author: ajs5433
 */

#include <iostream>
#include "buttonInputAnalyzer.h"
#include <sys/neutrino.h>

void * buttonInputAnalyzer::buttonMode_thread(void *object){
	std::cout<<"testing button"<<std::endl;
	return NULL;
}

void * buttonInputAnalyzer::buttonSet_thread(void *object){

	return NULL;
}

buttonInputAnalyzer::buttonInputAnalyzer(){

	buttonMode = ThreadCreate(0, &buttonMode_thread, NULL, NULL);
	buttonSet = ThreadCreate(0, &buttonSet_thread, NULL, NULL);
}

void buttonInputAnalyzer::assignDisplay(display d)
{
	myDisplay = d;
	std::cout<<"printing"<<std::endl;
}

