#include <Wire.h>
#include "ina219.h"

INA219 monitor;

void initPowerMonitor()
{
  pinMode(6, OUTPUT);
  digitalWrite(6, HIGH);
  monitor.begin(64);
  monitor.configure(1, 0, 3, 3, 7);
  monitor.calibrate(0.04, 0.09, 26, 2.3);
}

void powerMonitorCheck()
{
  if (monitor.shuntCurrent() * 1000 > 500)
    digitalWrite(6, LOW);
}
