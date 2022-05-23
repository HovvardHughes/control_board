
#include <constants.h>
#include <Arduino.h>
#include <OneButton.h>
#include <Ticker.h>
#include <Relay.h>

int buzzerState = LOW;

void BuzzTicker()
{
  buzzerState = !buzzerState;
  digitalWrite(BUZZER_PIN, buzzerState);
  Serial.println("Buzz!");
}

Ticker timer1(BuzzTicker, BUZZ_TIME, BUZZ_NUMBER * 2); // once, immediately

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

OneButton powerButton(POWER_BUTTON_PIN);                  // Setup a new OneButton on pin 19
OneButton inputSelectorButton(INPUT_SELECTOR_BUTTON_PIN); // Setup a new OneButton on pin 18
OneButton mainPowerOnButton(MAINPOWERON_PIN);             // Setup a new (virtual) OneButton on pin 21

static void setPowerToRelays(int relayIndexesAndDelayCommands[], int size, int state)
{
  for (size_t i = 0; i < size; i++)
  {
    int relayIndex = relayIndexesAndDelayCommands[i];

    if (relayIndex == DELAY_COMMAND)
    {
      Serial.println("Pre-programmedHeatingFilamentsBeginning...");
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
  timer1.start();
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

  // Setup PULLUPS: INPUT_PULLUP - means pushbutton connected to VCC, INPUT_PULLDOWN - means pushbutton connected to GND
  pinMode(POWER_BUTTON_PIN, INPUT_PULLUP);
  pinMode(INPUT_SELECTOR_BUTTON_PIN, INPUT_PULLUP);
  pinMode(MAINPOWERON_PIN, INPUT_PULLDOWN);

  pinMode(POWER_BUTTON_LED_PIN, OUTPUT);
  // digitalWrite(POWER_BUTTON_LED_PIN, HIGH);  //dummy

  pinMode(INPUT_SELECTOR_BUTTON_LED_PIN, OUTPUT);
  // digitalWrite(INPUT_SELECTOR_BUTTON_LED_PIN, HIGH);    //dummy

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
  // watching the push buttons:
  powerButton.tick();
  inputSelectorButton.tick();
  mainPowerOnButton.tick();

  timer1.update(); // it will check the Ticker and if necessary, it will run the callback function.
}
