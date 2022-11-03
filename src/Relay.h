#include <Arduino.h>

class Relay
{
public:
  byte iONumber;

  Relay(int iONumberArg)
  {
    iONumber = iONumberArg;
  }

  void setup()
  {
    pinMode(iONumber, OUTPUT);
  }

  void write(int newState)
  {
    digitalWrite(iONumber, newState);
  }

  bool read()
  {
    return digitalRead(iONumber);
  }
};