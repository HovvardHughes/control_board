#include <communicator/communicatorCommands.h>

class VolumeEngine
{
private:
  const int freq = 5000;
  const int ledChannel = 0;
  const int resolution = 8;

  int lastWrittenPwm = -1;

public:
  void setup()
  {
    ledcSetup(REVERSE_VOLUME_CHANNEL, FREQUENCY, LED_RESOLUTION);
    ledcAttachPin(REVERSE_VOLUME_PIN, REVERSE_VOLUME_CHANNEL);
    ledcSetup(FORWARD_VOLUME_CHANNEL, FREQUENCY, LED_RESOLUTION);
    ledcAttachPin(FORWARD_VOLUME_PIN, FORWARD_VOLUME_CHANNEL);
  }

  void handleServerCommand(u_int8_t *commmand)
  {
    if (strcmp((char *)commmand, REVERSE_LOW_VOLUME_PWM) == 0)
    {
      ledcWrite(FORWARD_VOLUME_CHANNEL, 0);
      ledcWrite(REVERSE_VOLUME_CHANNEL, 150);
    }

    if (strcmp((char *)commmand, REVERSE_HIGH_VOLUME_PWM) == 0)
    {
      ledcWrite(FORWARD_VOLUME_CHANNEL, 0);
      ledcWrite(REVERSE_VOLUME_CHANNEL, 255);
    }

    if (strcmp((char *)commmand, FORWARD_LOW_VOLUME_PWM) == 0)
    {
      ledcWrite(REVERSE_VOLUME_CHANNEL, 0);
      ledcWrite(FORWARD_VOLUME_CHANNEL, 150);
    }

    if (strcmp((char *)commmand, FORWARD_HIGH_VOLUME_PWM) == 0)
    {
      ledcWrite(REVERSE_VOLUME_CHANNEL, 0);
      ledcWrite(FORWARD_VOLUME_CHANNEL, 255);
    }

    if (strcmp((char *)commmand, TURN_OFF_VOLUME_PWM) == 0)
    {
      ledcWrite(REVERSE_VOLUME_CHANNEL, 0);
      ledcWrite(FORWARD_VOLUME_CHANNEL, 0);
    }
  }
};
