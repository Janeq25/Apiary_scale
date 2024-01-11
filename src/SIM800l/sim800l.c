#include "sim800l.h"

static const char* TAG = "GSM";
static uart_port_t used_uart;
static char* response_buf;
size_t response_buf_size;

gsm_err_t status = GSM_ERR_MODULE_NOT_CONNECTED;



gsm_err_t gsm_init(uart_port_t uart_port, uint tx_pin, uint rx_pin, uint rx_buffer_size, char* response_buffer){
    used_uart = uart_port;
    response_buf_size = rx_buffer_size;
    response_buf = response_buffer;
    uart_config_t uart_config = {
        .baud_rate = 4800,
        .data_bits = UART_DATA_8_BITS,
        .parity    = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_DEFAULT,
    };
    // Configure UART parameters
    if (uart_driver_install(used_uart, 1024, 0, 0, NULL, 0) == ESP_FAIL){
        ESP_LOGI(TAG, "failed to initialise uart");
        status = GSM_ERR_MODULE_NOT_CONNECTED;
        return GSM_ERR_MODULE_NOT_CONNECTED;
    }

        if (uart_param_config(used_uart, &uart_config) == ESP_FAIL){
        ESP_LOGI(TAG, "failed to initialise uart");
        status = GSM_ERR_MODULE_NOT_CONNECTED;
        return GSM_ERR_MODULE_NOT_CONNECTED;
    }

        if (uart_set_pin(used_uart, tx_pin, rx_pin, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE) == ESP_FAIL){
        ESP_LOGI(TAG, "failed to initialise uart");
        status = GSM_ERR_MODULE_NOT_CONNECTED;
        return GSM_ERR_MODULE_NOT_CONNECTED;
    }


    #ifdef GSM_DEBUG
        if (gsm_send_command(GSM_COMMAND_SET_EXTENDED_ERROR_REPORT, 100) != GSM_OK) return status;
        if (strstr(response_buf, "OK") == 0){
        ESP_LOGI(TAG, "set verbose responses failed");
        status = GSM_FAIL;
        return GSM_FAIL;
        }
    #endif

    ESP_LOGI(TAG, "initialized sim800l uart connection");

    return GSM_OK;
}

gsm_err_t gsm_send_command(char* command, uint ms_to_wait){
    memset(response_buf, 0, response_buf_size);
    size_t received_length = 0;

    #ifdef GSM_DEBUG
        ESP_LOGI(TAG, "waiting %u_ms, sending request: %s", ms_to_wait, command);
    #endif


    if (uart_write_bytes(used_uart, command, strlen(command)) == -1){
        ESP_LOGI(TAG, "uart write failed on command: %s", command);
        status = GSM_ERR_MODULE_NOT_CONNECTED;
        return GSM_ERR_MODULE_NOT_CONNECTED;
    }

    if (uart_wait_tx_done(used_uart, pdMS_TO_TICKS(1000)) == ESP_FAIL){
        ESP_LOGI(TAG, "uart write timed out");
        status = GSM_ERR_MODULE_NOT_CONNECTED;
        return GSM_ERR_MODULE_NOT_CONNECTED;
    }

    vTaskDelay(pdMS_TO_TICKS(100));

    received_length = uart_read_bytes(used_uart, (uint8_t*)response_buf, 1024, pdMS_TO_TICKS(ms_to_wait));
    if (received_length == -1){
        ESP_LOGI(TAG, "uart read failed");
        status = GSM_ERR_MODULE_NOT_CONNECTED;
        return GSM_ERR_MODULE_NOT_CONNECTED;
    }
    // else if (received_length == 0){
    //     ESP_LOGW(TAG, "invalid command");
    //     return -1;
    // }
     
    
    #ifdef GSM_DEBUG
        ESP_LOGI(TAG, "response: %s", response_buf);
    #endif

    return GSM_OK;
}

gsm_err_t gsm_connection_status(){
    
    if (gsm_send_command(GSM_COMMAND_SIM_STATUS, 100) != GSM_OK) return status;
    if (strstr(response_buf, "CSMINS: 0,1") == 0){
        ESP_LOGI(TAG, "sim not inserted");
        status = GSM_ERR_SIM_NOT_INSERTED;
        return GSM_ERR_SIM_NOT_INSERTED;
    }


    if (gsm_send_command(GSM_COMMAND_IS_PASS_REQUIRED, 100) != GSM_OK) return status;
    if (strstr(response_buf, "READY") == 0){
        ESP_LOGI(TAG, "pin required");
        status = GSM_ERR_PIN_REQUIRED;
        return GSM_ERR_PIN_REQUIRED;
    }

    if (gsm_send_command(GSM_COMMAND_GET_SIG_LEVEL, 100) != GSM_OK) return status;
    if (strstr(response_buf, "CSQ: 0,0") != 0){
        ESP_LOGI(TAG, "no signal");
        status = GSM_ERR_NO_SIGNAL;
        return GSM_ERR_NO_SIGNAL;
    }

    
    if (gsm_send_command(GSM_COMMAND_CHECK_NET_CONN, 100) != GSM_OK) return status;
    if (strstr(response_buf, "CGATT: 0") != 0){
        ESP_LOGI(TAG, "not attached gprs service");
        status = GSM_ERR_NOT_ATTACHED_GPRS_SERV;
        return GSM_ERR_NOT_ATTACHED_GPRS_SERV;
    }


    if (gsm_send_command(GSM_COMMAND_IS_REGISTERED, 100) != GSM_OK) return status;
    if (strstr(response_buf, "CREG: 0,1") == 0){
        ESP_LOGI(TAG, "not registered in home network");
        status = GSM_ERR_NOT_REGISTERED_IN_NETWORK;
        return GSM_ERR_NOT_REGISTERED_IN_NETWORK;
    }

    return GSM_OK;
}

// requires testing
gsm_err_t gsm_enter_pin(const char* pin){
    char command[13] = GSM_COMMAND_ENTER_PIN;

    strcat(command, pin);
    strcat(command, "\n");
    if (gsm_send_command(command, 100) != GSM_OK) return status;
    if (strstr(response_buf, "OK") != 0){
        ESP_LOGI(TAG, "pin accepted");
        status = GSM_OK;
        return GSM_OK;
    }
    else{
        ESP_LOGI(TAG, "pin not accepted");
        status = GSM_ERR_PIN_REQUIRED;
        return GSM_ERR_PIN_REQUIRED;
    }

}

gsm_err_t gsm_send_http_request(char* url, char* request, char* GET_request_response_buffer, size_t timeout){

    if (gsm_connection_status() != GSM_OK) return status;

    
    if (gsm_send_command(GSM_COMMAND_SET_GPRS, 100) != GSM_OK) return status;
    if (strstr(response_buf, "OK") == 0){
        ESP_LOGI(TAG, "failed to initialise gprs mode");
        status = GSM_ERR_GPRS_FAILED;
        return GSM_ERR_GPRS_FAILED;
    }

    if (gsm_send_command(GSM_COMMAND_START_GPRS, 100) != GSM_OK) return status;
    if (strstr(response_buf, "OK") == 0){
        ESP_LOGI(TAG, "failed to initialise gprs mode");
        status = GSM_ERR_GPRS_FAILED;
        return GSM_ERR_GPRS_FAILED;
    }

    if (gsm_send_command(GSM_COMMAND_HTTP_INIT, 100) != GSM_OK) return status;
    if (strstr(response_buf, "OK") == 0){
        ESP_LOGI(TAG, "failed to initialise http mode");
        status = GSM_ERR_HTTP_FAILED;
        return GSM_ERR_HTTP_FAILED;
    }



    return GSM_OK;

}


// gsm_err_t send_send_GET_request(char* url, char* request, char* GET_request_response_buffer, size_t timeout){

//     char request_buffer[MAX_HTTP_REQUEST_LENGTH] = GSM_HTTP_CONTENT;

//     char url_command_buffer[MAX_HTTP_URL_LENGTH + 20] = GSM_HTTP_URL;

//     strcat(url_command_buffer, "\"");
//     strcat(url_command_buffer, url);
//     strcat(url_command_buffer, "\"\n");

//     printf("url: %s", url_command_buffer);

//     strcat(request_buffer, "\"");
//     strcat(request_buffer, request);
//     strcat(request_buffer, "\"\n");

//     printf("request: %s", request_buffer);


//     gsm_send_command(GSM_TERMINATE_HTTP_SETVICE, 1000);
//     //gsm_send_command(GSM_SET_GPRS, 1000);
//     //gsm_send_command(GSM_SET_APN, 5000);
//     gsm_send_command(GSM_START_GPRS, 5000);

//     gsm_send_command(GSM_HTTP_SESSION_PARAMS, 1000);
//     gsm_send_command(GSM_HTTP_INIT, 1000);

//     gsm_send_command(url_command_buffer, 10000);

//     gsm_send_command(request_buffer, 1000);

//     gsm_send_command(GSM_INITIATE_HTTP_REQUEST, timeout);
//     gsm_send_command(GSM_READ_HTTP_RESPONSE, 1000);
//     gsm_send_command(GSM_TERMINATE_HTTP_SETVICE, 1000);

//     gsm_send_command(GSM_STOP_GPRS, 1000);


//     return ESP_OK;
// }


// gsm_err_t gsm_call(const char* phone_number){

//     char message[128] = {0};

//     strcat(message, GSM_CALL);
//     strcat(message, phone_number);
//     strcat(message, ";\n");

//     gsm_send_command(message, 1000);

//     return ESP_OK;
// }

// gsm_err_t gsm_send_sms(const char* phone_number, const char* contents){

//     char message[20] = {0};
//     char sms_contents[MAX_SMS_LENGTH] = {0};
//     char end_of_sms = 26;

//     gsm_send_command(GSM_ENTER_TEXT_MODE, 100);
    
//     strcat(message, GSM_SMS_COMMAND);
//     strcat(message, "\"");
//     strcat(message, phone_number);
//     strcat(message, "\"\n");

//     gsm_send_command(message, 2000);

//     strcat(sms_contents, contents);
//     strcat(sms_contents, &end_of_sms);


//     gsm_send_command(sms_contents, 2000);

//     gsm_send_command(GSM_EXIT_TEXT_MODE, 100);

//     return ESP_OK;
// }