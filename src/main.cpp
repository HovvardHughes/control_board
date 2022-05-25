
#include <constants.h>
#include <Arduino.h>
#include <OneButton.h>
#include <Relay.h>
#include <AsyncTimer.h>

AsyncTimer t;
bool isLongButtonTaskRunning;

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

void Buzz()
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

void WithRunningTaskCheck(void (*action)())
{
  Serial.println("WithRunningTaskCheck...");
  if (!isLongButtonTaskRunning)
    action();
}

void WithPowerCheck(void (*action)())
{
  Serial.println("WithPowerCheck...");
  if (power)
    WithRunningTaskCheck(action);
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
                    Buzz(); 
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

void onLongPressPowerButtonStart()
{
  Serial.println("PowerButtonLongpressStart:PoweringOffVU's...");

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

  for (size_t i = 0; i < INPUT_RELAY_COUNT; i++)
  {

    Relay relay = allInputRelays[i];

    bool invertedState = !relay.readState();

    relay.writeState(invertedState);

    if (invertedState)
      currentInputRelay = relay;
  }
}

void onLongPressMainPowerOnButtonStart()
{
  Serial.println("MasterDeviceOff:PowerSequenceWasStopping...");

  if (isLongButtonTaskRunning)
    return;

  if (power)
    power = false;

  setPowerStateToRelaysInReverseOrder();
}

void onLongPressMainPowerOnButtonStop()
{
  Serial.println("MasterDeviceOn:MainPowerOnSequenceWasStarting...");

  if (isLongButtonTaskRunning)
    return;

  if (!power)
    power = true;

  setPowerStateToRelaysInLinearOrder();
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

  powerButton.attachClick([]()
                          { WithRunningTaskCheck(onClickPowerButton); });

  powerButton.attachDoubleClick([]()
                                { WithPowerCheck(onDoubleClickPowerButton); });
  powerButton.attachDoubleClick([]()
                                { WithPowerCheck(onLongPressPowerButtonStart); });

  inputSelectorButton.attachDoubleClick([]()
                                        { WithPowerCheck(onClickInputSelectorButton); });

  mainPowerOnButton.attachDoubleClick([]()
                                      { WithRunningTaskCheck(onLongPressMainPowerOnButtonStart); });
  mainPowerOnButton.attachDoubleClick([]()
                                      { WithRunningTaskCheck(onLongPressMainPowerOnButtonStop); });
}

void loop()
{
  t.handle();
  // watching the push buttons:
  powerButton.tick();
  inputSelectorButton.tick();
  mainPowerOnButton.tick();
}
