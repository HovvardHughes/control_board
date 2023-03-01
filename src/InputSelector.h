#include <Led.h>
#include <ControlBoardEEPROM.h>

class InputSelector
{
private:
  ControlBoardEEPROM *_eeprom;
  uint8_t _pinOfSelectedRelay;

  uint8_t getPinOfNotSelectedRelay()
  {
    return _pinOfSelectedRelay == MAIN_INPUT_RELAY_PIN ? SECONDARY_INPUT_RELAY_PIN : MAIN_INPUT_RELAY_PIN;
  }

public:
  InputSelector(ControlBoardEEPROM *eeprom)
  {
    _eeprom = eeprom;
  }

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
    writeToRelay(getPinOfNotSelectedRelay(), state);
  }

  bool writeStateFromSavedData()
  {
    InputSelectorState state = _eeprom->readInputSelectorState();

    _pinOfSelectedRelay = state.selectedRelayPin;
    
    writeToRelay(MAIN_INPUT_RELAY_PIN, state.isMainRelayPoweredOn);
    writeToRelay(SECONDARY_INPUT_RELAY_PIN, state.isSecondaryRelayPowered);

    return state.isMainRelayPoweredOn || state.isSecondaryRelayPowered;
  }

  void swapRelays()
  {
    uint8_t prevPinOfSelectedRelay = _pinOfSelectedRelay;
    _pinOfSelectedRelay = getPinOfNotSelectedRelay();

    writeToRelay(prevPinOfSelectedRelay, LOW);
    writeToRelay(_pinOfSelectedRelay, HIGH);
  }

  uint8_t getCountToInvertStateFromRelay(uint8_t pin)
  {
    return pin == MAIN_INPUT_RELAY_PIN ? 2 : 4;
  }

  uint8_t getCountToInvertStateFromTurnedOnRelays()
  {
    uint8_t count = 0;

    if (digitalRead(MAIN_INPUT_RELAY_PIN))
      count += getCountToInvertStateFromRelay(MAIN_INPUT_RELAY_PIN);

    if (digitalRead(SECONDARY_INPUT_RELAY_PIN))
      count += getCountToInvertStateFromRelay(SECONDARY_INPUT_RELAY_PIN);

    return count;
  }

  void saveStateToEeprom()
  {
    _eeprom->writeInputSelectorState(readRelay(MAIN_INPUT_RELAY_PIN), readRelay(SECONDARY_INPUT_RELAY_PIN), _pinOfSelectedRelay);
  }

  uint8_t areAllRelays(uint8_t state)
  {
    return digitalRead(MAIN_INPUT_RELAY_PIN) == state && digitalRead(SECONDARY_INPUT_RELAY_PIN) == state;
  }
};
