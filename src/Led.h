#include <Arduino.h>
#include <constants.h>
#include <arduino-timer.h>

class Led
{
private:
  uint8_t _channel;
  uint8_t _pin;
  Timer<> *_timer;

  static bool writeInvertedCallback(void *p)
  {
    Led *ptr = (Led *)p;
    ptr->writeInverted();
    return false;
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

  void write(uint32_t duty)
  {
    ledcWrite(_channel, duty);
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

  void blink(unsigned long delay = LED_BLINK_DELAY)
  {
    writeInverted();

    _timer->in(delay, writeInvertedCallback, this);
  }

  uint32_t read()
  {
    return ledcRead(_channel);
  }
};