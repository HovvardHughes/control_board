#include <Arduino.h>
#include "OneButton.h"

#define PIN_INPUT 19
#define PIN_LED 15
#define COM_PORT_SPEED 115200
#define DELAY_IN_MILLIS 1000
#define DELAY_COMMAND -1

class PowerRelay
{
public:
  int order;
  int iONumber;
  bool writable;

  PowerRelay(int orderArg, int iONumberArg)
  {
    order = orderArg;
    iONumber = iONumberArg;
    writable = true;
  }

  void writeState(int newState)
  {
    if (writable)
      digitalWrite(iONumber, newState);
  }

  bool readState()
  {
    return digitalRead(iONumber);
  }
};

bool power = false;

#define POWER_RELAY_COUNT 5
PowerRelay allPowerRelays[POWER_RELAY_COUNT] = {
    PowerRelay(0, 25),
    PowerRelay(1, 26),
    PowerRelay(2, 27),
    PowerRelay(3, 14),
    PowerRelay(4, 12)};

OneButton button(PIN_INPUT, true);

static void setPowerToRelays(int relayIndexesAndDelayCommands[], int size, int state)
{
  for (size_t i = 0; i < size; i++)
  {
    int relayIndex = relayIndexesAndDelayCommands[i];

    if (relayIndex == DELAY_COMMAND)
    {
      delay(DELAY_IN_MILLIS);
      continue;
    }

    PowerRelay powerRelay = allPowerRelays[relayIndex];
    powerRelay.writeState(state);
  }
}

void onDoubleClick()
{
  if (!power)
    return;

  bool state = allPowerRelays[2].readState();
  int indexesToPowerOffRelaysAndDelayCommands[] = {2, DELAY_COMMAND, 1, 3};
  int indexesToPowerOnRelaysAndDelayCommands[] = {1, 3, DELAY_COMMAND, 2};

  setPowerToRelays(state ? indexesToPowerOffRelaysAndDelayCommands : indexesToPowerOnRelaysAndDelayCommands, 4, !state);
}

void onClick()
{
  if (!power)
    power = true;
  else
    power = false;

  int indexesToPowerOnRelaysAndDelayCommands[] = {0, 1, 3, DELAY_COMMAND, 2, 4};
  int indexesToPowerOffRelaysAndDelayCommands[] = {2, 4, DELAY_COMMAND, 0, 1, 3};
  int size = POWER_RELAY_COUNT + 1;

  setPowerToRelays(power ? indexesToPowerOnRelaysAndDelayCommands : indexesToPowerOffRelaysAndDelayCommands, size, power);
}

void onLongPressStart()
{
  if (!power)
    return;

  PowerRelay &powerRelay = allPowerRelays[4];
  if (powerRelay.readState())
  {
    powerRelay.writeState(LOW);
    powerRelay.writable = false;
  }
}

void setup()
{
  Serial.begin(COM_PORT_SPEED);
  Serial.println("One Button Example with polling.");

  pinMode(PIN_LED, OUTPUT); // sets the digital pin as output

  digitalWrite(PIN_LED, HIGH);

  for (size_t i = 0; i < POWER_RELAY_COUNT; i++)
  {
    PowerRelay powerRelay = allPowerRelays[i];
    pinMode(powerRelay.iONumber, OUTPUT);
  }

  button.attachDoubleClick(onDoubleClick);
  button.attachClick(onClick);
  button.attachLongPressStart(onLongPressStart);
}

void loop()
{
  // put your main code here, to run repeatedly:

  // keep watching the push button:
  button.tick();

  // You can implement other code in here or just wait a while
  delay(10);
}
