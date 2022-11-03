
#include <timer.h>
#include <InputSelector.h>
#include <Buzzer.h>

#define POWER_RELAY_COUNT 5
#define VU_INDEX 3
#define VU_UNKNOWN_INDEX 4

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

    static bool turnOnSleepMode(void *p)
    {
        PowerController *ptr = (PowerController *)p;

        ptr->_allPowerRelays[1].write(LOW);
        ptr->_allPowerRelays[3].write(LOW);

        ptr->_powerLed.startPwm(LONG_LED_PWM_INTERVAL);

        ptr->_isSleepModeOn = true;

        return false;
    }

    static bool turnOffSleepMode(void *p)
    {
        PowerController *ptr = (PowerController *)p;

        ptr->_allPowerRelays[2].write(HIGH);

        ptr->_powerLed.finishPwm(MAX_PWM_DUTY);

        ptr->_isSleepModeOn = false;

        return false;
    }

    static bool turnOnPower(void *p)
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

    static bool turnOffPower(void *p)
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

    static bool turnOnVU(void *p)
    {
        PowerController *ptr = (PowerController *)p;
        ptr->_allPowerRelays[VU_UNKNOWN_INDEX].write(HIGH);
        return false;
    }

    static bool turnOffVU(void *p)
    {
        PowerController *ptr = (PowerController *)p;
        ptr->_allPowerRelays[VU_INDEX].write(LOW);
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

    void setSleepMode(uint8_t state)
    {
        _buzzer->buzz(4);

        if (state)
        {
            _allPowerRelays[2].write(LOW);
            _timer->in(LONG_TASK_DELAY, turnOnSleepMode, this);
        }
        else
        {
            _allPowerRelays[1].write(HIGH);
            _allPowerRelays[3].write(HIGH);
            _timer->in(LONG_TASK_DELAY, turnOffSleepMode, this);
        }
    }

    void setPower(uint8_t state)
    {
        _buzzer->buzz(2);

        if (state)
        {
            _allPowerRelays[0].write(HIGH);
            _allPowerRelays[1].write(HIGH);
            _allPowerRelays[3].write(HIGH);

            _powerLed.startPwm(SHORT_LED_PWM_INTERVAL);

            _timer->in(LONG_TASK_DELAY, turnOnPower, this);
        }
        else
        {
            _allPowerRelays[2].write(LOW);
            _allPowerRelays[4].write(LOW);

            _powerLed.startPwm(SHORT_LED_PWM_INTERVAL);

            _timer->in(LONG_TASK_DELAY, turnOffPower, this);
        };
    }

    void setVU(uint8_t state)
    {
        _buzzer->buzz(6);

        if (state)
        {
            _allPowerRelays[VU_INDEX].write(state);
            _timer->in(LONG_TASK_DELAY, turnOnVU, this);
        }
        else
        {
            _allPowerRelays[VU_UNKNOWN_INDEX].write(state);
            _timer->in(LONG_TASK_DELAY, turnOffVU, this);
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

    bool isVUOn()
    {
        return _allPowerRelays[VU_INDEX].read() && _allPowerRelays[VU_UNKNOWN_INDEX].read();
    }
};