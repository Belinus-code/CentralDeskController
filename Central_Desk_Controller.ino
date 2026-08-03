#include <Arduino.h>
#include <WDT.h>
#include <Preferences.h>
#include "config.h"
#include "src/io/system_io.h"
#include "src/logic/pc.h"
#include "src/logic/ac.h"
#include "src/rgb/rgb_controller.h"
#include "src/logic/time_manager.h"
#include "src/network/network.h"
#include "src/network/mqtt_manager.h"
#include "src/io/serial_handler.h"

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
  delay(1000); // Otherwise first few Serial prints are somehow lost
  Serial.begin(115200);

  // Check if (re)start was caused by Watchdog
  if (R_SYSTEM->RSTSR1_b.WDTRF)
  {
    Serial.println("[System] Warning! Restart caused by Watchdog!");
    R_SYSTEM->RSTSR1_b.WDTRF = 0;
  }
  Serial.println("[System] Setup started. Creating Objects.");

  io.init(&pc);
  pc.init(&io);
  rgb.init(&io, prefs);
  ac.init(&io, &rgb);
  network.init();
  time_manager.init(network.getWifiUDP());

  mqtt_manager.init(network.getWifiClient(), &network);
  mqtt_manager.registerNode(&pc);
  mqtt_manager.registerNode(&ac);
  mqtt_manager.registerNode(&rgb);

  serial_handler.init(&io, &pc, &ac, &rgb, &time_manager, &network);

  WDT.begin(5000);
  Serial.println("[System] Finished Setup, starting loop.");
}

void loop()
{
  pc.update();
  ac.update();
  rgb.update();
  network.update();
  time_manager.update();
  mqtt_manager.update();
  serial_handler.update();
  WDT.refresh();
}
