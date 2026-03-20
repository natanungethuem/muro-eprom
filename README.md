# muro-eprom

Firmware for an ESP8266-based board that controls LED tapes on a climbing wall (bouldering routes), with remote control over MQTT (TLS) and local persistence in EEPROM.

This project is structured as an Arduino sketch and is intended to run on ESP8266 hardware.

## What this firmware does

- Connects to Wi-Fi (startup credentials or configurable credentials).
- Connects to HiveMQ Cloud over secure MQTT (port 8883).
- Subscribes to a topic based on the device external IP.
- Receives compact MQTT commands to:
	- configure Wi-Fi credentials,
	- configure LED colors by route role,
	- activate LEDs for specific holds,
	- clear LEDs,
	- return a connection heartbeat.
- Stores Wi-Fi and LED configuration in EEPROM (intended behavior).

## Project structure

- `muroboard.ino`: main setup, loop, MQTT callback, EEPROM read/write routines.
- `leds.h` / `leds.cpp`: LED strips declarations and helper blink routine (`pisca`).
- `mqtt.h` / `mqtt.cpp`: Wi-Fi, NTP time sync, MQTT reconnect and helper functions.
- `data/certs.ar`: certificate archive used by BearSSL cert store in LittleFS.

## Hardware assumptions

- ESP8266 board.
- One main NeoPixel strip on UART RX pin:
	- `NUMPIXELS = 180`
- One foot-hold NeoPixel strip on `D4`:
	- `NUMPIXELS_FOOT = 50`

## Dependencies (Arduino libraries)

- Core / platform:
	- ESP8266 Arduino Core
	- EEPROM
	- LittleFS / FS
	- BearSSL / CertStoreBearSSL
- Networking / messaging:
	- `ESP8266WiFi`
	- `PubSubClient`
	- `time.h`, `TZ.h`
- LEDs:
	- `NeoPixelBus`

Install these through Arduino IDE Library Manager and ESP8266 board package manager.

## Build and flash

1. Open `muroboard.ino` in Arduino IDE.
2. Select an ESP8266 board and correct COM port.
3. Ensure LittleFS support is available for your board package.
4. Upload filesystem data (certificate files), then upload firmware.

### LittleFS certificate files

At runtime the sketch expects:

- `/certs.ar`
- `/certs.idx`

The code initializes cert store with:

```cpp
certStore.initCertStore(LittleFS, PSTR("/certs.idx"), PSTR("/certs.ar"));
```

So both files must exist in LittleFS for MQTT TLS validation to work.

## Runtime flow

1. Initialize serial and both LED strips.
2. Try loading Wi-Fi config from EEPROM.
3. Connect Wi-Fi (`acessa_wifi` or fallback `acessa_wifi_inicio`).
4. Initialize LittleFS and synchronize time via NTP.
5. Build secure client and connect to HiveMQ broker.
6. Fetch external IP and subscribe to this topic.
7. Process incoming commands in MQTT callback.

## MQTT configuration

- Broker: `b4a5c2c5ebbd4e9a87e07ab57b7fb677.s1.eu.hivemq.cloud`
- Port: `8883` (TLS)
- MQTT auth used in reconnect:
	- username: `muroboard`
	- password: `Mur0board`
- Status/ack topic used by firmware: `conexao`

## Command protocol (payload)

The sketch processes messages only when the first topic character is numeric (`'0'..'9'`) and dispatches by `payload[0]`.

### `payload[0] == 's'`

Set Wi-Fi SSID.

- Payload bytes `1..N`: SSID characters.
- Action: writes `ssid_config`, saves EEPROM, publishes `OK` on `conexao`.

### `payload[0] == 'p'`

Set Wi-Fi password.

- Payload bytes `1..N`: password characters.
- Action: writes `pass_config`, saves EEPROM, publishes `OK`.

### `payload[0] == 'w'`

Reconnect flow.

- Action: reruns connection init (`inicio(true)`).

### `payload[0] == 'c'`

Set RGB color profile for route states.

- Intended selector in `payload[1]`:
	- `'i'`: start
	- `'b'`: boulder
	- `'t'`: top
- Next 3 bytes: color channels.

### `payload[0] == 'a'`

Activate holds.

- Payload interpreted in pairs from index `1`:
	- status code character
	- LED index byte (1-based in payload, converted to 0-based)
- Status codes currently mapped in code:
	- Main strip segment A: `i`/`b`/`t`
	- Main strip offset +120: `j`/`c`/`u`
	- Foot strip offset +5: `k`/`d`/`v`
- After processing all pairs, strips are shown and `OK` is published.

### `payload[0] == 'd'`

Clear main strip LEDs and publish `OK`.

### `payload[0] == 'v'`

Heartbeat response: publish `OK` on `conexao`.

## LED color defaults

Initial `config_leds` values (9 bytes):

- Start: `(0, 150, 150)`
- Boulder: `(150, 0, 0)`
- Top: `(150, 150, 0)`

Stored in `config_leds[0..8]` as three RGB triplets.

## EEPROM layout (as implemented)

- Wi-Fi marker: addresses `10`, `11` (`'M'`, `'B'`)
- SSID: `12..39`
- Password: `40..69`
- LED config: `72..80`

## Current code limitations and bugs

The following issues are present in the current code and should be fixed for production reliability:

1. `le_eeprom_wifi()` returns `false` immediately, so stored Wi-Fi is never loaded.
2. `le_eeprom_leds()` returns `false` immediately, so stored LED config is never loaded.
3. LED EEPROM marker mismatch:
	 - write uses marker at `10/11`
	 - read checks marker at `70/71`
4. In command `'c'`, index selector is overwritten by `i = 0;`, so all writes target only the first color triplet.
5. `mqtt.h` currently has invalid C++ declarations (struct syntax and undeclared external symbols), and `mqtt.cpp` references names that are not defined in that class context. The `.ino` currently relies on global-style symbols.

## Suggested next improvements

- Refactor Wi-Fi and MQTT state into a single coherent module (either class-based or global C style, not mixed).
- Fix EEPROM read paths and marker consistency.
- Add payload length checks in callback to avoid out-of-bounds reads.
- Document and version the MQTT command protocol (topic schema + payload examples).
- Externalize broker credentials and Wi-Fi defaults from source code.

## License

No license file is currently present in this repository.
