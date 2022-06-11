#include <Arduino.h>
#include <constants.h>

byte _countToBuzz = 0;

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

  void buzz(byte count, unsigned long interval = SHORT_BUZZ_INTERVAL)
  {
    _countToBuzz  = count * 2;

    digitalWrite(BUZZER_PIN, HIGH);
    _countToBuzz--;

    Serial.println(_countToBuzz);

    _timer->every(interval, [](void *) -> bool
               {
                digitalWrite(BUZZER_PIN, !digitalRead(BUZZER_PIN));
                  _countToBuzz--;

                _countToBuzz;
                return _countToBuzz > 0; });
  }
};