#include <OneButton.h>
#include <PowerController.h>

extern PowerController powerController;

extern Buzzer buzzer;
extern InputSelector inputSelector;
extern Led inputSelectorLed;

void onClickPowerButton()
{
  if (powerController.isSleepModeOn())
  {
    Serial.println("TurnOffSleepMode...");
    powerController.turnOffSleepMode();
  }
  else
  {
    if (powerController.isPowerOn())
    {
      Serial.println("TurnOffPower...");
      powerController.turnOffPower();
    }
    else
    {
      Serial.println("TurnOnPower...");
      powerController.turnOnPower();
    }
  }
}

void onDoubleClickPowerButton()
{
  if (powerController.isSleepModeOn())
  {
    Serial.println("TurnOffSleepMode...");
    powerController.turnOffSleepMode();
  }
  else
  {
    Serial.println("TurnOnSleepMode...");
    powerController.turnOnSleepMode();
  }
}

void onLongPressPowerButtonStart()
{
  if (powerController.isSleepModeOn())
  {
    Serial.println("TurnOffSleepMode...");
    powerController.turnOffSleepMode();
  }
  else
  {
    Serial.println("PoweringOffVU's...");
    powerController.turnOffVUOnce();
  }
}

void onClickInputSelectorButton()
{
  const bool areAllRelaysLow = inputSelector.areAllRelays(LOW);

  if (areAllRelaysLow)
  {
    Serial.println("TurnOnSelectedInput...");
    inputSelector.writeToSeletedRelay(HIGH);
  }
  else if (inputSelector.areAllRelays(HIGH))
  {
    Serial.println("TurnOnNotSelectedInput...");
    inputSelector.writeToNotSelected(LOW);
  }
  else
  {
    Serial.println("SwapInputs...");
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
    Serial.println("TurnOnSelectedInput...");

    inputSelector.writeToNotSelected(LOW);

    const byte invertCount = inputSelector.getInvertCount();
    inputSelectorLed.blink(invertCount);
    buzzer.buzz(invertCount);
  }
  else
  {
    Serial.println("TurnOnAllInputs...");

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
    Serial.println("TurnOnSelectedInput...");

    inputSelector.writeToSeletedRelay(HIGH);

    const byte invertCount = inputSelector.getInvertCount();
    inputSelectorLed.blink(invertCount + 1);
    buzzer.buzz(invertCount);
  }
  else
  {
    Serial.println("TurnOffAllInputs...");

    inputSelector.writeToAllRelays(LOW);
    inputSelectorLed.writeMin();
  }
}

void onLongPressMainPowerOnButtonStart()
{
  if (powerController.isPowerOn())
  {
    powerController.turnOffPower();
    Serial.println("MainPowerOnSequenceWasStopping...");
  }
}

void onLongPressMainPowerOnButtonStop()
{
  if (!powerController.isPowerOn())
  {
    Serial.println("MainPowerOnSequenceWasStarting...");
    powerController.turnOnPower();
  }
}