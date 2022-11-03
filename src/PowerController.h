
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

    uint8_t _allPowerRelayPins[POWER_RELAY_COUNT] = {
        25,
        26,
        27,
        14,
        12};

    Led _powerLed;

    static bool turnOnSleepMode(void *p)
    {
        PowerController *ptr = (PowerController *)p;

        digitalWrite(ptr->_allPowerRelayPins[1], LOW);
        digitalWrite(ptr->_allPowerRelayPins[3], LOW);

        ptr->_powerLed.startPwm(LONG_LED_PWM_INTERVAL);

        ptr->_isSleepModeOn = true;

        return false;
    }

    static bool turnOffSleepMode(void *p)
    {
        PowerController *ptr = (PowerController *)p;

        digitalWrite(ptr->_allPowerRelayPins[2], HIGH);

        ptr->_powerLed.finishPwm(MAX_PWM_DUTY);

        ptr->_isSleepModeOn = false;

        return false;
    }

    static bool turnOnPower(void *p)
    {
        PowerController *ptr = (PowerController *)p;

        digitalWrite(ptr->_allPowerRelayPins[2], HIGH);
        digitalWrite(ptr->_allPowerRelayPins[4], HIGH);

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

        digitalWrite(ptr->_allPowerRelayPins[0], LOW);
        digitalWrite(ptr->_allPowerRelayPins[1], LOW);
        digitalWrite(ptr->_allPowerRelayPins[3], LOW);

        ptr->_inputSelector->writeToAllRelays(LOW);
        ptr->_inputSelectorLed->writeMin();

        ptr->_powerLed.finishPwm(MIN_PWM_DUTY);

        ptr->_isPowerOn = false;

        return false;
    }

    static bool turnOnVU(void *p)
    {
        PowerController *ptr = (PowerController *)p;
        digitalWrite(ptr->_allPowerRelayPins[VU_UNKNOWN_INDEX], HIGH);
        return false;
    }

    static bool turnOffVU(void *p)
    {
        PowerController *ptr = (PowerController *)p;
        digitalWrite(ptr->_allPowerRelayPins[VU_INDEX], LOW);
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
            digitalWrite(_allPowerRelayPins[2], LOW);
            _timer->in(LONG_TASK_DELAY, turnOnSleepMode, this);
        }
        else
        {
            digitalWrite(_allPowerRelayPins[1], HIGH);
            digitalWrite(_allPowerRelayPins[3], HIGH);
            _timer->in(LONG_TASK_DELAY, turnOffSleepMode, this);
        }
    }

    void setPower(uint8_t state)
    {
        _buzzer->buzz(2);

        if (state)
        {
            digitalWrite(_allPowerRelayPins[0], HIGH);
            digitalWrite(_allPowerRelayPins[1], HIGH);
            digitalWrite(_allPowerRelayPins[3], HIGH);

            _powerLed.startPwm(SHORT_LED_PWM_INTERVAL);

            _timer->in(LONG_TASK_DELAY, turnOnPower, this);
        }
        else
        {
            digitalWrite(_allPowerRelayPins[2], LOW);
            digitalWrite(_allPowerRelayPins[4], LOW);

            _powerLed.startPwm(SHORT_LED_PWM_INTERVAL);

            _timer->in(LONG_TASK_DELAY, turnOffPower, this);
        };
    }

    void setVU(uint8_t state)
    {
        _buzzer->buzz(6);

        if (state)
        {
            digitalWrite(_allPowerRelayPins[VU_INDEX], HIGH);
            _timer->in(LONG_TASK_DELAY, turnOnVU, this);
        }
        else
        {
            digitalWrite(_allPowerRelayPins[VU_UNKNOWN_INDEX], LOW);
            _timer->in(LONG_TASK_DELAY, turnOffVU, this);
        }
    }

    void setup()
    {
        _powerLed.setup();
        for (size_t i = 0; i < POWER_RELAY_COUNT; i++)
            pinMode(_allPowerRelayPins[i], OUTPUT);
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
        return digitalRead(_allPowerRelayPins[VU_INDEX]) && digitalRead(_allPowerRelayPins[VU_UNKNOWN_INDEX]);
    }
};