#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <ArduinoJson.h>
#include <credentials.h>

// ===== CONFIG =====
#define RELAY_PIN 26
// ==================

WiFiClientSecure client;
UniversalTelegramBot bot(BOT_TOKEN, client);

unsigned long lastCheck = 0;
const unsigned long interval = 2000;

unsigned long lastWifiTry = 0;
const unsigned long wifiRetryInterval = 10000;

bool relayState = false;

void processMsgs(int numMsgs);  // ← forward declaration

void setRelay(bool state) {
  relayState = state;
  // BC547 collector pulls relay module IN to GND (active LOW module):
  // GPIO HIGH → transistor ON → relay ON
  // GPIO LOW  → transistor OFF → relay OFF
  digitalWrite(RELAY_PIN, state ? HIGH : LOW);
}

void connectWiFi() {
  if (WiFi.status() == WL_CONNECTED) return;

  Serial.print("Connecting to WiFi");
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  unsigned long startAttempt = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - startAttempt < 15000) {
    delay(500);
    Serial.print(".");
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nWiFi connected");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("\nWiFi connection failed");
  }
}

void processMsgs(int numMsgs) {
  for (int i = 0; i < numMsgs; i++) {
    String chat_id = bot.messages[i].chat_id;
    String text    = bot.messages[i].text;

    text.toLowerCase();
    text.trim();

    // Strip @bot_name suffix from group commands (e.g. /on@mybot → /on)
    int atIndex = text.indexOf('@');
    if (atIndex > 0) {
      text = text.substring(0, atIndex);
    }

    if (chat_id != CHAT_ID) {
      // Notify owner only - do not respond to unauthorized sender
      String alert = "Denied access!\n";
      alert += "Name: "    + bot.messages[i].from_name  + "\n";
      alert += "User ID: " + bot.messages[i].from_id    + "\n";
      alert += "Chat ID: " + chat_id                    + "\n";
      alert += "Title: "   + bot.messages[i].chat_title + "\n";
      alert += "Msg: "     + bot.messages[i].text;
      bot.sendMessage(CHAT_ID, alert, "");
      continue;
    }

    if (text == "/on") {
      setRelay(true);
      bot.sendMessage(CHAT_ID, "Lights On", "");
    }
    else if (text == "/off") {
      setRelay(false);
      bot.sendMessage(CHAT_ID, "Lights Off", "");
    }
    else if (text == "/status") {
      String msg = "📊 Status:"
                   "\nWiFi: "   + WiFi.SSID() +
                   "\nRSSI: "   + String(WiFi.RSSI()) + " dBm" +
                   "\nIP: "     + WiFi.localIP().toString() +
                   "\nRelay: "  + (relayState ? "On" : "Off");
      bot.sendMessage(CHAT_ID, msg, "");
    }
    else {
      bot.sendMessage(CHAT_ID, "Commands:\n/on\n/off\n/status", "");
    }
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(RELAY_PIN, OUTPUT);
  setRelay(false);
  client.setInsecure();
  connectWiFi();
  if (WiFi.status() == WL_CONNECTED) {
    bot.sendMessage(CHAT_ID, "ESP32 ready.", "");
  }
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) {
    if (millis() - lastWifiTry > wifiRetryInterval) {
      lastWifiTry = millis();
      connectWiFi();
    }
    return;
  }

  if (millis() - lastCheck > interval) {
    int newMsgs = bot.getUpdates(bot.last_message_received + 1);

    if (newMsgs < 0) {
      Serial.println("Error fetching Telegram messages");
      lastCheck = millis();
      return;
    }

    while (newMsgs > 0) {
      processMsgs(newMsgs);
      newMsgs = bot.getUpdates(bot.last_message_received + 1);
    }

    lastCheck = millis();
  }
}

// =====  Hardware wiring =====
// Relay module: 1ch JQC3F-05VDC-C (active LOW trigger)
// ESP32 5V        → relay module VCC
// ESP32 GND       → relay module GND
// GPIO26          → 470Ω resistor → BC547B base
// BC547B emitter  → GND
// BC547B collector→ relay module IN
// BC547B base     → 47kΩ resistor → GND (keeps BC547B off during boot)
