#ifndef H_SIM800
#define H_SIM800

#include <driver/uart.h>
#include <esp_err.h>
#include <string.h>
#include <esp_log.h>


#define IS_READY                    "AT+CPAS\n"
#define CHECK_NET_REG               "AT+CGREG?\n"
#define CHECK_NET_CONN              "AT+CGATT\n"
#define GET_SIG_LEVEL               "AT+CSQ\n"
#define IS_PASS_REQUIRED            "AT+CPIN?\n"
#define SET_EXTENDED_ERROR_REPORT   "AT+CMEE=2\n"
#define LIST_NETWORK_OPERATORS      "AT+COPS=?\n"
#define IS_REGISTERED               "AT+CREG?\n"
#define CALL                        "ATD+"

esp_err_t gsm_init(uart_port_t uart_port, uint tx_pin, uint rx_pin, uint rx_buffer_size, uint8_t* response_buffer);
uint16_t gsm_send_command(char* command);
esp_err_t gsm_call(const char* phone_number);


#endif //H_SIM800
