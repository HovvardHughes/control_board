#include <communicatorCommands.h>

#define MAX_NOT_ZERO_DUTY_DURATION 1000 * 20

#define MIN_VOLUME_DUTY 175
#define MIDDLE_VOLUME_DUTY 225
#define MAX_VOLUME_DUTY 255

class VolumeEngine
{
private:
  unsigned long forwardNotZeroChannelDutyStartedMillis;
  unsigned long reverseNotZeroChannelDutyStartedMillis;

  void write(uint32_t forwardChannelDuty, uint32_t reverseChannelDuty)
  {

    ledcWrite(FORWARD_VOLUME_CHANNEL, forwardChannelDuty);
    ledcWrite(REVERSE_VOLUME_CHANNEL, reverseChannelDuty);
    forwardNotZeroChannelDutyStartedMillis = forwardChannelDuty > MIN_PWM_DUTY ? millis() : 0;
    reverseNotZeroChannelDutyStartedMillis = reverseChannelDuty > MIN_PWM_DUTY ? millis() : 0;
  }

public:
  void setup()
  {
    ledcSetup(REVERSE_VOLUME_CHANNEL, PWM_FREQUENCY, PWM_RESOLUTION);
    ledcAttachPin(REVERSE_VOLUME_PIN, REVERSE_VOLUME_CHANNEL);
    ledcSetup(FORWARD_VOLUME_CHANNEL, PWM_FREQUENCY, PWM_RESOLUTION);
    ledcAttachPin(FORWARD_VOLUME_PIN, FORWARD_VOLUME_CHANNEL);
  }

  void check()
  {
    if ((ledcRead(FORWARD_VOLUME_CHANNEL) > MIN_PWM_DUTY && (millis() - forwardNotZeroChannelDutyStartedMillis > MAX_NOT_ZERO_DUTY_DURATION)) ||
        (ledcRead(REVERSE_VOLUME_CHANNEL) > MIN_PWM_DUTY && (millis() - reverseNotZeroChannelDutyStartedMillis > MAX_NOT_ZERO_DUTY_DURATION)))
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

  void turnOff()
  {
    write(MIN_PWM_DUTY, MIN_PWM_DUTY);
  }
};
