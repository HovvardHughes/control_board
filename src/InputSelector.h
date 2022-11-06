#include <Led.h>

class InputSelector
{
private:
  byte _pinOfSelectedRelay = MAIN_INPUT_RELAY_PIN;
  byte _pinOfNotSelectedRelay = SECONDARY_INPUT_RELAY_PIN;

public:
  void setup()
  {
    pinMode(MAIN_INPUT_RELAY_PIN, OUTPUT);
    pinMode(SECONDARY_INPUT_RELAY_PIN, OUTPUT);
  }

  void setPinOfSelectedRelay(int pin)
  {
    _pinOfSelectedRelay = pin;
  }

  bool readRelay(uint8_t pin)
  {
    return digitalRead(pin);
  }

  void writeToAllRelays(uint8_t state)
  {
    digitalWrite(MAIN_INPUT_RELAY_PIN, state);
    digitalWrite(SECONDARY_INPUT_RELAY_PIN, state);
  }

  void writeToRelay(uint8_t pin, uint8_t state)
  {
    digitalWrite(pin, state);
  }

  void writeToSeletedRelay(uint8_t state)
  {
    writeToRelay(_pinOfSelectedRelay, state);
  }

  void writeToNotSelectedRelay(uint8_t state)
  {
    writeToRelay(_pinOfNotSelectedRelay, state);
  }

  void writeToNotSelectedAndSetSelected(uint8_t state)
  {
    writeToRelay(_pinOfNotSelectedRelay, state);
    _pinOfSelectedRelay = _pinOfNotSelectedRelay;
  }

  void swapRelays()
  {
    byte temp = _pinOfSelectedRelay;
    _pinOfSelectedRelay = _pinOfNotSelectedRelay;
    _pinOfNotSelectedRelay = temp;

    writeToRelay(_pinOfNotSelectedRelay, LOW);
    writeToRelay(_pinOfSelectedRelay, HIGH);
  }

  byte getCountToInvertFromRelay(uint8_t pin)
  {
    return pin == MAIN_INPUT_RELAY_PIN ? 2 : 4;
  }

  byte getCountToInvertFromTurnedOnRelays()
  {
    byte count = 0;

    if (digitalRead(MAIN_INPUT_RELAY_PIN))
      count += getCountToInvertFromRelay(MAIN_INPUT_RELAY_PIN);

    if (digitalRead(SECONDARY_INPUT_RELAY_PIN))
      count += getCountToInvertFromRelay(SECONDARY_INPUT_RELAY_PIN);

    return count;
  }

  bool areAllRelays(uint8_t state)
  {
    return digitalRead(MAIN_INPUT_RELAY_PIN) == state && digitalRead(SECONDARY_INPUT_RELAY_PIN) == state;
  }
};
