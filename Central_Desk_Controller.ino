#include <Arduino.h>
#include <WDT.h>
#include <Preferences.h>
#include "config.h"
#include "system_io.h"
#include "pc.h"
#include "ac.h"
#include "rgb.h"
#include "time_manager.h"
#include "network.h"
#include "mqtt_manager.h"
#include "serial_handler.h"

using namespace Desk;

Preferences prefs;
SystemIO io;
PC pc;
AC ac;
RGBController rgb;
TimeManager time_manager;
Network network;
MqttManager mqtt_manager;
SerialHandler serial_handler;

void setup()
{
  noInterrupts();
  Serial.begin(115200);
  Serial.println("Setup started. Creating Objects.");

  io.init(&pc);
  pc.init(&io);
  rgb.init(&io, prefs);
  ac.init(&io, &rgb);
  network.init();
  time_manager.init(network.getWifiUDP());
  mqtt_manager.init(network.getWifiClient(), &network);
  serial_handler.init(&io, &pc, &ac, &rgb, &time_manager, &network);

  interrupts();
  Serial.println("Finished Setup, starting loop...");
}

void loop()
{
  WDT.refresh();
}
