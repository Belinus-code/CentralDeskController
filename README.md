# Central Desk Controller

Firmware für einen **Arduino Uno R4 WiFi**, der als zentrale Steuereinheit für einen Schreibtisch-Aufbau dient. Sie schaltet den PC per Relais, steuert eine Klimaanlage per Infrarot, betreibt einen adressierbaren RGB-LED-Streifen und überwacht Temperatur/Luftfeuchtigkeit sowie einen Wassertank-Füllstand. Steuerung erfolgt über physische Taster/Schalter, MQTT (z.B. für Home Assistant) und eine serielle Konsole.

## Features

- **PC-Steuerung**: Ein-/Ausschalten und Hard-Reset des PCs über ein Relais, mit Status-Erkennung über einen Analogeingang (LED-Helligkeit des PCs).
- **Klimaanlagen-Steuerung**: Sendet IR-Codes an eine Klimaanlage (Ein/Aus, Modus, Lüfterstufe, Timer-Abschaltung). Überwacht zwei DHT22-Sensoren (vor/nach der Anlage) sowie einen Wassertank-Sensor und löst bei vollem Tank einen Alarm (RGB-Blinken) bzw. eine automatische Abschaltung aus.
- **RGB-Steuerung**: Adressierbarer LED-Streifen (FastLED, 211 LEDs) mit nutzerdefinierten Animationen (statische Farbe, Blinken, Paletten-Fade), die in den `Preferences` (Flash) gespeichert werden. Unterstützt "Prioritäts"-Animationen, die z.B. für Alarme die Nutzer-Animation überlagern.
- **MQTT-Anbindung**: Alle Module (PC, AC, RGB) publizieren Status/Sensordaten und abonnieren Befehls-Topics über einen zentralen `MqttManager`.
- **NTP-Zeit**: Zeitsynchronisation inkl. automatischer Umschaltung Sommer-/Winterzeit (Europa).
- **Serielle Konsole**: Textbasierte Kommandos über den USB-Seriellmonitor zum Debuggen und manuellen Steuern aller Module.
- **Watchdog**: Ein Hardware-Watchdog (5 s) sorgt für einen automatischen Neustart bei einem Hänger.

## Hardware

Zielplattform: **Arduino Uno R4 WiFi** (`arduino:renesas_uno:unor4wifi`)

| Pin | Funktion |
|-----|----------|
| 2 | Taster (Button) |
| 3 | Schalter (Switch, PC/AC-Modusumschaltung) |
| 4 | Stromversorgung der DHT-Sensoren (schaltbar) |
| 6 | DHT22 – AC Sensor 2 (nach der Anlage) |
| 7 | DHT22 – AC Sensor 1 (vor der Anlage) |
| 8 | DHT22 – Gehäuse-innen |
| 9 | Relais (PC-Ein/Aus/Reset) |
| 11 | RGB-LED-Datenleitung |
| 12 | Key (Schlüsselschalter) |
| A0 | PC-Status (Analogmessung LED-Helligkeit) |
| A1 | IR-Sender (Klimaanlage) |
| A2 | Wassertank-Füllstandssensor |

Weitere Konstanten (Schwellwerte, Timings) siehe [config.h](config.h).

## Projektstruktur

```
Central_Desk_Controller.ino   Einstiegspunkt (setup/loop), verdrahtet alle Module
config.h                      Zentrale Konfiguration: Pins, Timings, MQTT-Topics, NTP
secrets.h                     WLAN- & MQTT-Zugangsdaten (nicht eingecheckt, siehe unten)

src/io/
  system_io.*                 Low-Level I/O: Pins, Interrupts, Sensoren, IR-Versand
  serial_handler.*             Serielle Kommandozeile

src/logic/
  pc.*                         PC-Ein/Aus-/Reset-Logik inkl. MQTT-Anbindung
  ac.*                         Klimaanlagen-Logik (IR, Sensoren, Wasseralarm, Timer)
  time_manager.*                NTP-Zeit inkl. Sommer-/Winterzeit-Berechnung

src/network/
  network.*                     WLAN-Verbindungsmanagement
  mqtt_manager.*                 Zentraler MQTT-Client, Publish/Subscribe-Routing

src/rgb/
  rgb_controller.*               Ansteuerung des LED-Streifens, Timer-getriebenes Update
  animation_manager.*            Verwaltung/Persistenz der Animationen (Preferences)
  animations.*                   Animationstypen: StaticColor, Blink, Palette
```

## Einrichtung

### Abhängigkeiten

- Arduino-Core: **Arduino UNO R4 Boards** (Renesas)
- Bibliotheken (über den Arduino Library Manager oder `arduino-cli lib install`):
  - `FastLED`
  - `DHT sensor library` (Adafruit) + `Adafruit Unified Sensor`
  - `ArduinoMqttClient`
  - `NTPClient`
  - `WiFiS3` (Teil des UNO-R4-Cores)

### `secrets.h` anlegen

Aus Sicherheitsgründen ist `secrets.h` in [.gitignore](.gitignore) ausgeschlossen und muss lokal angelegt werden:

```cpp
#pragma once

#define WIFI_SSID "..."
#define WIFI_PASS "..."
#define BROKER_HOST_ADRESS "..."
#define BROKER_HOST_PORT 1883
#define BROKER_USER "..."
#define BROKER_PASSWORD "..."
```

## Bauen & Hochladen

Über `arduino-cli` (siehe [.vscode/tasks.json](.vscode/tasks.json)):

```bash
arduino-cli compile --fqbn arduino:renesas_uno:unor4wifi .
```

```bash
arduino-cli upload -p COM9 --fqbn arduino:renesas_uno:unor4wifi .
```

Den seriellen Port (`-p COM9`) bei Bedarf an das eigene System anpassen.

## Serielle Kommandos

Über den Seriellmonitor (115200 Baud) stehen folgende Befehle zur Verfügung:

| Befehl | Beschreibung |
|--------|--------------|
| `help` | Zeigt verfügbare Befehle |
| `dump` | Gibt den kompletten Systemstatus aus (Uptime, IP, PC-Status, Sensorwerte, Wasserstand, Eingänge) |
| `pc TOGGLE` / `pc RESET` | Schaltet den PC um bzw. löst einen Hard-Reset aus |
| `ac <cmd>` | Sendet einen AC-Befehl (siehe unten) |
| `rgb <cmd>` | Steuert die RGB-Animationen (siehe unten) |

**AC-Befehle** (`ac <cmd>`): `on`, `off`, `toggle`, `on/off`, `cool`, `dry`, `fan`, `sleep`, `up`, `down`, `high`, `low`, `TIMER <Minuten>` (0 = Timer abbrechen)

**RGB-Befehle** (`rgb <cmd>`):

| Befehl | Beschreibung |
|--------|--------------|
| `rgb set NAME` | Wechselt zur Animation `NAME` |
| `rgb new static NAME COLOR` | Erstellt eine statische Farb-Animation (Hex-Farbe) |
| `rgb new blink NAME COLOR_ON COLOR_OFF TICKS` | Erstellt eine Blink-Animation |
| `rgb new fade NAME PALETTE SPEED DELTA` | Erstellt eine Paletten-Fade-Animation (Paletten: 0 Rainbow, 1 Party, 2 Ocean, 3 Forest, 4 Heat, 5 Lava, 6 Matrix) |
| `rgb list` | Listet alle gespeicherten Animationen |
| `rgb delete NAME` | Löscht eine Animation |
| `rgb setting list/show/set NAME [INDEX] [DATA]` | Liest/ändert Animationsparameter |
| `rgb toggle` | Schaltet die Beleuchtung ein/aus |
| `rgb brightness UP/DOWN/+/-/MIN/MAX` | Passt die Helligkeit an |

## MQTT-Topics

Basis-Präfix: `linus/sundgau74/...` (siehe [config.h](config.h))

| Topic | Richtung | Beschreibung |
|-------|----------|--------------|
| `desk/temperature`, `desk/humidity` | publish | Temperatur/Luftfeuchtigkeit im Gehäuse |
| `ac/temperature_before`, `ac/temperature_after` | publish | Temperatur vor/nach der Klimaanlage |
| `ac/humidity_before`, `ac/humidity_after` | publish | Luftfeuchtigkeit vor/nach der Klimaanlage |
| `ac/water_full` | publish | Wassertank voll (Bool) |
| `ac/feedback` | publish | Rückmeldungen (z.B. Timer-Status) |
| `ac/command` | subscribe | AC-Befehle (identisch zu den seriellen AC-Befehlen) |
| `pc/status` | publish | PC-Status (ON/OFF) |
| `pc/command` | subscribe | `TOGGLE` / `RESET` |
| `desk_rgb/status`, `desk_rgb/status_dig` | publish | Aktuelle RGB-Animation |
| `desk_rgb/command` | subscribe | RGB-Befehle |
