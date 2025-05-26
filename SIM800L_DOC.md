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

## AT Command Sequence Explanation

### Initialization (`gsm_init`)
1. **UART Setup:** Configure UART for SIM800L communication.
2. **Module Ready:** `AT` - Check if module responds.
3. **Module Reset:** `AT+CFUN=1,1` - Reset the module.
4. **Extended Error Reporting:** `AT+CMEE=2` - Enable verbose error messages (debug).
5. **Disable Echo:** `ATE0` - Disable command echo.
6. **GPRS Enable:**
    - `AT+SAPBR=3,1,"Contype","GPRS"` - Set bearer profile to GPRS.
    - `AT+SAPBR=3,1,"APN","internet"` - Set APN.
    - `AT+SAPBR=3,1,"USER","internet"` - Set APN user.
    - `AT+SAPBR=3,1,"PWD","internet"` - Set APN password.
    - `AT+SAPBR=1,1` - Open GPRS context.
    - `AT+SAPBR=2,1` - Query GPRS context IP.
7. **NTP Time Sync:**
    - `AT+CNTPCID=1` - Set NTP profile.
    - `AT+CNTP="pool.ntp.org",8` - Set NTP server.
    - `AT+CNTP` - Start NTP sync.

### Status Check (`gsm_get_status`)
- `AT` - Check module.
- `AT+CSMINS?` - SIM status.
- `AT+CPIN?` - PIN status.
- `AT+CSQ` - Signal quality.
- `AT+CGATT?` - GPRS attach status.
- `AT+CREG?` - Network registration.

### HTTP GET Request (`gsm_send_http_request`)
1. `AT+HTTPINIT` - Initialize HTTP service.
2. `AT+HTTPPARA="CID",1` - Set HTTP bearer profile.
3. `AT+HTTPPARA="URL","..."` - Set URL.
4. `AT+HTTPACTION=0` - Start GET request.
5. `AT+HTTPREAD` - Read response.
6. `AT+HTTPTERM` - Terminate HTTP service.

### HTTP POST Request (`gsm_send_http_request_post`)
1. `AT+HTTPINIT` - Initialize HTTP service.
2. `AT+HTTPPARA="CID",1` - Set HTTP bearer profile.
3. `AT+HTTPPARA="URL","..."` - Set URL.
4. `AT+HTTPPARA=CONTENT,application/json` - Set content type.
5. `AT+HTTPDATA=1024,1000` - Prepare to send data.
6. Send data payload.
7. `AT+HTTPACTION=1` - Start POST request.
8. `AT+HTTPREAD` - Read response.
9. `AT+HTTPTERM` - Terminate HTTP service.

### Time Synchronization (`gsm_get_time`)
- `AT+CCLK?` - Get current time from module (after NTP sync).

### Sleep Mode (`gsm_enable_sleep`)
- `AT+SAPBR=0,1` - Close GPRS context.
- `AT+CSCLK=2` - Enable sleep mode.

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

## Notes
- APN, user, and password are set for Orange network by default. Change in `sim800l.h` if needed.
- All functions return a `gsm_err_t` error code for status checking.
- Debug output is enabled by default (`GSM_DEBUG`).

---
For more details, see the comments and implementation in `sim800l.c` and `sim800l.h`.
