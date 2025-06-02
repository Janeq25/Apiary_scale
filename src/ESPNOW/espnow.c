#include "espnow.h"


#include "esp_now.h"
#include "esp_wifi.h"
#include <string.h>

// Initialize ESP-NOW
esp_err_t espnow_init(esp_now_recv_cb_t cb) {
    esp_err_t ret = esp_now_init();
    if (ret == ESP_OK) {
        esp_now_set_pmk((uint8_t *)"pmk1234567890123");
        esp_now_register_recv_cb(cb);
    }
    return ret;
}

// Add peer
esp_err_t espnow_add_peer(const uint8_t *peer_addr) {
    esp_now_peer_info_t peerInfo = {0};
    memcpy(peerInfo.peer_addr, peer_addr, ESP_NOW_ETH_ALEN);
    peerInfo.channel = 0;
    peerInfo.ifidx = ESP_IF_WIFI_STA;
    peerInfo.encrypt = false;
    return esp_now_add_peer(&peerInfo);
}

// Send data
esp_err_t espnow_send(const uint8_t *peer_addr, const uint8_t *data, size_t len) {
    return esp_now_send(peer_addr, data, len);
}


esp_err_t espnow_broadcast_send(const uint8_t *data, size_t len) {
    uint8_t broadcast_addr[ESP_NOW_ETH_ALEN] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
    return esp_now_send(broadcast_addr, data, len);
}