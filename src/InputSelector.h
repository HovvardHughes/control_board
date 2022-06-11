#include <ControlBoardEEPROM.h>
#include <Led.h>
#include <Arduino.h>
#include <OneButton.h>

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

  void writeTo(int ioNumber, int state)
  {
    for (size_t i = 0; i < INPUT_RELAY_COUNT; i++)
    {
      Relay relay = _allRelays[i];
      if (relay.iONumber == ioNumber)
        relay.write(state);
    }
  }

public:
  void setup()
  {
    for (size_t i = 0; i < INPUT_RELAY_COUNT; i++)
      _allRelays[i].setup();

    tryReinitCurrentInputRelayFromEEPROM();
  }

  void writeToAll(int state)
  {
    for (size_t i = 0; i < INPUT_RELAY_COUNT; i++)
      _allRelays[i].write(state);
  }

  void
  writeToSeleted(int state)
  {
    writeTo(_selectedRelayIONumber, state);
  }

  void writeToNotSelected(int state)
  {
    writeTo(_notSelectedRelayIONumber, state);
  }

  void swap()
  {
    byte temp = _selectedRelayIONumber;
    _selectedRelayIONumber = _notSelectedRelayIONumber;
    _notSelectedRelayIONumber = temp;

    _controlBoardEEPROM.writeCurrentInputRelayIONumber(_selectedRelayIONumber);

    writeTo(_notSelectedRelayIONumber, LOW);
    writeTo(_selectedRelayIONumber, HIGH);
  }

  bool all(int state)
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
