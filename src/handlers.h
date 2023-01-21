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

                             uint8_t invertCount = inputSelector.getCountToInvertStateFromRelay(pin);
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
  if (!powerController.isPowerOn())
    return;

  if (powerController.isSleepModeOn())
    taskController.runLongTask([]()
                               { powerController.setSleepMode(false); },
                               DisplayMessageType::TURN_OFF_SLEEP_MODE,
                               LONG_TASK_RUNTIME);
  else
    taskController.runLongTask([]()
                               { powerController.setSleepMode(true); },
                               DisplayMessageType::TURN_ON_SLEEP_MODE,
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
                               DisplayMessageType::POWER_OFF,
                               LONG_TASK_RUNTIME);
  else
    taskController.runLongTask([]()
                               { powerController.setPower(true); },
                               DisplayMessageType::POWER_ON,
                               LONG_TASK_RUNTIME);
}

void onLongPressPowerButtonStart()
{
  if (powerController.isVUOn())
    taskController.runLongTask([]()
                               { powerController.setVU(false); },
                               DisplayMessageType::TURN_OFF_VU,
                               LONG_TASK_RUNTIME);
  else
    taskController.runLongTask([]()
                               { powerController.setVU(true); },
                               DisplayMessageType::TURN_ON_VU,
                               LONG_TASK_RUNTIME);
}

void processMainPowerSource()
{

  bool changed = powerController.updateMainPowerState();

  if (!changed ||
      taskController.isRunningTask() ||
      powerController.isSleepModeOn() ||
      powerController.isMainPowerSourceOn() == powerController.isPowerOn())
    return;

  onClickPowerButton();
}

void onClickInputSelectorButton()
{
  if (!powerController.isPowerOn())
    return;

  taskController.runFastTask([]()
                             {
    const bool wereTurnedOff = inputSelector.areAllRelays(LOW);

    if (wereTurnedOff)
      inputSelector.writeToSeletedRelay(HIGH);
    else if (inputSelector.areAllRelays(HIGH))
      inputSelector.writeToNotSelectedRelay(LOW);
    else
      inputSelector.swapRelays();

    const uint8_t invertCount = inputSelector.getCountToInvertStateFromTurnedOnRelays();
    inputSelectorLed.blink(wereTurnedOff ? invertCount + 1 : invertCount);
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
                             const uint8_t invertCount = inputSelector.getCountToInvertStateFromTurnedOnRelays();
                             inputSelectorLed.blink(invertCount);
                             buzzer.buzz(invertCount);
                           }
                           else
                           {
                             const bool wereTurnedOff = inputSelector.areAllRelays(LOW);
                             inputSelector.writeToAllRelays(HIGH);
                             inputSelectorLed.blink(wereTurnedOff ?  3 : 2, LONG_LED_BLINK_INTERVAL);
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
                             const uint8_t invertCount = inputSelector.getCountToInvertStateFromTurnedOnRelays();
                             inputSelectorLed.blink(invertCount + 1);
                             buzzer.buzz(invertCount);
                           }
                           else
                           {
                             inputSelector.writeToAllRelays(LOW);
                             inputSelectorLed.writeMin();
                           } });
}

void powerOffEmergency(DisplayMessageType displayMessageType)
{
  powerController.powerOffEmergency();
  taskController.setDisplayMessageType(displayMessageType);
}