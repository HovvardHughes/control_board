#include <Arduino.h>
#include <constants.h>
#include <arduino-timer.h>

class Led
{
private:
  uint8_t _channel;
  uint8_t _pin;
  Timer<> *_timer;
  int _scheduledDuty = -1;
  uint32_t _countToInvertState = 0;

  void write(uint32_t duty)
  {
    ledcWrite(_channel, duty);
  }

  static bool writeInvertedCallback(void *p)
  {
    Led *ptr = (Led *)p;

    ptr->writeInverted();
    ptr->_countToInvertState--;
    return ptr->_countToInvertState > 0;
  }

public:
  Led(uint8_t channel, uint8_t pin, Timer<> *timer)
  {
    _channel = channel;
    _pin = pin;
    _timer = timer;
  }

  void setup()
  {
    pinMode(_pin, OUTPUT);
    ledcSetup(_channel, FREQUENCY, LED_RESOLUTION);
    ledcAttachPin(_pin, _channel);
  }

  uint32_t read()
  {
    return ledcRead(_channel);
  }

  void writeMax()
  {
    ledcWrite(_channel, MAX_LED__DUTY);
  }

  void writeMin()
  {
    ledcWrite(_channel, MIN_LED__DUTY);
  }

  void writeInverted()
  {
    write(read() == MIN_LED__DUTY ? MAX_LED__DUTY : MIN_LED__DUTY);
  }

  void blink(uint32_t countToInvertState, unsigned long interval = LONG_LED_BLINK_INTERVAL)
  {
    _countToInvertState = countToInvertState;

    writeInverted();
    _countToInvertState--;

    _timer->every(SHORT_LED_BLINK_INTERVAL, writeInvertedCallback, this);
  }
};