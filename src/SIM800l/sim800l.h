#ifndef H_SIM800
#define H_SIM800

#include <driver/uart.h>
#include <esp_err.h>
#include <string.h>
#include <esp_log.h>

#define MAX_SMS_LENGTH 100
#define MAX_HTTP_REQUEST_LENGTH 128
#define MAX_HTTP_URL_LENGTH 128

#define ACCESS_POINT_NAME "fast.t-mobile.com"

#define GSM_IS_READY                    "AT+CPAS\n"
#define GSM_CHECK_NET_REG               "AT+CGREG?\n"
#define GSM_CHECK_NET_CONN              "AT+CGATT\n"
#define GSM_GET_SIG_LEVEL               "AT+CSQ\n"
#define GSM_IS_PASS_REQUIRED            "AT+CPIN?\n"
#define GSM_SET_EXTENDED_ERROR_REPORT   "AT+CMEE=2\n"
#define GSM_LIST_NETWORK_OPERATORS      "AT+COPS=?\n"
#define GSM_IS_REGISTERED               "AT+CREG?\n"
#define GSM_CALL                        "ATD" 
#define GSM_ENTER_TEXT_MODE             "AT+CMGF=1"
#define GSM_EXIT_TEXT_MODE              "AT+CMGF=0\n"
#define GSM_SMS_COMMAND                 "AT+CMGS="
#define GSM_SET_GPRS                    "AT+SAPBR=3,1,\"Contype\",\"GPRS\"\n"
#define GSM_SET_APN                     "AT+SAPBR=3,1,\"APN\"," "\"" ACCESS_POINT_NAME "\"\n" 
#define GSM_START_GPRS                  "AT+SAPBR=2,1\n"
#define GSM_HTTP_INIT                   "AT+HTTPINIT\n"
#define GSM_HTTP_SESSION_PARAMS         "AT+HTTPPARA=\"CID\",1\n"
#define GSM_HTTP_URL                    "AT+HTTPPARA=\"URL\"," // \"http://google.com/\"
#define GSM_HTTP_CONTENT                "AT+HTTPPARA=\"CONTENT\"," // \"application/json/\"
#define GSM_INITIATE_HTTP_REQUEST       "AT+HTTPACTION=0\n"
#define GSM_READ_HTTP_RESPONSE          "AT+HTTPREAD\n"
#define GSM_TERMINATE_HTTP_SETVICE      "AT+CIPSHUT\n"
#define GSM_STOP_GPRS                   "AT+SAPBR=0,1\n"


esp_err_t gsm_init(uart_port_t uart_port, uint tx_pin, uint rx_pin, uint rx_buffer_size, uint8_t* response_buffer);
uint16_t gsm_send_command(char* command, uint ms_to_wait);
esp_err_t gsm_call(const char* phone_number);
esp_err_t gsm_send_sms(const char* phone_number, const char* contents);
esp_err_t send_send_POST_request(char* url, char* request);
esp_err_t send_send_GET_request(char* url, char* request, char* GET_request_response_buffer, size_t timeout);


#endif //H_SIM800
