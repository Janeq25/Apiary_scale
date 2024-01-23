#include "driver/gpio.h"
#include <BUTTONS/buttons.h>

enum Button_Name eButton_Name;

enum Button_State ebut0_prev_level = RELEASED; 
enum Button_State ebut1_prev_level = RELEASED;
//enum Button_State ebut2_prev_level = RELEASED;

enum Button_State ebut0_level = RELEASED; 
enum Button_State ebut1_level = RELEASED; 

uint8_t but_pin;

uint8_t but_pin0;
uint8_t but_pin1;
uint8_t but_pin2;
   
void Set_Button_0_state(void *arg)
{
    ebut0_level = PRESSED;
}

void Set_Button_1_state(void* arg)
{
    ebut1_level = PRESSED;
}

void Button_Init(uint8_t pin0, uint8_t pin1)
{
    but_pin0 = pin0;
    but_pin1 = pin1;
    //but_pin2 = pin2;

    ESP_ERROR_CHECK(gpio_set_direction(but_pin0, GPIO_MODE_INPUT));
    ESP_ERROR_CHECK(gpio_set_direction(but_pin1, GPIO_MODE_INPUT));
    //gpio_set_direction(but_pin2, GPIO_MODE_INPUT);
    ESP_ERROR_CHECK(gpio_pullup_en(but_pin0));
    ESP_ERROR_CHECK(gpio_pullup_en(but_pin1));
    //gpio_pullup_en(but_pin2);

    gpio_install_isr_service(0);

    gpio_set_intr_type(but_pin0, GPIO_INTR_POSEDGE);
    gpio_set_intr_type(but_pin1, GPIO_INTR_POSEDGE);

    gpio_isr_handler_add(but_pin0, Set_Button_0_state, NULL);
    gpio_isr_handler_add(but_pin1, Set_Button_1_state, NULL);
}

enum Button_State eButton_Read(enum Button_Name eButton_Name)
{
    enum Button_State ebut_level = RELEASED;

    switch(eButton_Name)
    {
    case BUTTON_0:
        ebut_level = ebut0_level;
        ebut0_level = RELEASED; 
        return ebut_level;
        break;
    case BUTTON_1:
        ebut_level = ebut1_level;
        ebut1_level = RELEASED; 
        return ebut_level;
        break;
    default:
        return RELEASED;
        break;
    }
}

//enum Button_State eButton_Read(enum Button_Name eButton_Name)
//{
//    enum Button_State ebut_level = RELEASED;
//    enum Button_State ebut_prev_level = RELEASED;
//
//    switch(eButton_Name)
//    {
//    case BUTTON_0:
//        but_pin = but_pin0;
//        break;
//    case BUTTON_1:
//        but_pin = but_pin1;
//        break;
//    // case BUTTON_2:
//    //     but_pin = but_pin2;
//    //     break;
//    // default:
//    //     break;
//    }
//
//    switch(gpio_get_level(but_pin))
//    {
//    case 0:
//        ebut_level = PRESSED;
//        break;
//    case 1:
//        ebut_level = RELEASED;
//        break;
//    default:
//        ebut_level = RELEASED;
//        break;
//    }
//
//    switch(eButton_Name)
//    {
//    case BUTTON_0:
//        ebut_prev_level = ebut0_prev_level;
//        ebut0_prev_level = ebut_level;
//        break;
//    case BUTTON_1:
//        ebut_prev_level = ebut1_prev_level;
//        ebut1_prev_level = ebut_level;
//        break;
//    // case BUTTON_2:
//    //     ebut_prev_level = ebut2_prev_level;
//    //     ebut2_prev_level = ebut_level;
//    //     break;
//    default:
//        break;
//    }
//
//    switch(ebut_prev_level)
//    {
//        case PRESSED:
//            if(ebut_level == RELEASED)
//            {
//                return PRESSED;
//            }
//            else
//            {
//                return RELEASED;
//            }
//            break;
//        case RELEASED:
//            if(ebut_level == RELEASED) 
//            {
//                return RELEASED;
//            }
//            else
//            {
//                return RELEASED;
//            }
//            break;
//        default:
//                return RELEASED;
//            break;
//    }
//}