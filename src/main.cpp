#include <ControlBoardEEPROM.h>
#include <Led.h>
#include <Arduino.h>
#include <OneButton.h>

bool power;
auto timer = timer_create_default();

int dutyCycle = 0;
int currentDutyCycleOperation = HIGH;

ControlBoardEEPROM controlBoardEEPROM = ControlBoardEEPROM();

Relay allPowerRelays[] = {
    Relay(25),
    Relay(26),
    Relay(27),
    Relay(14),
    Relay(12)};

Relay allInputRelays[] = {
    Relay(MAIN_INPUT_RELAY_IO_NUMBER),
    Relay(SECONDARY_INPUT_RELAY_IO_NUMBER)};

Relay currentInputRelay = allInputRelays[0];

Led powerLed = Led(POWER_BUTTON_LED_CHANNEL, POWER_BUTTON_LED_PIN);
Led inputSelectorLed = Led(INPUT_SELECTOR_BUTTON_LED_CHANNEL, INPUT_SELECTOR_BUTTON_LED_PIN);

OneButton powerButton(POWER_BUTTON_PIN);                  // Setup a new OneButton on pin 19
OneButton inputSelectorButton(INPUT_SELECTOR_BUTTON_PIN); // Setup a new OneButton on pin 18
OneButton mainPowerOnButton(MAIN_POWER_ON_PIN);           // Setup a new (virtual) OneButton on pin 21

void tryReinitCurrentInputRelayFromEEPROM()
{
  const byte readIONumber = controlBoardEEPROM.readCurrentInputRelayIONumber();
  for (size_t i = 0; i < INPUT_RELAY_COUNT; i++)
  {
    Relay relay = allInputRelays[i];
    if (relay.iONumber == readIONumber)
      currentInputRelay = relay;
  }
}

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

void buzzOneTime(unsigned long delay = SHORT_BUZZ_TIME)
{
  digitalWrite(BUZZER_PIN, HIGH);

  timer.in(delay, [](void *) -> bool
           {
                digitalWrite(BUZZER_PIN, LOW);
                return false; });
}

void buzzTwoTimes()
{
  digitalWrite(BUZZER_PIN, HIGH);

  timer.in(SHORT_BUZZ_TIME, [](void *) -> bool
           {
              digitalWrite(BUZZER_PIN, LOW);
              timer.in(SHORT_BUZZ_TIME, [](void *) -> bool
                {  
                  buzzOneTime();
                  return false;
                }
                ); 
            return false; });
}

void turnOnPower()
{
  power = true;

  allPowerRelays[0].write(HIGH);
  allPowerRelays[1].write(HIGH);
  allPowerRelays[3].write(HIGH);

  timer.in(DELAY_IN_MILLIS, [](void *) -> bool
           {  
            allPowerRelays[2].write(HIGH);
            allPowerRelays[4].write(HIGH);

            currentInputRelay.write(HIGH); 


            powerLed.writeMax();
            inputSelectorLed.writeMax();

            buzzTwoTimes();

            return false; });
}

void turnOffPower()
{
  power = false;

  allPowerRelays[2].write(LOW);
  allPowerRelays[4].write(LOW);

  timer.in(DELAY_IN_MILLIS, [](void *) -> bool
           {
            allPowerRelays[0].write(LOW);
            allPowerRelays[1].write(LOW);
            allPowerRelays[3].write(LOW);

            for (size_t i = 0; i < INPUT_RELAY_COUNT; i++)
              allInputRelays[i].write(LOW);

            powerLed.writeMin();
            inputSelectorLed.writeMin();

            return false; });
}

void onClickPowerButton()
{
  Serial.println("PowerButtonClick:InitialisingPowerSequence...");

  buzzOneTime();

  if (!power)
    turnOnPower();
  else
    turnOffPower();
}

void onDoubleClickPowerButton()
{
  Serial.println("PowerButtonDoubleClick:Stand-bySequenceWasStarting...");

  const bool currentState = allPowerRelays[2].read();
  if (currentState)
  {
    allPowerRelays[2].write(LOW);
    timer.in(DELAY_IN_MILLIS, [](void *) -> bool
             {  
                allPowerRelays[1].write(LOW);
                allPowerRelays[3].write(LOW); 
                return false; });
  }
  else
  {
    allPowerRelays[1].write(HIGH);
    allPowerRelays[3].write(HIGH);
    timer.in(DELAY_IN_MILLIS, [](void *) -> bool
             { 
               allPowerRelays[2].write(HIGH); 
               return false; });
  }

  buzzTwoTimes();
}

void onLongPressPowerButtonStart()
{
  Serial.println("PowerButtonLongPressStart:PoweringOffVU's...");

  Relay firstRelay = allPowerRelays[3];
  const bool wasWritable = firstRelay.isWritable();
  if (wasWritable)
  {
    buzzOneTime(LONG_BUZZ_TIME);

    firstRelay.writeAndForbidWriting(LOW);

    timer.in(
        DELAY_IN_MILLIS, [](void *) -> bool
        {
          allPowerRelays[3].writeAndForbidWriting(LOW);
          digitalWrite(BUZZER_PIN, LOW);
          return false; });
  }
}

bool allInputSelectorsHasState(int state)
{
  for (size_t i = 0; i < INPUT_RELAY_COUNT; i++)
  {
    Relay relay = allInputRelays[i];
    if (relay.read() != state)
      return false;
  }

  return true;
}

void onClickInputSelectorButton()
{
  Serial.println("InptSelectorButtonCick:SwitcingInputs...");

  if (allInputSelectorsHasState(LOW))
    currentInputRelay.write(HIGH);
  else if (allInputSelectorsHasState(HIGH))
  {
    for (size_t i = 0; i < INPUT_RELAY_COUNT; i++)
    {
      Relay relay = allInputRelays[i];
      if (relay.iONumber != currentInputRelay.iONumber)
        relay.write(LOW);
    }
  }
  else
  {
    for (size_t i = 0; i < INPUT_RELAY_COUNT; i++)
    {
      Relay relay = allInputRelays[i];
      const bool invertedState = !relay.read();
      relay.write(invertedState);

      if (invertedState)
      {
        currentInputRelay = relay;
        controlBoardEEPROM.writeCurrentInputRelayIONumber(relay.iONumber);
      }
    }
  }
  inputSelectorLed.blink(&timer);
  buzzOneTime();
}

void onDoubleClickInputSelectorButton()
{
  Serial.println("InptSelectorButtonLongPressStart:TurnOnInputsOrTurnOnPrevious...");

  bool buzTwoTimes = true;

  for (size_t i = 0; i < INPUT_RELAY_COUNT; i++)
  {
    Relay relay = allInputRelays[i];
    if (relay.read() && relay.iONumber != currentInputRelay.iONumber)
    {
      relay.write(LOW);
      buzTwoTimes = false;
    }
    else
      relay.write(HIGH);
  }

  if (buzTwoTimes)
    buzzTwoTimes();
  else
    buzzOneTime();
}

void onLongPressInputSelectorButtonStart()
{
  Serial.println("InptSelectorButtonLongPressStart:TurnOffInputsOrTurnOnPrevious...");

  for (size_t i = 0; i < INPUT_RELAY_COUNT; i++)
  {
    Relay relay = allInputRelays[i];
    if (relay.read())
      relay.write(LOW);
    else if (relay.iONumber == currentInputRelay.iONumber)
    {
      relay.write(HIGH);
      buzzOneTime();
    }
  }
}

void onLongPressMainPowerOnButtonStart()
{
  Serial.println("MasterDeviceOff:PowerSequenceWasStopping...");

  if (power)
    turnOffPower();
}

void onLongPressMainPowerOnButtonStop()
{
  Serial.println("MasterDeviceOn:MainPowerOnSequenceWasStarting...");

  if (!power)
    turnOnPower();
}

void setup()
{
  Serial.begin(COM_PORT_SPEED);
  Serial.println("OneButton Starting...");

  tryReinitCurrentInputRelayFromEEPROM();

  // Setup PULLUPS: INPUT_PULLUP - means pushbutton connected to VCC, INPUT_PULLDOWN - means pushbutton connected to GND
  pinMode(POWER_BUTTON_PIN, INPUT_PULLUP);
  pinMode(INPUT_SELECTOR_BUTTON_PIN, INPUT_PULLUP);
  pinMode(MAIN_POWER_ON_PIN, INPUT_PULLDOWN);

  pinMode(POWER_BUTTON_LED_PIN, OUTPUT);
  pinMode(INPUT_SELECTOR_BUTTON_LED_PIN, OUTPUT);

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

  powerButton.attachClick([]()
                          { withRunningTaskCheck(onClickPowerButton); });
  powerButton.attachDoubleClick([]()
                                { withPowerCheck(onDoubleClickPowerButton); });
  powerButton.attachLongPressStart([]()
                                   { withPowerCheck(onLongPressPowerButtonStart); });

  inputSelectorButton.attachClick([]()
                                  { withPowerCheck(onClickInputSelectorButton); });
  inputSelectorButton.attachLongPressStart([]()
                                           { withPowerCheck(onLongPressInputSelectorButtonStart); });
  inputSelectorButton.attachDoubleClick([]()
                                        { withPowerCheck(onDoubleClickInputSelectorButton); });

  mainPowerOnButton.attachLongPressStart([]()
                                         { withRunningTaskCheck(onLongPressMainPowerOnButtonStart); });
  mainPowerOnButton.attachLongPressStop([]()
                                        { withRunningTaskCheck(onLongPressMainPowerOnButtonStop); });

  powerLed.setup();
  inputSelectorLed.setup();
}

void loop()
{
  timer.tick();
  // watching the push buttons:
  powerButton.tick();
  inputSelectorButton.tick();
  mainPowerOnButton.tick();
}
