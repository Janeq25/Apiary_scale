#ifndef H_SIM800
#define H_SIM800

#include <driver/uart.h>
#include <esp_err.h>
#include <string.h>
#include <esp_log.h>

typedef const char * gsm_command_t;

extern gsm_command_t IS_READY;
extern gsm_command_t CHECK_NET_REG;
extern gsm_command_t CHECK_NET_CONN; 
extern gsm_command_t GET_SIG_LEVEL;
extern gsm_command_t IS_PASS_REQUIRED;

esp_err_t gsm_init(uart_port_t uart_port, int tx_pin, int rx_pin);
esp_err_t gsm_send_command(gsm_command_t command);


#endif //H_SIM800