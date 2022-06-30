#include <Arduino.h>
#include <timer.h>


class TaskController
{
private:
    bool _isRunningTask;

    Timer<> *_timer;
    PowerController *_powerController;

    static bool finishTask(void *p)
    {
        TaskController *ptr = (TaskController *)p;

        ptr->_isRunningTask = false;

        return false;
    }

public:
    TaskController(Timer<> *timer, PowerController *powerController)
    {
        _timer = timer;
        _powerController = powerController;
    }

    void runTask(void (*action)(), String taskTitle, unsigned long estimatedTime)
    {
        Serial.println("Check that previous task is completed to run new");

        if (_isRunningTask)
        {
            Serial.println("Cannot run task, because previous is not completed");
            return;
        }

        Serial.println("Run task :: ");
        Serial.print(taskTitle);

        action();

        _timer->in(estimatedTime, finishTask, this);
    }

    void runTaskWithPowerCheck(void (*action)(), String taskTitle, unsigned long estimatedTime)
    {
        Serial.println("Check power to run task");

        if (!_powerController->isPowerOn())
        {
            Serial.println("Cannot run task, because power is off");
            return;
        }

        runTask(action,  taskTitle, estimatedTime);
    }
};