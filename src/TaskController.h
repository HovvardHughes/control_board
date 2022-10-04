#include <Arduino.h>
#include <timer.h>

enum TaskType
{
    POWER_ON,
    POWER_OFF,

    TURN_ON_SLEEP_MODE,
    TURN_OFF_SLEEP_MODE,

    TURN_OFF_VU
};

class TaskController
{
private:
    bool _isRunningTask;

    Timer<> *_timer;

    static bool finishTask(void *p)
    {
        TaskController *ptr = (TaskController *)p;

        ptr->_isRunningTask = false;

        return false;
    }

public:
    TaskController(Timer<> *timer)
    {
        _timer = timer;
    }

    void runTask(void (*action)(), TaskType taskType, unsigned long estimatedTime)
    {
        Serial.println("Check that previous task is completed to run new");

        if (_isRunningTask)
        {
            Serial.println("Cannot run task, because previous is not completed");
            return;
        }

        _isRunningTask = true;

        Serial.println("Run task :: ");
        Serial.print(taskType);

        action();

        _timer->in(estimatedTime, finishTask, this);

        return;
    }

    bool isRunningTask()
    {
        return _isRunningTask;
    }
};