#include "checkers.h"
#include <InputSelector.h>
#include <Buzzer.h>
#include <OneButton.h>

bool power;

Timer<> timer = timer_create_default();

Buzzer buzzer = Buzzer(&timer);

Relay allPowerRelays[] = {
    Relay(25),
    Relay(26),
    Relay(27),
    Relay(14),
    Relay(12)};

Led powerLed = Led(POWER_BUTTON_LED_CHANNEL, POWER_BUTTON_LED_PIN, &timer);
Led inputSelectorLed = Led(INPUT_SELECTOR_BUTTON_LED_CHANNEL, INPUT_SELECTOR_BUTTON_LED_PIN, &timer);

OneButton powerButton(POWER_BUTTON_PIN);
OneButton inputSelectorButton = OneButton(INPUT_SELECTOR_BUTTON_PIN);
OneButton mainPowerOnButton(MAIN_POWER_ON_PIN);

InputSelector inputSelector = InputSelector();

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
            powerLed.writeMax();

            inputSelector.writeToSeletedRelay(HIGH);
            inputSelectorLed.writeMax();

            buzzer.buzz(4);

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
            powerLed.writeMin();

            inputSelector.writeToAllRelays(LOW);
            inputSelectorLed.writeMin();


            return false; });
}

void onClickPowerButton()
{
  Serial.println("PowerButtonClick:InitialisingPowerSequence...");

  buzzer.buzz(2);

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

  buzzer.buzz(4);
}

void onLongPressPowerButtonStart()
{
  Serial.println("PowerButtonLongPressStart:PoweringOffVU's...");

  Relay firstRelay = allPowerRelays[3];
  const bool wasWritable = firstRelay.isWritable();
  if (wasWritable)
  {
    buzzer.buzz(2, LONG_BUZZ_INTERVAL);

    firstRelay.writeAndForbidWriting(LOW);

    timer.in(
        DELAY_IN_MILLIS, [](void *) -> bool
        {
          allPowerRelays[3].writeAndForbidWriting(LOW);
          digitalWrite(BUZZER_PIN, LOW);
          powerLed.writeMin();
          return false; });
  }

  powerLed.writeInverted();
}

void onClickInputSelectorButton()
{

  const bool areAllRelaysLow = inputSelector.areAllRelays(LOW);

  if (areAllRelaysLow)
  {
    Serial.println("InputSelectorButtonCick:TurnOnSelectedInput...");
    inputSelector.writeToSeletedRelay(HIGH);
  }
  else if (inputSelector.areAllRelays(HIGH))
  {
    Serial.println("InputSelectorButtonCick:TurnOnNotSelectedInput...");
    inputSelector.writeToNotSelected(LOW);
  }
  else
  {
    Serial.println("InputSelectorButtonCick:SwapInputs...");
    inputSelector.swapRelays();
  }

  const byte invertCount = inputSelector.getCountToInvert();
  inputSelectorLed.blink(areAllRelaysLow ? invertCount + 1 : invertCount);
  buzzer.buzz(invertCount);
}

void onDoubleClickInputSelectorButton()
{
  if (inputSelector.areAllRelays(HIGH))
  {
    Serial.println("InputSelectorButtonDoubleClick:TurnOnSelectedInput...");

    inputSelector.writeToNotSelected(LOW);

    const byte invertCount = inputSelector.getCountToInvert();
    inputSelectorLed.blink(invertCount);
    buzzer.buzz(invertCount);
  }
  else
  {
    Serial.println("InputSelectorButtonDoubleClick:TurnOnAllInputs...");

    const bool areAllRelaysLow = inputSelector.areAllRelays(LOW);

    inputSelector.writeToAllRelays(HIGH);

    inputSelectorLed.blink(areAllRelaysLow ? 3 : 2, LONG_LED_BLINK_INTERVAL);
    buzzer.buzz(2, LONG_BUZZ_INTERVAL);
  }
}

void onLongPressInputSelectorButtonStart()
{
  if (inputSelector.areAllRelays(LOW))
  {
    Serial.println("InptSelectorButtonLongPressStart:TurnOnSelectedInput...");

    inputSelector.writeToSeletedRelay(HIGH);

    const byte invertCount = inputSelector.getCountToInvert();
    inputSelectorLed.blink(invertCount + 1);
    buzzer.buzz(invertCount);
  }
  else
  {
    Serial.println("InptSelectorButtonLongPressStart:TurnOffAllInputs...");

    inputSelector.writeToAllRelays(LOW);
    inputSelectorLed.writeMin();
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

  // Setup PULLUPS: INPUT_PULLUP - means pushbutton connected to VCC, INPUT_PULLDOWN - means pushbutton connected to GND
  pinMode(POWER_BUTTON_PIN, INPUT_PULLUP);
  pinMode(INPUT_SELECTOR_BUTTON_PIN, INPUT_PULLUP);
  pinMode(MAIN_POWER_ON_PIN, INPUT_PULLDOWN);

  powerLed.setup();
  inputSelectorLed.setup();

  buzzer.setup();

  for (size_t i = 0; i < POWER_RELAY_COUNT; i++)
    allPowerRelays[i].setup();

  inputSelector.setup();

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
