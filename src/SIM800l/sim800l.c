#include "sim800l.h"

static const char* TAG = "GSM";
static uart_port_t used_uart;
static uint8_t* response_buf;
size_t response_buf_size;



gsm_command_t IS_READY = "AT+CPAS\n";
gsm_command_t CHECK_NET_REG = "AT+CGREG?\n";
gsm_command_t CHECK_NET_CONN = "AT+CGATT\n";
gsm_command_t GET_SIG_LEVEL = "AT+CSQ\n";
gsm_command_t IS_PASS_REQUIRED = "AT+CPIN?\n";
gsm_command_t SET_EXTENDED_ERROR_REPORT = "AT+CMEE=2\n";
gsm_command_t CALL = "ATC\n";
gsm_command_t LIST_NETWORK_OPERATORS = "AT+COPS=?\n";
gsm_command_t IS_REGISTERED = "AT+CREG?\n";


esp_err_t gsm_init(uart_port_t uart_port, uint tx_pin, uint rx_pin, uint rx_buffer_size, uint8_t* response_buffer){
    used_uart = uart_port;
    response_buf_size = rx_buffer_size;
    response_buf = response_buffer;
    uart_config_t uart_config = {
        .baud_rate = 9600,
        .data_bits = UART_DATA_8_BITS,
        .parity    = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_DEFAULT,
    };
    // Configure UART parameters
    ESP_ERROR_CHECK(uart_driver_install(used_uart, 1024, 0, 0, NULL, 0));
    ESP_ERROR_CHECK(uart_param_config(used_uart, &uart_config));
    ESP_ERROR_CHECK(uart_set_pin(used_uart, tx_pin, rx_pin, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE));

    ESP_LOGI(TAG, "set sim800l connection");

    return ESP_OK;
}



uint16_t gsm_send_command(char* command){
    memset(response_buf, 0, response_buf_size);
    size_t received_length = 0;


    
    if (uart_write_bytes(used_uart, command, strlen(command)) == -1){
        ESP_LOGI(TAG, "uart write failed on command: %s", command);
        return -1;
    }

    if (uart_wait_tx_done(used_uart, pdMS_TO_TICKS(1000)) == ESP_FAIL){
        ESP_LOGI(TAG, "uart write timed out");
        return -1;
    }

    vTaskDelay(pdMS_TO_TICKS(10));

    received_length = uart_read_bytes(used_uart, response_buf, 1024, 100);
    if (received_length == -1){
        ESP_LOGI(TAG, "uart read failed");
        return -1;
    }
    // else if (received_length == 0){
    //     ESP_LOGW(TAG, "invalid command");
    //     return -1;
    // }
     
    

    //ESP_LOGI(TAG, "request: %s, response: %s", command, response_buf);
    printf("GSM: request: %s, response: %s", command, response_buf);

    return received_length;
}

esp_err_t gsm_call(const char* phone_number){

    char message[128] = {0};

    strcat(message, "ATD+");
    strcat(message, "731335526");
    strcat(message, ";\n");

    gsm_send_command(message);

    return ESP_OK;
}