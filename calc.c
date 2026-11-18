#include "calc.h"
#include "lcd_hd44780.h"
#include <stdio.h>

//=================================================================
// Calculator Global Variables 
//=================================================================
static float num1 = 0.0;
static float num2 = 0.0;
static int digits1 = 0;
static int digits2 = 0;

static int has_dec1 = 0;
static float dec_mult1 = 1.0;
static int is_neg1 = 0;

static int has_dec2 = 0;
static float dec_mult2 = 1.0;
static int is_neg2 = 0;

static char math_operator = '\0';
static float result = 0.0;
static char display_buffer[16];
static int shift_active = 0;

// Resets all variables and clears the LCD
void Calc_Init(void) {
    LCD_Clear();
    num1 = 0.0;
    num2 = 0.0;
    digits1 = 0;
    digits2 = 0;
    
    has_dec1 = 0;
    dec_mult1 = 1.0;
    is_neg1 = 0;
    
    has_dec2 = 0;
    dec_mult2 = 1.0;
    is_neg2 = 0;
    
    math_operator = '\0';
    result = 0.0;
    shift_active = 0;
}

// CalcFSM receives one variable called Val
void CalcFSM(char Val) {
    static int State = 1; 
    
    //===== Shift Key Layer (2nd Function) =====
    if (shift_active == 1) {
        shift_active = 0; 
        if (Val == 'D') Val = '/';
        else if (Val == '0') Val = '.';
        else if (Val == '*') Val = 'n'; 
        else return; 
    } 
    else if (Val == 'D') {
        shift_active = 1;
        return; 
    }

    //===== Clear =====
    if (Val == 'C') {
        Calc_Init();
        State = 1;
        return;
    }
    
    //===== Auto-Reset on new number after equals =====
    if (State == 4 && ((Val >= '0' && Val <= '9') || Val == '.' || Val == 'n')) {
        Calc_Init();
        State = 1;
    }

    // Switch-case statement passing State as parameter
    switch (State) {
        
        //=========================================
        // STATE 1: Setting the First Value
        //=========================================
        case 1:
            if (Val >= '0' && Val <= '9') {
                if (digits1 < MAX_DIGITS) {
                    if (has_dec1 == 0) {
                        num1 = (num1 * 10.0) + (Val - '0');
                    } else {
                        dec_mult1 = dec_mult1 / 10.0;
                        num1 = num1 + ((Val - '0') * dec_mult1);
                    }
                    digits1++;
                    LCD_WriteChar(Val);
                }
            }
            else if (Val == '.') {
                if (has_dec1 == 0) {
                    has_dec1 = 1;
                    LCD_WriteChar('.');
                }
            }
            else if (Val == 'n') {
                if (digits1 == 0 && is_neg1 == 0) {
                    is_neg1 = 1;
                    LCD_WriteChar('-');
                }
            }
            // Transition to State 2 if operator pressed
            else if ((Val == 'A' || Val == 'B' || Val == '/' || Val == '*') && digits1 > 0) {
                if (Val == 'A') math_operator = '+';
                if (Val == 'B') math_operator = '-';
                if (Val == '/') math_operator = '/';
                if (Val == '*') math_operator = '*';
                
                LCD_WriteChar(math_operator);
                State = 2; // Transition to operation state
            }
            break;

        //=========================================
        // STATE 2: Setting the Operation
        //=========================================
        case 2:
            // If they press a number/decimal/negative, start num2 on the second line
            if ((Val >= '0' && Val <= '9') || Val == '.' || Val == 'n') {
                
                // Move cursor to the second line
                LCD_SetCursor(1, 0); 
                State = 3; 
                
                // Process that first keypress for num2
                if (Val == '.') {
                    has_dec2 = 1;
                    LCD_WriteChar('.');
                } else if (Val == 'n') {
                    is_neg2 = 1;
                    LCD_WriteChar('-');
                } else {
                    num2 = (num2 * 10.0) + (Val - '0');
                    digits2++;
                    LCD_WriteChar(Val);
                }
            }
            // Allow changing the operator if they made a mistake
            else if (Val == 'A' || Val == 'B' || Val == '/' || Val == '*') {
                if (Val == 'A') math_operator = '+';
                if (Val == 'B') math_operator = '-';
                if (Val == '/') math_operator = '/';
                if (Val == '*') math_operator = '*';
                
                // Overwrite the old operator on the screen
                LCD_SetCursor(0, digits1 + (is_neg1 ? 1 : 0) + (has_dec1 ? 1 : 0));
                LCD_WriteChar(math_operator);
            }
            break;

        //=========================================
        // STATE 3: Setting the Second Value
        //=========================================
        case 3:
            if (Val >= '0' && Val <= '9') {
                if (digits2 < MAX_DIGITS) {
                    if (has_dec2 == 0) {
                        num2 = (num2 * 10.0) + (Val - '0');
                    } else {
                        dec_mult2 = dec_mult2 / 10.0;
                        num2 = num2 + ((Val - '0') * dec_mult2);
                    }
                    digits2++;
                    LCD_WriteChar(Val);
                }
            }
            else if (Val == '.') {
                if (has_dec2 == 0) {
                    has_dec2 = 1;
                    LCD_WriteChar('.');
                }
            }
            else if (Val == 'n') {
                if (digits2 == 0 && is_neg2 == 0) {
                    is_neg2 = 1;
                    LCD_WriteChar('-');
                }
            }
            // Transition to State 4 when equals is pressed
            else if (Val == '#' && digits2 > 0) {
                int div_error = 0;
                
                if (is_neg1 == 1) num1 = -num1;
                if (is_neg2 == 1) num2 = -num2;
                
                if (math_operator == '+') result = num1 + num2;
                else if (math_operator == '-') result = num1 - num2;
                else if (math_operator == '*') result = num1 * num2;
                else if (math_operator == '/') {
                    if (num2 != 0.0) {
                        result = num1 / num2;
                    } else {
                        div_error = 1;
                    }
                }
                
                // Clear screen to show ONLY the answer
                LCD_Clear();
                
                if (div_error == 1) {
                    sprintf(display_buffer, "ERROR"); 
                } else {
                    sprintf(display_buffer, "%g", result);
                }
                
                LCD_WriteString(display_buffer);
                
                // Move to State 4
                State = 4;
            }
            break;

        //=========================================
        // STATE 4: Computing Result / Idle
        //=========================================
        case 4:
            // Calculator sits here showing the answer until a new number is pressed
            break;
    }
}