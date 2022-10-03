#include <buttonHandlers.h>
#include <VolumeEngine.h>

extern Buzzer buzzer;

extern InputSelector inputSelector;

extern Led inputSelectorLed;

extern VolumeEngine volumeEngine;

void onClickHtmlPowerButton()
{
  onClickPowerButton();
}

void onClickSleepButton()
{
  onDoubleClickPowerButton();
}

void onClickInputSelectorCheckbox(int relayIONumber, int state)
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

    if (!inputSelector.areAllRelays(HIGH))
      inputSelector.setSelectedRelayIONumber(relayIONumber);

    byte invertCount = inputSelector.getInvertCount(relayIONumber);
    {
      inputSelectorLed.blink(wereTurnedOff ? invertCount + 1 : invertCount);
      buzzer.buzz(invertCount);
    }
  }
}

void onVolumeChanged(u_int8_t *commmand)
{
  volumeEngine.handleServerCommand(commmand);
}