/*
 * ============================================================
 *  ESP32 — ESP-NOW TEXT SENDER
 * ============================================================
 *  HARDWARE : ESP32 Dev Board (any variant)
 *  PROTOCOL : ESP-NOW (built-in, no router needed)
 *  LIBRARIES: esp_now.h, WiFi.h  (both included in ESP32 core)
 *
 *  SETUP STEPS:
 *  1. Flash the ESP8266 Receiver sketch first.
 *  2. Open its Serial Monitor and note the MAC address printed.
 *  3. Paste that MAC into receiverMAC[] below.
 *  4. Flash this sketch onto the ESP32.
 *
 *  REPO: https://github.com/YOUR_USERNAME/esp-now-oled-messenger
 * ============================================================
 */

#include <Arduino.h>
#include <esp_now.h>
#include <WiFi.h>

// ── ① Replace with YOUR ESP8266's actual MAC address ─────────
//    Example: EC:FA:BC:CB:47:96  →  {0xEC, 0xFA, 0xBC, 0xCB, 0x47, 0x96}
uint8_t receiverMAC[] = {0xEC, 0xFA, 0xBC, 0xCB, 0x47, 0x96};
// ─────────────────────────────────────────────────────────────

// ── Payload structure — must be IDENTICAL on both devices ─────
typedef struct MessagePacket {
    char text[64];   // up to 63 characters + null terminator
    int  packetID;   // handy for debugging dropped packets
} MessagePacket;

MessagePacket outgoing;
int packetCounter = 0;

// ── Delivery callback ─────────────────────────────────────────
// Fires after every send attempt — tells if receiver acknowledged
void onDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
    Serial.print("[ESP-NOW] Delivery status: ");
    Serial.println(status == ESP_NOW_SEND_SUCCESS ? "SUCCESS ✓" : "FAILED ✗");
}

void setup() {
    Serial.begin(115200);
    delay(500);
    Serial.println("\n=== ESP32 ESP-NOW Sender ===");

    // ESP-NOW requires Wi-Fi in station mode (no AP connection needed)
    WiFi.mode(WIFI_STA);
    WiFi.disconnect();   // make sure we're not associated to any AP

    Serial.print("Sender MAC: ");
    Serial.println(WiFi.macAddress());

    // Initialise ESP-NOW
    if (esp_now_init() != ESP_OK) {
        Serial.println("ERROR: esp_now_init() failed. Halting.");
        while (true) { delay(1000); }
    }

    // Register send callback
    esp_now_register_send_cb(onDataSent);

    // Register the peer (ESP8266 receiver)
    esp_now_peer_info_t peerInfo = {};
    memcpy(peerInfo.peer_addr, receiverMAC, 6);
    peerInfo.channel  = 0;     // 0 = current channel
    peerInfo.encrypt  = false; // no encryption for simplicity

    if (esp_now_add_peer(&peerInfo) != ESP_OK) {
        Serial.println("ERROR: Failed to add peer. Check MAC address.");
        while (true) { delay(1000); }
    }

    Serial.println("ESP-NOW initialised. Sending messages every 3 s...\n");
}

// ── Message list to cycle through ────────────────────────────
const char* messages[] = {
    "Hello from ESP32!",
    "ESP-NOW is fast!",
    "No router needed",
    "Direct P2P link",
    "Temperature: 27C",
    "Humidity   : 65%",
    "Packet sent!"
};
const int MESSAGE_COUNT = sizeof(messages) / sizeof(messages[0]);
int msgIndex = 0;

void loop() {
    // Build packet
    packetCounter++;
    snprintf(outgoing.text, sizeof(outgoing.text), "%s", messages[msgIndex]);
    outgoing.packetID = packetCounter;

    Serial.printf("[TX #%d] \"%s\"\n", outgoing.packetID, outgoing.text);

    // Send to peer
    esp_err_t result = esp_now_send(receiverMAC,
                                    (uint8_t *)&outgoing,
                                    sizeof(outgoing));
    if (result != ESP_OK) {
        Serial.printf("  esp_now_send error: %s\n", esp_err_to_name(result));
    }

    // Advance through message list
    msgIndex = (msgIndex + 1) % MESSAGE_COUNT;

    delay(3000);   // send every 3 seconds
}
