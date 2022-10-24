#include <Arduino.h>
#include <timer.h>

enum TaskType
{
    NONE,
    POWER_ON,
    POWER_OFF,
    TURN_ON_SLEEP_MODE,
    TURN_OFF_SLEEP_MODE,
    TURN_OFF_VU,
};

class TaskController
{
private:
    bool _isTaskRunningForbidden;
    bool _isRunningTask;
    TaskType _runningTaskType;

    Timer<> *_timer;

    void (*_prePostTaskAction)();

    static bool finishTask(void *p)
    {
        TaskController *ptr = (TaskController *)p;
        ptr->_runningTaskType = TaskType::NONE;
        ptr->_isRunningTask = false;
        ptr->_isTaskRunningForbidden = false;
        ptr->_prePostTaskAction();

        return false;
    }

    bool checkRunningTask()
    {
        Serial.println("Check that previous task is completed to run new");

        if (_isRunningTask || _isTaskRunningForbidden)
        {
            Serial.println("Cannot run task, because previous is not completed");
            return true;
        }

        return false;
    }

public:
    TaskController(Timer<> *timer, void (*prePostTaskAction)())
    {
        _timer = timer;
        _prePostTaskAction = prePostTaskAction;
    }

    void runTask(std::function<void()> action, TaskType taskType, unsigned long estimatedTime)
    {
        if (checkRunningTask())
            return;

        _runningTaskType = taskType;
        _isRunningTask = true;

        _prePostTaskAction();

        Serial.println("Run task :: ");
        Serial.print(taskType);

        action();

        _timer->in(estimatedTime, finishTask, this);
    }


    void runFastTask(std::function<void()> action)
    {
        if (checkRunningTask())
            return;

        _isTaskRunningForbidden = true;
        _timer->in(FAST_TASK_RUNTIME, finishTask, this);

        action();
    }

    bool isRunningTask()
    {
        return _isRunningTask;
    }

    TaskType getRunningTaskType()
    {
        return _runningTaskType;
    }
};