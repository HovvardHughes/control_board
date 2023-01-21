
#include <arduino-timer.h>
#include <InputSelector.h>
#include <Buzzer.h>
#include <VolumeEngine.h>
#include <ControlBoardEEPROM.h>

#define MAIN_POWER_SOURCE_STATE_CHANGED_DEBOUNCE_MILLIS 3000

class PowerController
{

private:
    Timer<> *_timer;

    InputSelector *_inputSelector;
    Led *_inputSelectorLed;
    Buzzer *_buzzer;
    VolumeEngine *_volumeEngine;
    ControlBoardEEPROM *_eeprom;

    bool _isPowerOn;
    bool _isSleepModeOn;
    bool _VUTurnedOffManually;

    Led _powerLed;

    int _isMainPowerSourceOn;
    unsigned long _mainPowerSourceStateChangedStartMillis;

    void writeWithVUCheck(uint8_t pin, uint8_t state)
    {
        if ((pin == PRLY3_PIN || pin == PRLY4_PIN) && _VUTurnedOffManually)
            return;

        digitalWrite(pin, state);
    }

    static bool turnOnSleepMode(void *p)
    {
        PowerController *ptr = (PowerController *)p;

        ptr->writeWithVUCheck(PRLY1_PIN, LOW);
        ptr->writeWithVUCheck(PRLY3_PIN, LOW);

        ptr->_powerLed.startPwm(LONG_LED_PWM_INTERVAL);

        ptr->_isSleepModeOn = true;

        return false;
    }

    static bool turnOffSleepMode(void *p)
    {
        PowerController *ptr = (PowerController *)p;

        ptr->writeWithVUCheck(PRLY2_PIN, HIGH);

        ptr->_isSleepModeOn = false;

        return false;
    }

    static bool turnOnPower(void *p)
    {
        PowerController *ptr = (PowerController *)p;

        ptr->writeWithVUCheck(PRLY2_PIN, HIGH);
        ptr->writeWithVUCheck(PRLY4_PIN, HIGH);

        ptr->_inputSelector->writeToSeletedRelay(HIGH);
        ptr->_inputSelectorLed->writeMax();

        ptr->_buzzer->buzz(4);

        ptr->_powerLed.completePwm(MAX_PWM_DUTY);

        ptr->_isPowerOn = true;

        return false;
    }

    static bool turnOffPower(void *p)
    {
        PowerController *ptr = (PowerController *)p;

        ptr->writeWithVUCheck(PRLY0_PIN, LOW);
        ptr->writeWithVUCheck(PRLY1_PIN, LOW);
        ptr->writeWithVUCheck(PRLY3_PIN, LOW);

        ptr->_inputSelector->writeToAllRelays(LOW);
        ptr->_inputSelectorLed->writeMin();

        ptr->_powerLed.completePwm(MIN_PWM_DUTY);

        ptr->_isPowerOn = false;

        return false;
    }

    static bool turnOnVU(void *p)
    {
        PowerController *ptr = (PowerController *)p;
        digitalWrite(PRLY4_PIN, HIGH);
        ptr->_VUTurnedOffManually = false;
        ptr->_eeprom->writeVUTurnedOffManually(false);
        return false;
    }

    static bool turnOffVU(void *p)
    {
        PowerController *ptr = (PowerController *)p;
        digitalWrite(PRLY3_PIN, LOW);
        ptr->_VUTurnedOffManually = true;
        ptr->_eeprom->writeVUTurnedOffManually(true);
        return false;
    }

public:
    PowerController(Timer<> *timer, InputSelector *inputSelector, Led *inputSelectorLed, Buzzer *buzzer, VolumeEngine *volumeEngine, ControlBoardEEPROM *eeprom)
    {
        _timer = timer;
        _inputSelector = inputSelector;
        _inputSelectorLed = inputSelectorLed;
        _buzzer = buzzer;
        _volumeEngine = volumeEngine;
        _eeprom = eeprom;
        _powerLed = Led(POWER_BUTTON_LED_CHANNEL, POWER_BUTTON_LED_PIN, timer);
    }

    void setSleepMode(uint8_t state)
    {
        _buzzer->buzz(4);

        if (state)
        {
            _volumeEngine->turnOff();
            writeWithVUCheck(PRLY2_PIN, LOW);
            _timer->in(LONG_TASK_DELAY, turnOnSleepMode, this);
        }
        else
        {
            _powerLed.completePwm(MAX_PWM_DUTY);
            writeWithVUCheck(PRLY1_PIN, HIGH);
            writeWithVUCheck(PRLY3_PIN, HIGH);
            _timer->in(LONG_TASK_DELAY, turnOffSleepMode, this);
        }
    }

    void setPower(uint8_t state)
    {
        _buzzer->buzz(2);

        if (state)
        {
            writeWithVUCheck(PRLY0_PIN, HIGH);
            writeWithVUCheck(PRLY1_PIN, HIGH);
            writeWithVUCheck(PRLY3_PIN, HIGH);

            _powerLed.startPwm(SHORT_LED_PWM_INTERVAL);

            _timer->in(LONG_TASK_DELAY, turnOnPower, this);
        }
        else
        {
            _volumeEngine->turnOff();
            writeWithVUCheck(PRLY2_PIN, LOW);
            writeWithVUCheck(PRLY4_PIN, LOW);

            _powerLed.startPwm(SHORT_LED_PWM_INTERVAL);

            _timer->in(LONG_TASK_DELAY, turnOffPower, this);
        };
    }

    void powerOffEmergency()
    {
        _buzzer->buzz(10, LONG_BUZZ_INTERVAL);

        _volumeEngine->turnOff();

        digitalWrite(PRLY2_PIN, LOW);
        digitalWrite(PRLY4_PIN, LOW);
        digitalWrite(PRLY0_PIN, LOW);
        digitalWrite(PRLY1_PIN, LOW);
        digitalWrite(PRLY3_PIN, LOW);
        _powerLed.writeMin();

        _inputSelector->writeToAllRelays(LOW);
        _inputSelectorLed->writeMin();

        _isSleepModeOn = false;
        _isPowerOn = false;
    }

    void setVU(uint8_t state)
    {
        _buzzer->buzz(2, LONG_BUZZ_INTERVAL);

        if (state)
        {
            digitalWrite(PRLY3_PIN, HIGH);
            _timer->in(LONG_TASK_DELAY, turnOnVU, this);
        }
        else
        {
            digitalWrite(PRLY4_PIN, LOW);
            _timer->in(LONG_TASK_DELAY, turnOffVU, this);
        }
    }

    void setup()
    {
        _powerLed.setup();
        pinMode(PRLY0_PIN, OUTPUT);
        pinMode(PRLY1_PIN, OUTPUT);
        pinMode(PRLY2_PIN, OUTPUT);
        pinMode(PRLY3_PIN, OUTPUT);
        pinMode(PRLY4_PIN, OUTPUT);
        pinMode(MAIN_POWER_SOURCE_PIN, INPUT_PULLDOWN);
        _VUTurnedOffManually = _eeprom->readVUTurnedOffManually();
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
        return digitalRead(PRLY3_PIN) && digitalRead(PRLY4_PIN);
    }

    bool isMainPowerSourceOn()
    {
        return _isMainPowerSourceOn;
    }

    bool updateMainPowerState()
    {
        int state = digitalRead(MAIN_POWER_SOURCE_PIN);

        if (state != _isMainPowerSourceOn)
        {
            if (_mainPowerSourceStateChangedStartMillis == 0)
                _mainPowerSourceStateChangedStartMillis = millis();
            else
            {
                if (millis() - _mainPowerSourceStateChangedStartMillis >= MAIN_POWER_SOURCE_STATE_CHANGED_DEBOUNCE_MILLIS)
                {
                    _mainPowerSourceStateChangedStartMillis = 0;
                    _isMainPowerSourceOn = state;
                    return true;
                }
            }
        }
        else
            _mainPowerSourceStateChangedStartMillis = 0;

        return false;
    }
};