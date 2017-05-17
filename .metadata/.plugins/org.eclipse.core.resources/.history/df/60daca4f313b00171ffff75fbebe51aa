/*
 * display.h
 *
 *  Created on: May 12, 2017
 *      Author: ajs5433
 */
#define displayValues	0
#define displayClock	1


#ifndef DISPLAY_H_
#define DISPLAY_H_



class display
{
	int status;

	//Threads
	int frequency_thread;

public:
		display();

		void *ledPWM(void *object);
		void setDisplayMode();
		void setSpeed();
		void activateDisplay();

};
#endif /* DISPLAY_H_ */
