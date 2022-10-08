#include <Arduino.h>
#include <constants.h>

class Buzzer
{
private:
  Timer<> *_timer;
  byte _countToInvertState = 0;

  static bool invertBuzzState(void *p)
  {
    Buzzer *ptr = (Buzzer *)p;

    digitalWrite(BUZZER_PIN, !digitalRead(BUZZER_PIN));
    ptr->_countToInvertState--;

    return ptr->_countToInvertState > 0;
  }

public:
  Buzzer(Timer<> *timer)
  {
    _timer = timer;
  }

  void setup()
  {
    pinMode(BUZZER_PIN, OUTPUT);
  }

  void buzz(byte countToInvertState, unsigned long interval = SHORT_BUZZ_INTERVAL)
  {
    if (_countToInvertState > 0)
      return;

    _countToInvertState = countToInvertState;

    digitalWrite(BUZZER_PIN, HIGH);
    _countToInvertState--;

    _timer->every(interval, invertBuzzState, this);
  }
};