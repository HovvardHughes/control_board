#include <Led.h>

class InputSelector
{
private:
  uint8_t _allRelayPins[INPUT_RELAY_COUNT] = {
      MAIN_INPUT_RELAY_PIN,
      SECONDARY_INPUT_RELAY_PIN};

  byte _pinOfselectedRelay = _allRelayPins[0];
  byte _pinOfNotSelectedRelay = _allRelayPins[1];

public:
  void setup()
  {
    for (size_t i = 0; i < INPUT_RELAY_COUNT; i++)
      pinMode(_allRelayPins[i], OUTPUT);
  }

  void setSelectedRelayIONumber(int iONumber)
  {
    _pinOfselectedRelay = iONumber;
  }

  bool readRelay(uint8_t pin)
  {
    for (size_t i = 0; i < INPUT_RELAY_COUNT; i++)
    {
      uint8_t currentPin = _allRelayPins[i];
      if (currentPin == pin)
        return digitalRead(currentPin);
    }

    return false;
  }

  void writeToAllRelays(uint8_t state)
  {
    for (size_t i = 0; i < INPUT_RELAY_COUNT; i++)
      digitalWrite(_allRelayPins[i], state);
  }

  void writeToRelay(uint8_t pin, uint8_t state)
  {
    for (size_t i = 0; i < INPUT_RELAY_COUNT; i++)
    {
      uint8_t currentPin = _allRelayPins[i];
      if (currentPin == pin)
      {
        digitalWrite(currentPin, state);
        break;
      }
    }
  }

  void writeToSeletedRelay(uint8_t state)
  {
    writeToRelay(_pinOfselectedRelay, state);
  }

  void writeToNotSelected(uint8_t state)
  {
    writeToRelay(_pinOfNotSelectedRelay, state);
  }

  void writeToNotSelectedAndSetSelected(uint8_t state)
  {
    writeToRelay(_pinOfNotSelectedRelay, state);
    _pinOfselectedRelay = _pinOfNotSelectedRelay;
  }

  void swapRelays()
  {
    byte temp = _pinOfselectedRelay;
    _pinOfselectedRelay = _pinOfNotSelectedRelay;
    _pinOfNotSelectedRelay = temp;

    writeToRelay(_pinOfNotSelectedRelay, LOW);
    writeToRelay(_pinOfselectedRelay, HIGH);
  }

  byte getInvertCount(uint8_t pin)
  {
    return pin == MAIN_INPUT_RELAY_PIN ? 2 : 4;
  }

  byte getInvertCount()
  {
    byte count = 0;

    for (size_t i = 0; i < INPUT_RELAY_COUNT; i++)
    {
      uint8_t pin = _allRelayPins[i];
      if (digitalRead(pin))
        count += getInvertCount(pin);
      ;
    }

    return count;
  }

  bool areAllRelays(uint8_t state)
  {
    for (size_t i = 0; i < INPUT_RELAY_COUNT; i++)
      if (digitalRead(_allRelayPins[i]))
        return false;

    return true;
  }
};
