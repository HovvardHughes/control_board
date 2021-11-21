#include <Arduino.h>
#include "OneButton.h"

#define PIN_INPUT 23
#define PIN_LED 17
#define COM_PORT_SPEED 115200
#define DELAY_IN_MILLIS 5000
#define DELAY_COMMAND  -1

#define my_sizeof(type) ((char *)(&type + 1) - (char *)(&type))

class PowerRelay
{
public:
  int order;
  int iONumber;
  int state;

  PowerRelay(int orderArg, int iONumberArg, int stateArg)
  {

    order = orderArg;
    iONumber = iONumberArg;
    state = stateArg;
  }

  void write(int newState)
  {
    // test 
    state = newState;
    digitalWrite(iONumber, newState);
  }
};

int ledState = HIGH;
bool power = false;

#define POWER_RELAY_COUNT 5
PowerRelay allPowerRelays[POWER_RELAY_COUNT] = {
    PowerRelay(0, 25, LOW),
    PowerRelay(1, 26, LOW),
    PowerRelay(2, 27, LOW),
    PowerRelay(3, 14, LOW),
    PowerRelay(4, 12, LOW)};

OneButton button(PIN_INPUT, true);

// 1.
// Single Press = MAIN PWRON/OFF, Usual operation
// ON - 1+2+4..timer..3+5	OFF - 3+5..timer..1+2+4
// 2.
// Double Press = Stand-By ON/OFF
// ON - 2+4..timer..3	OFF - 3..timer..2+4     5=>previous state
// 3.
// Long Press = Indicaion OFF
// OFF - 5..timer..4
// Must work only on power-off mode, to prevent sparking!
// (Power-on requires power cycling for whole system.)


void doubleClick()
{
  Serial.println("x2");

  ledState = !ledState; // reverse the LED
  digitalWrite(PIN_LED, ledState);
}


void setPowerToRelays(int relayIndexesAndDelayCommands[], int size, int state)
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
    powerRelay.write(state);
  }
}

void onButtonClick()
{
  if (!power)
    power = true;
  else
    power = false;

  int indexesToPowerOnRelaysAndDelayCommands[] = {0, 1, 3, DELAY_COMMAND, 2, 4};
  int indexesToPowerOffRelaysAndDelayCommands[] = {2, 4, DELAY_COMMAND, 0, 1, 3};
  int size = POWER_RELAY_COUNT + 1;

  setPowerToRelays(power ? indexesToPowerOnRelaysAndDelayCommands : indexesToPowerOffRelaysAndDelayCommands, size, int(power));
}

void setup()
{
  Serial.begin(COM_PORT_SPEED);
  Serial.println("One Button Example with polling.");

  pinMode(PIN_LED, OUTPUT); // sets the digital pin as output

  digitalWrite(PIN_LED, ledState);

  for (size_t i = 0; i < POWER_RELAY_COUNT; i++)
  {
    PowerRelay powerRelay = allPowerRelays[i];
    pinMode(powerRelay.iONumber, OUTPUT);
  }

  // link the doubleclick function to be called on a doubleclick event.
  button.attachDoubleClick(doubleClick);
  button.attachClick(onButtonClick);
}

void loop()
{
  // put your main code here, to run repeatedly:

  // keep watching the push button:
  button.tick();

  // You can implement other code in here or just wait a while
  delay(10);
}
