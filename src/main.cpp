#include <Arduino.h>
#include "OneButton.h"

#define POWER_BUTTON_PIN 19
#define INPUT_SELECTOR_BUTTON_PIN 18

#define PIN_LED 15
#define COM_PORT_SPEED 115200
#define DELAY_IN_MILLIS 1000
#define DELAY_COMMAND -1

#define POWER_RELAY_COUNT 5
#define INPUT_RELAY_COUNT 2

class Relay
{
public:
  int iONumber;
  bool writable;

  Relay(int iONumberArg)
  {
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

Relay allPowerRelays[] = {
    Relay(25),
    Relay(26),
    Relay(27),
    Relay(14),
    Relay(12)};

Relay allInputRelays[] = {
    Relay(32),
    Relay(33)};

Relay currentInputRelay = allInputRelays[0];

OneButton powerButton(POWER_BUTTON_PIN);
OneButton inputSelectorButton(INPUT_SELECTOR_BUTTON_PIN);

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

    Relay powerRelay = allPowerRelays[relayIndex];
    powerRelay.writeState(state);
  }
}

void onDoubleClickPowerButton()
{
  if (!power)
    return;

  bool state = allPowerRelays[2].readState();
  int indexesToPowerOffRelaysAndDelayCommands[] = {2, DELAY_COMMAND, 1, 3};
  int indexesToPowerOnRelaysAndDelayCommands[] = {1, 3, DELAY_COMMAND, 2};

  setPowerToRelays(state ? indexesToPowerOffRelaysAndDelayCommands : indexesToPowerOnRelaysAndDelayCommands, 4, !state);
}

void onClickPowerButton()
{
  if (!power)
    power = true;
  else
    power = false;

  int indexesToPowerOnRelaysAndDelayCommands[] = {0, 1, 3, DELAY_COMMAND, 2, 4};
  int indexesToPowerOffRelaysAndDelayCommands[] = {2, 4, DELAY_COMMAND, 0, 1, 3};
  int size = POWER_RELAY_COUNT + 1;

  setPowerToRelays(power ? indexesToPowerOnRelaysAndDelayCommands : indexesToPowerOffRelaysAndDelayCommands, size, power);
  currentInputRelay.writeState(power);
}

void onLongPressPowerButtonStart()
{
  if (!power)
    return;

  Relay &relay = allPowerRelays[4];
  if (relay.readState())
  {
    relay.writeState(LOW);
    relay.writable = false;
  }
}

void onClickInputSelectorButton()
{
  if (!power)
    return;

  for (size_t i = 0; i < INPUT_RELAY_COUNT; i++)
  {

    Relay relay = allInputRelays[i];

    bool newState = !relay.readState();

    relay.writeState(newState);

    if (newState)
      currentInputRelay = relay;
  }
}

void setup()
{
  Serial.begin(COM_PORT_SPEED);
  Serial.println("One Button Example with polling.");

  pinMode(PIN_LED, OUTPUT);
  digitalWrite(PIN_LED, HIGH);

  for (size_t i = 0; i < POWER_RELAY_COUNT; i++)
  {
    Relay relay = allPowerRelays[i];
    pinMode(relay.iONumber, OUTPUT);
  }

  for (size_t i = 0; i < INPUT_RELAY_COUNT; i++)
  {
    Relay relay = allInputRelays[i];
    pinMode(relay.iONumber, OUTPUT);
  }

  powerButton.attachDoubleClick(onDoubleClickPowerButton);
  powerButton.attachClick(onClickPowerButton);
  powerButton.attachLongPressStart(onLongPressPowerButtonStart);

  inputSelectorButton.attachClick(onClickInputSelectorButton);
}

void loop()
{
  powerButton.tick();
  inputSelectorButton.tick();
}
