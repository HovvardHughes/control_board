#include <InputSelector.h>
#include <Buzzer.h>

extern bool power;
extern bool sleepMode;

extern Buzzer buzzer;
extern Timer<> timer;

extern Led inputSelectorLed;
extern InputSelector inputSelector;

Relay allPowerRelays[] = {
    Relay(25),
    Relay(26),
    Relay(27),
    Relay(14),
    Relay(12)};

Led powerLed = Led(POWER_BUTTON_LED_CHANNEL, POWER_BUTTON_LED_PIN, &timer);

void turnOnSleepMode()
{
  allPowerRelays[2].write(LOW);

  timer.in(DELAY_IN_MILLIS, [](void *) -> bool
           {  
                allPowerRelays[1].write(LOW);
                allPowerRelays[3].write(LOW);

                powerLed.startPwm(LONG_LED_PWM_INTERVAL); 

                sleepMode = true;

                return false; });
}

void turnOffSleepMode()
{
  allPowerRelays[1].write(HIGH);
  allPowerRelays[3].write(HIGH);

  timer.in(DELAY_IN_MILLIS, [](void *) -> bool
           { 
               allPowerRelays[2].write(HIGH);

               powerLed.finishPwm(MAX_LED__DUTY); 

               sleepMode = false;

               return false; });
}

void turnOnPower()
{
  allPowerRelays[0].write(HIGH);
  allPowerRelays[1].write(HIGH);
  allPowerRelays[3].write(HIGH);

  powerLed.startPwm(SHORT_LED_PWM_INTERVAL);

  timer.in(DELAY_IN_MILLIS, [](void *) -> bool
           {  
            allPowerRelays[2].write(HIGH);
            allPowerRelays[4].write(HIGH);

            inputSelector.writeToSeletedRelay(HIGH);
            inputSelectorLed.writeMax();

            buzzer.buzz(4);

            powerLed.finishPwm(MAX_LED__DUTY);
            power = true;

            return false; });
}

void turnOffPower()
{
  allPowerRelays[2].write(LOW);
  allPowerRelays[4].write(LOW);

  powerLed.startPwm(SHORT_LED_PWM_INTERVAL);

  timer.in(DELAY_IN_MILLIS, [](void *) -> bool
           {
            allPowerRelays[0].write(LOW);
            allPowerRelays[1].write(LOW);
            allPowerRelays[3].write(LOW);

            inputSelector.writeToAllRelays(LOW);
            inputSelectorLed.writeMin();

            powerLed.finishPwm(MIN_LED__DUTY);
            power = false;

            return false; });
}

void setupPowerManagement()
{
  for (size_t i = 0; i < POWER_RELAY_COUNT; i++)
    allPowerRelays[i].setup();
}