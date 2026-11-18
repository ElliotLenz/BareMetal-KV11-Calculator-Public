#include "MKV11Z7.h"
#include "lcd_hd44780.h"
#include "keypad_4x4.h"
#include "calc.h"

int main(void){
		//Turn on clock for PORTC, PORTA, and PORTD
		SIM->SCGC5 |= SIM_SCGC5_PORTC_MASK | SIM_SCGC5_PORTA_MASK | SIM_SCGC5_PORTD_MASK;
		
		//Initialize Pins
		Keypad_Init();
		LCD_Init();
		Calc_Init();
		
		char last_key = '\0';
		
		while(1){
					char current_key = Keypad_Scan();
			
					//If a key is pressed AND it is a new key pressed		
					if (current_key != '\0' && current_key != last_key){
						
							CalcFSM(current_key);
	
							//Debounce delay to prevent physical button bouncing
							for(volatile int i = 0; i < 50000; i++);
					}
					last_key = current_key;
		}

}