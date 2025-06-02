#pragma once

#include <stdint.h>
#include <stdbool.h>
#include "esp_now.h"
#include "esp_wifi.h"
#include "esp_err.h"

esp_err_t espnow_init(esp_now_recv_cb_t cb);
esp_err_t espnow_register_send_cb(esp_now_send_cb_t cb);
esp_err_t espnow_register_recv_cb(esp_now_recv_cb_t cb);
void espnow_recv_cb(const uint8_t *mac_addr, const uint8_t *data, int data_len);
esp_err_t espnow_add_peer(const uint8_t *peer_addr);
esp_err_t espnow_send(const uint8_t *peer_addr, const uint8_t *data, size_t len);
void espnow_deinit(void);
esp_err_t espnow_broadcast_send(const uint8_t *data, size_t len);

