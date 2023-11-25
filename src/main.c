#include <stdio.h>
#include <esp_err.h>
#include <inttypes.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <driver/gpio.h>

#include "HD44780/HD44780.h" //display
#include "HX711/hx711_lib.h" //tensometer

// ------------------------------------------------ display config ------------------------------------------------

#define LCD_ADDR 0x27 //display
#define SDA_PIN  19
#define SCL_PIN  18
#define LCD_COLS 16
#define LCD_ROWS 2

// ------------------------------------------------ buttons config ------------------------------------------------

#define BUTTON_0_GPIO 32
#define BUTTON_1_GPIO 33

// ------------------------------------------------ tensometer config ------------------------------------------------

#define SCALE_CONST 233.82
#define SCALE_CONST 233.82 
#define SCALE_AVERAGE_READS 10


// ------------------------------------------------ tensometer globals ------------------------------------------------

hx711_t tensometer;
int32_t scale_offset = 0;

// ------------------------------------------------ tensometer ------------------------------------------------
int32_t tensometer_read_once(){
    int32_t tensometer_data;    
    hx711_read_data(&tensometer, &tensometer_data);
    hx711_wait(&tensometer, 500);

    return tensometer_data;
}

int32_t tensometer_read_average(){
    int32_t tensometer_data = 0;
    int32_t sum = 0;
    int32_t mean = 0;
    for (uint8_t i = 0; i < SCALE_AVERAGE_READS; i++){
        hx711_read_data(&tensometer, &tensometer_data);
        hx711_wait(&tensometer, 500);
        sum += tensometer_data;
    }
    mean = sum/SCALE_AVERAGE_READS;
    mean = mean - scale_offset;
    mean = mean/SCALE_CONST;
    return mean;

}

esp_err_t tensometer_init(){

    tensometer.dout = 33;
    tensometer.pd_sck = 32;
    tensometer.gain = 0;

    if(hx711_init(&tensometer) != ESP_OK){
        printf("failed to initialise tensometer");
        return ESP_ERR_INVALID_RESPONSE;
    }

    scale_offset = tensometer_read_once();

    return ESP_OK;
}

// ------------------------------------------------ buttons ------------------------------------------------
    enum Button_Name {BUTTON_0, BUTTON_1};
    enum Button_State {PRESSED, RELEASED};

    enum Button_Name Button_Name;

    static enum Button_State ebut0_prev_level = RELEASED; 
    static enum Button_State ebut1_prev_level = RELEASED;
   
    uint8_t but_pin;

    static uint8_t but_pin0;
    static uint8_t but_pin1;
   
    void Button_Init(uint8_t pin0, uint8_t pin1)
    {
        but_pin0 = pin0;
        but_pin1 = pin1;

        gpio_set_direction(but_pin0, GPIO_MODE_INPUT);
        gpio_set_direction(but_pin1, GPIO_MODE_INPUT);
        gpio_pullup_en(but_pin0);
        gpio_pullup_en(but_pin1);
    }

    enum Button_State eButton_Read(enum Button_Name Button_Name)
    {
        enum Button_State ebut_level = RELEASED;
        enum Button_State ebut_prev_level = RELEASED;
        
        switch(Button_Name)
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

        switch(Button_Name)
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

// ------------------------------------------------ display ------------------------------------------------

void LCD_DemoTask()
{
    LCD_setCursor(0, 0);
    LCD_writeStr("--- 16x2 LCD ---");
    LCD_setCursor(0, 1);
    LCD_writeStr("LCD Library Demo");

    //LCD_setCursor(0, 0);
    //LCD_writeChar('O');
    //LCD_setCursor(1, 0);
    //LCD_writeChar('K');
    //LCD_setCursor(0, 1);
    //LCD_writeChar('O');
    //LCD_setCursor(1, 1);
    //LCD_writeChar('K');
}

    char int0_string[16] = {0};
    int number0 = 0;

void LCD_Button0_test()
{
    switch (eButton_Read(BUTTON_0))
    {
        case RELEASED:
            number0 = number0;
            break;
        case PRESSED:
            number0++;
            break;
        default:
            number0 = number0;
            break;
    }

    sprintf(int0_string, "%d", number0);
   
    LCD_setCursor(0, 0);
    LCD_writeStr("But0_val= ");
    LCD_setCursor(10, 0);
    LCD_writeStr(int0_string);
}

    char int1_string[16] = {0};
    int number1 = 0;

void LCD_Button1_test()
{
    switch (eButton_Read(BUTTON_1))
    {
        case RELEASED:
            number1 = number1;
            break;
        case PRESSED:
            number1++;
            break;
        default:
            number1 = number1;
            break;
    }

    sprintf(int1_string, "%d", number1);
   
    LCD_setCursor(0, 1);
    LCD_writeStr("But1_val= ");
    LCD_setCursor(10, 1);
    LCD_writeStr(int1_string);
}


// ------------------------------------------------ main ------------------------------------------------


void app_main() {

    tensometer_init();

    Button_Init(BUTTON_0_GPIO, BUTTON_1_GPIO);
   
    LCD_init(LCD_ADDR, SDA_PIN, SCL_PIN, LCD_COLS, LCD_ROWS);
    LCD_home();
    LCD_clearScreen();

    while(1)
    {

        //printf("tensometer data: %" PRIi32 "\n", tensometer_read_average());
        //printf("tensometer_raw data: %" PRIi32 "\n", tensometer_read_once());

        LCD_Button0_test();
        LCD_Button1_test();
    }
}