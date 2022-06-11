#include <Arduino.h>
#include <constants.h>

byte _countToInvertState = 0;

class Buzzer
{
private:
  Timer<> *_timer;

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
    _countToInvertState  = countToInvertState;

    digitalWrite(BUZZER_PIN, HIGH);
    _countToInvertState--;

    Serial.println(_countToInvertState);

    _timer->every(interval, [](void *) -> bool
               {
                digitalWrite(BUZZER_PIN, !digitalRead(BUZZER_PIN));
                  _countToInvertState--;

                _countToInvertState;
                return _countToInvertState > 0; });
  }
};