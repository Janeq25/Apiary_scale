#include <stdio.h>
#include <esp_err.h>
#include <inttypes.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>


#include "HX711/hx711_lib.h"
#include "DHT11/dht_espidf.h"

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

// ------------------------------------------------ thermometer ------------------------------------------------

void thermometer_read(){
    read_dht_sensor_data((gpio_num_t)TERMOMETER_PIN, DHT11, &thermometer_data);


}

// ------------------------------------------------ main ------------------------------------------------




void app_main() {

   // ------------------------------------------------ initializations ------------------------------------------------

    tensometer_init();
    

    while(1){

        //printf("tensometer data: %" PRIi32 "\n", tensometer_read_average());
        //printf("tensometer_raw data: %" PRIi32 "\n", tensometer_read_once());
        thermometer_read();
        printf("thermometer - temp: %lf, humid: %lf \n",thermometer_data.temperature, thermometer_data.humidity);
        vTaskDelay(pdMS_TO_TICKS(1000));

    }
}