#include <constants.h>

class Led
{
private:
  uint8_t _channel;
  uint8_t _pin;

  Timer<> *_timer;

  uint32_t _countToInvertState;

  uintptr_t _currentPwmTask;
  uint8_t _dutyCycleInCurrentPwmTask;
  bool _incrementInCurrentPwmTask;

  void write(uint32_t duty)
  {
    ledcWrite(_channel, duty);
  }

  static bool writeInvertedIteration(void *p)
  {
    Led *ptr = (Led *)p;

    ptr->write(ptr->read() == MIN_PWM_DUTY ? MAX_PWM_DUTY : MIN_PWM_DUTY);
    ptr->_countToInvertState--;

    return ptr->_countToInvertState > 0;
  }

  static bool pwmIteratation(void *p)
  {
    Led *ptr = (Led *)p;

    if (ptr->_dutyCycleInCurrentPwmTask == MAX_PWM_DUTY)
    {
      ptr->_incrementInCurrentPwmTask = false;
    }

    if (ptr->_dutyCycleInCurrentPwmTask == MIN_PWM_DUTY)
    {
      ptr->_incrementInCurrentPwmTask = true;
    }

    if (ptr->_incrementInCurrentPwmTask)
      ptr->_dutyCycleInCurrentPwmTask++;
    else
      ptr->_dutyCycleInCurrentPwmTask--;

    ptr->write(ptr->_dutyCycleInCurrentPwmTask);

    return true;
  }

public:
  Led()
  {
  }

  Led(uint8_t channel, uint8_t pin, Timer<> *timer)
  {
    _channel = channel;
    _pin = pin;
    _timer = timer;
  }

  void setup()
  {
    pinMode(_pin, OUTPUT);
    ledcSetup(_channel, PWM_FREQUENCY, PWM_RESOLUTION);
    ledcAttachPin(_pin, _channel);
  }

  uint32_t read()
  {
    return ledcRead(_channel);
  }

  void writeMax()
  {
    ledcWrite(_channel, MAX_PWM_DUTY);
  }

  void writeMin()
  {
    ledcWrite(_channel, MIN_PWM_DUTY);
  }

  void blink(uint32_t countToInvertState, unsigned long interval = SHORT_LED_BLINK_INTERVAL)
  {
    if (_countToInvertState > 0)
      return;

    _countToInvertState = countToInvertState;

    writeInvertedIteration(this);

    _timer->every(interval, writeInvertedIteration, this);
  }

  void startPwm(unsigned long interval)
  {
    _dutyCycleInCurrentPwmTask = read();
    _incrementInCurrentPwmTask = _dutyCycleInCurrentPwmTask < MAX_PWM_DUTY;

    pwmIteratation(this);

    _currentPwmTask = _timer->every(interval, pwmIteratation, this);
  };

  void completePwm(uint8_t finiteDutyCycle)
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