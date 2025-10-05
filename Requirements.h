/*
Requirements:
- Control PC Switch Setup:
    If Key is turned, Make RGB Stripe solid Red
    + If Switch is on, Make RGB Stripe blink Red
    + While Button is pressed, toggle Relay

- Publish data to MQTT
  Publish on Change, but max once every Second:
  - Publish DHT Data to MQTT
  - Publish Computer State to MQTT
  - Publish RGB-Stripe Programm to MQTT

- Get commands from MQTT
  Toggle or force-Reset Computer
  Change RGB-Stripe Programm

- Serial Port Communication for Debugging
  - Check Connection
  - Status Dump
    - IP-Adress
    - Time
    - DHT Data
    - Computer State, RAW Value and Treshhold
    - RGB-Stripe State
  - Change RGB-Programm
  - For Future: Enable RGB-Programm Upload and Access

- Watchdog
*/