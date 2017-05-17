/*
 * display.h
 *
 *  Created on: May 12, 2017
 *      Author: ajs5433
 */
#ifndef DISPLAY_H_
#define DISPLAY_H_

/*Defines shared between classes*/
#define LOW 			0x00
#define HIGH 			0xFF
#define BASE_ADDR 		0x280
#define PORTA 			(BASE_ADDR + 8)
#define PORTB 			(BASE_ADDR + 9)
#define DIR_CTRL 		(BASE_ADDR + 11)

/*Specific defines for the Display*/
#define displaySpeed		0
#define displayClockTime	1

/*				Software ID	//	FPGA pins		//	QNX pins										*/
#define Anode0			1	//   FPGA 6			//
#define Anode1			2	//   FPGA 8			//
#define Anode2			4	//   FPGA 10
#define Anode3			8	//   FPGA 12

#define segmentA		1	//   FPGA 14
#define segmentB		2	//   FPGA 16
#define segmentC		4	//   FPGA 18
#define segmentD		8	//   FPGA 20
#define segmentE		16	//   FPGA 22
#define segmentF		32	//   FPGA 24
#define segmentG		64	//   FPGA 26
#define segmentDP		128	//   FPGA 28

class display
{
	int displayMode;		//Current display mode: either displayClock or displaySpeed
	int displayTime;		//represents the time in micro-seconds that each display will be on

	/*Display Values*/
	int Display1;
	int Display2;
	int Display3;
	int Display4;

	/*Threads variables*/
	int updateDisplays;
	int displayMain;

public:
		display(int temp);

		static void *updateDisplays_thread(void *object);
		static void *displayMain_thread(void *object);

		//Internal Methods
		//void setSpeed();
		//void setClockTime();

		//External Methods
		void displaySpeedOn();
		void displaySpeedOff();

};
#endif /* DISPLAY_H_ */
