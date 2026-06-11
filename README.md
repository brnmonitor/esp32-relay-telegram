# ESP32 Relay Telegram Bot

Control a relay via Telegram bot commands using an ESP32.
Built for home automation with a simple and secure single-chat authorization.

## Features

- Control relay on/off via Telegram commands
- Single chat authorization (ignores and logs unauthorized access)
- Status report with WiFi SSID, RSSI and IP
- Automatic WiFi reconnection
- Startup notification when ESP32 comes online

## Hardware

### Components List

| Component | Details | Notes |
|-----------|---------|-------|
| ESP32 | DevKit V1 (30-pin) | Other ESP32 boards may work with pin adjustments |
| Relay module | JQC3F-05VDC-C 1 channel | Active LOW trigger, includes flyback diode |
| Transistor | BC547B (NPN) | Class B, hFE 200-450 |
| Resistor | 1k5Ω | BC547B base current limiting |
| Resistor | 47kΩ | BC547B base pull-down, prevents relay trigger on boot |
| Protoboard | 400 or 830 points | Any size that fits the circuit |
| Jumper wires | Male-Male | For protoboard connections |
| USB cable | Micro USB | ESP32 power and programming |
| Power supply | 5V | Can use USB power bank or USB charger |

### Wiring
```
ESP32 5V → Relay module VCC  
ESP32 GND → Relay module GND  
ESP32 GPIO26 → 1k5Ω resistor → BC547B base
BC547B emitter → GND  
BC547B collector → Relay module IN  
BC547B base → 47kΩ resistor → GND  
```
> The 47kΩ pull-down on the base ensures the BC547B stays off
> during ESP32 boot, preventing relay from triggering on startup.

> Recommended base resistor: 1kΩ to 4.7kΩ.
> Tested value: 1.5kΩ.

## Telegram Commands

| Command    | Description                |
|------------|----------------------------|
| `/on`      | Turn relay on              |
| `/off`     | Turn relay off             |
| `/status`  | Show WiFi and relay status |

## Setup

### 1. Prerequisites

Install these libraries in Arduino IDE:

| Library              | Install via             |
|----------------------|-------------------------|
| UniversalTelegramBot | Arduino Library Manager |
| ArduinoJson          | Arduino Library Manager |

### 2. Create a Telegram Bot

1. Open Telegram and talk to [@BotFather](https://t.me/botfather)
2. Send `/newbot` and follow the instructions
3. Copy the **bot token** provided

### 3. Get Your Chat ID

1. Talk to [@userinfobot](https://t.me/userinfobot)
2. Copy your **chat ID**

### 4. Configure Credentials

Copy the example credentials file:  
```bash
cp credentials.h.example credentials.h
```
Edit credentials.h with your values:  
```
#define WIFI_SSID      "your_wifi_network"
#define WIFI_PASSWORD  "your_wifi_password"
#define BOT_TOKEN      "your_telegram_bot_token"
#define CHAT_ID        "your_telegram_chat_id"
```
### 5. Flash
Open esp32-relay-telegram.ino in Arduino IDE  
Select your ESP32 board and port  
Click Upload  

### Security
Only the configured CHAT_ID can control the relay
Any unauthorized access attempt is logged and forwarded to the owner
credentials.h is excluded from version control via .gitignore
TLS is used for Telegram API communication (setInsecure skips
certificate validation — acceptable for local hobby use)
