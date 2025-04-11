#include <stdio.h>
#include <esp_err.h>
#include <inttypes.h>
#include <string.h>
#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_sleep.h>
#include <driver/rtc_io.h>
#include <nvs_flash.h>
#include <nvs.h>

#include "BUTTONS/buttons.h" //buttons
#include "HD44780/HD44780.h" //display
#include "RTC/rtc.h" //rtc
#include "HX711/hx711_lib.h" //tensometer
#include "DHT11/dht11.h" //thermometer
#include "SIM800l/sim800l.h"


// ------------------------------------------------ google sheets credentials ------------------------------------------------

// #define SCRIPT_ID "AKfycbyK8iICZ1q51I5TIr3U0ChHTZSPx7FYG67E91rD2zMAY5iXMSy5kZ1k4eL9s-KQDR0P"
// #define SCRIPT_URL "https://script.google.com/macros/s/" SCRIPT_ID "/exec?"
#define SCRIPT_URL "http://rnmko-2a02-a31a-a097-fb00-10a5-2629-5310-99a9.a.free.pinggy.link/iot_test_dev2/save_data"
#define SCRIPT_URL_BUFFER_SIZE 200

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
#define TENSOLETER_INVERT_OUTPUT

// ------------------------------------------------ thermometer config ------------------------------------------------

#define TERMOMETER_PIN 26

// ------------------------------------------------ GSM config ------------------------------------------------

#define GSM_TX_PIN 17
#define GSM_RX_PIN 16
#define USED_UART UART_NUM_2
#define GSM_RESPONSE_BUFFER_SIZE 1024

// ------------------------------------------------ main globals ------------------------------------------------

#define NOT_SET             0
#define THIRTY_SEC_us       30000000
#define ONE_HOUR_us         3600000000
#define SIX_HOURS_us        6*ONE_HOUR_us
#define TWELVE_HOURS_us     12*ONE_HOUR_us
#define TWENTYFOUR_HOURS_us 24*ONE_HOUR_us



static const char* TAG = "MAIN";
enum state_e {INIT, TIME_SCREEN, WEIGHT_SCREEN, TEMP_SCREEN, MEASUREMENT, SLEEP, SLEEP_SETTINGS, WAIT_FOR_RESET};
RTC_DATA_ATTR enum state_e state = INIT;

RTC_DATA_ATTR uint64_t wakeup_interval = NOT_SET;

// ------------------------------------------------ tensometer globals ------------------------------------------------


hx711_t tensometer;
RTC_DATA_ATTR int32_t scale_offset = 0;
int32_t tensometer_reading = 0;


// ------------------------------------------------ tensometer ------------------------------------------------

int32_t tensometer_read_average(){
    int32_t tensometer_data = 0;
    int32_t sum = 0;
    int32_t mean = 0;
    for (uint8_t i = 0; i < SCALE_AVERAGE_READS; i++){
        hx711_read_data(&tensometer, &tensometer_data);
        hx711_wait(&tensometer, 500);
        #ifdef TENSOLETER_INVERT_OUTPUT
            sum += (tensometer_data * -1);
        #else
            sum += (tensometer_data);
        #endif
        
    }
    mean = sum/SCALE_AVERAGE_READS;
    mean = mean - scale_offset;
    mean = mean/SCALE_CONST;
    return mean;

}
void tensometer_set_offset(){
    int32_t tensometer_data = 0;
    hx711_read_data(&tensometer, &tensometer_data);
    hx711_wait(&tensometer, 500);
    #ifdef TENSOLETER_INVERT_OUTPUT
        scale_offset = (tensometer_data * -1);
    #else
        scale_offset = (tensometer_data);
    #endif
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

    ESP_LOGI(TAG, "scale_offset: %" PRIi32 "\n", scale_offset);

    return status;
}

// ------------------------------------------------ thermometer globals ------------------------------------------------

struct dht11_reading thermometer_reading;

// ------------------------------------------------ display ------------------------------------------------

char LCD_row1_buf[LCD_COLS+8];
char LCD_row2_buf[LCD_COLS+8];


void LCD_Write_screen(char* row1, char* row2){

    LCD_clearScreen();
    LCD_setCursor(0, 0);
    LCD_writeStr(row1);
    LCD_setCursor(0, 1);
    LCD_writeStr(row2);

}


// ------------------------------------------------ Non Volatile Memory ------------------------------------------------

struct tm timeinfo;

nvs_handle_t NV_memory_handle;

esp_err_t NV_write_globvars(){
    if(nvs_open("storage", NVS_READWRITE, &NV_memory_handle) != ESP_OK){
        ESP_LOGI(TAG, "failed opening nv memory for write");
        return ESP_FAIL;
    }

    if (nvs_set_i32(NV_memory_handle, "scale_offset", scale_offset)){
        ESP_LOGI(TAG, "failed saving scale_offset");
        return ESP_FAIL;
    }

    if (nvs_set_u64(NV_memory_handle, "wakeup_interval", wakeup_interval)){
        ESP_LOGI(TAG, "failed saving wakeup_interval");
        return ESP_FAIL;
    }

    nvs_close(NV_memory_handle);

    return ESP_OK;
    
}

esp_err_t NV_read_globvars(){
    if(nvs_open("storage", NVS_READONLY, &NV_memory_handle) != ESP_OK){
        ESP_LOGI(TAG, "failed opening nv memory for read");
        return ESP_FAIL;
    }

    if (nvs_get_i32(NV_memory_handle, "scale_offset", &scale_offset)){
        ESP_LOGI(TAG, "failed reading scale_offset");
        return ESP_FAIL;
    }

    if (nvs_get_u64(NV_memory_handle, "wakeup_interval", &wakeup_interval)){
        ESP_LOGI(TAG, "failed reading wakeup_interval");
        return ESP_FAIL;
    }

    nvs_close(NV_memory_handle);

    return ESP_OK;
    
}

// ------------------------------------------------ deepsleep -------------------------------------------------


void goto_sleep(){
    LCD_Off();
    gsm_enable_sleep();

    NV_write_globvars();
    state = INIT;
    gpio_pullup_en(BUTTON_0_GPIO);
    rtc_gpio_hold_en(BUTTON_0_GPIO);
    esp_sleep_enable_ext0_wakeup(BUTTON_0_GPIO, 0);
    esp_sleep_enable_timer_wakeup(wakeup_interval);
    esp_deep_sleep_start();
}



// ------------------------------------------------ sim800l (uart) ------------------------------------------------


char response_buffer[GSM_RESPONSE_BUFFER_SIZE] = {0};
char script_response_buffer[2048] = {0};
char ntc_response[1024] = {0};

esp_err_t synchronise_clock(){
    int hour=0, min=0, sec=0, day=0, mon=0, year=0;
    char* datetime_ptr = 0;
    char buffer[3] = {0};
    if(gsm_get_status() != GSM_OK){
        ESP_LOGI(TAG, "time sync failed gsm_status error");
        return ESP_FAIL;
    }

    if (gsm_send_http_request("http://worldtimeapi.org/api/timezone/Poland", ntc_response, 5000) != GSM_OK){
        ESP_LOGI(TAG, "time sync failed http request error");
        return ESP_FAIL;
    }

    datetime_ptr = strstr(ntc_response, "datetime");

    if (datetime_ptr == 0){
        ESP_LOGI(TAG, "time sync failed invalid http response");
        return ESP_FAIL;
    }
    else{

        strncpy(buffer, datetime_ptr + 13, 2);
        year = atoi(buffer);

        strncpy(buffer, datetime_ptr + 16, 2);
        mon = atoi(buffer);

        strncpy(buffer, datetime_ptr + 19, 2);
        day = atoi(buffer);

        strncpy(buffer, datetime_ptr + 22, 2);
        hour = atoi(buffer);

        strncpy(buffer, datetime_ptr + 25, 2);
        min = atoi(buffer);


        setTimeDateRTCIntern(hour, min, sec, day, mon, year);

        return ESP_OK;

    }

    return ESP_FAIL;
}

// esp_err_t send_measurements(){
//     char url_buffer[SCRIPT_URL_BUFFER_SIZE];
//     char data_buf[24] = {0};

//     memset(url_buffer, 0, SCRIPT_URL_BUFFER_SIZE);

//     strcat(url_buffer, SCRIPT_URL);

//     strcat(url_buffer, "col1=");

//     sprintf(data_buf, "%02d:%02d:%02d", timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
    
//     strcat(url_buffer, data_buf);

//     strcat(url_buffer, "&col2=");

//     sprintf(data_buf, "%02d-%02d-%02d", timeinfo.tm_mday, timeinfo.tm_mon, timeinfo.tm_year-100);

//     strcat(url_buffer, data_buf);

//     strcat(url_buffer, "&col3=");

//     tensometer_reading = tensometer_read_average();
//     sprintf(data_buf, "%dg", (int)tensometer_reading);

//     strcat(url_buffer, data_buf);

//     strcat(url_buffer, "&col4=");

//     thermometer_reading = DHT11_read();
//     sprintf(data_buf, "%d", thermometer_reading.temperature);

//     strcat(url_buffer, data_buf);

//     strcat(url_buffer, "&col5=");

//     sprintf(data_buf, "%d", thermometer_reading.humidity);

//     strcat(url_buffer, data_buf);

//     if (gsm_send_http_request(url_buffer, script_response_buffer, 10000) != GSM_OK){
//         LCD_Write_screen("Data send", "failed GSM ERR");
//         vTaskDelay(pdMS_TO_TICKS(1500));
//         return ESP_FAIL;
//     }
//     else{
//         if (strstr(script_response_buffer, "Moved")){
//             LCD_Write_screen("Server Response", "DATA RECEIVED");
//             vTaskDelay(pdMS_TO_TICKS(1500));
//             return ESP_OK;
//         }else{
//             LCD_Write_screen("Server No", "Response");
//             vTaskDelay(pdMS_TO_TICKS(1500));
//             return ESP_FAIL;
//         }
//     }

// }

esp_err_t send_measurements(){
    char url_buffer[] = SCRIPT_URL;
    char data_buf[] = "{\"Point0\": {\"timestamp\": \"20250411120611\", \"temperature\": 43, \"humidity\": 5}, \"Point1\": {\"timestamp\": \"20250411120612\", \"temperature\": 89, \"humidity\": 44}, \"Point2\": {\"timestamp\": \"20250411120613\", \"temperature\": 74, \"humidity\": 50}, \"Point3\": {\"timestamp\": \"20250411120614\", \"temperature\": 77, \"humidity\": 24}, \"Point4\": {\"timestamp\": \"20250411120615\", \"temperature\": 69, \"humidity\": 67}, \"Point5\": {\"timestamp\": \"20250411120616\", \"temperature\": 35, \"humidity\": 76}, \"Point6\": {\"timestamp\": \"20250411120617\", \"temperature\": 83, \"humidity\": 99}, \"Point7\": {\"timestamp\": \"20250411120618\", \"temperature\": 38, \"humidity\": 10}, \"Point8\": {\"timestamp\": \"20250411120619\", \"temperature\": 59, \"humidity\": 74}, \"Point9\": {\"timestamp\": \"20250411120620\", \"temperature\": 92, \"humidity\": 59}}";

    

    if (gsm_send_http_request_post(url_buffer, data_buf, script_response_buffer, 10000) != GSM_OK){
        LCD_Write_screen("Data send", "failed GSM ERR");
        vTaskDelay(pdMS_TO_TICKS(1500));
        return ESP_FAIL;
    }

    ESP_LOGI(TAG, "Server response: %s", script_response_buffer);
    
    return ESP_OK;

}


// ------------------------------------------------ main ------------------------------------------------

void app_main() {

    while(1)
    {
        switch (state){

            case INIT:

                ESP_LOGI(TAG, "initialising screen");
                LCD_init(LCD_ADDR, LCD_SDA_PIN, LCD_SCL_PIN, LCD_COLS, LCD_ROWS);
                LCD_home();
                LCD_clearScreen();
                LCD_setCursor(0,0);
                LCD_writeStr("Starting");
                vTaskDelay(pdMS_TO_TICKS(500));
                ESP_LOGI(TAG, "initialising screen finished");

                ESP_LOGI(TAG, "initialising buttons");
                LCD_Write_screen("initialising", "Buttons");
                rtc_gpio_hold_dis(BUTTON_0_GPIO);
                Button_Init(BUTTON_0_GPIO, BUTTON_1_GPIO);

                ESP_LOGI(TAG, "Initialising Memory");
                LCD_Write_screen("initialising", "Memory");
                if (nvs_flash_init() != ESP_OK){
                    ESP_LOGI(TAG, "memory init failed");
                    LCD_Write_screen("Memory init", "Failed, Erasing");
                    nvs_flash_erase();
                    nvs_flash_init();
                    vTaskDelay(pdMS_TO_TICKS(1500));
                }
                else{
                    if (NV_read_globvars() != ESP_OK){
                        LCD_Write_screen("Failed", "Loading settings");
                        vTaskDelay(pdMS_TO_TICKS(1500));
                    }
                }

                ESP_LOGI(TAG, "Initialising Thermometer");
                LCD_Write_screen("initialising", "Thermometer");
                DHT11_init(TERMOMETER_PIN);
                thermometer_reading = DHT11_read();
                if (thermometer_reading.status == DHT11_OK){

                    sprintf(LCD_row1_buf, "Temp: %d C", thermometer_reading.temperature);
                    sprintf(LCD_row2_buf, "Humid: %d ", thermometer_reading.humidity);
                    LCD_Write_screen(LCD_row1_buf, LCD_row2_buf);
                }
                else{
                    ESP_LOGI(TAG, "DHT11 status: %d", thermometer_reading.status);
                    LCD_Write_screen("Thermometer init", "Failed");
                    state = WAIT_FOR_RESET;
                }

                vTaskDelay(pdMS_TO_TICKS(500));

                ESP_LOGI(TAG, "Initialising Tensometer");
                LCD_Write_screen("initialising", "Tensometer");
                if (tensometer_init() == ESP_OK){
                    tensometer_reading = tensometer_read_average();
                    sprintf(LCD_row1_buf, "%d g", (int)tensometer_reading);
                    LCD_Write_screen("Weight: ", LCD_row1_buf);
                }
                else{
                    ESP_LOGI(TAG, "Tensometer init failed");
                    LCD_Write_screen("Tensometer init", "Failed");
                    state = WAIT_FOR_RESET;
                }

                vTaskDelay(pdMS_TO_TICKS(500));

                ESP_LOGI(TAG, "initialising Sim800l");
                LCD_Write_screen("initialising", "GSM");
                switch (gsm_init(USED_UART, GSM_TX_PIN, GSM_RX_PIN, GSM_RESPONSE_BUFFER_SIZE, response_buffer)){
                    case GSM_OK:
                    ESP_LOGI(TAG, "GSM ok");

                    LCD_Write_screen("Downloading", "Time");
                    vTaskDelay(pdMS_TO_TICKS(500));
                    if (synchronise_clock() == ESP_FAIL){
                        LCD_Write_screen("Time Sync", "ERROR");
                        vTaskDelay(pdMS_TO_TICKS(5000));
                    }

                    LCD_Write_screen("Sending", "Measurements");
                    timeinfo = updateTime();
                    if (send_measurements() == ESP_FAIL){
                        LCD_Write_screen("Resending data", "again");
                        send_measurements();
                    }

                    vTaskDelay(pdMS_TO_TICKS(1000));

                    if(eButton_Read(BUTTON_0_GPIO) == PRESSED){
                        state = TIME_SCREEN;
                        break;
                    }
                    else if (eButton_Read(BUTTON_1_GPIO) == PRESSED){
                        state = TIME_SCREEN;
                        break;
                    }
                    else if (wakeup_interval == NOT_SET){
                        state = TIME_SCREEN;
                        break;
                    }
                    else{
                        LCD_Write_screen("Going to", "Sleep");
                        vTaskDelay(pdMS_TO_TICKS(1000));
                        goto_sleep();
                    }

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
                break;

            case TIME_SCREEN:

                timeinfo = updateTime();

                sprintf(LCD_row1_buf, "Time: %02d:%02d:%02d", timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
                sprintf(LCD_row2_buf, "Date: %02d-%02d-%02d", timeinfo.tm_mday, timeinfo.tm_mon, timeinfo.tm_year-100);

                LCD_Write_screen(LCD_row1_buf, LCD_row2_buf);

                if (eButton_Read(BUTTON_0) == PRESSED){
                    state = WEIGHT_SCREEN;
                    break;
                }
                else{
                    state = TIME_SCREEN;
                }

                if (eButton_Read(BUTTON_1) == PRESSED){
                    LCD_Write_screen("Downloading", "Time");
                    vTaskDelay(pdMS_TO_TICKS(500));
                    if (synchronise_clock() == ESP_FAIL){
                        LCD_Write_screen("Time Sync", "ERROR");
                        state = WAIT_FOR_RESET;
                        break;
                    }
                }
                else{
                    state = TIME_SCREEN;
                }


                vTaskDelay(pdMS_TO_TICKS(200));

            break;      

            case WEIGHT_SCREEN:
                tensometer_reading = tensometer_read_average();
                sprintf(LCD_row1_buf, "%d g", (int)tensometer_reading);
                LCD_Write_screen("Weight: ", LCD_row1_buf);

                if (eButton_Read(BUTTON_0) == PRESSED){
                    state = TEMP_SCREEN;
                    break;
                }
                else{
                    state = WEIGHT_SCREEN;
                }

                if (eButton_Read(BUTTON_1) == PRESSED){
                    tensometer_set_offset();
                    ESP_LOGI(TAG, "scale_offset: %" PRIi32 "\n", scale_offset);
                }
                else{
                    state = WEIGHT_SCREEN;
                }

                //vTaskDelay(pdMS_TO_TICKS(500));

            break;

            case TEMP_SCREEN:

                thermometer_reading = DHT11_read();
                if (thermometer_reading.status == DHT11_OK){
                    sprintf(LCD_row1_buf, "Temp: %d C", thermometer_reading.temperature);
                    sprintf(LCD_row2_buf, "Humid: %d %%", thermometer_reading.humidity);
                    LCD_Write_screen(LCD_row1_buf, LCD_row2_buf);
                }
                else{
                    ESP_LOGI(TAG, "DHT11 status: %d", thermometer_reading.status);
                    LCD_Write_screen("Thermometer init", "Failed");
                    state = WAIT_FOR_RESET;
                }

                if (eButton_Read(BUTTON_0) == PRESSED){
                    state = MEASUREMENT;
                    break;
                }
                else{
                    state = TEMP_SCREEN;
                }

                if (eButton_Read(BUTTON_1) == PRESSED){
                    state = TEMP_SCREEN;
                    break;
                }
                else{
                    state = TEMP_SCREEN;
                }

                vTaskDelay(pdMS_TO_TICKS(200));

            break;        

            case MEASUREMENT:
                LCD_Write_screen("Manual", "Send?");

                if (eButton_Read(BUTTON_0) == PRESSED){
                    state = SLEEP_SETTINGS;
                    break;
                }
                else{
                    state = MEASUREMENT;
                }
                
                if (eButton_Read(BUTTON_1) == PRESSED){
                    LCD_Write_screen("Sending", "Data");
                    vTaskDelay(pdMS_TO_TICKS(500));
                    send_measurements();
                }

                state = MEASUREMENT;
                vTaskDelay(pdMS_TO_TICKS(200));
            break;

            
            case SLEEP_SETTINGS:

                switch(wakeup_interval){
                    case NOT_SET:
                    LCD_Write_screen("Wakeup Every", "not set");
                    if (eButton_Read(BUTTON_1) == PRESSED){
                        wakeup_interval = THIRTY_SEC_us;
                        break;
                    }
                    break;


                    case THIRTY_SEC_us:
                    LCD_Write_screen("Wakeup Every", "30 seconds");
                    if (eButton_Read(BUTTON_1) == PRESSED){
                        wakeup_interval = ONE_HOUR_us;
                        break;
                    }
                    break;

                    case ONE_HOUR_us:
                    LCD_Write_screen("Wakeup Every", "1 hour");
                    if (eButton_Read(BUTTON_1) == PRESSED){
                        wakeup_interval = SIX_HOURS_us;
                        break;
                    }
                    break;

                    case SIX_HOURS_us:
                    LCD_Write_screen("Wakeup Every", "6 hours");
                    if (eButton_Read(BUTTON_1) == PRESSED){
                        wakeup_interval = TWELVE_HOURS_us;
                        break;
                    }
                    break;

                    case TWELVE_HOURS_us:
                    LCD_Write_screen("Wakeup Every", "12 hours");
                    if (eButton_Read(BUTTON_1) == PRESSED){
                        wakeup_interval = TWENTYFOUR_HOURS_us;
                        break;
                    }
                    break;

                    case TWENTYFOUR_HOURS_us:
                    LCD_Write_screen("Wakeup Every", "24 hours");
                    if (eButton_Read(BUTTON_1) == PRESSED){
                        wakeup_interval = NOT_SET;
                        break;
                    }
                    break;
                    
                }
                

                if (eButton_Read(BUTTON_0) == PRESSED){
                    state = SLEEP;
                    break;
                }
                else{
                    state = SLEEP_SETTINGS;
                }

                state = SLEEP_SETTINGS;
                vTaskDelay(pdMS_TO_TICKS(200));
            break;


                case SLEEP:
                LCD_Write_screen("Sleep", "Mode?");

                if (eButton_Read(BUTTON_0) == PRESSED){
                    state = TIME_SCREEN;
                    break;
                }
                else{
                    state = SLEEP;
                }
                
                if (eButton_Read(BUTTON_1) == PRESSED){
                    LCD_Write_screen("Sleep", "Active");
                    vTaskDelay(pdMS_TO_TICKS(500));
                    goto_sleep();
                }

                state = SLEEP;
                vTaskDelay(pdMS_TO_TICKS(200));
            break;

            case WAIT_FOR_RESET:

            state = WAIT_FOR_RESET;

            vTaskDelay(pdMS_TO_TICKS(2000));
            LCD_Write_screen("Reseting", "");
            NV_write_globvars();
            vTaskDelay(pdMS_TO_TICKS(2000));
            esp_restart();

            break;


        }
    }
}