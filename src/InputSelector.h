#include <ControlBoardEEPROM.h>
#include <Led.h>
#include <Arduino.h>

class InputSelector
{
private:
  ControlBoardEEPROM _controlBoardEEPROM = ControlBoardEEPROM();

  Relay _allRelays[2] = {
      Relay(MAIN_INPUT_RELAY_IO_NUMBER),
      Relay(SECONDARY_INPUT_RELAY_IO_NUMBER)};

  byte _selectedRelayIONumber = _allRelays[0].iONumber;
  byte _notSelectedRelayIONumber = _allRelays[1].iONumber;

  void tryReinitCurrentInputRelayFromEEPROM()
  {
    const byte readIONumber = _controlBoardEEPROM.readCurrentInputRelayIONumber();
    for (size_t i = 0; i < INPUT_RELAY_COUNT; i++)
    {
      Relay relay = _allRelays[i];
      if (relay.iONumber == readIONumber)
        _selectedRelayIONumber = relay.iONumber;
    }
  }

  byte writeToRelay(int iONumber, int state)
  {
    for (size_t i = 0; i < INPUT_RELAY_COUNT; i++)
    {
      Relay relay = _allRelays[i];
      if (relay.iONumber == iONumber)
        relay.write(state);
    }
    return iONumber == MAIN_INPUT_RELAY_IO_NUMBER ? MAIN_INPUT_RELAY_INVERT_COUNT : SECONDARY_INPUT_RELAY_INVERT_COUNT;
  }

  byte getCountToInvert() {

  }

public:
  void setup()
  {
    for (size_t i = 0; i < INPUT_RELAY_COUNT; i++)
      _allRelays[i].setup();

    tryReinitCurrentInputRelayFromEEPROM();
  }

  void writeToAllRelays(int state)
  {
    for (size_t i = 0; i < INPUT_RELAY_COUNT; i++)
      _allRelays[i].write(state);
  }

  byte writeToSeletedRelay(int state)
  {
    return writeToRelay(_selectedRelayIONumber, state);
  }

  byte writeToNotSelected(int state)
  {
    return writeToRelay(_notSelectedRelayIONumber, state);
  }

  byte swapRelays()
  {
    byte temp = _selectedRelayIONumber;
    _selectedRelayIONumber = _notSelectedRelayIONumber;
    _notSelectedRelayIONumber = temp;

    _controlBoardEEPROM.writeCurrentInputRelayIONumber(_selectedRelayIONumber);

    writeToRelay(_notSelectedRelayIONumber, LOW);
    return writeToRelay(_selectedRelayIONumber, HIGH);
  }

  bool areAllRelays(int state)
  {
    for (size_t i = 0; i < INPUT_RELAY_COUNT; i++)
    {
      Relay relay = _allRelays[i];
      if (relay.read() != state)
        return false;
    }

    return true;
  }
};
