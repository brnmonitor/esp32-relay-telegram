#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <ArduinoJson.h>
#include <credentials.h>

// ===== CONFIG =====

constexpr int MODULE_RELAY_COUNT = 4;

struct Relay {
	int pin;
	bool state;
};

// Index set to match GPIO physical order
Relay relays[MODULE_RELAY_COUNT] = {
	{26, false},
	{25, false},
	{33, false},
	{32, false}
};
// ==================

WiFiClientSecure client;
UniversalTelegramBot bot(BOT_TOKEN, client);

unsigned long lastCheck = 0;
const unsigned long interval = 2000;

unsigned long lastWifiTry = 0;
const unsigned long wifiRetryInterval = 10000;



void processMsgs(int numMsgs);  // ← forward declaration

void setRelay(int relay_index, bool state) {
  relays[relay_index].state = state;
  // BC547 collector pulls relay module IN to GND (active LOW module):
  // GPIO HIGH → transistor ON → relay ON
  // GPIO LOW  → transistor OFF → relay OFF
  digitalWrite(relays[relay_index].pin, relays[relay_index].state ? HIGH : LOW);
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

    Serial.println(text);

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
    if (text == "/all_lights_on"){
        setRelay(0, true);
        setRelay(1, true);
        setRelay(2, true);
        bot.sendMessage(CHAT_ID, "All Lights On", "");
    }
    if (text == "/all_lights_off"){
        setRelay(0, false);
        setRelay(1, false);
        setRelay(2, false);
        bot.sendMessage(CHAT_ID, "All Lights Off", "");
    }
    if (text == "/sala1_on") {
        setRelay(0, true);
        bot.sendMessage(CHAT_ID, "Lights sala1 On", "");
    } 
    else if (text == "/sala1_off") {
        setRelay(0, false);
        bot.sendMessage(CHAT_ID, "Lights sala1 Off", "");
    } 
    else if (text == "/sala2_on") {
        setRelay(1, true);
        bot.sendMessage(CHAT_ID, "Lights sala2 On", "");
    } 
    else if (text == "/sala2_off") {
        setRelay(1, false);
        bot.sendMessage(CHAT_ID, "Lights sala2 Off", "");
    } 
    else if (text == "/garagem_on") {
        setRelay(2, true);
        bot.sendMessage(CHAT_ID, "Lights garagem On", "");
    } 
    else if (text == "/garagem_off") {
        setRelay(2, false);
        bot.sendMessage(CHAT_ID, "Lights garagem Off", "");
    } 
    else if (text == "/free_on") {
        setRelay(3, true);
        bot.sendMessage(CHAT_ID, "free On", "");
    } 
    else if (text == "/free_off") {
        setRelay(3, false);
        bot.sendMessage(CHAT_ID, "free Off", "");
    } 
    else if (text == "/status2") {
        String msg = "Status:";
        msg += "\nWiFi: " + WiFi.SSID();
        msg += "\nRSSI: " + String(WiFi.RSSI()) + " dBm";
        msg += "\nIP: " + WiFi.localIP().toString();
        msg += "\nRelays:";

        for (int relayIndex = 0; relayIndex < MODULE_RELAY_COUNT; relayIndex++) {
            msg += "\nR" + String(relayIndex + 1) + ": ";
            msg += relays[relayIndex].state ? "On" : "Off";
        }

        bot.sendMessage(CHAT_ID, msg, "");
    }
  }
}

void setup() {
  Serial.begin(115200);
  for (int i = 0; i < MODULE_RELAY_COUNT; i++) {
	  pinMode(relays[i].pin, OUTPUT);
	  setRelay(i, false);
  }

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
// Relay module: 4ch JQC3F-05VDC-C (active LOW trigger)
// ESP32 5V        → relay module VCC
// ESP32 GND       → relay module GND

// GPIO25          → 2KΩ resistor → BC547B_1 base
// GPIO26          → 2KΩ resistor → BC547B_2 base
// GPIO32          → 2KΩ resistor → BC547B_3 base
// GPIO33          → 2KΩ resistor → BC547B_4 base

// BC547B_1 emitter  → GND
// BC547B_2 emitter  → GND
// BC547B_3 emitter  → GND
// BC547B_4 emitter  → GND

// BC547B_1 collector → relay_1 module IN
// BC547B_2 collector → relay_2 module IN
// BC547B_3 collector → relay_3 module IN
// BC547B_4 collector → relay_4 module IN

// BC547B_1 base     → 47kΩ resistor → GND (keeps BC547B off during boot)
// BC547B_2 base     → 47kΩ resistor → GND (keeps BC547B off during boot)
// BC547B_3 base     → 47kΩ resistor → GND (keeps BC547B off during boot)
// BC547B_4 base     → 47kΩ resistor → GND (keeps BC547B off during boot)
