#include <OneWire.h>
#include <DallasTemperature.h>

#define MEASUREMENT_INTERVAL 60 * 1000
#define MAX_NORMAL_TEMPERATURE_DEGREES_CELSIUS 65.00
#define HIGH_TEMPERATURE_MAX_DURATION 5 * 60 * 1000
#define RESOLUTION 9

class TemperatureMeasurer
{
private:
    OneWire _oneWire = OneWire(ONE_WIRE_BUS);
    DallasTemperature _sensors = DallasTemperature(&_oneWire);

    Timer<> *_timer;
    TaskController *_taskController;
    void (*_powerOff)();
    bool (*_isPowerOn)();
    bool (*_isRunningTask)();

    float _lastMeasurement = 0.00;
    unsigned long highTemperatureStaredMillis = 0;

    static bool requestAndCheckTemperature(void *p)
    {
        TemperatureMeasurer *ptr = (TemperatureMeasurer *)p;

        if(ptr->_lastMeasurement < MAX_NORMAL_TEMPERATURE_DEGREES_CELSIUS && ptr->_isRunningTask())
            return true;

        ptr->_sensors.requestTemperatures();
        ptr->_lastMeasurement = ptr->_sensors.getTempCByIndex(0);

        if (!ptr->_isPowerOn())
        {
            ptr->highTemperatureStaredMillis = 0.0;
            return true;
        }

        if (ptr->_lastMeasurement > MAX_NORMAL_TEMPERATURE_DEGREES_CELSIUS)
        {
            Serial.println("Питание больше");
            if (ptr->highTemperatureStaredMillis > 0.0 && millis() - ptr->highTemperatureStaredMillis > HIGH_TEMPERATURE_MAX_DURATION)
            {
                ptr->_powerOff();
                ptr->highTemperatureStaredMillis = 0.0;
            }
            else
            {
                if (ptr->highTemperatureStaredMillis == 0.0)
                    ptr->highTemperatureStaredMillis = millis();
            }
        }
        else
            ptr->highTemperatureStaredMillis = 0.0;

        return true;
    }

public:
    TemperatureMeasurer(Timer<> *timer, TaskController *taskController, void (*powerOff)(), bool (*isPowerOn)(), bool (*isRunningTask)())
    {
        _timer = timer;
        _taskController = taskController;
        _powerOff = powerOff;
        _isPowerOn = isPowerOn;
        _isRunningTask = isRunningTask;
    }
    void setup()
    {
        _sensors.begin();
        _sensors.setResolution(RESOLUTION);
        requestAndCheckTemperature(this);
        _timer->every(MEASUREMENT_INTERVAL, requestAndCheckTemperature, this);
    }

    float getInCelsius()
    {
        return _lastMeasurement;
    }
};