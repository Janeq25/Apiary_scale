#ifndef H_SIM800
#define H_SIM800

#include <driver/uart.h>
#include <esp_err.h>
#include <string.h>
#include <esp_log.h>

#define GSM_DEBUG
//#define ALLOW_REDIRECTIONS


#define MAX_HTTP_URL_LENGTH 200
#define MAX_HTTP_REQUEST_LENGTH 128


// // Play Config
// #define APN_NAME "internet"
// #define APN_USER ""
// #define APN_PASS ""

//Orange Config
#define APN_NAME "internet"
#define APN_USER "internet"
#define APN_PASS "internet"

#define GSM_COMMAND_IS_READY                    "AT\n\r"
#define GSM_COMMAND_SIM_GSM_STATUS              "AT+CSMINS?\n\r"
#define GSM_COMMAND_CHECK_NET_REG               "AT+CGREG?\n\r"
#define GSM_COMMAND_CHECK_NET_CONN              "AT+CGATT?\n\r"
#define GSM_COMMAND_ACTIVATE_PDP_CONTEXT        "AT+CGACT=1,1\n\r"
#define GSM_COMMAND_GET_SIG_LEVEL               "AT+CSQ\n\r"
#define GSM_COMMAND_IS_PASS_REQUIRED            "AT+CPIN?\n\r"
#define GSM_COMMAND_ENTER_PIN                   "AT+CPIN="
#define GSM_COMMAND_SET_EXTENDED_ERROR_REPORT   "AT+CMEE=2\n\r"
#define GSM_COMMAND_LIST_NETWORK_OPERATORS      "AT+COPS=?\n\r"
#define GSM_COMMAND_IS_REGISTERED               "AT+CREG?\n\r"
#define GSM_COMMAND_RESET                       "AT+CFUN=1,1\n\r"
#define GSM_ENABLE_SLEEP_MODE                   "AT+CSCLK=2\n\r"
#define GSM_ENABLE_ECHO                         "ATE1\n\r"
#define GSM_DISABLE_ECHO                        "ATE0\n\r"
#define GSM_COMMAND_ENABLE_TIME_SYNC            "AT+CLTS=1\n\r"
#define GSM_COMMAND_GET_TIME_SYNC               "AT+CCLK?\n\r"
#define GSM_CNTP                                "AT+CNTPCID=1\r\n"
#define GSM_CNTP_SERVER                         "AT+CNTP=\"pool.ntp.org\",8\r\n"
#define GSM_CNTP_SYNC                           "AT+CNTP\r\n"


//#define GSM_COMMAND_SET_APN                     "AT+CSTT=\"" APN_NAME "\",\"" APN_USER "\",\"" APN_PASS "\"\n\r"
// #define GSM_COMMAND_SET_APN                     "AT+CSTT=\"" APN_NAME "\"\n\r"
// #define GSM_COMMAND_GPRS_OR_CSD                 "AT+CIICR\n\r"

#define GSM_COMMAND_STOP_GPRS                   "AT+SAPBR=0,1\n\r"
#define GSM_COMMAND_SET_GPRS                    "AT+SAPBR=3,1,\"Contype\",\"GPRS\"\n\r"
#define GSM_COMMAND_SET_APN                     "AT+SAPBR=3,1,\"APN\"," "\"" APN_NAME "\"\n\r" 
#define GSM_COMMAND_SET_APN_USER                "AT+SAPBR=3,1,\"USER\"," "\"" APN_USER "\"\n\r" 
#define GSM_COMMAND_SET_APN_PASS                "AT+SAPBR=3,1,\"PWD\"," "\"" APN_PASS "\"\n\r" 
#define GSM_COMMAND_START_GPRS                  "AT+SAPBR=1,1\n\r"
#define GSM_COMMAND_GET_IP                      "AT+SAPBR=2,1\n\r"
#define GSM_COMMAND_HTTP_INIT                   "AT+HTTPINIT\n\r"
#define GSM_COMMAND_HTTP_SESSION_PARAMS         "AT+HTTPPARA=\"CID\",1\n\r"
#define GSM_COMMAND_HTTP_URL                    "AT+HTTPPARA=\"URL\"," // \"http://google.com/\"
#define GSM_COMMAND_HTTP_CONTENT                "AT+HTTPPARA=CONTENT,application/json\n\r"
#define GSM_COMMAND_ALLOW_REDIRECT              "AT+HTTPPARA=\"REDIR\",1\n\r"
#define GSM_COMMAND_ENABLE_SSL                  "AT+HTTPSSL=1\n\r"
#define GSM_COMMAND_DISABLE_SSL                 "AT+SSLOPT=0,1\n\r"
#define GSM_COMMAND_HTTP_ACTION_GET             "AT+HTTPACTION=0\n\r"
#define GSM_COMMAND_READ_HTTP_RESPONSE          "AT+HTTPREAD\n\r"
#define GSM_COMMAND_TERMINATE_HTTP_SETVICE      "AT+HTTPTERM\n\r"
#define GSM_COMMAND_HTTP_ACTION_POST            "AT+HTTPACTION=1\n\r"
#define GSM_COMMAND_HTTP_DATA                   "AT+HTTPDATA=1024,1000\n\r" // "2000,10000\n\r"
#define GSM_COMMAND_GSM_SET_TIMEZONE            "AT+CTZR=4\n\r"


// #define GSM_COMMAND_CALL                        "ATD" 
// #define GSM_COMMAND_ENTER_TEXT_MODE             "AT+CMGF=1"
// #define GSM_COMMAND_EXIT_TEXT_MODE              "AT+CMGF=0\n\r"
// #define GSM_COMMAND_SMS_COMMAND                 "AT+CMGS="


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

gsm_err_t gsm_init(uart_port_t uart_port, uint8_t tx_pin, uint8_t rx_pin, uint8_t rx_buffer_size, char* response_buffer);
gsm_err_t gsm_get_status();
gsm_err_t gsm_enter_pin(const char* pin);
gsm_err_t gsm_send_command(char* command, uint16_t ms_to_wait);
gsm_err_t gsm_send_http_request(char* url, char* GET_request_response_buffer, size_t timeout);
gsm_err_t gsm_send_http_request_post(char* url, char* data, char* GET_request_response_buffer, size_t timeout);
gsm_err_t gsm_enable_sleep();
gsm_err_t gsm_get_time(char* time_buffer);


// gsm_err_t gsm_call(const char* phone_number);
// gsm_err_t gsm_send_sms(const char* phone_number, const char* contents);
// gsm_err_t send_send_POST_request(char* url, char* request);
// gsm_err_t send_send_GET_request(char* url, char* request, char* GET_request_response_buffer, size_t timeout);


#endif //H_SIM800
