 /* 
 Setup:
 * Connect a btn0 to pin 19 (ButtonPin) and ground.
 * Connect a btn1 to pin 18 (ButtonPin) and ground.
 * 
 * The Serial interface is used for output the detected events. 
 */

#include <Arduino.h>
#include "OneButton.h"

/*
GPIO Pins Corresponding:

GPIO      Connected to...
2         BTNLED1             'INPUT_SELECTOR_BUTTON_LED'
4         SPK                 'BUZZER'
12        PRLY4               'PowerRelay(12)'  (360VDCOUT)
13        1WIRE               'TEMP_SENS'
14        PRLY3               'PowerRelay(14)'  (6.3ACOUT)
15        BTNLED0             'POWER_BUTTON_LED'
16        VOLUME MOTOR REV
17        VOLUME MOTOR FWD
18        BTN1                'INPUT_SELECTOR_BUTTON'
19        BTN0                'POWER_BUTTON'
21        MPWRON              'MAINPOWERON'
22        SDA                 'I2C_SDA'
23        SCL                 'I2C_SCL'
25        PRLY0               'PowerRelay(25)'  (230VACIN)
26        PRLY1               'PowerRelay(26)'  (12ACOUT)
27        PRLY2               'PowerRelay(27)'  (250VACOUT)
32        INPTRLY0            'InputRelay(32)'  (IN1)
33        INPTRLY1            'InputRelay(33)'  (IN2)
36        ADC1                'SELFVIN'
*/

//BUTTONS
#define POWER_BUTTON_PIN 19
#define INPUT_SELECTOR_BUTTON_PIN 18
#define MAINPOWERON_PIN 21
//LEDS
#define POWER_BUTTON_LED_PIN 15
#define INPUT_SELECTOR_BUTTON_LED_PIN 2
//OTHER
#define BUZZER_PIN 4


#define COM_PORT_SPEED 115200
#define DELAY_IN_MILLIS 1000  //For Debugging
//#define DELAY_IN_MILLIS 25000    //For Release - Pre-programmed Heating Filaments Delay
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

OneButton powerButton(POWER_BUTTON_PIN);  // Setup a new OneButton on pin 19
OneButton inputSelectorButton(INPUT_SELECTOR_BUTTON_PIN); // Setup a new OneButton on pin 18
OneButton mainPowerOnButton(MAINPOWERON_PIN); // Setup a new (virtual) OneButton on pin 21

static void setPowerToRelays(int relayIndexesAndDelayCommands[], int size, int state)
{
  for (size_t i = 0; i < size; i++)
  {
    int relayIndex = relayIndexesAndDelayCommands[i];

    if (relayIndex == DELAY_COMMAND)
    {
      Serial.println ("Pre-programmedHeatingFilamentsBeginning...");
      delay(DELAY_IN_MILLIS);
      continue;
    }

    Relay powerRelay = allPowerRelays[relayIndex];
    powerRelay.writeState(state);
  }
}

void onDoubleClickPowerButton()
{
  Serial.println("PowerButtonDoubleClick:Stand-bySequenceWasStarting...");

  if (!power)
    return;

  bool state = allPowerRelays[2].readState();
  int indexesToPowerOffRelaysAndDelayCommands[] = {2, DELAY_COMMAND, 1, 3};
  int indexesToPowerOnRelaysAndDelayCommands[] = {1, 3, DELAY_COMMAND, 2};

  setPowerToRelays(state ? indexesToPowerOffRelaysAndDelayCommands : indexesToPowerOnRelaysAndDelayCommands, 4, !state);
}

void onClickPowerButton()
{
  Serial.println("PowerButtonClick:InitialisingPowerSequence...");

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
  Serial.println("PowerButtonLongpressStart:PoweringOffVU's...");

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
  Serial.println("InptSelectorButtonCick:SwitcingInputs...");

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

void onLongPressMainPowerOnButtonStart()
{ 
  Serial.println("MasterDeviceOff:PowerSequenceWasStopping...");
     if (power)
    power = false; 

    int indexesToPowerOffRelaysAndDelayCommands[] = {2, 4, DELAY_COMMAND, 0, 1, 3};
    int size = POWER_RELAY_COUNT + 1;

  setPowerToRelays(indexesToPowerOffRelaysAndDelayCommands, size, power);
  currentInputRelay.writeState(power);
}

void onLongPressMainPowerOnButtonStop()
{
  Serial.println("MasterDeviceOn:MainPowerOnSequenceWasStarting...");
    if (!power)
    power = true;

    int indexesToPowerOnRelaysAndDelayCommands[] = {0, 1, 3, DELAY_COMMAND, 2, 4};
    int size = POWER_RELAY_COUNT + 1;

  setPowerToRelays(indexesToPowerOnRelaysAndDelayCommands, size, power);
  currentInputRelay.writeState(power);
}

void setup()
{
  Serial.begin(COM_PORT_SPEED);
  Serial.println("OneButton Starting...");

  //Setup PULLUPS: INPUT_PULLUP - means pushbutton connected to VCC, INPUT_PULLDOWN - means pushbutton connected to GND
  pinMode(POWER_BUTTON_PIN, INPUT_PULLUP);
  pinMode(INPUT_SELECTOR_BUTTON_PIN, INPUT_PULLUP);
  pinMode(MAINPOWERON_PIN, INPUT_PULLDOWN);

  pinMode(POWER_BUTTON_LED_PIN, OUTPUT);
 // digitalWrite(POWER_BUTTON_LED_PIN, HIGH);  //dummy

  pinMode(INPUT_SELECTOR_BUTTON_LED_PIN, OUTPUT);
 //digitalWrite(INPUT_SELECTOR_BUTTON_LED_PIN, HIGH);    //dummy

  pinMode(BUZZER_PIN, OUTPUT);


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

  mainPowerOnButton.attachLongPressStart(onLongPressMainPowerOnButtonStart);
  mainPowerOnButton.attachLongPressStop(onLongPressMainPowerOnButtonStop);
}

void loop()
{
  //watching the push buttons:
  powerButton.tick();
  inputSelectorButton.tick();
  mainPowerOnButton.tick();
}
