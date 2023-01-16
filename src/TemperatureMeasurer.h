#include <OneWire.h>
#include <DallasTemperature.h>

#define MEASUREMENT_INTERVAL 1000 * 60

class TemperatureMeasurer
{
private:
    OneWire _oneWire = OneWire(ONE_WIRE_BUS);
    DallasTemperature _sensors = DallasTemperature(&_oneWire);

    Timer<> *_timer;
    TaskController *_taskController;

    static bool requestTemperatures(void *p)
    {
        TemperatureMeasurer *ptr = (TemperatureMeasurer *)p;
        ptr->_sensors.requestTemperatures();
        return true;
    }

public:
    TemperatureMeasurer(Timer<> *timer, TaskController *taskController)
    {
        _timer = timer;
        _taskController = taskController;
    }
    void setup()
    {
        _sensors.begin();
        _timer->every(MEASUREMENT_INTERVAL, requestTemperatures, this);
    }

    float getInCelsius()
    {
        return _sensors.getTempCByIndex(0);
    }
};