#include <constants.h>

class Buzzer
{
private:
  Timer<> *_timer;
  uint8_t _countToInvertState = 0;

  static bool invert(void *p)
  {
    Buzzer *ptr = (Buzzer *)p;

    digitalWrite(BUZZER_PIN, !digitalRead(BUZZER_PIN));
    ptr->_countToInvertState--;

    bool anyInversionsLeft = ptr->_countToInvertState > 0;

    if (!anyInversionsLeft)
      digitalWrite(BUZZER_PIN, LOW);

    return anyInversionsLeft;
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

  void buzz(uint8_t countToInvertState, unsigned long interval = SHORT_BUZZ_INTERVAL)
  {
    if (_countToInvertState > 0)
      return;

    _countToInvertState = countToInvertState;

    digitalWrite(BUZZER_PIN, HIGH);
    _countToInvertState--;

    _timer->every(interval, invert, this);
  }
};