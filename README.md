# ESP32 Relay Telegram Bot

Control a relay module via Telegram bot commands using an ESP32.

This project is intended for simple home automation experiments. It uses a Telegram bot, a single authorized chat ID, WiFi reconnection, relay status reporting, and a transistor interface between the ESP32 GPIO and the relay module input.

## Features

* Control a relay with Telegram commands
* Single-chat authorization
* Optional unauthorized access alert sent only to the owner chat
* Relay status reporting
* WiFi SSID, RSSI and IP reporting
* Automatic WiFi reconnection
* Startup notification when the ESP32 comes online
* Safer relay input driving using an external BC547B NPN transistor

## Hardware

### Components

| Component    | Details                              | Notes                                            |
| ------------ | ------------------------------------ | ------------------------------------------------ |
| ESP32        | DevKit V1, 30-pin                    | Other ESP32 boards may work with pin adjustments |
| Relay module | JQC3F-05VDC-C 1-channel relay module | Active LOW trigger                               |
| Transistor   | BC547B NPN                           | Used to pull the relay module input to GND       |
| Resistor     | 1.5 kΩ                               | GPIO-to-base current limiting                    |
| Resistor     | 47 kΩ                                | Base pull-down to prevent floating during boot   |
| Protoboard   | 400 or 830 points                    | Optional, for testing                            |
| Jumper wires | Male-to-male                         | For protoboard wiring                            |
| USB cable    | Micro USB                            | ESP32 power and programming                      |
| Power supply | 5 V                                  | USB charger, USB power bank, or ESP32 USB power  |

## Wiring

The relay module used in this project is an **active LOW trigger** module. This means the relay turns on when the `IN` pin is pulled to `GND`.

The BC547B transistor is used as a low-side signal switch. The ESP32 does not drive the relay module input directly; it drives the transistor base instead.

```text
ESP32 5V/VIN  -> Relay module VCC
ESP32 GND     -> Relay module GND

ESP32 GPIO26  -> 1.5 kΩ resistor -> BC547B base
BC547B emitter -> GND
BC547B collector -> Relay module IN

BC547B base -> 47 kΩ resistor -> GND
```

### Why use the BC547B?

Many 5 V relay modules use a 5 V input circuit. Although some modules can be triggered directly from a 3.3 V ESP32 GPIO, using a transistor makes the interface more predictable.

With this circuit:

```text
GPIO26 LOW  -> transistor OFF -> relay module IN stays HIGH -> relay OFF
GPIO26 HIGH -> transistor ON  -> relay module IN is pulled LOW -> relay ON
```

### Base resistor

Recommended base resistor range:

```text
1 kΩ to 4.7 kΩ
```

Tested value:

```text
1.5 kΩ
```

A lower value such as 470 Ω can also work, but it draws more GPIO current than necessary for this input-level switching use.

### Base pull-down resistor

The 47 kΩ resistor between the BC547B base and GND keeps the transistor turned off while the ESP32 is booting or resetting.

During boot, the GPIO may briefly be in high-impedance mode. Without the pull-down resistor, the transistor base could float and partially turn on, causing an unwanted relay trigger.

## Telegram Commands

| Command   | Description                |
| --------- | -------------------------- |
| `/on`     | Turn relay on              |
| `/off`    | Turn relay off             |
| `/status` | Show WiFi and relay status |

## Setup

### 1. Install Arduino libraries

Install these libraries using the Arduino IDE Library Manager:

| Library              | Install via             |
| -------------------- | ----------------------- |
| UniversalTelegramBot | Arduino Library Manager |
| ArduinoJson          | Arduino Library Manager |

### 2. Create a Telegram bot

1. Open Telegram.
2. Talk to [@BotFather](https://t.me/botfather).
3. Send `/newbot`.
4. Follow the instructions.
5. Copy the bot token.

### 3. Get the chat ID

1. Talk to [@userinfobot](https://t.me/userinfobot).
2. Copy the chat ID.
3. Use that value as `CHAT_ID`.

For groups, the chat ID is usually a negative number.

### 4. Configure credentials

Copy the example credentials file:

```bash
cp credentials.h.example credentials.h
```

Edit `credentials.h` with your own values:

```cpp
#pragma once

#define WIFI_SSID     "your_wifi_name_here"
#define WIFI_PASSWORD "your_wifi_password_here"
#define BOT_TOKEN     "your_telegram_bot_token_here"
#define CHAT_ID       "your_telegram_chat_id_here"
```

The real `credentials.h` file should not be committed to Git.

### 5. Flash the ESP32

1. Open `esp32-relay-telegram.ino` in Arduino IDE.
2. Select the correct ESP32 board.
3. Select the correct serial port.
4. Upload the sketch.

After boot, the ESP32 sends a Telegram message to the authorized chat:

```text
ESP32 online and ready.
```

## Security Notes

Only the configured `CHAT_ID` can control the relay.

Messages from unauthorized chats do not receive any reply. The current code may optionally forward an alert to the authorized owner chat so that unexpected access attempts can be observed during testing.

Keep `credentials.h` out of version control. The `.gitignore` file should exclude it.

If a bot token is ever exposed, revoke it and generate a new one using BotFather.

Telegram communication uses HTTPS. The sketch uses `client.setInsecure()`, which skips certificate validation. This is simpler for hobby projects, but certificate validation should be used for a more security-sensitive setup.

## Repository Structure

```text
esp32-relay-telegram/
├── esp32-relay-telegram/
│   ├── esp32-relay-telegram.ino
│   └── credentials.h.example
├── .gitignore
├── LICENSE
└── README.md
```

## License

This project is licensed under the MIT License.
