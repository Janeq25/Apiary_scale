#ifndef H_SIM800
#define H_SIM800

#include <driver/uart.h>
#include <esp_err.h>
#include <string.h>
#include <esp_log.h>

#define GSM_DEBUG

#define MAX_SMS_LENGTH 100
#define MAX_HTTP_REQUEST_LENGTH 128
#define MAX_HTTP_URL_LENGTH 128

#define ACCESS_POINT_NAME "fast.t-mobile.com"

#define GSM_COMMAND_IS_READY                    "AT\n"
#define GSM_COMMAND_SIM_STATUS                  "AT+CSMINS?\n"
#define GSM_COMMAND_CHECK_NET_REG               "AT+CGREG?\n"
#define GSM_COMMAND_CHECK_NET_CONN              "AT+CGATT?\n"
#define GSM_COMMAND_GET_SIG_LEVEL               "AT+CSQ\n"
#define GSM_COMMAND_IS_PASS_REQUIRED            "AT+CPIN?\n"
#define GSM_COMMAND_ENTER_PIN                   "AT+CPIN="
#define GSM_COMMAND_SET_EXTENDED_ERROR_REPORT   "AT+CMEE=2\n"
#define GSM_COMMAND_LIST_NETWORK_OPERATORS      "AT+COPS=?\n"
#define GSM_COMMAND_IS_REGISTERED               "AT+CREG?\n"
#define GSM_COMMAND_CALL                        "ATD" 
#define GSM_COMMAND_ENTER_TEXT_MODE             "AT+CMGF=1"
#define GSM_COMMAND_EXIT_TEXT_MODE              "AT+CMGF=0\n"
#define GSM_COMMAND_SMS_COMMAND                 "AT+CMGS="
#define GSM_COMMAND_SET_GPRS                    "AT+SAPBR=3,1,\"Contype\",\"GPRS\"\n"
#define GSM_COMMAND_SET_APN                     "AT+SAPBR=3,1,\"APN\"," "\"" ACCESS_POINT_NAME "\"\n" 
#define GSM_COMMAND_START_GPRS                  "AT+SAPBR=1,1\n"
#define GSM_COMMAND_HTTP_INIT                   "AT+HTTPINIT\n"
#define GSM_COMMAND_HTTP_SESSION_PARAMS         "AT+HTTPPARA=\"CID\",1\n"
#define GSM_COMMAND_HTTP_URL                    "AT+HTTPPARA=\"URL\"," // \"http://google.com/\"
#define GSM_COMMAND_HTTP_CONTENT                "AT+HTTPPARA=\"CONTENT\"," // \"application/json/\"
#define GSM_COMMAND_INITIATE_HTTP_REQUEST       "AT+HTTPACTION=0\n"
#define GSM_COMMAND_READ_HTTP_RESPONSE          "AT+HTTPREAD\n"
#define GSM_COMMAND_TERMINATE_HTTP_SETVICE      "AT+HTTPTERM\n"
#define GSM_COMMAND_STOP_GPRS                   "AT+SAPBR=0,1\n"


#define GSM_OK                              0
#define GSM_FAIL                            -1
#define GSM_ERR_PIN_REQUIRED                1
#define GSM_ERR_MODULE_NOT_CONNECTED        2
#define GSM_ERR_NO_SIGNAL                   3
#define GSM_ERR_SIM_NOT_INSERTED            4
#define GSM_ERR_NOT_ATTACHED_GPRS_SERV      5
#define GSM_ERR_HTTP_FAILED                 6
#define GSM_ERR_GPRS_FAILED                 7
#define GSM_ERR_NOT_REGISTERED_IN_NETWORK   8

typedef int gsm_err_t;

gsm_err_t gsm_init(uart_port_t uart_port, uint tx_pin, uint rx_pin, uint rx_buffer_size, char* response_buffer);
gsm_err_t gsm_connection_status();
gsm_err_t gsm_enter_pin(const char* pin);
gsm_err_t gsm_send_command(char* command, uint ms_to_wait);
gsm_err_t gsm_send_http_request(char* url, char* request, char* GET_request_response_buffer, size_t timeout);
// gsm_err_t gsm_call(const char* phone_number);
// gsm_err_t gsm_send_sms(const char* phone_number, const char* contents);
// gsm_err_t send_send_POST_request(char* url, char* request);
// gsm_err_t send_send_GET_request(char* url, char* request, char* GET_request_response_buffer, size_t timeout);


#endif //H_SIM800
