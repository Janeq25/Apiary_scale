#include <stdio.h>
#include <esp_err.h>
#include <inttypes.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_sleep.h>

#include "BUTTONS/buttons.h" //buttons
#include "HD44780/HD44780.h" //display
#include "RTC/rtc.h" //rtc
#include "HX711/hx711_lib.h" //tensometer
#include "DHT11/dht11.h" //thermometer

// ------------------------------------------------ display config ------------------------------------------------

#define LCD_ADDR 0x27 //display
#define LCD_SDA_PIN  19
#define LCD_SCL_PIN  18
#define LCD_COLS 16
#define LCD_ROWS 2

// ------------------------------------------------ buttons config ------------------------------------------------

#define BUTTON_0_GPIO 23
#define BUTTON_1_GPIO 22
#define BUTTON_2_GPIO 1

// ------------------------------------------------ tensometer config ------------------------------------------------

#define SCALE_CONST 233.82 
#define SCALE_AVERAGE_READS 10
#define TENSOMETER_DOUT_PIN 33
#define TENSOMETER_SCK_PIN 32
#define TENSOMETER_GAIN 0

// ------------------------------------------------ thermometer config ------------------------------------------------

#define TERMOMETER_PIN 26


// ------------------------------------------------ tensometer globals ------------------------------------------------

hx711_t tensometer;
int32_t scale_offset = 0;



// ------------------------------------------------ tensometer ------------------------------------------------

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

    tensometer.dout = TENSOMETER_DOUT_PIN;
    tensometer.pd_sck = TENSOMETER_SCK_PIN;
    tensometer.gain = TENSOMETER_GAIN;

    hx711_init(&tensometer);
    // if(hx711_init(&tensometer) != ESP_OK){
    //     printf("ERR - failed to initialise tensometer\n");
    //     return ESP_ERR_INVALID_RESPONSE;
    // }

    scale_offset = tensometer_read_average();
    scale_offset = scale_offset * SCALE_CONST;
    printf("scale_offset: %" PRIi32 "\n", scale_offset);

    return ESP_OK;
}

// ------------------------------------------------ buttons ------------------------------------------------

char int_string[16] = {0};

char* int_to_string(int number)
{
    sprintf(int_string, "%d", number);

    return int_string;
}

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

    LCD_setCursor(0, 0);
    LCD_writeStr("But0_val= ");
    LCD_setCursor(10, 0);
    LCD_writeStr(int_to_string(number0));
}

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

    LCD_setCursor(0, 1);
    LCD_writeStr("But1_val= ");
    LCD_setCursor(10, 1);
    LCD_writeStr(int_to_string(number1));
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

// ------------------------------------------------ RTC --------------------------------------------------------


uint8_t prev_second;
uint8_t prev_minute;
uint8_t prev_hour;

void LCD_time()
{   
    LCD_setCursor(0, 0);
    LCD_writeStr("Time: ");

    struct tm timeinfo = updateTime();
    
    if((prev_second + 1) == timeinfo.tm_sec || (prev_minute + 1) == timeinfo.tm_min || (prev_hour + 1) == timeinfo.tm_hour)
    {
        if(timeinfo.tm_hour < 10)
        {
            LCD_setCursor(6, 0);
            LCD_writeStr(" ");
            LCD_setCursor(7, 0);
            LCD_writeStr(int_to_string(timeinfo.tm_hour));
            LCD_setCursor(8, 0);
            LCD_writeStr(":");
        }
        else
        {
            LCD_setCursor(6, 0);
            LCD_writeStr(int_to_string(timeinfo.tm_hour));
            LCD_setCursor(8, 0);
            LCD_writeStr(":");
        }
        
        if(timeinfo.tm_min < 10)
        {
            LCD_setCursor(9, 0);
            LCD_writeStr("0");
            LCD_setCursor(10, 0);
            LCD_writeStr(int_to_string(timeinfo.tm_min));
            LCD_setCursor(11, 0);
            LCD_writeStr(":");
        }
        else
        {
            LCD_setCursor(9, 0);
            LCD_writeStr(int_to_string(timeinfo.tm_min));
            LCD_setCursor(11, 0);
            LCD_writeStr(":");
        }

        if(timeinfo.tm_sec < 10)
        {
            LCD_setCursor(12, 0);
            LCD_writeStr("0");
            LCD_setCursor(13, 0);
            LCD_writeStr(int_to_string(timeinfo.tm_sec));
        }
        else
        {
            LCD_setCursor(12, 0);
            LCD_writeStr(int_to_string(timeinfo.tm_sec));
        }  
        
        LCD_setCursor(0, 1);
        LCD_writeStr("Date: ");

        if(timeinfo.tm_mday < 10)
        {
            LCD_setCursor(6, 1);
            LCD_writeStr("0");
            LCD_setCursor(7, 1);
            LCD_writeStr(int_to_string(timeinfo.tm_mday));
            LCD_setCursor(8, 1);
            LCD_writeStr("/");
        }
        else
        {
            LCD_setCursor(6, 1);
            LCD_writeStr(int_to_string(timeinfo.tm_mday));
            LCD_setCursor(8, 1);
            LCD_writeStr(":");
        }

        if(timeinfo.tm_mon + 1 < 10)
        {
            LCD_setCursor(9, 1);
            LCD_writeStr("0");
            LCD_setCursor(10, 1);
            LCD_writeStr(int_to_string(timeinfo.tm_mon + 1));
            LCD_setCursor(11, 1);
            LCD_writeStr("/");
        }
        else
        {
            LCD_setCursor(9, 1);
            LCD_writeStr(int_to_string(timeinfo.tm_mon + 1));
            LCD_setCursor(11, 1);
            LCD_writeStr("/");
        }
        if(timeinfo.tm_mon < 10)
        {
            LCD_setCursor(9, 1);
            LCD_writeStr("0");
            LCD_setCursor(10, 1);
            LCD_writeStr(int_to_string(timeinfo.tm_mon + 1));
            LCD_setCursor(11, 1);
            LCD_writeStr("/");
        }
        else
        {
            LCD_setCursor(12, 1);
            LCD_writeStr(int_to_string(timeinfo.tm_year + 1900));
        }

        prev_second = timeinfo.tm_sec;
        prev_minute = timeinfo.tm_min;
        prev_hour = timeinfo.tm_hour;
    }
    else
    {
        prev_second = timeinfo.tm_sec;
        prev_minute = timeinfo.tm_min;
        prev_hour = timeinfo.tm_hour;
    }   
}

void set_time()
{
    switch (eButton_Read(BUTTON_0))
    {
        case RELEASED:
            break;
        case PRESSED:
            struct tm timeinfo = updateTime();
            //printf("%02d:%02d:%02d:%02d:%02d:%02d\n",timeinfo.tm_hour,timeinfo.tm_min,timeinfo.tm_sec,timeinfo.tm_mday, timeinfo.tm_mon, timeinfo.tm_year);
            setTimeDateRTCIntern((timeinfo.tm_hour + 1), timeinfo.tm_min, timeinfo.tm_sec , timeinfo.tm_mday, timeinfo.tm_mon, (timeinfo.tm_year - 100));
            break;
        default:
            break;
    }

    switch (eButton_Read(BUTTON_1))
    {
        case RELEASED:
            break;
        case PRESSED:
            struct tm timeinfo = updateTime();
            //printf("%02d:%02d:%02d:%02d:%02d:%02d\n",timeinfo.tm_hour,timeinfo.tm_min,timeinfo.tm_sec,timeinfo.tm_mday, timeinfo.tm_mon, timeinfo.tm_year);
            setTimeDateRTCIntern(timeinfo.tm_hour, (timeinfo.tm_min + 1), timeinfo.tm_sec , timeinfo.tm_mday, timeinfo.tm_mon, (timeinfo.tm_year - 100));
            break;
        default:
            break;
    }

    LCD_time();
}

// ------------------------------------------------ deepsleep variables------------------------------------------------

RTC_DATA_ATTR u_int8_t time_set = 0;

// ------------------------------------------------ main ------------------------------------------------

void app_main() {

   // ------------------------------------------------ initializations ------------------------------------------------

    DHT11_init(TERMOMETER_PIN);
    tensometer_init();
    
    // LCD_init(LCD_ADDR, LCD_SDA_PIN, LCD_SCL_PIN, LCD_COLS, LCD_ROWS);
    // LCD_home();
    // LCD_clearScreen();

    Button_Init(BUTTON_0_GPIO, BUTTON_1_GPIO, BUTTON_2_GPIO);

    switch (time_set)
    {
    case 0:
        setTimeDateRTCIntern(0, 0, 0, 4, 11, 23);
        time_set = 1;
        break;
    case 1:
        time_set = 1;
        break;
    default:
        time_set = 1;
        break;
    }

    
    while(1)
    {
        printf("tensometer data: %" PRIi32 "\n", tensometer_read_average());
        vTaskDelay(pdMS_TO_TICKS(1000));
        
        printf("thermometer - temp: %i, humid: %i, status: %i\n", DHT11_read().temperature, DHT11_read().humidity, DHT11_read().status);
        vTaskDelay(pdMS_TO_TICKS(1000));
        
        // switch (eButton_Read(BUTTON_2))
        // {
        // case PRESSED:
        // LCD_Off();
        // esp_sleep_enable_timer_wakeup(30000000);
        // esp_deep_sleep_start();
        //     break;
        // case RELEASED:
        // thermometer_read();
        // LCD_time();
        // set_time();
        //     break;
        // default:
        //     break;
        // }


    }
}