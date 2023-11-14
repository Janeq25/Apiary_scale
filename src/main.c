#include <stdio.h>
#include <esp_err.h>
#include <inttypes.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include "HD44780/HD44780.h" //display
#include "HX711/hx711_lib.h" //tensometer

// ------------------------------------------------ display config ------------------------------------------------

#define LCD_ADDR 0x27 //display
#define SDA_PIN  19
#define SCL_PIN  18
#define LCD_COLS 16
#define LCD_ROWS 2

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

// ------------------------------------------------ main ------------------------------------------------


void app_main() {

    tensometer_init();

    LCD_init(LCD_ADDR, SDA_PIN, SCL_PIN, LCD_COLS, LCD_ROWS);
    LCD_home();
    LCD_clearScreen();

    while(1){

        printf("tensometer data: %" PRIi32 "\n", tensometer_read_average());
        //printf("tensometer_raw data: %" PRIi32 "\n", tensometer_read_once());

        LCD_DemoTask();
    }
}