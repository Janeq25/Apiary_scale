#include "sim800l.h"

static const char* TAG = "GSM";
static uart_port_t used_uart;

gsm_command_t IS_READY = "AT+CPAS\n";
gsm_command_t CHECK_NET_REG = "AT+CGREG?\n";
gsm_command_t CHECK_NET_CONN = "AT+CGATT\n";
gsm_command_t GET_SIG_LEVEL = "AT+CSQ\n";
gsm_command_t IS_PASS_REQUIRED = "AT+CPIN?\n";


esp_err_t gsm_init(uart_port_t uart_port, int tx_pin, int rx_pin){
    used_uart = uart_port;
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



esp_err_t gsm_send_command(gsm_command_t command){
    uint8_t data[128] = {0};
    size_t received_length = 0;


    
    uart_write_bytes(used_uart, command, strlen(command));
    uart_wait_tx_done(used_uart, 100);

    vTaskDelay(pdMS_TO_TICKS(5));

    uart_get_buffered_data_len(used_uart, &received_length);
    uart_read_bytes(used_uart, &data, 1024, 100);

    ESP_LOGI(TAG, "request: %s, response: %s", command, data);

    return ESP_OK;
}