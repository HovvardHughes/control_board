#include <Arduino.h>
#include <constants.h>

byte _buzzedTimes = 0;

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

  void buzzOneTime(unsigned long delay = SHORT_BUZZ_TIME)
  {
    digitalWrite(BUZZER_PIN, HIGH);

    _timer->in(delay, [](void *) -> bool
               {
                digitalWrite(BUZZER_PIN, LOW);
                return false; });
  }

  void buzzTwoTimes()
  {
    digitalWrite(BUZZER_PIN, HIGH);
    _buzzedTimes++;

    Serial.println(_buzzedTimes);

    _timer->every(SHORT_BUZZ_TIME, [](void *) -> bool
               {
                digitalWrite(BUZZER_PIN, !digitalRead(BUZZER_PIN));
                _buzzedTimes++;
                
                if(_buzzedTimes != 4) 
                  return true;

                _buzzedTimes = 0;
                return false; });
  }
};