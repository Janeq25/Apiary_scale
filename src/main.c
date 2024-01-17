#include <stdio.h>
#include <esp_err.h>
#include <inttypes.h>
#include <string.h>
#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_sleep.h>

#include "BUTTONS/buttons.h" //buttons
#include "HD44780/HD44780.h" //display
#include "RTC/rtc.h" //rtc
#include "HX711/hx711_lib.h" //tensometer
#include "DHT11/dht11.h" //thermometer
#include "SIM800l/sim800l.h"

// ------------------------------------------------ display config ------------------------------------------------

#define LCD_ADDR 0x27 //display
#define LCD_SDA_PIN  14
#define LCD_SCL_PIN  27
#define LCD_COLS 16
#define LCD_ROWS 2

// ------------------------------------------------ buttons config ------------------------------------------------

#define BUTTON_0_GPIO 15
#define BUTTON_1_GPIO 0

// ------------------------------------------------ tensometer config ------------------------------------------------

#define SCALE_CONST 233.82 
#define SCALE_AVERAGE_READS 10
#define TENSOMETER_DOUT_PIN 33
#define TENSOMETER_SCK_PIN 32
#define TENSOMETER_GAIN 0

// ------------------------------------------------ thermometer config ------------------------------------------------

#define TERMOMETER_PIN 26

// ------------------------------------------------ GSM config ------------------------------------------------

#define GSM_TX_PIN 17
#define GSM_RX_PIN 16
#define USED_UART UART_NUM_2
#define GSM_RESPONSE_BUFFER_SIZE 1024

// ------------------------------------------------ main globals ------------------------------------------------

static const char* TAG = "MAIN";
enum state_e {INIT, TIME_SCREEN, WEIGHT_SCREEN, TEMP_SCREEN, MEASUREMENT, SLEEP, WAKEUP, WAIT_FOR_RESET};
enum state_e state = INIT;

// ------------------------------------------------ tensometer globals ------------------------------------------------


hx711_t tensometer;
int32_t scale_offset = 0;
int32_t tensometer_reading = 0;

// ------------------------------------------------ sim800l (uart) ------------------------------------------------


char response_buffer[GSM_RESPONSE_BUFFER_SIZE] = {0};


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

    esp_err_t status;

    tensometer.dout = TENSOMETER_DOUT_PIN;
    tensometer.pd_sck = TENSOMETER_SCK_PIN;
    tensometer.gain = TENSOMETER_GAIN;

    status = hx711_init(&tensometer);
    // if(hx711_init(&tensometer) != ESP_OK){
    //     printf("ERR - failed to initialise tensometer\n");
    //     return ESP_ERR_INVALID_RESPONSE;
    // }

    scale_offset = tensometer_read_average();
    scale_offset = scale_offset * SCALE_CONST;
    ESP_LOGI(TAG, "scale_offset: %" PRIi32 "\n", scale_offset);

    return status;
}

// ------------------------------------------------ thermometer globals ------------------------------------------------

struct dht11_reading thermometer_reading;

// ------------------------------------------------ display ------------------------------------------------

void LCD_DemoTask()
{
    LCD_clearScreen();
    LCD_setCursor(0, 0);
    LCD_writeStr("--- 16x2 LCD ---");
    LCD_setCursor(0, 1);
    LCD_writeStr("LCD Library Demo");

}

void LCD_Write_screen(char* row1, char* row2){

    LCD_clearScreen();
    LCD_setCursor(0, 0);
    LCD_writeStr(row1);
    LCD_setCursor(0, 1);
    LCD_writeStr(row2);

}

void time_screen(){


}

// ------------------------------------------------ RTC --------------------------------------------------------




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

    time_screen();
}

// ------------------------------------------------ deepsleep variables------------------------------------------------

RTC_DATA_ATTR u_int8_t time_set = 0;

// ------------------------------------------------ main ------------------------------------------------

void app_main() {



    // gsm_get_status();
    // char GET_request_buffer[1024];
    // gsm_send_http_request("http://worldclockapi.com/api/jsonp/cet/now?callback=mycallback", "", GET_request_buffer, 10000);
    


    while(1)
    {



        switch (state){
            case INIT:

                ESP_LOGI(TAG, "Initialising screen");
                LCD_init(LCD_ADDR, LCD_SDA_PIN, LCD_SCL_PIN, LCD_COLS, LCD_ROWS);
                LCD_home();
                LCD_clearScreen();
                LCD_setCursor(0,0);
                LCD_writeStr("Starting");
                vTaskDelay(pdMS_TO_TICKS(500));
                ESP_LOGI(TAG, "Initialising screen finished");


                ESP_LOGI(TAG, "Initialising Thermometer");
                LCD_Write_screen("Inititlising", "Thermometer");
                DHT11_init(TERMOMETER_PIN);
                thermometer_reading = DHT11_read();
                if (thermometer_reading.status == DHT11_OK){
                    char temp[LCD_COLS];
                    char humid[LCD_COLS];
                    sprintf(temp, "Temp: %d C", thermometer_reading.temperature);
                    sprintf(humid, "Humid: %d ", thermometer_reading.humidity);
                    LCD_Write_screen(temp, humid);
                }
                else{
                    ESP_LOGI(TAG, "DHT11 status: %d", thermometer_reading.status);
                    LCD_Write_screen("Thermometer init", "Failed");
                    state = WAIT_FOR_RESET;
                }

                vTaskDelay(pdMS_TO_TICKS(500));

                ESP_LOGI(TAG, "Initialising Tensometer");
                LCD_Write_screen("Inititlising", "Tensometer");
                if (tensometer_init() == ESP_OK){
                    tensometer_reading = tensometer_read_average();
                    char weight[LCD_COLS];
                    sprintf(weight, "%d g", (int)tensometer_reading);
                    LCD_Write_screen("Weight: ", weight);
                }
                else{
                    ESP_LOGI(TAG, "Tensometer init failed");
                    LCD_Write_screen("Tensometer init", "Failed");
                    state = WAIT_FOR_RESET;
                }

                vTaskDelay(pdMS_TO_TICKS(500));

                ESP_LOGI(TAG, "initialising buttons");
                LCD_Write_screen("Inititlising", "Buttons");
                Button_Init(BUTTON_0_GPIO, BUTTON_1_GPIO);

                ESP_LOGI(TAG, "initialising Sim800l");
                LCD_Write_screen("Inititlising", "GSM");
                switch (gsm_init(USED_UART, GSM_TX_PIN, GSM_RX_PIN, GSM_RESPONSE_BUFFER_SIZE, response_buffer)){
                    case GSM_OK:
                    ESP_LOGI(TAG, "GSM ok");
                    break;

                    case GSM_ERR_MODULE_NOT_CONNECTED:
                    LCD_Write_screen("GSM module", "Not Connected");
                    state = WAIT_FOR_RESET;
                    break;

                    case GSM_ERR_SIM_NOT_INSERTED:
                    LCD_Write_screen("GSM module", "Insert SIM");
                    state = WAIT_FOR_RESET;
                    break;

                    case GSM_ERR_PIN_REQUIRED:
                    LCD_Write_screen("GSM module", "PIN Required");
                    state = WAIT_FOR_RESET;
                    break;

                    case GSM_ERR_NO_SIGNAL:
                    LCD_Write_screen("GSM module", "No signal");
                    state = WAIT_FOR_RESET;
                    break;

                    case GSM_ERR_NOT_ATTACHED_GPRS_SERV:
                    LCD_Write_screen("GSM module", "GPRS error");
                    state = WAIT_FOR_RESET;
                    break;

                    case GSM_ERR_NOT_REGISTERED_IN_NETWORK:
                    LCD_Write_screen("GSM module", "not registered");
                    state = WAIT_FOR_RESET;
                    break;

                    default:
                    LCD_Write_screen("GSM module", "ERROR");
                    state = WAIT_FOR_RESET;
                    break;
                }

                vTaskDelay(pdMS_TO_TICKS(1000));

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

                state = TIME_SCREEN;
            break;

            case TIME_SCREEN:
                char row1[24];
                char row2[24];

                struct tm timeinfo = updateTime();

                sprintf(row1, "Time: %02d:%02d:%02d", timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
                sprintf(row2, "Date: %02d-%02d-%02d", timeinfo.tm_mday, timeinfo.tm_mon, timeinfo.tm_year-100);

                LCD_Write_screen(row1, row2);

                if (eButton_Read(BUTTON_0) == PRESSED){
                    state = WEIGHT_SCREEN;
                }
                else{
                    state = TIME_SCREEN;
                }

                vTaskDelay(pdMS_TO_TICKS(1000));

            break;      

            case WEIGHT_SCREEN:

                tensometer_reading = tensometer_read_average();
                char weight[LCD_COLS];
                sprintf(weight, "%d g", (int)tensometer_reading);
                LCD_Write_screen("Weight: ", weight);



                if (eButton_Read(BUTTON_0) == PRESSED){
                    state = TEMP_SCREEN;
                }
                else{
                    state = WEIGHT_SCREEN;
                }

                vTaskDelay(pdMS_TO_TICKS(1000));

            break;

            case TEMP_SCREEN:

                thermometer_reading = DHT11_read();
                if (thermometer_reading.status == DHT11_OK){
                    char temp[LCD_COLS];
                    char humid[LCD_COLS];
                    sprintf(temp, "Temp: %d C", thermometer_reading.temperature);
                    sprintf(humid, "Humid: %d ", thermometer_reading.humidity);
                    LCD_Write_screen(temp, humid);
                }
                else{
                    ESP_LOGI(TAG, "DHT11 status: %d", thermometer_reading.status);
                    LCD_Write_screen("Thermometer init", "Failed");
                    state = WAIT_FOR_RESET;
                }

                if (eButton_Read(BUTTON_0) == PRESSED){
                    state = TIME_SCREEN;
                }
                else{
                    state = TEMP_SCREEN;
                }

                vTaskDelay(pdMS_TO_TICKS(1000));

            break;        

            case MEASUREMENT:

            break;

            case SLEEP:

            break;

            case WAKEUP:

            break;

            case WAIT_FOR_RESET:

            state = WAIT_FOR_RESET;
            vTaskDelay(pdMS_TO_TICKS(10));
            break;


        }

        // printf("tensometer data: %" PRIi32 "\n", tensometer_read_average());
        // printf("thermometer - temp: %i, humid: %i \n", DHT11_read().temperature, DHT11_read().humidity);
        
        // time_screen();
        // vTaskDelay(pdMS_TO_TICKS(1000));







        
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