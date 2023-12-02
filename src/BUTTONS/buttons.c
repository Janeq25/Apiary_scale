#include "driver/gpio.h"
#include <BUTTONS/buttons.h>

enum Button_Name eButton_Name;

enum Button_State ebut0_prev_level = RELEASED; 
enum Button_State ebut1_prev_level = RELEASED;

uint8_t but_pin;

uint8_t but_pin0;
uint8_t but_pin1;
   
void Button_Init(uint8_t pin0, uint8_t pin1)
{
    but_pin0 = pin0;
    but_pin1 = pin1;

    gpio_set_direction(but_pin0, GPIO_MODE_INPUT);
    gpio_set_direction(but_pin1, GPIO_MODE_INPUT);
    gpio_pullup_en(but_pin0);
    gpio_pullup_en(but_pin1);
}

enum Button_State eButton_Read(enum Button_Name eButton_Name)
{
    enum Button_State ebut_level = RELEASED;
    enum Button_State ebut_prev_level = RELEASED;
    
    switch(eButton_Name)
    {
    case BUTTON_0:
        but_pin = but_pin0;
        break;
    case BUTTON_1:
        but_pin = but_pin1;
        break;
    default:
        break;
    }

    switch(gpio_get_level(but_pin))
    {
    case 0:
        ebut_level = PRESSED;
        break;
    case 1:
        ebut_level = RELEASED;
        break;
    default:
        ebut_level = RELEASED;
        break;
    }

    switch(eButton_Name)
    {
    case BUTTON_0:
        ebut_prev_level = ebut0_prev_level;
        ebut0_prev_level = ebut_level;
        break;
    case BUTTON_1:
        ebut_prev_level = ebut1_prev_level;
        ebut1_prev_level = ebut_level;
        break;
    default:
        break;
    }

    switch(ebut_prev_level)
    {
        case PRESSED:
            if(ebut_level == RELEASED)
            {
                return PRESSED;
            }
            else
            {
                return RELEASED;
            }
            break;
        case RELEASED:
            if(ebut_level == RELEASED) 
            {
                return RELEASED;
            }
            else
            {
                return RELEASED;
            }
            break;
        default:
                return RELEASED;
            break;
    }
}