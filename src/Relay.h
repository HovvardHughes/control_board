#include <Arduino.h>

class Relay
{
public:
  byte iONumber;
  bool writable;

  Relay(int iONumberArg)
  {
    iONumber = iONumberArg;
    writable = true;
  }

  void writeState(int newState)
  {
    if (writable)
      digitalWrite(iONumber, newState);
  }

  bool readState()
  {
    return digitalRead(iONumber);
  }
};