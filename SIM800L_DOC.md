# SIM800L Library Documentation

This library provides an interface for communicating with the SIM800L GSM/GPRS module using UART on ESP32. It allows for initialization, status checking, HTTP requests (GET/POST), time synchronization, and power management via AT commands.

## Features
- UART initialization and configuration for SIM800L
- GPRS and APN setup
- HTTP GET and POST requests
- Time synchronization using NTP
- Power management (sleep mode)
- Status and error handling

## Usage Example
```c
#include "SIM800l/sim800l.h"

char response_buffer[1024];

void app_main() {
    // Initialize SIM800L
    if (gsm_init(UART_NUM_2, 17, 16, 1024, response_buffer) != GSM_OK) {
        // Handle error
    }

    // Check status
    if (gsm_get_status() != GSM_OK) {
        // Handle error
    }

    // Send HTTP GET request
    char http_response[1024];
    if (gsm_send_http_request("http://example.com", http_response, 5000) == GSM_OK) {
        // Use http_response
    }

    // Send HTTP POST request
    char post_data[] = "{\"key\":\"value\"}";
    if (gsm_send_http_request_post("http://example.com", post_data, http_response, 5000) == GSM_OK) {
        // Use http_response
    }

    // Get time from network
    char time_buffer[64];
    if (gsm_get_time(time_buffer) == GSM_OK) {
        // Use time_buffer
    }

    // Enable sleep mode
    gsm_enable_sleep();
}
```

## AT Command Reference

See below for explanations of all AT commands used in this library:

### Basic Module Configuration

- **UART Setup**
  - Configures serial communication between microcontroller and SIM800L (typically 9600 or 115200 baud)
- **`AT`**
  - Basic "Attention" command to check if module is responsive
  - Expected response: "OK"
- **`AT+CFUN=1,1`**
  - Full functionality reset (1,1 = reset after setting)
  - Restarts the module completely
- **`AT+CMEE=2`**
  - Enables extended error reporting (verbose mode)
  - Provides detailed error codes for troubleshooting
- **`ATE0`**
  - Disables command echo (module won't repeat commands back)
  - Reduces serial traffic

### GPRS Configuration

- **`AT+SAPBR=3,1,"Contype","GPRS"`**
  - Sets Bearer Profile 1 connection type to GPRS
- **`AT+SAPBR=3,1,"APN","internet"`**
  - Sets APN (Access Point Name) for GPRS connection
  - Replace "internet" with your carrier's APN
- **`AT+SAPBR=3,1,"USER","internet"`**
  - Sets username for APN authentication (if required)
- **`AT+SAPBR=3,1,"PWD","internet"`**
  - Sets password for APN authentication (if required)
- **`AT+SAPBR=1,1`**
  - Opens GPRS context (activates data connection)
- **`AT+SAPBR=2,1`**
  - Queries GPRS context to get assigned IP address

### NTP Time Synchronization

- **`AT+CNTPCID=1`**
  - Sets NTP client to use Bearer Profile 1
- **`AT+CNTP="pool.ntp.org",8`**
  - Configures NTP server (pool.ntp.org) with timezone (+8)
- **`AT+CNTP`**
  - Initiates NTP time synchronization

### Status Check Commands

- **`AT+CSMINS?`**
  - Checks SIM card insertion status
- **`AT+CPIN?`**
  - Checks if PIN is required/verified
- **`AT+CSQ`**
  - Gets signal quality report (0-31, higher is better)
- **`AT+CGATT?`**
  - Checks GPRS attachment status (1=attached)
- **`AT+CREG?`**
  - Checks network registration status

### HTTP GET Request

- **`AT+HTTPINIT`**
  - Initializes HTTP service
- **`AT+HTTPPARA="CID",1`**
  - Sets HTTP to use Bearer Profile 1
- **`AT+HTTPPARA="URL","..."`**
  - Sets target URL for HTTP request
- **`AT+HTTPACTION=0`**
  - Executes HTTP GET request (0=GET)
- **`AT+HTTPREAD`**
  - Reads HTTP response data
- **`AT+HTTPTERM`**
  - Terminates HTTP service

### HTTP POST Request

- **`AT+HTTPPARA=CONTENT,application/json`**
  - Sets content type for POST request
- **`AT+HTTPDATA=1024,1000`**
  - Prepares to receive data (1024 bytes, 1000ms timeout)
  - After this command, module expects raw data
- **`AT+HTTPACTION=1`**
  - Executes HTTP POST request (1=POST)

### Time Management

- **`AT+CCLK?`**
  - Gets current time from module's RTC (after NTP sync)
  - Format: "yy/MM/dd,hh:mm:ss±zz"

### Sleep Mode

- **`AT+SAPBR=0,1`**
  - Closes GPRS context (disconnects data)
- **`AT+CSCLK=2`**
  - Enables sleep mode (2=enable with wakeup on UART activity)

### Notes
- All commands should end with CRLF (\r\n)
- Responses typically end with CRLF
- Timeouts vary (typically 1-5 seconds for operations)
- Error responses begin with "+CME ERROR:" when extended errors enabled
- APN settings vary by mobile network provider

## Error Codes
- `GSM_OK` (0): Success
- `GSM_FAIL` (-1): General failure
- `GSM_ERR_PIN_REQUIRED` (1): SIM PIN required
- `GSM_ERR_MODULE_NOT_CONNECTED` (2): Module not connected
- `GSM_ERR_NO_SIGNAL` (3): No GSM signal
- `GSM_ERR_SIM_NOT_INSERTED` (4): SIM not inserted
- `GSM_ERR_NOT_ATTACHED_GPRS_SERV` (5): Not attached to GPRS
- `GSM_ERR_HTTP_FAILED` (6): HTTP request failed
- `GSM_ERR_GPRS_FAILED` (7): GPRS setup failed
- `GSM_ERR_NOT_REGISTERED_IN_NETWORK` (8): Not registered in network

## Additional Notes
- APN, user, and password are set for Orange network by default. Change in `sim800l.h` if needed.
- All functions return a `gsm_err_t` error code for status checking.
- Debug output is enabled by default (`GSM_DEBUG`).

---
For more details, see the comments and implementation in `sim800l.c` and `sim800l.h`.
