#include <ControlBoardEEPROM.h>
#include <Led.h>
#include <Arduino.h>
#include <OneButton.h>
#include <Buzzer.h>

class InputSelector
{
private:
  ControlBoardEEPROM _controlBoardEEPROM = ControlBoardEEPROM();

  Relay _allRelays[2] = {
      Relay(MAIN_INPUT_RELAY_IO_NUMBER),
      Relay(SECONDARY_INPUT_RELAY_IO_NUMBER)};

  byte _selectedRelayIONumber = _allRelays[0].iONumber;

  byte _notSelectedRelayIONumber = _allRelays[1].iONumber;

  Led _led = Led(INPUT_SELECTOR_BUTTON_LED_CHANNEL, INPUT_SELECTOR_BUTTON_LED_PIN, &timer);

  Buzzer *_buzzer;

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

  void writeTo(int ioNumber, int state, bool silently = false)
  {
    for (size_t i = 0; i < INPUT_RELAY_COUNT; i++)
    {
      Relay relay = _allRelays[i];
      if (relay.iONumber == ioNumber)
        relay.write(state);
    }

    if (!silently)
    {
      _buzzer->buzzOneTime();
      _led.blink();
    }
  }

public:
  OneButton button = OneButton(INPUT_SELECTOR_BUTTON_PIN);

  InputSelector(Timer<> *timer, Buzzer *buzzer)
  {
    _led = Led(INPUT_SELECTOR_BUTTON_LED_CHANNEL, INPUT_SELECTOR_BUTTON_LED_PIN, timer);
    _buzzer = buzzer;
  };

  void setup()
  {
    pinMode(INPUT_SELECTOR_BUTTON_PIN, INPUT_PULLUP);

    for (size_t i = 0; i < INPUT_RELAY_COUNT; i++)
      _allRelays[i].setup();

    _led.setup();

    tryReinitCurrentInputRelayFromEEPROM();
  }

  void writeToLed(int state)
  {
    if (state)
      _led.writeMax();
    else
      _led.writeMin();
  }

  void writeToAll(int state, bool silently = false)
  {
    for (size_t i = 0; i < INPUT_RELAY_COUNT; i++)
      _allRelays[i].write(state);

    if (!silently)
    {
      _led.blink();
      _buzzer->buzzTwoTimes();
    }
  }

  void
  writeToSeleted(int state, bool silently = false)
  {
    writeTo(_selectedRelayIONumber, state, silently);
  }

  void writeToNotSelected(int state, bool silently = false)
  {
    writeTo(_notSelectedRelayIONumber, state, silently);
  }

  void swap()
  {
    byte temp = _selectedRelayIONumber;
    _selectedRelayIONumber = _notSelectedRelayIONumber;
    _notSelectedRelayIONumber = temp;

    _controlBoardEEPROM.writeCurrentInputRelayIONumber(_selectedRelayIONumber);

    writeTo(_notSelectedRelayIONumber, LOW, true);
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
