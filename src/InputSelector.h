#include <Led.h>
#include <Arduino.h>
#include <Relay.h>

class InputSelector
{
private:
  Relay _allRelays[2] = {
      Relay(MAIN_INPUT_RELAY_IO_NUMBER),
      Relay(SECONDARY_INPUT_RELAY_IO_NUMBER)};

  byte _selectedRelayIONumber = _allRelays[0].iONumber;
  byte _notSelectedRelayIONumber = _allRelays[1].iONumber;

public:
  void setup()
  {
    for (size_t i = 0; i < INPUT_RELAY_COUNT; i++)
      _allRelays[i].setup();
  }

  void setSelectedRelayIONumber(int iONumber)
  {
    _selectedRelayIONumber = iONumber;
  }

  bool readRelay(int iONumber)
  {
    for (size_t i = 0; i < INPUT_RELAY_COUNT; i++)
    {
      Relay relay = _allRelays[i];
      if (relay.iONumber == iONumber)
        return relay.read();
    }

    return false;
  }

  void writeToAllRelays(int state)
  {
    for (size_t i = 0; i < INPUT_RELAY_COUNT; i++)
      _allRelays[i].write(state);
  }

  void writeToRelay(int iONumber, int state)
  {
    for (size_t i = 0; i < INPUT_RELAY_COUNT; i++)
    {
      Relay relay = _allRelays[i];
      if (relay.iONumber == iONumber)
        relay.write(state);
    }
  }

  void writeToSeletedRelay(int state)
  {
    writeToRelay(_selectedRelayIONumber, state);
  }

  void writeToNotSelected(int state)
  {
    writeToRelay(_notSelectedRelayIONumber, state);
  }

  void swapRelays()
  {
    byte temp = _selectedRelayIONumber;
    _selectedRelayIONumber = _notSelectedRelayIONumber;
    _notSelectedRelayIONumber = temp;

    writeToRelay(_notSelectedRelayIONumber, LOW);
    writeToRelay(_selectedRelayIONumber, HIGH);
  }

  byte getInvertCount(int iOUnumber)
  {
    return iOUnumber == MAIN_INPUT_RELAY_IO_NUMBER ? MAIN_INPUT_RELAY_INVERT_COUNT : SECONDARY_INPUT_RELAY_INVERT_COUNT;
  }

  byte getInvertCount()
  {
    byte count = 0;

    for (size_t i = 0; i < INPUT_RELAY_COUNT; i++)
    {
      Relay relay = _allRelays[i];
      if (relay.read())
        count += getInvertCount(relay.iONumber);
      ;
    }

    return count;
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
