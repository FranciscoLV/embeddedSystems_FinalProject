#include <avr/io.h>
#include "io.c"
#include "timer.h"

#define A2 ((~PINA) & 0x04)
#define A3 ((~PINA) & 0x08)
#define ballThrown PORTB	//BALL THAT ITS BEING THROWN
#define score PORTC			//score
#define usage PORTD			//COMPLEXITY BEING USED

enum touchPad_States{startTou, offTou, onTou} touchPad_States;
enum motionStates{startMot, offMot, waitMot, onMot} motionStates;
enum GameStates{start, motion, waitTouch,  touch, waitReset}GameStates;
enum joyStickStates{startJoy, offJoy, onJoy}joyStickStates;

unsigned short x,y; 	
unsigned char balls = 0;

//Functionality - Sets bit on a PORTx
//Parameter: Takes in a uChar for a PORTx, the pin number and the binary value
//Returns: The new value of the PORTx
unsigned char SetBit(unsigned char pin, unsigned char number, unsigned char bin_value){
	return (bin_value ? pin | (0x01 << number) : pin & ~(0x01 << number));
}

void InitADC() {
	ADMUX=(1<<REFS0);                   //setting the reference of ADC such THAT Areff=Avcc
	ADCSRA=(1<<ADEN)|(1<<ADPS2)|(1<<ADPS1)|(1<<ADPS0);
}
//used something new datatype her to check if it works or not, unsigned integer short can aolso be written as unint16_t

uint16_t ReadADC(uint8_t ch)
{	InitADC();
	ch=ch&0b00000111; //im using this channel to be a extra safety net for the correct reading
	ADMUX|=ch; // it will read from the first three channels
	//conversion steps
	ADCSRA|=(1<<ADSC);
	while(!(ADCSRA & (1<<ADIF)));
	ADCSRA|=(1<<ADIF); // got from one of the labs
	return(ADC);
}

//code for reading which direction is the joystick pointing to0
uint16_t joyStickFlicked(uint16_t x,uint16_t y)
{
	//1-> up, 2-> right, 3-> down 4-> left
	if(x>255 && x<765 && y>615) // we know 1024 is the max value, these values are exact half of the max values which i had using the joystick
	return(1); // direction is up

	else if(x>615 && y>255 && y<765)
	return(2); //direction is right

	else if(x>255 && x<765 && y<255)
	return(3); //direction is down

	else if(x<405 && y>255 && y<765)
	return(4); // direction is left

	else
	return(0); //default state
}

//Functionality - Gets bit from a PINx
//Parameter: Takes in a uChar for a PINx and the pin number
//Returns: The value of the PINx
unsigned char GetBit(unsigned char port, unsigned char number){
	return ( port & (0x01 << number) );
}

unsigned long int findGCD(unsigned long int a, unsigned long int b){
	unsigned long int c;
	while(1){
		c = a%b;
		if(c==0){return b;}
		a = b;
		b = c;
	}
	return 0;
}

typedef struct _task {
    int state;
    unsigned long period;
    unsigned long elapsedTime;
    int(*TickFct)(int);
} task;

int GameLogic(int GameStates){
	//enum GameStates{start, motion, waitTouch,  touch};
	static unsigned char timeCounter = 0;
	static unsigned char balls = 0;;
	
	switch(GameStates){
		case start:
			GameStates = motion;
			break;
		case motion:
			x = ReadADC(0);
			y = ReadADC(1);
			if(!(A3) && joyStickFlicked(x,y) == 0 && balls < 5){
				GameStates = motion;
			}
			else if((A3) && joyStickFlicked(x,y) == 0 && balls < 5){
				balls++;
				if(ballThrown == 0)
					ballThrown = 1;
				else if(ballThrown == 1)
					ballThrown = 3;
				else if(ballThrown == 3)
					ballThrown = 7;
				else if(ballThrown == 7)
					ballThrown = 15;
				else if(ballThrown == 15)
					ballThrown = 31;
				else if(ballThrown ==  31)
					ballThrown = 31;				
				GameStates = waitTouch;
			}
			else if(balls >= 5){
				GameStates = waitReset;	
			}
			else if(joyStickFlicked(x,y) != 0){
				GameStates = start;
			}
			break;
			
		case waitTouch:
			timeCounter++;
			x = ReadADC(0);
			y = ReadADC(1);
			if(timeCounter <= 8 && !(A2) && joyStickFlicked(x,y) == 0){
				GameStates = waitTouch;
			}
			else if(timeCounter <= 8 && (A2) && joyStickFlicked(x,y) == 0){
				timeCounter = 0;
				GameStates = touch;
			}
			else if(timeCounter <= 8 && !(A2) && joyStickFlicked(x,y) != 0){
				GameStates = start;
			}
			else if(timeCounter > 8){
				timeCounter = 0;
				GameStates = motion;
			}
			break;
		
		case touch:
			GameStates = motion;
			break;
			
		case waitReset:
			x = ReadADC(0);
			y = ReadADC(1);
			if(joyStickFlicked(x,y) == 0){
				GameStates = waitReset;
			}
			else if(joyStickFlicked(x,y) != 0){
				GameStates = start;
			}
			break;
		
		default:
			GameStates = start;
			break;
	}
	
	switch(GameStates){
		case start:
			ballThrown = 0;
			score = 0;
			timeCounter = 0;
			balls = 0;
			break;
		
		case motion:
			break;
		
		case waitTouch:
			break;
		
		case touch:
			if(score == 0)
				score = 1;
			else if(score == 1)
				score = 3;
			else if(score == 3)
				score = 7;
			else if(score == 7)
				score = 15;
			else if(score == 15)
				score = 31;
			else if(score ==  31)
				score = 31;
			break;
			
		case waitReset:
			break;
			
		default:
			score = 0;
			ballThrown = 0;
	}
	
	return GameStates;
	
}

int joyStick(int joyStickStates){
	switch(joyStickStates){
		case startJoy:
			joyStickStates = offJoy;
			break;
	
		case offJoy:
			x = ReadADC(0);
			y = ReadADC(1);
			if(joyStickFlicked(x,y) == 0){
				joyStickStates = offJoy;
			}
			else if(joyStickFlicked(x,y) != 0){
				joyStickStates = onJoy;
			}
			break;
	
		case onJoy:
			x = ReadADC(0);
			y = ReadADC(1);
			if(joyStickFlicked(x,y) != 0){
				joyStickStates = onJoy;
			}
			else if(joyStickFlicked(x,y) == 0){
				joyStickStates = offJoy;
			}
			break;
	
		default:
			joyStickStates = startJoy;
			break;
	}

	switch(joyStickStates){
		case startJoy:
			break;
	
		case offJoy:
			usage = 0;
			break;
		
		case onJoy:
			usage = 4;
			break;
		
		default:
			break;
	}
	return joyStickStates;
}

int motionSensor(int motionStates){
	switch(motionStates){
		case startMot:
			motionStates = offMot;
			break;
		
		case offMot:
			if(!(A3)){
				motionStates = offMot;
			}
			else if(A3){
				motionStates = onMot;
			}
			break;
			
		case onMot:
			if(A3){
				motionStates = onMot;
			}
			else if(!(A3)){
				motionStates = offMot;
			}
			break;
		
		default:
			motionStates = startMot;
			break;
	}
	
	switch(motionStates){
		case startMot:
			break;
		
		case offMot:
			usage = 0;
			break;
		
		case onMot:
			usage = 2;
			break;
		
		default:
			break;
	}
	return motionStates;
}

int padTouch(int touchPad_States){
	switch(touchPad_States){
		case startTou:
			touchPad_States = offTou;
			break;
		
		case offTou:
			if(!(A2)){
				touchPad_States = offTou;
			}
			else if(A2){
				touchPad_States = onTou;
			}
			break;
			
		case onTou:
			if(A2){
				touchPad_States = onTou;
			}
			else if(!(A2)){
				touchPad_States = offTou;
			}
			break;
		
		default:
			touchPad_States = startTou;
			break;
	}
	
	switch(touchPad_States){
		case startTou:
			break;
			
		case offTou:
			usage = 0;
			break;
		
		case onTou:
			usage = 1;
			break;
		
		default:
			break;
	}
	return touchPad_States;
}

int main(void)
{
	DDRA = 0x00; PORTA = 0xFF; // input
	DDRB = 0xFF; PORTB = 0x00; // output
	DDRC = 0xFF; PORTC = 0x00; //output
	DDRD = 0xFF; PORTD = 0x00; //output
	
	unsigned long int touchPeriod = 100;
	unsigned long int motionPeriod = 100;
	unsigned long int gamePeriod = 300;
	unsigned long int joyStickPeriod = 100;
	
	unsigned long int tmpGCD = 1;
	tmpGCD = findGCD(gamePeriod, joyStickPeriod);;
	tmpGCD = findGCD(tmpGCD, touchPeriod);
	tmpGCD = findGCD(tmpGCD, motionPeriod);
	
	unsigned long int GCD = tmpGCD;
	
	touchPeriod /= GCD;
	motionPeriod /= GCD;
	gamePeriod /= GCD;
	joyStickPeriod /= GCD;

	static task TouchTask, MotionTask, joyStickTask, GameTask;
	task *tasks[] = {&TouchTask, &MotionTask, &joyStickTask, &GameTask};
	const unsigned short NumTasks = sizeof(tasks) / sizeof(task*);
	
	TouchTask.state = startTou;
	TouchTask.period = touchPeriod;
	TouchTask.elapsedTime = touchPeriod;
	TouchTask.TickFct = &padTouch;
	//i++;
	
	MotionTask.state = startMot;
	MotionTask.period = motionPeriod;
	MotionTask.elapsedTime = motionPeriod;
	MotionTask.TickFct = &motionSensor;
	//i++;
	
	GameTask.state = start;
	GameTask.period = gamePeriod;
	GameTask.elapsedTime = gamePeriod;
	GameTask.TickFct = &GameLogic;
	//i++;
	
	joyStickTask.state = startJoy;
	joyStickTask.period = joyStickPeriod;
	joyStickTask.elapsedTime = joyStickPeriod;
	joyStickTask.TickFct = &joyStick;
	//i++;

	TimerSet(GCD);
	TimerOn();
	InitADC();
	while(1) {
		for(unsigned short j = 0; j < NumTasks; j++)
		{
			if(tasks[j]->elapsedTime == tasks[j]->period)
			{
				tasks[j]->state = tasks[j]->TickFct(tasks[j]->state);
				tasks[j]->elapsedTime = 0;
			}
			tasks[j]->elapsedTime += 1;
		}
		while(!TimerFlag);
		TimerFlag = 0;
	}
	return 0;
}