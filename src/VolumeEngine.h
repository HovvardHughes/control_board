#include <communicator/communicatorCommands.h>

class VolumeEngine
{
private:
  const int freq = 5000;
  const int ledChannel = 0;
  const int resolution = 8;

  int lastForwardVolumeChannelDuty = -1;
  int lastReverseVolumeChannelDuty = -1;

  void write(uint32_t forwardVolumeChannelDuty, uint32_t reverseVolumeChannelDuty)
  {
    if (forwardVolumeChannelDuty != lastForwardVolumeChannelDuty)
    {
      ledcWrite(FORWARD_VOLUME_CHANNEL, forwardVolumeChannelDuty);
      lastForwardVolumeChannelDuty = forwardVolumeChannelDuty;
    }

    if (reverseVolumeChannelDuty != lastReverseVolumeChannelDuty)
    {
      ledcWrite(REVERSE_VOLUME_CHANNEL, reverseVolumeChannelDuty);
      lastReverseVolumeChannelDuty = reverseVolumeChannelDuty;
    }
  }

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
      write(0, 150);

    if (strcmp((char *)commmand, REVERSE_MEDIUM_VOLUME_PWM) == 0)
      write(0, 200);

    if (strcmp((char *)commmand, REVERSE_HIGH_VOLUME_PWM) == 0)
      write(0, 255);

    if (strcmp((char *)commmand, FORWARD_LOW_VOLUME_PWM) == 0)
      write(150, 0);

    if (strcmp((char *)commmand, FORWARD_MEDIUM_VOLUME_PWM) == 0)
      write(200, 0);

    if (strcmp((char *)commmand, FORWARD_HIGH_VOLUME_PWM) == 0)
      write(255, 0);

    if (strcmp((char *)commmand, TURN_OFF_VOLUME_PWM) == 0)
      write(0, 0);
  }
};
