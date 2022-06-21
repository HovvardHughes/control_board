#include <OneButton.h>
#include <powerManagement.h>

extern bool power;
extern bool sleepMode;

extern Timer<> timer ;

extern Buzzer buzze;

extern OneButton powerButton;
extern OneButton inputSelectorButton;
extern OneButton mainPowerOnButton;

extern InputSelector inputSelector;

extern Led inputSelectorLed;

void onClickPowerButton()
{
  buzzer.buzz(2);

  if (!power)
  {
    Serial.println("PowerButtonClick:TurnOnPower...");
    turnOnPower();
  }
  else
  {
    Serial.println("PowerButtonClick:TurnOffPower...");
    turnOffPower();
  }
}

void onDoubleClickPowerButton()
{
  buzzer.buzz(4);

  if (allPowerRelays[2].read())
  {
    Serial.println("PowerButtonDoubleClick:TurnOnSleepMode...");
    turnOnSleepMode();
  }
  else
  {
    Serial.println("PowerButtonDoubleClick:TurnOffSleepMode...");
    turnOffSleepMode();
  }
}

void onLongPressPowerButtonStart()
{
  Serial.println("PowerButtonLongPressStart:PoweringOffVU's...");

  Relay firstRelay = allPowerRelays[4];
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

  const byte invertCount = inputSelector.getInvertCount();
  inputSelectorLed.blink(areAllRelaysLow ? invertCount + 1 : invertCount);
  buzzer.buzz(invertCount);
}

void onDoubleClickInputSelectorButton()
{
  if (inputSelector.areAllRelays(HIGH))
  {
    Serial.println("InputSelectorButtonDoubleClick:TurnOnSelectedInput...");

    inputSelector.writeToNotSelected(LOW);

    const byte invertCount = inputSelector.getInvertCount();
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

    const byte invertCount = inputSelector.getInvertCount();
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