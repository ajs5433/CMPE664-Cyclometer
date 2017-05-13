#include <cstdlib>
#include <iostream>
#include <sys/neutrino.h>
#include <termios.h>
#include <unistd.h>
#include <time.h>
#include <stdio.h>
#include <stdint.h>       /* for uintptr_t */
#include <hw/inout.h>     /* for in*() and out*() functions */
#include <sys/mman.h>     /* for mmap_device_io() */


#define state_closed_door 		0
#define state_opening_door 		1
#define state_opened_door 		2
#define state_closing_door 		3
#define state_lowering_door_paused 	4
#define state_rising_door_paused	5
#define hardware				0
#define software				1

#define PORT_LENGTH 	1
#define CLEAR 			0x00
#define HIGH 			0xFF
#define BASE_ADDR 		0x280
#define PORTA 			(BASE_ADDR + 8)
#define PORTB 			(BASE_ADDR + 9)
#define DIR_CTRL 		(BASE_ADDR + 11)

#define full_open		1
#define full_close		2
#define IR_beam			4
#define overcurrent		8
#define remote_push_btn	16
#define motor_up		1
#define motor_down		2
#define RESET_OFF		32

int closedDoor();
int openingDoor();
int openedDoor();
int closingDoor();
int loweringDoorPaused();
int risingDoorPaused();
void setupInfrarredBeam();

double timeElapsed = 0.00;
bool IRactive = false;						//Indicates when the IR sensor is active
bool garageActive = true;					//Indicates when the garage is active
bool OCactive = false;						//Overcurrent active
bool opening_door_flag = false;
bool closing_door_flag = false;

int keyPress = 0;
int closeTimer = 10;
int openTimer = 10;
int current_state = state_closed_door;
int simulation_type = hardware;
int sleeptime = 100;						// 500us for threads using usleep

//Declaring Events
bool buttonPressed_e = false;
bool infraredBeam_e = false;
bool overCurrent_e = false;
bool countingTen_e = false;
bool full_open_e = false;
bool full_close_e = false;

uintptr_t dir_ctrl, PORT_A, PORT_B;
struct timespec start, end;
double time_open;

// MOTOR CLASS

class motorC{
	public:
		void pause(){
			if (simulation_type == hardware) out8( PORT_A,CLEAR+RESET_OFF );
			else out8(PORT_A, CLEAR);

			std::cout << "Motor Paused" << std::endl;
			OCactive = false;
		}
		void turningRight(){
			if (simulation_type == hardware) out8( PORT_A, motor_up+RESET_OFF);
			else out8(PORT_A, CLEAR);

			std::cout << "Motor turning right" << std::endl;
			OCactive = true;
		}
		void turningLeft(){
			if (simulation_type == hardware) out8( PORT_A, motor_down+IR_beam+RESET_OFF);
			else out8(PORT_A, CLEAR);

			std::cout << "Motor turning left" << std::endl;
			OCactive = true;
		}
};

/* KEYBOARD INPUT CONFIGURATION:
 * The following code is geared to detect when a certain key from the keyboard has been pressed.
 * The idea was directly obtained from "http://www.openqnx.com/phpbbforum/viewtopic.php?t=6668"
 */

static struct termios initial_settings, new_settings;
static int peek_character = -1;

void init_keyboard()
 {
 tcgetattr(0,&initial_settings);
 new_settings = initial_settings;
 new_settings.c_lflag &= ~ICANON;
 new_settings.c_lflag &= ~ECHO;
 new_settings.c_lflag &= ~ISIG;
 new_settings.c_cc[VMIN] = 1;
 new_settings.c_cc[VTIME] = 0;
 tcsetattr(0, TCSANOW, &new_settings);
 }

 void close_keyboard(){
	 tcsetattr(0, TCSANOW, &initial_settings);
 }

 int kbhit(){
	 unsigned char ch;
	 int nread;
	 if (peek_character != -1) return 1;
	 new_settings.c_cc[VMIN]=0;
	 tcsetattr(0, TCSANOW, &new_settings);
	 nread = read(0,&ch,1);
	 new_settings.c_cc[VMIN]=1;
	 tcsetattr(0, TCSANOW, &new_settings);
	 if(nread == 1){
		 peek_character = ch;
		 return 1;
	 }
	 return 0;
 }

 int readch(){
	 char ch;
	 if(peek_character != -1){
		 ch = peek_character;
		 peek_character = -1;
		 return ch;
		 }
	 read(0,&ch,1);
	 return ch;
 }

 /*
  * End of code from "http://www.openqnx.com/phpbbforum/viewtopic.php?t=6668"
  */

 /*
  * KEYBOARD INPUT THREAD:
  */

 void *IR(void *object){
		init_keyboard();

		while(garageActive){

			if(kbhit()==1){
				char input=getchar();

				switch (input){
						case ('s'):
						case ('S'):
							if(simulation_type==hardware){
								std::cout<<"\n**********************************************************************************"<<std::endl;
								std::cout<<"\n              CHANGING SIMULATION TYPE FROM HARDWARE TO SOFTWARE"<<std::endl;
								std::cout<<"\n**********************************************************************************"<<std::endl;
								simulation_type = software;
								out8(PORT_A, CLEAR);
								usleep(5000);
							}else if(simulation_type==software){
								std::cout<<"\n**********************************************************************************"<<std::endl;
								std::cout<<"\n              CHANGING SIMULATION TYPE FROM SOFTWARE TO HARDWARE"<<std::endl;
								std::cout<<"\n**********************************************************************************"<<std::endl;
								simulation_type = hardware;
								out8(PORT_A, CLEAR+RESET_OFF );
								usleep(5000);
								}

							if(current_state==state_closed_door)std::cout<<"\n=========== DOOR IS CLOSED ==========="<<std::endl;
							else if(current_state==state_opening_door)std::cout<<"\n..........OPENING DOOR.........."<<std::endl;
							else if(current_state==state_opened_door)std::cout<<"\n((((((((((((( DOOR OPEN )))))))))))))"<<std::endl;
							else if(current_state==state_closing_door)std::cout<<"\n..........CLOSING DOOR.........."<<std::endl;
							else if((current_state==state_lowering_door_paused)||(current_state==state_rising_door_paused))
									std::cout<<"\n----------DOOR PAUSED----------"<<std::endl;
							break;
						case ('q'):
						case ('Q'):
							std::cout<<"\nQUIT BUTTON PRESSED!! EXITING!"<<std::endl;
							close_keyboard();
							garageActive = false;
							break;
			        	case ('r'):
			            case ('R'):
			            	if(simulation_type==software){
			            		buttonPressed_e = true;
			            		std::cout<<"Button Pressed!"<<std::endl;
			            	}else{
			            		std::cout<<"Currently in Hardware mode: software sensors not active"<<std::endl;
			            	}
			        		break;
			            case ('i'):
			            case ('I'):
			            	if(simulation_type==software){
			            		if(IRactive){
			            			infraredBeam_e = true;
			            			std::cout<<"Infrared beam detected!"<<std::endl;
			            		}else{
			            			std::cout<<"...Infrared beam not active"<<std::endl;
			            			}
			            	}else{
			            	std::cout<<"Currently in Hardware mode: software sensors not active"<<std::endl;
			            	}
			            	break;
			            case (10):
			            		//Enter Key
			            		break;
			            case ('o'):
			            case ('O'):
			            	if(simulation_type==software){
			            		if(OCactive){
			            			overCurrent_e  = true;
			            			std::cout<<"Overcurrent!! Motor drawing too much current, it may be jammed"<<std::endl;
			            		}else{
			            			std::cout<<"...Overcurrent currently not active"<<std::endl;
			            		}
			            	}else{
			            		std::cout<<"Currently in Hardware mode: software sensors not active"<<std::endl;
			            	}
			            	break;
	                    default:
	                    	std::cout<<"...extra sensors not active"<<std::endl;
	                    	break;
			        }

				}usleep(sleeptime);
			}
		return NULL;
}

 //SOFTWARE SIMULATION THREADS START
 void *tenSecondsCounterUp(void *object){
	while(garageActive){
		if(simulation_type==software){
			while(garageActive&&(countingTen_e == false || opening_door_flag == false));
			std::cout<<"openinng the door will take around "<<openTimer<<" seconds"<<std::endl;
			sleep(openTimer);
			if (countingTen_e == true && opening_door_flag)
				countingTen_e = false;
		}

		usleep(sleeptime);
	}
	return NULL;
}

void *tenSecondsCounterDown(void *object){
	while(garageActive){
		if(simulation_type==software){
			while(garageActive&&(countingTen_e == false || closing_door_flag == false));
			std::cout<<"closing the door will take around "<<closeTimer<<" seconds"<<std::endl;
			sleep(closeTimer);
			if (countingTen_e == true && closing_door_flag)
				countingTen_e = false;
		}
		usleep(sleeptime);
	}
	return NULL;
}
void *countingEffects(void *object){
	while(garageActive){
		while(garageActive&&(current_state == state_closing_door || current_state == state_opening_door)){
			sleep(1);
			if(current_state == state_closing_door || current_state == state_opening_door)
				std::cout<<"                 .                 "<<std::endl;
		}
		usleep(sleeptime);
	}
	return NULL;
}
//SOFTWARE SIMULATION THREADS END

//HARDWARE SIMULATION THREADS START
void *btn(void *object){
	while(garageActive){
		if(simulation_type==hardware){
			std::cout<<"Inside Button thread"<<std::endl;
			while((in8(PORT_B)& remote_push_btn) == false);
			buttonPressed_e = true;
			std::cout<<"Button Pressed!"<<std::endl;
			while(in8(PORT_B)& remote_push_btn);
		}
		usleep(sleeptime);
	}
	return NULL;
}
void *ir(void *object){
	while(garageActive){
		if(simulation_type==hardware){
			while((in8(PORT_B)& IR_beam) == false);
					if(IRactive){
						infraredBeam_e = true;
						std::cout<<"Infrared beam detected!"<<std::endl;
					}else{
						std::cout<<"...Infrared beam not active"<<std::endl;
					}
					while(in8(PORT_B)& IR_beam );
		}
		usleep(sleeptime);
	}
	return NULL;
}

void *oc(void *object){
	while(garageActive){
		if(simulation_type==hardware){
			while((in8(PORT_B)& overcurrent) == false);
					if(OCactive){
						overCurrent_e  = true;
						std::cout<<"Overcurrent!! Motor drawing too much current, it may be jammed"<<std::endl;
					}else{
						std::cout<<"...Overcurrent currently not active"<<std::endl;
					}
					while(in8(PORT_B)& overcurrent );
		}
		usleep(sleeptime);
	}
	return NULL;
}

void *fo(void *object){
	while(garageActive){
		if(simulation_type==hardware){
			while((in8(PORT_B)& full_open) == false);
			out8( PORT_A, CLEAR+RESET_OFF );
			full_open_e = true;
			std::cout<<"Full Open"<<std::endl;
			while(in8(PORT_B)& full_open );
		}usleep(sleeptime);
	}
	return NULL;
}
void *fc(void *object){
	while(garageActive){
		if(simulation_type==hardware){
			while((in8(PORT_B)& full_close) == false);
			out8( PORT_A, CLEAR+RESET_OFF );
			full_close_e = true;
			std::cout<<"Full Close"<<std::endl;
			while(in8(PORT_B)& full_close);
		}
		usleep(sleeptime);
	}
	return NULL;
}
//HARDWARE SIMULATION THREADS END

	//creating motor so it can be accessed by all methods
	motorC *motor = new motorC();

	int main(int argc, char *argv[]) {

		/*
		 * SETTING UP HARDWARE SIMULATION
		 * */
		//--------INITIALIZING QNX
		int privity_err;

		/*root access */
		privity_err = ThreadCtl( _NTO_TCTL_IO, NULL );
		if ( privity_err == -1 ){
			printf( "Can't get root permissions\n" );
			return -1;
		}

		/*Handles for device registers*/
		dir_ctrl = mmap_device_io(PORT_LENGTH, DIR_CTRL);
		PORT_A = mmap_device_io( PORT_LENGTH, PORTA );
		PORT_B = mmap_device_io( PORT_LENGTH, PORTB );

		/*setting PORTA as output and PORTB as input*/
		out8(dir_ctrl, 0x01);

		int btn_thread;
		int ir_thread;
		int oc_thread;
		int full_open_thread;
		int full_close_thread;

		btn_thread = ThreadCreate(0, &btn, NULL, NULL);
		ir_thread = ThreadCreate(0, &ir, NULL, NULL);
		oc_thread = ThreadCreate(0, &oc, NULL, NULL);
		full_open_thread = ThreadCreate(0, &fo, NULL, NULL);
		full_close_thread = ThreadCreate(0, &fc, NULL, NULL);

		/*
		 * SETTING UP SOFTWARE SIMULATION
		 * */
		int IR_thread;
		int tenSecondsCounterUp_thread;
		int tenSecondsCounterDown_thread;
		int countingEffects_thread;

		IR_thread = ThreadCreate(0, &IR, NULL, NULL);
		tenSecondsCounterUp_thread = ThreadCreate(0, &tenSecondsCounterUp, NULL, NULL);
		tenSecondsCounterDown_thread = ThreadCreate(0, &tenSecondsCounterDown, NULL, NULL);
		countingEffects_thread = ThreadCreate(0, &countingEffects, NULL, NULL);


		//HARDWARE RESET
		//out8(PORT_A, RESET_OFF);
		out8(PORT_A, 0xff);
		sleep(1);
		out8(PORT_A, CLEAR);
		sleep(1);
		out8(PORT_A, RESET_OFF);

		while(garageActive){
		        switch (current_state){
		        	case(state_closed_door):
		        			std::cout<<"\n=========== DOOR IS CLOSED ==========="<<std::endl;
							current_state = closedDoor();
		        			break;
		        	case(state_opening_door):
							std::cout<<"\n..........OPENING DOOR.........."<<std::endl;
							current_state = openingDoor();
							break;
		        	case(state_opened_door):
							std::cout<<"\n((((((((((((( DOOR OPEN )))))))))))))"<<std::endl;
							current_state = openedDoor();
		        		    break;
		        	case(state_closing_door):
							std::cout<<"\n..........CLOSING DOOR.........."<<std::endl;
		        			current_state = closingDoor();
		        		    break;
		        	case(state_lowering_door_paused):
							std::cout<<"\n----------DOOR PAUSED----------"<<std::endl;
							current_state = loweringDoorPaused();
		        		    break;
		        	case(state_rising_door_paused):
							std::cout<<"\n----------DOOR PAUSED----------"<<std::endl;
							current_state = risingDoorPaused();
							break;
		        	default:
		        			std::cout<<"ERROR!! current state not determined\n...POWERING OFF!"<<std::endl;
		        			garageActive = false;
		        			break;
		        }
		}
		delete motor;
		return EXIT_SUCCESS;
	}

	int closedDoor(){
		int next_state = 10;

		motor->pause();
		while(buttonPressed_e==false && garageActive);
		full_close_e = false;
		buttonPressed_e = false;

		next_state = state_opening_door;
		return next_state;
	}
	int openingDoor(){
		int next_state = 10;

		opening_door_flag = true;
		countingTen_e = true;
		clock_gettime(CLOCK_REALTIME,&start);

		motor->turningRight();
		while(overCurrent_e==false&&countingTen_e&&buttonPressed_e == false && full_open_e==false && garageActive);

		//SETUP FOR SOFTWARE NEXT STATE
		clock_gettime(CLOCK_REALTIME,&end);
		timeElapsed = (end.tv_sec - start.tv_sec);
		opening_door_flag = false;
		countingTen_e = false;

		//HARDWARE AND SOFTWARE COMMANDS
		if (buttonPressed_e){
			buttonPressed_e =false;
			next_state = state_rising_door_paused;
			closeTimer = timeElapsed>10?10:(10-(openTimer-timeElapsed));
		}
		else if(overCurrent_e){
			overCurrent_e = false;
			next_state = state_rising_door_paused;
			closeTimer = timeElapsed>10?10:(10-(openTimer-timeElapsed));
		}else{
			next_state = state_opened_door;
			closeTimer = 10;
		}

		return next_state;
		}

	int openedDoor(){
		int next_state = 10;
		closeTimer = 10;					//Time it would take to close the door in seconds

		motor->pause();
		while(buttonPressed_e==false&& garageActive);
		full_open_e = false;
		buttonPressed_e = false;
		next_state = state_closing_door;

		return next_state;
		}

	int closingDoor(){
		int next_state = 10;

		closing_door_flag = true;
		IRactive = true;
		std::cout << "***IR beam turned on***"<< std::endl;

		if (simulation_type==hardware)
			out8( PORT_A, IR_beam+ RESET_OFF);

		countingTen_e = true;					//software
		clock_gettime(CLOCK_REALTIME,&start);	//software

		motor->turningLeft();
		while(buttonPressed_e == false && countingTen_e &&full_close_e==false && infraredBeam_e == false && overCurrent_e == false  && garageActive);

		clock_gettime(CLOCK_REALTIME,&end);
		timeElapsed = (end.tv_sec - start.tv_sec);

		if (buttonPressed_e){
			next_state = state_lowering_door_paused;
			buttonPressed_e =false;
			openTimer = timeElapsed>10?10:(10-(closeTimer-timeElapsed));
			}
		else if(infraredBeam_e){
			next_state = state_opening_door;
			infraredBeam_e=false;
			openTimer = timeElapsed>10?10:(10-(closeTimer-timeElapsed));
		}else if(overCurrent_e){
			next_state = state_opening_door;
			overCurrent_e=false;
			openTimer = timeElapsed>10?10:(10-(closeTimer-timeElapsed));
		}else{
			next_state = state_closed_door;
			openTimer = 10;
		}
		closing_door_flag = false;
		countingTen_e = false;
		IRactive = false;
		infraredBeam_e = false;
		std::cout << "***IR beam turned off***"<< std::endl;

		return next_state;
		}
	int loweringDoorPaused(){
		int next_state=10;

		std::cout << "Door paused after "<< timeElapsed << " seconds closing"<< std::endl;
		motor->pause();
		while(buttonPressed_e==false&& garageActive);
		buttonPressed_e = false;
		next_state = state_opening_door;
		return next_state;
	}
	int risingDoorPaused(){
		int next_state = 10;

		std::cout << "Door paused after "<< timeElapsed << " seconds opening"<< std::endl;

		motor->pause();
		while(buttonPressed_e==false && garageActive);
		buttonPressed_e = false;
		next_state = state_closing_door;
		return next_state;
	}


