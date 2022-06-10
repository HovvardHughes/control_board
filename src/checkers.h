#include <Arduino.h>
#include <arduino-timer.h>

extern bool power;
extern Timer<> timer;

void withRunningTaskCheck(void (*action)())
{
  Serial.println("WithRunningTaskCheck...");
  if (timer.empty())
    action();
  else
    Serial.println("WithRunningTaskCheck:Skip action");
}

void withPowerCheck(void (*action)())
{
  Serial.println("WithPowerCheck...");
  if (power)
    return withRunningTaskCheck(action);
  else
    Serial.println("WithPowerCheck:Skip action");
}
