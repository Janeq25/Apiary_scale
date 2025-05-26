# Remote Apiary Scale

This project is an embedded system for monitoring beehive conditions (weight, temperature, humidity) and sending the collected data to a remote server at specified intervals. It is designed for the ESP32 (NodeMCU-32S) platform using the ESP-IDF framework and PlatformIO.

## Features

- **Weight Measurement:** Uses an HX711 load cell amplifier to read the weight of the beehive.
- **Temperature & Humidity:** Uses a DHT11 sensor to measure environmental conditions.
- **Time Synchronization:** Synchronizes the device clock via GSM (SIM800L module).
- **Data Transmission:** Sends collected data to a remote server using HTTP POST requests over GSM.
- **User Interface:** 16x2 I2C LCD display for status and data presentation.
- **User Input:** Two physical buttons for navigation and configuration.
- **Power Management:** Supports deep sleep and configurable wakeup intervals for energy efficiency.
- **Non-Volatile Storage:** Stores calibration and configuration data in NVS (non-volatile storage).

## Hardware Connections

- **ESP32 Board:** NodeMCU-32S
- **Load Cell (HX711):** Pins 32 (SCK), 33 (DOUT)
- **DHT11 Sensor:** Pin 26
- **LCD Display:** I2C, SDA (14), SCL (27)
- **Buttons:** GPIO 0, GPIO 15
- **SIM800L GSM Module:** UART2, TX (17), RX (16)

## Main Components

### 1. Initialization (`app_main`)

- Initializes peripherals: LCD, buttons, NVS, DHT11, HX711, SIM800L.
- Reads configuration/calibration from NVS.
- Displays status and sensor readings on the LCD.
- Synchronizes time via GSM.
- Sends initial measurement to the server.
- Enters main state machine for user interaction and periodic operation.

### 2. State Machine

- **INIT:** System setup and first measurement.
- **TIME_SCREEN:** Shows current time/date.
- **WEIGHT_SCREEN:** Shows current weight.
- **TEMP_SCREEN:** Shows temperature and humidity.
- **MEASUREMENT:** Allows manual data sending.
- **SLEEP_SETTINGS:** Configure wakeup interval.
- **SLEEP:** Prepares for deep sleep.
- **WAIT_FOR_RESET:** Handles errors and restarts the device.

### 3. Sensor Handling

- **Weight:** Averaged readings, calibration offset, and scaling.
- **Temperature/Humidity:** Single reading per cycle, error handling for sensor failures.

### 4. Data Handling

- **Formatting:** JSON structure for server compatibility.
- **Timestamping:** Uses synchronized RTC time.
- **Transmission:** HTTP POST via SIM800L; handles server response and errors.

### 5. Power Management

- **Deep Sleep:** Configurable intervals (30s, 1h, 6h, 12h, 24h).
- **Wakeup Sources:** Button press or timer.

### 6. Non-Volatile Storage

- Stores calibration offset and wakeup interval.
- Reads/writes on boot and before sleep.

## Configuration

- All hardware and operational parameters are defined as macros at the top of `main.c`.
- Server URL is set via `SCRIPT_URL`.

## File Structure

- `src/main.c`: Main application logic and state machine.
- `src/BUTTONS/buttons.c/.h`: Button handling.
- `src/HD44780/HD44780.c/.h`: LCD display driver.
- `src/RTC/rtc.c/.h`: RTC handling.
- `src/HX711/hx711_lib.c/.h`: Load cell interface.
- `src/DHT11/dht11.c/.h`: Temperature/humidity sensor.
- `src/SIM800l/sim800l.c/.h`: GSM module interface.

## PlatformIO Configuration

- `platformio.ini`: Configured for ESP32 (NodeMCU-32S) with ESP-IDF framework.

## Example Data Payload

```json
{
  "Point0": {
    "timestamp": "20250411120611",
    "temperature": 43,
    "humidity": 5
  }
}
```

## Usage

1. Flash the firmware to the ESP32.
2. Connect all sensors and modules as per the pin configuration.
3. Power on the device; it will initialize, synchronize time, and send data.
4. Use the buttons to navigate screens and configure sleep intervals.

---

This documentation provides an overview of the system, its features, hardware connections, and main code structure. For further details, refer to the comments and function definitions in `main.c` and the respective driver files.
