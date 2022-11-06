
#include <arduino-timer.h>
#include <InputSelector.h>
#include <Buzzer.h>
#include <VolumeEngine.h>

#define PRLY0 25
#define PRLY1 26
#define PRLY2 27
#define PRLY3 14
#define PRLY4 12

class PowerController
{

private:
    Timer<> *_timer;

    InputSelector *_inputSelector;
    Led *_inputSelectorLed;
    Buzzer *_buzzer;
    VolumeEngine *_volumeEngine;

    bool _isPowerOn;
    bool _isSleepModeOn;
    bool _VUTurnedOffManually;

    Led _powerLed;

    void writeWithVUCheck(uint8_t pin, uint8_t state)
    {
        if ((pin == PRLY3 || pin == PRLY4) && _VUTurnedOffManually)
            return;

        digitalWrite(pin, state);
    }

    static bool turnOnSleepMode(void *p)
    {
        PowerController *ptr = (PowerController *)p;

        ptr->writeWithVUCheck(PRLY1, LOW);
        ptr->writeWithVUCheck(PRLY3, LOW);

        ptr->_powerLed.startPwm(LONG_LED_PWM_INTERVAL);

        ptr->_isSleepModeOn = true;

        return false;
    }

    static bool turnOffSleepMode(void *p)
    {
        PowerController *ptr = (PowerController *)p;

        ptr->writeWithVUCheck(PRLY2, HIGH);

        ptr->_powerLed.finishPwm(MAX_PWM_DUTY);

        ptr->_isSleepModeOn = false;

        return false;
    }

    static bool turnOnPower(void *p)
    {
        PowerController *ptr = (PowerController *)p;

        ptr->writeWithVUCheck(PRLY2, HIGH);
        ptr->writeWithVUCheck(PRLY4, HIGH);

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

        ptr->writeWithVUCheck(PRLY0, LOW);
        ptr->writeWithVUCheck(PRLY1, LOW);
        ptr->writeWithVUCheck(PRLY3, LOW);

        ptr->_inputSelector->writeToAllRelays(LOW);
        ptr->_inputSelectorLed->writeMin();

        ptr->_powerLed.finishPwm(MIN_PWM_DUTY);

        ptr->_isPowerOn = false;

        return false;
    }

    static bool turnOnVU(void *p)
    {
        PowerController *ptr = (PowerController *)p;
        digitalWrite(PRLY4, HIGH);
        ptr->_VUTurnedOffManually = false;
        return false;
    }

    static bool turnOffVU(void *p)
    {
        PowerController *ptr = (PowerController *)p;
        digitalWrite(PRLY3, LOW);
        ptr->_VUTurnedOffManually = true;
        return false;
    }

public:
    PowerController(Timer<> *timer, InputSelector *inputSelector, Led *inputSelectorLed, Buzzer *buzzer, VolumeEngine *volumeEngine)
    {
        _timer = timer;
        _inputSelector = inputSelector;
        _inputSelectorLed = inputSelectorLed;
        _buzzer = buzzer;
        _volumeEngine = volumeEngine;
        _powerLed = Led(POWER_BUTTON_LED_CHANNEL, POWER_BUTTON_LED_PIN, timer);
    }

    void setSleepMode(uint8_t state)
    {
        _buzzer->buzz(4);

        if (state)
        {
            writeWithVUCheck(PRLY2, LOW);
            _timer->in(LONG_TASK_DELAY, turnOnSleepMode, this);
        }
        else
        {
            _volumeEngine->turnOff();
            writeWithVUCheck(PRLY1, HIGH);
            writeWithVUCheck(PRLY3, HIGH);
            _timer->in(LONG_TASK_DELAY, turnOffSleepMode, this);
        }
    }

    void setPower(uint8_t state)
    {
        _buzzer->buzz(2);

        if (state)
        {
            writeWithVUCheck(PRLY0, HIGH);
            writeWithVUCheck(PRLY1, HIGH);
            writeWithVUCheck(PRLY3, HIGH);

            _powerLed.startPwm(SHORT_LED_PWM_INTERVAL);

            _timer->in(LONG_TASK_DELAY, turnOnPower, this);
        }
        else
        {
            _volumeEngine->turnOff();
            writeWithVUCheck(PRLY2, LOW);
            writeWithVUCheck(PRLY4, LOW);

            _powerLed.startPwm(SHORT_LED_PWM_INTERVAL);

            _timer->in(LONG_TASK_DELAY, turnOffPower, this);
        };
    }

    void setVU(uint8_t state)
    {
        _buzzer->buzz(2, LONG_BUZZ_INTERVAL);

        if (state)
        {
            digitalWrite(PRLY3, HIGH);
            _timer->in(LONG_TASK_DELAY, turnOnVU, this);
        }
        else
        {
            digitalWrite(PRLY4, LOW);
            _timer->in(LONG_TASK_DELAY, turnOffVU, this);
        }
    }

    void setup()
    {
        _powerLed.setup();
        pinMode(PRLY0, OUTPUT);
        pinMode(PRLY1, OUTPUT);
        pinMode(PRLY2, OUTPUT);
        pinMode(PRLY3, OUTPUT);
        pinMode(PRLY4, OUTPUT);
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
        return digitalRead(PRLY3) && digitalRead(PRLY4);
    }
};