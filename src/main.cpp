
#include <ControlBoardEEPROM.h>
#include <Arduino.h>
#include <OneButton.h>
#include <AsyncTimer.h>

bool power;

AsyncTimer t;
bool isLongButtonTaskRunning;

ControlBoardEEPROM controlBoardEEPROM;

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

void tryReinitCurrentInputRelayFromEEPROM()
{
  byte readIONumber = controlBoardEEPROM.readCurrentInputRelayIONumberFromEEPROM();
  for (size_t i = 0; i < INPUT_RELAY_COUNT; i++)
  {
    Relay relay = allInputRelays[i];
    if (relay.iONumber == readIONumber)
      currentInputRelay = relay;
  }
}

OneButton powerButton(POWER_BUTTON_PIN);                  // Setup a new OneButton on pin 19
OneButton inputSelectorButton(INPUT_SELECTOR_BUTTON_PIN); // Setup a new OneButton on pin 18
OneButton mainPowerOnButton(MAIN_POWER_ON_PIN);           // Setup a new (virtual) OneButton on pin 21

void BuzzOneTime()
{
  digitalWrite(BUZZER_PIN, HIGH);

  t.setTimeout([]()
               { digitalWrite(BUZZER_PIN, LOW); },
               BUZZ_TIME);
}

void BuzzTwoTimes()
{
  digitalWrite(BUZZER_PIN, HIGH);

  t.setTimeout([]()
               {
                 digitalWrite(BUZZER_PIN, LOW);
                 t.setTimeout([]()
                             { 
                              digitalWrite(BUZZER_PIN, HIGH); 
                              t.setTimeout([]()
                                  { digitalWrite(BUZZER_PIN, LOW); },   
                                  BUZZ_TIME);
                               },
                             BUZZ_TIME); },
               BUZZ_TIME);
}

void setPowerStateToRelaysInLinearOrder()
{
  isLongButtonTaskRunning = true;

  allPowerRelays[0].writeState(power);
  allPowerRelays[1].writeState(power);
  allPowerRelays[3].writeState(power);
  t.setTimeout([]()
               {  
                    allPowerRelays[2].writeState(power);
                    allPowerRelays[4].writeState(power);
                    currentInputRelay.writeState(power); 
                    BuzzTwoTimes(); 
                    isLongButtonTaskRunning = false; },
               DELAY_IN_MILLIS);
}

void setPowerStateToRelaysInReverseOrder()
{
  isLongButtonTaskRunning = true;

  allPowerRelays[2].writeState(power);
  allPowerRelays[4].writeState(power);
  t.setTimeout([]()
               {
    allPowerRelays[0].writeState(power);
    allPowerRelays[1].writeState(power);
    allPowerRelays[3].writeState(power);
    currentInputRelay.writeState(power);
    isLongButtonTaskRunning = false; },
               DELAY_IN_MILLIS);
}

void onClickPowerButton()
{
  Serial.println("PowerButtonClick:InitialisingPowerSequence...");

  power = !power;
  if (power)
    setPowerStateToRelaysInLinearOrder();
  else
    setPowerStateToRelaysInReverseOrder();
}

void withRunningTaskCheck(void (*action)())
{
  Serial.println("WithRunningTaskCheck...");
  if (!isLongButtonTaskRunning)
    action();
}

void withPowerCheck(void (*action)())
{
  Serial.println("WithPowerCheck...");
  if (power)
    withRunningTaskCheck(action);
}
void onDoubleClickPowerButton()
{
  Serial.println("PowerButtonDoubleClick:Stand-bySequenceWasStarting...");

  isLongButtonTaskRunning = true;

  bool currentState = allPowerRelays[2].readState();
  if (currentState)
  {
    allPowerRelays[2].writeState(LOW);
    t.setTimeout([]()
                 {  
                    allPowerRelays[1].writeState(LOW);
                    allPowerRelays[3].writeState(LOW); 
                    isLongButtonTaskRunning = false; },
                 DELAY_IN_MILLIS);
  }
  else
  {
    allPowerRelays[1].writeState(HIGH);
    allPowerRelays[3].writeState(HIGH);
    t.setTimeout([]()
                 {
                   allPowerRelays[2].writeState(HIGH);
                   isLongButtonTaskRunning = false; },
                 DELAY_IN_MILLIS);
  }
}

void TurnOffPowerRelayAndForbidWriting(Relay *relay)
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
  isLongButtonTaskRunning = true;

  TurnOffPowerRelayAndForbidWriting(&allPowerRelays[4]);
  t.setTimeout([]()
               {
                 Relay* relay = &allPowerRelays[3];
                 bool wasWritable = relay->writable;

                 TurnOffPowerRelayAndForbidWriting(relay);
                
                 if(wasWritable) 
                    BuzzTwoTimes();
    
                  isLongButtonTaskRunning = false; },
               DELAY_IN_MILLIS);
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
      bool invertedState = !relay.readState();
      relay.writeState(invertedState);

      if (invertedState)
      {
        currentInputRelay = relay;
        controlBoardEEPROM.writeCurrentInputRelayIONumber(relay.iONumber);
      }
    }
  }

  BuzzOneTime();
}

void onLongPressMainPowerOnButtonStart()
{
  Serial.println("MasterDeviceOff:PowerSequenceWasStopping...");

  if (power)
    power = false;

  setPowerStateToRelaysInReverseOrder();
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
    BuzzTwoTimes();
  else
    BuzzOneTime();
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
      BuzzOneTime();
    }
  }
}

void onLongPressMainPowerOnButtonStop()
{
  Serial.println("MasterDeviceOn:MainPowerOnSequenceWasStarting...");

  if (!power)
    power = true;

  setPowerStateToRelaysInLinearOrder();
}

void setup()
{
  Serial.begin(COM_PORT_SPEED);
  Serial.println("OneButton Starting...");

  controlBoardEEPROM = ControlBoardEEPROM();
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
  t.handle();
  // watching the push buttons:
  powerButton.tick();
  inputSelectorButton.tick();
  mainPowerOnButton.tick();
}
