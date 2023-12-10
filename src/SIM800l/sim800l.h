#ifndef H_SIM800
#define H_SIM800

#include <driver/uart.h>
#include <esp_err.h>
#include <string.h>
#include <esp_log.h>

typedef const char * gsm_command_t;
typedef char * gsm_response_t;

extern gsm_command_t IS_READY;
extern gsm_command_t CHECK_NET_REG;
extern gsm_command_t CHECK_NET_CONN; 
extern gsm_command_t GET_SIG_LEVEL;
extern gsm_command_t IS_PASS_REQUIRED;
extern gsm_command_t SET_EXTENDED_ERROR_REPORT;
extern gsm_command_t CALL;
extern gsm_command_t LIST_NETWORK_OPERATORS;
extern gsm_command_t IS_REGISTERED;

esp_err_t gsm_init(uart_port_t uart_port, uint tx_pin, uint rx_pin, uint rx_buffer_size, uint8_t* response_buffer);
uint16_t gsm_send_command(char* command);
esp_err_t gsm_call(const char* phone_number);


#endif //H_SIM800
