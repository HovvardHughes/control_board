
#include <timer.h>
#include <InputSelector.h>
#include <Buzzer.h>

#define POWER_RELAY_COUNT 5
#define POWER_VU_INDEX 3

class PowerController
{

private:
    Timer<> *_timer;

    InputSelector *_inputSelector;
    Led *_inputSelectorLed;
    Buzzer *_buzzer;

    bool _isPowerOn;
    bool _isSleepModeOn;

    Relay _allPowerRelays[POWER_RELAY_COUNT] = {
        Relay(25),
        Relay(26),
        Relay(27),
        Relay(14),
        Relay(12)};

    Led _powerLed;

    static bool turnOnSleepModeInternal(void *p)
    {
        PowerController *ptr = (PowerController *)p;

        ptr->_allPowerRelays[1].write(LOW);
        ptr->_allPowerRelays[3].write(LOW);

        ptr->_powerLed.startPwm(LONG_LED_PWM_INTERVAL);

        ptr->_isSleepModeOn = true;

        return false;
    }

    static bool turnOffSleepModeInternal(void *p)
    {
        PowerController *ptr = (PowerController *)p;

        ptr->_allPowerRelays[2].write(HIGH);

        ptr->_powerLed.finishPwm(MAX_PWM_DUTY);

        ptr->_isSleepModeOn = false;

        return false;
    }

    static bool turnOnPowerInternal(void *p)
    {
        PowerController *ptr = (PowerController *)p;

        ptr->_allPowerRelays[2].write(HIGH);
        ptr->_allPowerRelays[4].write(HIGH);

        ptr->_inputSelector->writeToSeletedRelay(HIGH);
        ptr->_inputSelectorLed->writeMax();

        ptr->_buzzer->buzz(4);

        ptr->_powerLed.finishPwm(MAX_PWM_DUTY);

        ptr->_isPowerOn = true;

        return false;
    }

    static bool turnOffPowerInternal(void *p)
    {
        PowerController *ptr = (PowerController *)p;

        ptr->_allPowerRelays[0].write(LOW);
        ptr->_allPowerRelays[1].write(LOW);
        ptr->_allPowerRelays[3].write(LOW);

        ptr->_inputSelector->writeToAllRelays(LOW);
        ptr->_inputSelectorLed->writeMin();

        ptr->_powerLed.finishPwm(MIN_PWM_DUTY);

        ptr->_isPowerOn = false;

        return false;
    }

    static bool turnOffVUOnceInternal(void *p)
    {
        PowerController *ptr = (PowerController *)p;

        ptr->_allPowerRelays[POWER_VU_INDEX].writeAndForbidWriting(LOW);

        return false;
    }

public:
    PowerController(Timer<> *timer, InputSelector *inputSelector, Led *inputSelectorLed, Buzzer *buzzer)
    {
        _timer = timer;
        _inputSelector = inputSelector;
        _inputSelectorLed = inputSelectorLed;
        _buzzer = buzzer;
        _powerLed = Led(POWER_BUTTON_LED_CHANNEL, POWER_BUTTON_LED_PIN, timer);
    }

    void turnOnSleepMode()
    {
        _buzzer->buzz(4);

        _allPowerRelays[2].write(LOW);
        _timer->in(LONG_TASK_DELAY, turnOnSleepModeInternal, this);
    }

    void turnOffSleepMode()
    {
        _buzzer->buzz(4);

        _allPowerRelays[1].write(HIGH);
        _allPowerRelays[3].write(HIGH);

        _timer->in(LONG_TASK_DELAY, turnOffSleepModeInternal, this);
    }

    void turnOnPower()
    {
        _buzzer->buzz(2);

        _allPowerRelays[0].write(HIGH);
        _allPowerRelays[1].write(HIGH);
        _allPowerRelays[3].write(HIGH);

        _powerLed.startPwm(SHORT_LED_PWM_INTERVAL);

        _timer->in(LONG_TASK_DELAY, turnOnPowerInternal, this);
    }

    void turnOffPower()
    {

        _buzzer->buzz(2);

        _allPowerRelays[2].write(LOW);
        _allPowerRelays[4].write(LOW);

        _powerLed.startPwm(SHORT_LED_PWM_INTERVAL);

        _timer->in(LONG_TASK_DELAY, turnOffPowerInternal, this);
    }

    void turnOffVUOnce()
    {
        Relay firstRelay = _allPowerRelays[4];
        const bool wasWritable = firstRelay.isWritable();
        if (wasWritable)
        {
            _buzzer->buzz(2, LONG_BUZZ_INTERVAL);

            firstRelay.writeAndForbidWriting(LOW);

            _timer->in(
                LONG_TASK_DELAY, turnOffVUOnceInternal, this);
        }
    }

    void setup()
    {
        _powerLed.setup();
        for (size_t i = 0; i < POWER_RELAY_COUNT; i++)
            _allPowerRelays[i].setup();
    }

    bool isPowerOn()
    {
        return _isPowerOn;
    }

    bool isSleepModeOn()
    {
        return _isSleepModeOn;
    }

    bool isPowerVUOn()
    {
        return _allPowerRelays[POWER_VU_INDEX].read();
    }
};