#include <communicatorCommands.h>

#define MAX_NOT_ZERO_DUTY_TIME 1000 * 20

#define MIN_VOLUME_DUTY 175
#define MIDDLE_VOLUME_DUTY 225
#define MAX_VOLUME_DUTY 255

class VolumeEngine
{
private:
  int lastForwardChannelDuty;
  int lastReverseChannelDuty;
  unsigned long notZeroForwardChannelDutyDuration;
  unsigned long notZeroReverseChannelDutyDuration;

  void write(uint32_t forwardChannelDuty, uint32_t reverseChannelDuty)
  {
    if (forwardChannelDuty != lastForwardChannelDuty)
    {
      ledcWrite(FORWARD_VOLUME_CHANNEL, forwardChannelDuty);
      lastForwardChannelDuty = forwardChannelDuty;
    }

    if (reverseChannelDuty != lastReverseChannelDuty)
    {
      ledcWrite(REVERSE_VOLUME_CHANNEL, reverseChannelDuty);
      lastReverseChannelDuty = reverseChannelDuty;
    }

    notZeroForwardChannelDutyDuration = millis();
    notZeroReverseChannelDutyDuration = millis();
  }

public:
  void setup()
  {
    ledcSetup(REVERSE_VOLUME_CHANNEL, PWM_FREQUENCY, PWM_RESOLUTION);
    ledcAttachPin(REVERSE_VOLUME_PIN, REVERSE_VOLUME_CHANNEL);
    ledcSetup(FORWARD_VOLUME_CHANNEL, PWM_FREQUENCY, PWM_RESOLUTION);
    ledcAttachPin(FORWARD_VOLUME_PIN, FORWARD_VOLUME_CHANNEL);
  }

  void disableIfActiveForLongTime()
  {
    if ((lastForwardChannelDuty > MIN_PWM_DUTY && (millis() - notZeroForwardChannelDutyDuration > MAX_NOT_ZERO_DUTY_TIME)) ||
        (lastReverseChannelDuty > MIN_PWM_DUTY && (millis() - notZeroReverseChannelDutyDuration > MAX_NOT_ZERO_DUTY_TIME)))
      write(MIN_PWM_DUTY, MIN_PWM_DUTY);
  }

  void handleServerCommand(u_int8_t *commmand)
  {
    if (strcmp((char *)commmand, REVERSE_LOW_VOLUME_PWM) == 0)
      write(MIN_PWM_DUTY, MIN_VOLUME_DUTY);

    if (strcmp((char *)commmand, REVERSE_MEDIUM_VOLUME_PWM) == 0)
      write(MIN_PWM_DUTY, MIDDLE_VOLUME_DUTY);

    if (strcmp((char *)commmand, REVERSE_HIGH_VOLUME_PWM) == 0)
      write(MIN_PWM_DUTY, MAX_VOLUME_DUTY);

    if (strcmp((char *)commmand, FORWARD_LOW_VOLUME_PWM) == 0)
      write(MIN_VOLUME_DUTY, MIN_PWM_DUTY);

    if (strcmp((char *)commmand, FORWARD_MEDIUM_VOLUME_PWM) == 0)
      write(MIDDLE_VOLUME_DUTY, MIN_PWM_DUTY);

    if (strcmp((char *)commmand, FORWARD_HIGH_VOLUME_PWM) == 0)
      write(MAX_VOLUME_DUTY, MIN_PWM_DUTY);

    if (strcmp((char *)commmand, TURN_OFF_VOLUME_PWM) == 0)
      write(MIN_PWM_DUTY, MIN_PWM_DUTY);
  }
};
