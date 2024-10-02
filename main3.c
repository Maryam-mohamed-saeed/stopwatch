/*
 * main2.c
 *
 *  Created on: Sep 13, 2024
 *      Author: hp
 */
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
unsigned char sec_unit=0;
unsigned char sec_tenth=0;
unsigned char min_unit=0;
unsigned char min_tenth=0;
unsigned char hour_unit=0;
unsigned char hour_tenth=0;
unsigned char pause_mode=0;
unsigned char dec_mode=0;
unsigned char timer_tick=0;





void DIO_INIT(void){
	DDRC|=0X0F;
	DDRA|=0X3F;//00111111

	/*7->COUNT DOWN MODE
	 * 6->sec increment
	 * 5->sec decrement
	 * 4->min increment
	 * 3->min decrement
	 * 2->RESUME BUTTON
	 * 1->DECREMANT HOURS
	 * 0->INCREMANT HOURS
	 */
	DDRB=DDRB&~(1<<7)&~(1<<6)&~(1<<5)&~(1<<4)&~(1<<3)&~(1<<2)&~(1<<1)&~(1<<0);
	PORTB|=(1<<7)|(1<<6)|(1<<5)|(1<<4)|(1<<3)|(1<<2)|(1<<1)|(1<<0);//ENABLE INTERNAL PULL UP
	DDRD&=~(1<<0);//use for buzzer
	DDRD&=~(1<<2);//USE IT AS INT0 FOR RESET
	DDRD&=~(1<<3) ;//USE IT AS INT1 FOR PAUSE
	PORTD|=0X0D;//USE INTERNAL PULL UP FOR D0 D2 D3
	DDRD|=(1<<0)|(1<<4)|(1<<5);//PD4 for count_up led PD5 for count_down led PD0 for buzzer
	//default mode is increment
	//turn on the increment led
	PORTD|=(1<<4);
	//turn off the decrement led
	PORTD&=~(1<<5);
	//turn off the buzzer
	PORTD&=~(1<<0);

}

void INT0_INIT(void){
	MCUCR|=(1<<ISC01);// The falling edge of INT0  interrupt request
	GICR|=(1<< INT0);//External Interrupt Request 0 Enable
}

void INT1_INIT(void){
	//pause mode
	MCUCR|=(1<<ISC11)|(1<< ISC10);// The raising edge of INT1  interrupt request
	GICR|=(1<< INT1);//External Interrupt Request 1 Enable
}

void INT2_INIT(void){
	MCUCSR&=~(1<<ISC2);// falling edge  INT2 activates interrupt
	GICR|=(1<<INT2);
}


void TIMER1_INIT(void){
	/*Timer/Counter1 Control Register A
	 * The FOC1A/FOC1B bits are  active when a non-PWM

	 */

	TCCR1A=(1<<FOC1B)|(1<<FOC1A);

	/*Timer/Counter1 Control Register B
	 * CS12 clkI/O/1024 (From prescaler)
	 *  * WGM12 CTC TOP in OCR1A
	 */
	TCCR1B|=(1<<CS12)|(1<<CS10)|(1<<WGM12);

	/*FCPU = 16MHZ
	 * FTIMER = 16MHZ/1024
	 * Ttimer =64 MICRO
	 * */
	OCR1A=15625;

	/*Timer/Counter Interrupt Mask Register
	 * OCIE1A: Output Compare A Match Interrupt Enable
	 * I: Global Interrupt Enable
	 */
	TIMSK|=(1<<OCIE1A);
	SREG|=(1<<7);
}




void show_segments(void){
	//show_sec_unit
	PORTA|=(1<<5);
	PORTC=(PORTC&0XF0)|(sec_unit&0X0F);
	_delay_ms(2);
	PORTA&=~(1<<5);
	//show_sec_tenth
	PORTA|=(1<<4);
	PORTC=(PORTC&0XF0)|(sec_tenth&0X0F);
	_delay_ms(2);
	PORTA&=~(1<<4);
	//show_min_unit
	PORTA|=(1<<3);
	PORTC=(PORTC&0XF0)|(min_unit&0X0F);
	_delay_ms(2);
	PORTA&=~(1<<3);
	//show_min_tenth

	PORTA|=(1<<2);
	PORTC=(PORTC&0XF0)|(min_tenth&0X0F);
	_delay_ms(2);
	PORTA&=~(1<<2);
	//show_hour_unit
	PORTA|=(1<<1);
	PORTC=(PORTC&0XF0)|(hour_unit&0X0F);
	_delay_ms(2);
	PORTA&=~(1<<1);
	//show_hour_tenth
	PORTA|=(1<<0);
	PORTC=(PORTC&0XF0)|(hour_tenth&0X0F);
	_delay_ms(2);
	PORTA&=~(1<<0);
}



void check_change(void){
	if(!(PINB & (1<<1))){  // Button for incrementing hour
		_delay_ms(20);  // Debouncing delay
		if(!(PINB & (1<<1))){  // Confirm button press
			hour_unit++;
			if(hour_unit>9&&(hour_tenth<2)){

				hour_unit=0;
				hour_tenth++;

			}

			if((hour_unit>3)&&(hour_tenth==2)){

				hour_unit=0;
				hour_tenth=0;
			}
		}
		while(!(PINB & (1<<1))){show_segments();}

	}

	if(!(PINB & (1<<0))){  // Button for decrementing hour
		_delay_ms(20);  // Debouncing delay
		if(!(PINB & (1<<0))){  // Confirm button press

			if(hour_unit == 0){
				if(hour_tenth==2){
					hour_tenth=1;
					hour_unit=9;
				}
				else if(hour_tenth==1){
					hour_tenth=0;
					hour_unit=9;
				}
				else{
					hour_tenth=2;
					hour_unit=3;
				}
			}
			else{
				hour_unit--;
			}
		}
		while(!(PINB & (1<<0))){show_segments();}
	}

	if(!(PINB & (1<<4))){  // Button for incrementing min
		// Debouncing delay
		if(!(PINB & (1<<4))){  // Confirm button press
			min_unit++;
			if(min_unit>9){
				sec_unit=0;
				sec_tenth=0;
				min_unit=0;
				min_tenth++;

			}

			if(min_tenth>5){
				sec_unit=0;
				sec_tenth=0;
				min_unit=0;
				min_tenth=0;
				hour_unit++;

			}

			if(hour_unit>9&&(hour_tenth<2)){
				sec_unit=0;
				sec_tenth=0;
				min_unit=0;
				min_tenth=0;
				hour_unit=0;
				hour_tenth++;

			}

			if((hour_unit>3)&&(hour_tenth==2)){
				sec_unit=0;
				sec_tenth=0;
				min_unit=0;
				min_tenth=0;
				hour_unit=1;
				hour_tenth=0;
			}
		}
		while(!(PINB & (1<<4))){show_segments();}
	}


	if(!(PINB & (1<<3))){  // Button for decrementing min
		_delay_ms(20);  // Debouncing delay
		if(!(PINB & (1<<3))){  // Confirm button press

			if (min_unit == 0) {
				min_unit = 9;
				if(min_tenth == 0){
					min_tenth=5;
				}
				else{
					min_tenth--;
				}
			}else{
				min_unit--;
			}
		}
		while(!(PINB & (1<<3))){show_segments();}
	}

	if(!(PINB & (1<<6))){  // Button for incrementing sec
		_delay_ms(20);  // Debouncing delay
		if(!(PINB & (1<<6))){  // Confirm button press
			sec_unit++;
			dec_mode=1;
			if(sec_unit>9){
				sec_unit=0;
				sec_tenth++;

			}
			if(sec_tenth>5){
				sec_unit=0;
				sec_tenth=0;
				min_unit++;

			}
			if(min_unit>9){
				sec_unit=0;
				sec_tenth=0;
				min_unit=0;
				min_tenth++;

			}
			if(min_tenth>5){
				sec_unit=0;
				sec_tenth=0;
				min_unit=0;
				min_tenth=0;
				hour_unit++;

			}

			if(hour_unit>9&&(hour_tenth<2)){
				sec_unit=0;
				sec_tenth=0;
				min_unit=0;
				min_tenth=0;
				hour_unit=0;
				hour_tenth++;

			}
			if((hour_unit>3)&&(hour_tenth==2)){
				sec_unit=0;
				sec_tenth=0;
				min_unit=0;
				min_tenth=0;
				hour_unit=1;
				hour_tenth=0;
			}
		}
		while(!(PINB & (1<<6))){show_segments();}
	}


	if(!(PINB & (1<<5))){  // Button for decrementing sec
		_delay_ms(20);  // Debouncing delay
		if(!(PINB & (1<<5))){  // Confirm button press
			/*check if we reached the zero time*/
			if(sec_unit==0){
				sec_unit=9;
				if(sec_tenth==0){
					sec_tenth=5;

				}
				else{sec_tenth--;}
			}
			else{
				sec_unit--;
			}
		}
		while(!(PINB & (1<<5))){show_segments();}
	}

}



void timer_calc(void){
	if ((dec_mode==1)){
		if(sec_unit!=0){
			sec_unit--;
			/*check if we reached the zero time*/
			if((hour_unit==0)&&(hour_tenth==0)&&(min_tenth==0)&&(min_tenth==0)&&(min_unit==0)&&(sec_tenth==0)&&(sec_unit==0)){
				//set buzzer
				PORTD|=(1<<0);
			}}
		else if(sec_tenth!=0){
			//sec_unit=0 and we didnt reach the end time
			sec_unit=9;
			sec_tenth--;
		}
		else if(min_unit!=0){
			/*sec_unit=0 and sec_tenth=0
			 * and we didnt reach the end time*/
			sec_unit=9;
			sec_tenth=5;
			min_unit--;
		}
		else if(min_tenth!=0){
			/*sec_unit=0 and sec_tenth=0 and min_unit=0
			 * and we didnt reach the end time*/
			sec_unit=9;
			sec_tenth=5;
			min_unit=9;
			min_tenth--;
		}
		else if(hour_unit!=0){
			/*sec_unit=0 and sec_tenth=0 and min_unit=0
			 *  and min_tenth=0  we didnt reach the end time*/
			sec_unit=9;
			sec_tenth=5;
			min_unit=9;
			min_tenth=5;
			hour_unit--;
		}
		else if(hour_tenth!=0){
			/*sec_unit=0 and sec_tenth=0
			 *and min_unit=0  and min_tenth=0
			 *and hour_unit=0
			 *we didnt reach the end time */
			sec_unit=9;
			sec_tenth=5;
			min_unit=9;
			min_tenth=5;
			hour_unit=9;
			hour_tenth--;

		}

	}




	/***normal mode**/
	else if(dec_mode!=1){
		//stop the buzzer
		/*This case handle if we reached zero
		 *  during counting down and then switch the mode*/
		PORTD&=~(1<<0);
		//INCREMENT THE TIME BY ONE SECOND
		sec_unit++;
		if(sec_unit>9){
			sec_unit=0;
			sec_tenth++;

		}
		if(sec_tenth>5){
			sec_unit=0;
			sec_tenth=0;
			min_unit++;

		}
		if(min_unit>9){
			sec_unit=0;
			sec_tenth=0;
			min_unit=0;
			min_tenth++;

		}
		if(min_tenth>5){
			sec_unit=0;
			sec_tenth=0;
			min_unit=0;
			min_tenth=0;
			hour_unit++;

		}

		if(hour_unit>9&&(hour_tenth<2)){
			sec_unit=0;
			sec_tenth=0;
			min_unit=0;
			min_tenth=0;
			hour_unit=0;
			hour_tenth++;

		}
		if((hour_unit>3)&&(hour_tenth==2)){
			sec_unit=0;
			sec_tenth=0;
			min_unit=0;
			min_tenth=0;
			hour_unit=1;
			hour_tenth=0;
		}}

	//reset the flag of ISR of the timer
	timer_tick=0;
}



ISR(TIMER1_COMPA_vect){
	/*to make the code of ISR
	 * as simple as possible just set the flag*/
	timer_tick=1;

}


ISR(INT0_vect){
	//reset function
	sec_unit=0;
	sec_tenth=0;
	min_unit=0;
	min_tenth=0;
	hour_unit=0;
	hour_tenth=0;
	dec_mode=0;
	//count up which is the default
	PORTD&=~(1<<0);
	//stop buzzer
	PORTD&=~(1<<0);
}


ISR(INT1_vect){
	//pause mode
	pause_mode=1;
	//stop clock
	TCCR1B&=~(1<< CS10) &~(1<<CS11) &~(1<<CS12);
}



ISR(INT2_vect){
	//resume mode
	pause_mode=0;

	/*
	 * CS12 CS10 clkI/O/1024 (From prescaler)
	 */
	TCCR1B|=(1<<CS12)|(1<<CS10);

}



int main(){
	DIO_INIT();
	TIMER1_INIT();
	INT0_INIT();
	INT1_INIT();
	INT2_INIT();

	while(1){

		show_segments();
		if(timer_tick==1){
			//the ISR SET THE FLAG
			timer_calc();
		}


		if (pause_mode){
			// check if we adjust the time
			check_change();
		}

		if(!(PINB&(1<<7))){
			//check changing in mode
			_delay_ms(20);//for debouncing
			if(!(PINB&(1<<7))){

				//toggle the mode if true
				dec_mode=(dec_mode&0x01)^(0x01);
				if(dec_mode){
					//turn on the decrement led
					PORTD|=(1<<5);
					//turn off the increment led
					PORTD&=~(1<<4);
				}
				else{
					//turn on the increment led
					PORTD|=(1<<4);
					//turn off the decrement led
					PORTD&=~(1<<5);
				}

			}
			while(!(PINB&(1<<7))){show_segments();}
		}
	}
}






