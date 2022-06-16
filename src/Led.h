#include <Arduino.h>
#include <constants.h>
#include <arduino-timer.h>

class Led
{
private:
  uint8_t _channel;
  uint8_t _pin;

  Timer<> *_timer;

  uint32_t _countToInvertState = 0;

  uintptr_t _currentPwmTask = 0;
  uint8_t _dutyCycleInCurrentPwmTask = 0;
  bool _incrementInCurrentPwmTask = false;

  void write(uint32_t duty)
  {
    ledcWrite(_channel, duty);
  }

  static bool writeInvertedIteration(void *p)
  {
    Led *ptr = (Led *)p;

    ptr->writeInverted();
    ptr->_countToInvertState--;

    return ptr->_countToInvertState > 0;
  }

  static bool pwmIteratation(void *p)
  {
    Led *ptr = (Led *)p;

    uint8_t dutyCycle = ptr->_dutyCycleInCurrentPwmTask;

    ptr->write(dutyCycle);

    if (!ptr->_incrementInCurrentPwmTask || dutyCycle == MAX_LED__DUTY)
      ptr->_incrementInCurrentPwmTask = false;
    else if (dutyCycle == MIN_LED__DUTY)
      ptr->_incrementInCurrentPwmTask = true;

    if (ptr->_incrementInCurrentPwmTask)
      ptr->_dutyCycleInCurrentPwmTask++;
    else
      ptr->_dutyCycleInCurrentPwmTask--;

    return true;
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

  void blink(uint32_t countToInvertState, unsigned long interval = SHORT_LED_BLINK_INTERVAL)
  {
    if (_countToInvertState > 0)
      return;

    _countToInvertState = countToInvertState;

    writeInverted();
    _countToInvertState--;

    _timer->every(interval, writeInvertedIteration, this);
  }

  void startPwm(unsigned long interval, uint8_t initialDutyCycle, uint8_t increment)
  {
    _dutyCycleInCurrentPwmTask = initialDutyCycle;
    _incrementInCurrentPwmTask = increment;

    pwmIteratation(this);

    _currentPwmTask = _timer->every(interval, pwmIteratation, this);
  };

  void finishPwm(uint8_t finiteDutyCycle)
  {
    uint32_t currentDutyCycle = read();

    if (currentDutyCycle < finiteDutyCycle)
      for (size_t i = currentDutyCycle + 1; i <= finiteDutyCycle; i++)
        write(i);

    if (finiteDutyCycle < currentDutyCycle)
      for (size_t i = currentDutyCycle - 1; i >= finiteDutyCycle; i--)
      {
        write(i);
        if (i == 0)
          break;
      }

    _dutyCycleInCurrentPwmTask = 0;
    _incrementInCurrentPwmTask = false;
    _timer->cancel(_currentPwmTask);
  };
};