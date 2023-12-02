#include <stdio.h>
#include <esp_err.h>
#include <inttypes.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include <time.h>
#include <lwip/sockets.h>

#include <BUTTONS/buttons.h> //buttons
#include "HD44780/HD44780.h" //display
#include "HX711/hx711_lib.h" //tensometer
#include "DHT11/dht_espidf.h" //thermometer

// ------------------------------------------------ display config ------------------------------------------------

#define LCD_ADDR 0x27 //display
#define LCD_SDA_PIN  19
#define LCD_SCL_PIN  18
#define LCD_COLS 16
#define LCD_ROWS 2

// ------------------------------------------------ buttons config ------------------------------------------------

#define BUTTON_0_GPIO 32
#define BUTTON_1_GPIO 33

// ------------------------------------------------ tensometer config ------------------------------------------------

#define SCALE_CONST 233.82 
#define SCALE_AVERAGE_READS 10
#define TENSOMETER_DOUT_PIN 33
#define TENSOMETER_SCK_PIN 32
#define TENSOMETER_GAIN 0

// ------------------------------------------------ thermometer config ------------------------------------------------

#define TERMOMETER_PIN 3


// ------------------------------------------------ tensometer globals ------------------------------------------------

hx711_t tensometer;
int32_t scale_offset = 0;


// ------------------------------------------------ thermometer globals ------------------------------------------------

struct dht_reading thermometer_data;





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

    tensometer.dout = TENSOMETER_DOUT_PIN;
    tensometer.pd_sck = TENSOMETER_SCK_PIN;
    tensometer.gain = TENSOMETER_GAIN;

    if(hx711_init(&tensometer) != ESP_OK){
        printf("ERR - failed to initialise tensometer");
        return ESP_ERR_INVALID_RESPONSE;
    }

    scale_offset = tensometer_read_once();

    return ESP_OK;
}

// ------------------------------------------------ buttons ------------------------------------------------
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
// ------------------------------------------------ thermometer ------------------------------------------------

void thermometer_read(){
    read_dht_sensor_data((gpio_num_t)TERMOMETER_PIN, DHT11, &thermometer_data);


}

// ------------------------------------------------ RTC --------------------------------------------------------

uint8_t setTimeDateRTCIntern(uint8_t hour, uint8_t minutes, uint8_t seconds, uint8_t mday, uint8_t month, uint8_t year)
{
    struct timeval tv;
    struct tm mytm;
    char buf[256];
    /* Checks data */
    if(hour > 23)       { return 0; }
    if(minutes > 59)    { return 0; }
    if(seconds > 59)    { return 0; }
    if(mday > 31)       { return 0; }
    if(month > 12)      { return 0; }
    if(year > 30)       { return 0; }
    mytm.tm_hour = hour;
    mytm.tm_min = minutes;
    mytm.tm_sec = seconds;
    mytm.tm_mday = mday;
    mytm.tm_mon = month;
    mytm.tm_year = 100 + year;
    setenv("TZ", "GMT,M3.5.0/2,M10.5.0/3", 1);
    tzset();
    time_t t = mktime(&mytm);
  
    tv.tv_sec = t;
    tv.tv_usec = 0;

    settimeofday(&tv, NULL);

    sprintf(buf,"%02d:%02d:%02d",mytm.tm_hour,mytm.tm_min,mytm.tm_sec);
    printf("%s", buf);
    sprintf(buf,"%04d-%02d-%02d",mytm.tm_year+1900,mytm.tm_mon+1,mytm.tm_mday);
    printf("%s", buf);
    return 1;
}

void printTime(void)
{
    char strftime_buf[64];
    time_t now = 0;
    struct tm timeinfo;
   
    time(&now);
    /* Update struct tm with new data */
    localtime_r(&now, &timeinfo);
    strftime(strftime_buf, sizeof(strftime_buf), "%c", &timeinfo);
    //ESP_LOGI(TAG, "CET DST: %s", strftime_buf);
    printf("%02d:%02d:%02d\n",timeinfo.tm_hour,timeinfo.tm_min,timeinfo.tm_sec);
    printf("%04d-%02d-%02d\n",timeinfo.tm_year+1900,timeinfo.tm_mon+1,timeinfo.tm_mday);
    //printfTimeDateInfo(&now, &timeinfo);
    //ESP_LOGI(TAG, "CET DST: %s\n", strftime_buf);
    //vTaskDelay(1000 / portTICK_PERIOD_MS);
   
}

// ------------------------------------------------ main ------------------------------------------------




void app_main() {

   // ------------------------------------------------ initializations ------------------------------------------------

    tensometer_init();
    
    LCD_init(LCD_ADDR, LCD_SDA_PIN, LCD_SCL_PIN, LCD_COLS, LCD_ROWS);
    LCD_home();
    LCD_clearScreen();

    Button_Init(BUTTON_0_GPIO, BUTTON_1_GPIO);

    setTimeDateRTCIntern(15, 24, 34, 2, 12, 23);

    while(1)
    {

        //printf("tensometer data: %" PRIi32 "\n", tensometer_read_average());
        //printf("tensometer_raw data: %" PRIi32 "\n", tensometer_read_once());
        thermometer_read();
        //printf("thermometer - temp: %lf, humid: %lf \n",thermometer_data.temperature, thermometer_data.humidity);
        //vTaskDelay(pdMS_TO_TICKS(1000));

        LCD_Button0_test();
        LCD_Button1_test();

        switch (eButton_Read(BUTTON_0))
        {
            case RELEASED:
                break;
            case PRESSED:
                printTime();
                break;
            default:
                break;
        }
    }
}