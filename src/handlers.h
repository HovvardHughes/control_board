#include <OneButton.h>
#include <PowerController.h>
#include <TaskController.h>

extern PowerController powerController;
extern Buzzer buzzer;
extern InputSelector inputSelector;
extern Led inputSelectorLed;
extern TaskController taskController;
extern VolumeEngine volumeEngine;

void onClickInputSelectorCheckbox(int pin, int state)
{
  if (!powerController.isPowerOn())
    return;

  taskController.runFastTask([pin, state]()
                             {
                           const bool wereTurnedOff = inputSelector.areAllRelays(LOW);
                           inputSelector.writeToRelay(pin, state);

                           if (inputSelector.areAllRelays(LOW))
                           {
                             inputSelectorLed.writeMin();
                             return;
                           }

                           if (state == HIGH)
                           {
                             inputSelector.setPinOfSelectedRelay(pin);

                             byte invertCount = inputSelector.getCountToInvertStateFromRelay(pin);
                             {
                               inputSelectorLed.blink(wereTurnedOff ? invertCount + 1 : invertCount);
                               buzzer.buzz(invertCount);
                             }} });
}

void onVolumeChanged(u_int8_t *commmand)
{
  volumeEngine.handleServerCommand(commmand);
}

void onDoubleClickPowerButton()
{
  if (powerController.isSleepModeOn())
    taskController.runLongTask([]()
                               { powerController.setSleepMode(false); },
                               LongTaskType::TURN_OFF_SLEEP_MODE,
                               LONG_TASK_RUNTIME);
  else
    taskController.runLongTask([]()
                               { powerController.setSleepMode(true); },
                               LongTaskType::TURN_ON_SLEEP_MODE,
                               LONG_TASK_RUNTIME);
}

void onClickPowerButton()
{
  if (powerController.isSleepModeOn())
  {
    onDoubleClickPowerButton();
    return;
  }

  if (powerController.isPowerOn())
    taskController.runLongTask([]()
                               { powerController.setPower(false); },
                               LongTaskType::POWER_OFF,
                               LONG_TASK_RUNTIME);
  else
    taskController.runLongTask([]()
                               { powerController.setPower(true); },
                               LongTaskType::POWER_ON,
                               LONG_TASK_RUNTIME);
}

void onLongPressPowerButtonStart()
{
  if (!powerController.isPowerOn())
    return;

  if (powerController.isSleepModeOn())
  {
    onDoubleClickPowerButton();
    return;
  }

  if (powerController.isVUOn())
    taskController.runLongTask([]()
                               { powerController.setVU(false); },
                               LongTaskType::TURN_OFF_VU,
                               LONG_TASK_RUNTIME);
  else
    taskController.runLongTask([]()
                               { powerController.setVU(true); },
                               LongTaskType::TURN_ON_VU,
                               LONG_TASK_RUNTIME);
}

void onClickInputSelectorButton()
{
  if (!powerController.isPowerOn())
    return;

  taskController.runFastTask([]()
                             {
    const bool areAllRelaysLow = inputSelector.areAllRelays(LOW);

    if (areAllRelaysLow)
      inputSelector.writeToSeletedRelay(HIGH);
    else if (inputSelector.areAllRelays(HIGH))
      inputSelector.writeToNotSelectedRelay(LOW);
    else
      inputSelector.swapRelays();

    const byte invertCount = inputSelector.getCountToInvertStateFromTurnedOnRelays();
    inputSelectorLed.blink(areAllRelaysLow ? invertCount + 1 : invertCount);
    buzzer.buzz(invertCount); });
}

void onDoubleClickInputSelectorButton()
{
  if (!powerController.isPowerOn())
    return;

  taskController.runFastTask([]()
                             {
                           if (inputSelector.areAllRelays(HIGH))
                           {
                             inputSelector.writeToNotSelectedRelay(LOW);
                             const byte invertCount = inputSelector.getCountToInvertStateFromTurnedOnRelays();
                             inputSelectorLed.blink(invertCount);
                             buzzer.buzz(invertCount);
                           }
                           else
                           {
                             const bool areAllRelaysLow = inputSelector.areAllRelays(LOW);
                             inputSelector.writeToAllRelays(HIGH);
                             inputSelectorLed.blink(areAllRelaysLow ? 3 : 2, LONG_LED_BLINK_INTERVAL);
                             buzzer.buzz(2, LONG_BUZZ_INTERVAL);
                           } });
}

void onLongPressInputSelectorButtonStart()
{
  if (!powerController.isPowerOn())
    return;

  taskController.runFastTask([]()
                             {
                           if (inputSelector.areAllRelays(LOW))
                           {
                             inputSelector.writeToSeletedRelay(HIGH);
                             const byte invertCount = inputSelector.getCountToInvertStateFromTurnedOnRelays();
                             inputSelectorLed.blink(invertCount + 1);
                             buzzer.buzz(invertCount);
                           }
                           else
                           {
                             inputSelector.writeToAllRelays(LOW);
                             inputSelectorLed.writeMin();
                           } });
}