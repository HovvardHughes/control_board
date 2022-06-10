#include <Arduino.h>

class Relay
{
private:
  bool _isWritable;

public:
  byte iONumber;

  Relay(int iONumberArg)
  {
    iONumber = iONumberArg;
    _isWritable = true;
  }

  void setup()
  {
    pinMode(iONumber, OUTPUT);
  }

  void write(int newState)
  {
    if (_isWritable)
      digitalWrite(iONumber, newState);
  }

  void writeAndForbidWriting(int newState)
  {
    write(newState);
    _isWritable = false;
  }

  bool read()
  {
    return digitalRead(iONumber);
  }

  bool isWritable()
  {
    return _isWritable;
  }
};