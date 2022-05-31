#include <ControlBoardEEPROM.h>
#include <Arduino.h>
#include <OneButton.h>
#include <arduino-timer.h>

bool power;
auto timer = timer_create_default();

ControlBoardEEPROM controlBoardEEPROM = ControlBoardEEPROM();

Relay allPowerRelays[] = {
    Relay(25),
    Relay(26),
    Relay(27),
    Relay(14),
    Relay(12)};

Relay allInputRelays[] = {
    Relay(MAIN_INPUT_RELAY_IO_NUMBER),
    Relay(33)};

Relay currentInputRelay = allInputRelays[0];

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

  allPowerRelays[0].writeState(HIGH);
  allPowerRelays[1].writeState(HIGH);
  allPowerRelays[3].writeState(HIGH);

  timer.in(DELAY_IN_MILLIS, [](void *) -> bool
           {  
            allPowerRelays[2].writeState(HIGH);
            allPowerRelays[4].writeState(HIGH);

            currentInputRelay.writeState(HIGH); 

            buzzTwoTimes();;

            return false; });
}

void turnOffPower()
{
  power = false;

  allPowerRelays[2].writeState(LOW);
  allPowerRelays[4].writeState(LOW);

  timer.in(DELAY_IN_MILLIS, [](void *) -> bool
           {
            allPowerRelays[0].writeState(LOW);
            allPowerRelays[1].writeState(LOW);
            allPowerRelays[3].writeState(LOW);

             for (size_t i = 0; i < INPUT_RELAY_COUNT; i++)
              allInputRelays[i].writeState(LOW);

            return false; });
}

void onClickPowerButton()
{
  Serial.println("PowerButtonClick:InitialisingPowerSequence...");

  if (!power)
    turnOnPower();
  else
    turnOffPower();
}

void onDoubleClickPowerButton()
{
  Serial.println("PowerButtonDoubleClick:Stand-bySequenceWasStarting...");

  const bool currentState = allPowerRelays[2].readState();
  if (currentState)
  {
    allPowerRelays[2].writeState(LOW);
    timer.in(DELAY_IN_MILLIS, [](void *) -> bool
             {  
                allPowerRelays[1].writeState(LOW);
                allPowerRelays[3].writeState(LOW); 
                return false; });
  }
  else
  {
    allPowerRelays[1].writeState(HIGH);
    allPowerRelays[3].writeState(HIGH);
    timer.in(DELAY_IN_MILLIS, [](void *) -> bool
             { 
               allPowerRelays[2].writeState(HIGH); 
               return false; });
  }

  buzzTwoTimes();
}

void turnOffPowerRelayAndForbidWriting(Relay *relay)
{
  if (relay->readState())
  {
    relay->writeState(LOW);
    relay->writable = false;
  }
}

void onLongPressPowerButtonStart()
{
  Serial.println("PowerButtonLongPressStart:PoweringOffVU's...");

  Relay *firstRelay = &allPowerRelays[3];
  const bool wasWritable = firstRelay->writable;
  if (wasWritable)
  {
    buzzOneTime(LONG_BUZZ_TIME);

    turnOffPowerRelayAndForbidWriting(firstRelay);

    timer.in(
        DELAY_IN_MILLIS, [](void *) -> bool
        {
          turnOffPowerRelayAndForbidWriting(&allPowerRelays[3]);
          digitalWrite(BUZZER_PIN, LOW);
          return false; });
  }
}

bool allInputSelectorsHasState(int state)
{
  for (size_t i = 0; i < INPUT_RELAY_COUNT; i++)
  {
    Relay relay = allInputRelays[i];
    if (relay.readState() != state)
      return false;
  }

  return true;
}

void onClickInputSelectorButton()
{
  Serial.println("InptSelectorButtonCick:SwitcingInputs...");

  if (allInputSelectorsHasState(LOW))
    currentInputRelay.writeState(HIGH);
  else if (allInputSelectorsHasState(HIGH))
  {
    for (size_t i = 0; i < INPUT_RELAY_COUNT; i++)
    {
      Relay relay = allInputRelays[i];
      if (relay.iONumber != currentInputRelay.iONumber)
        relay.writeState(LOW);
    }
  }
  else
  {
    for (size_t i = 0; i < INPUT_RELAY_COUNT; i++)
    {
      Relay relay = allInputRelays[i];
      const bool invertedState = !relay.readState();
      relay.writeState(invertedState);

      if (invertedState)
      {
        currentInputRelay = relay;
        controlBoardEEPROM.writeCurrentInputRelayIONumber(relay.iONumber);
      }
    }
  }

  buzzOneTime();
}

void onDoubleClickInputSelectorButton()
{
  Serial.println("InptSelectorButtonLongPressStart:TurnOnInputsOrTurnOnPrevious...");

  bool buzTwoTimes = true;

  for (size_t i = 0; i < INPUT_RELAY_COUNT; i++)
  {
    Relay relay = allInputRelays[i];
    if (relay.readState() && relay.iONumber != currentInputRelay.iONumber)
    {
      relay.writeState(LOW);
      buzTwoTimes = false;
    }
    else
      relay.writeState(HIGH);
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
    if (relay.readState())
      relay.writeState(LOW);
    else if (relay.iONumber == currentInputRelay.iONumber)
    {
      relay.writeState(HIGH);
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
}

void loop()
{
  timer.tick();
  // watching the push buttons:
  powerButton.tick();
  inputSelectorButton.tick();
  mainPowerOnButton.tick();
}
