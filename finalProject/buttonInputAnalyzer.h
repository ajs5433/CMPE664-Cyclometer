/*
 * buttonInputAnalyzer.h
 *
 *  Created on: May 12, 2017
 *      Author: ajs5433
 */
#include "display.h"

#ifndef BUTTONINPUTANALYZER_H_
#define BUTTONINPUTANALYZER_H_
class buttonInputAnalyzer
{
	display myDisplay;
	int buttonMode;
	int buttonSet;

public:
	void test();
	buttonInputAnalyzer();
	void assignDisplay(display d);

	//threads
	static void *buttonSet_thread(void *object);
	static void *buttonMode_thread(void *object);
};

#endif /* BUTTONINPUTANALYZER_H_ */
