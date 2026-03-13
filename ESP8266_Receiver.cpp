/*
 * ============================================================
 *  ESP8266 — ESP-NOW TEXT RECEIVER + OLED DISPLAY
 * ============================================================
 *  HARDWARE:
 *    • ESP8266 (NodeMCU / Wemos D1 Mini / any variant)
 *    • 0.96" I²C OLED (SSD1306, 128×64 px)
 *
 *  WIRING (ESP8266 → OLED):
 *    VCC  →  3.3 V
 *    GND  →  GND
 *    SDA  →  D2  (GPIO 4)
 *    SCL  →  D1  (GPIO 5)
 *
 *  LIBRARIES (install via Arduino Library Manager):
 *    • ESP8266 core  — https://github.com/esp8266/Arduino
 *    • espnow        — built into ESP8266 core
 *    • Adafruit SSD1306       (search "Adafruit SSD1306")
 *    • Adafruit GFX Library   (installed automatically as dependency)
 *
 *  FIRST RUN:
 *    Open Serial Monitor at 115200 baud.
 *    Copy the printed MAC address and paste into ESP32 sender sketch.
 *
 *  REPO: https://github.com/YOUR_USERNAME/esp-now-oled-messenger
 * ============================================================
 */

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <espnow.h>

// OLED
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// ── OLED configuration ────────────────────────────────────────
#define SCREEN_WIDTH  128
#define SCREEN_HEIGHT  64
#define OLED_RESET     -1   // shared reset with MCU
#define OLED_I2C_ADDR  0x3C // most common; try 0x3D if blank screen

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT,
                          &Wire, OLED_RESET);

// ── Payload structure — must match ESP32 sender exactly ───────
typedef struct MessagePacket {
    char text[64];
    int  packetID;
} MessagePacket;

// Shared between ISR callback and main loop
volatile bool    newMessageFlag = false;
MessagePacket    latestPacket;
int              totalReceived  = 0;

// ── Helper: draw a full-screen message on the OLED ───────────
void showOLED(const char* line1,
              const char* line2 = "",
              const char* line3 = "",
              bool        big   = false) {
    display.clearDisplay();
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);

    if (big) {
        display.setTextSize(2);
        display.println(line1);
    } else {
        // Header bar (inverted colours)
        display.setTextSize(1);
        display.setTextColor(SSD1306_BLACK, SSD1306_WHITE);
        display.print(" ESP-NOW RECEIVER    ");
        display.setTextColor(SSD1306_WHITE);
        display.println();

        // Message body
        display.setTextSize(1);
        display.println();
        display.println(line1);
        display.println(line2);
        display.println(line3);
    }
    display.display();
}

// Helper: word-wrap a string into up to 3 lines of `width` chars each
void wrapText(const char* src, int width,
              char* l1, char* l2, char* l3) {
    strncpy(l1, src, width); l1[width] = '\0';
    int len = strlen(src);
    if (len > width) {
        strncpy(l2, src + width, width); l2[width] = '\0';
    } else { l2[0] = '\0'; }
    if (len > width * 2) {
        strncpy(l3, src + width * 2, width); l3[width] = '\0';
    } else { l3[0] = '\0'; }
}

// ── ESP-NOW receive callback (runs in ISR context — keep short) ─
void onDataReceived(uint8_t *mac, uint8_t *data, uint8_t len) {
    if (len == sizeof(MessagePacket)) {
        memcpy(&latestPacket, data, sizeof(MessagePacket));
        latestPacket.text[63] = '\0'; // safety null-term
        newMessageFlag = true;
        totalReceived++;
    }
}

void setup() {
    Serial.begin(115200);
    delay(500);
    Serial.println("\n=== ESP8266 ESP-NOW Receiver ===");

    // ── OLED init ───────────────────────────────────────────
    Wire.begin(4, 5); // SDA=D2(GPIO4), SCL=D1(GPIO5)
    if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_I2C_ADDR)) {
        Serial.println("ERROR: SSD1306 not found. Check wiring/address.");
    }
    display.clearDisplay();
    display.display();

    showOLED("ESP-NOW", "Receiver", "Starting...", true);
    delay(1500);

    // ── Wi-Fi in station mode ────────────────────────────────
    WiFi.mode(WIFI_STA);
    WiFi.disconnect();

    Serial.print("Receiver MAC: ");
    String mac = WiFi.macAddress();
    Serial.println(mac);

    // Show MAC on OLED at boot
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);
    display.println("My MAC Address:");
    display.println();
    display.println(mac.substring(0, 8));
    display.println(mac.substring(9));
    display.println();
    display.println("Waiting for data...");
    display.display();

    // ── ESP-NOW init ────────────────────────────────────────
    if (esp_now_init() != 0) {
        Serial.println("ERROR: esp_now_init() failed. Halting.");
        showOLED("ESP-NOW", "INIT FAILED", "Check serial", false);
        while (true) { delay(1000); }
    }

    esp_now_set_self_role(ESP_NOW_ROLE_SLAVE);
    esp_now_register_recv_cb(onDataReceived);

    Serial.println("Waiting for messages...\n");
}

void loop() {
    if (newMessageFlag) {
        newMessageFlag = false;

        Serial.printf("[RX #%d] \"%s\"\n",
                      latestPacket.packetID,
                      latestPacket.text);

        // Wrap text into 3 display lines (21 chars each at TextSize 1)
        char l1[22], l2[22], l3[22];
        wrapText(latestPacket.text, 21, l1, l2, l3);

        showOLED(l1, l2, l3, false);

        // Packet counter in bottom-right corner
        display.setTextSize(1);
        display.setCursor(90, 56);
        display.printf("Pkt:%d", totalReceived);
        display.display();
    }

    // Blink built-in LED to show the board is alive
    static unsigned long lastBlink = 0;
    if (millis() - lastBlink > 1000) {
        lastBlink = millis();
        digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
    }
}
