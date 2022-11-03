#include <OneButton.h>
#include <PowerController.h>
#include <TaskController.h>
#include <VolumeEngine.h>

extern PowerController powerController;
extern Buzzer buzzer;
extern InputSelector inputSelector;
extern Led inputSelectorLed;
extern TaskController taskController;
extern VolumeEngine volumeEngine;

void onClickInputSelectorCheckbox(int relayIONumber, int state)
{
  if (!powerController.isPowerOn())
    return;

  taskController.runFastTask([relayIONumber, state]()
                             {
                           const bool wereTurnedOff = inputSelector.areAllRelays(LOW);
                           inputSelector.writeToRelay(relayIONumber, state);

                           if (inputSelector.areAllRelays(LOW))
                           {
                             inputSelectorLed.writeMin();
                             return;
                           }

                           if (state == HIGH)
                           {
                             inputSelector.setSelectedRelayIONumber(relayIONumber);

                             byte invertCount = inputSelector.getInvertCount(relayIONumber);
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
    taskController.runTask([]()
                           { powerController.setSleepMode(false); },
                           TaskType::TURN_OFF_SLEEP_MODE,
                           LONG_TASK_RUNTIME);
  else
    taskController.runTask([]()
                           { powerController.setSleepMode(true); },
                           TaskType::TURN_ON_SLEEP_MODE,
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
    taskController.runTask([]()
                           { powerController.setPower(false); },
                           TaskType::POWER_OFF,
                           LONG_TASK_RUNTIME);
  else
    taskController.runTask([]()
                           { powerController.setPower(true); },
                           TaskType::POWER_ON,
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
    taskController.runTask([]()
                           { powerController.setVU(false); },
                           TaskType::TURN_OFF_VU,
                           LONG_TASK_RUNTIME);
  else
    taskController.runTask([]()
                           { powerController.setVU(true); },
                           TaskType::TURN_ON_VU,
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
      inputSelector.writeToNotSelected(LOW);
    else
      inputSelector.swapRelays();

    const byte invertCount = inputSelector.getInvertCount();
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
                             inputSelector.writeToNotSelected(LOW);
                             const byte invertCount = inputSelector.getInvertCount();
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
                             const byte invertCount = inputSelector.getInvertCount();
                             inputSelectorLed.blink(invertCount + 1);
                             buzzer.buzz(invertCount);
                           }
                           else
                           {
                             inputSelector.writeToAllRelays(LOW);
                             inputSelectorLed.writeMin();
                           } });
}