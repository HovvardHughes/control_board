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
    bool _isRunningTaskForbidden;
    bool _isRunningTask;
    TaskType _runningTaskType;

    Timer<> *_timer;

    void (*_prePostTaskAction)();

    static bool finishTask(void *p)
    {
        TaskController *ptr = (TaskController *)p;
        ptr->_runningTaskType = TaskType::NONE;
        ptr->_isRunningTask = false;
        ptr->_isRunningTaskForbidden = false;
        ptr->_prePostTaskAction();

        return false;
    }

    bool checkRunningTask()
    {
        Serial.println("Check that previous task is completed to run new");

        if (_isRunningTask || _isRunningTaskForbidden)
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

    void runTask(std::function < void() > action, TaskType taskType, unsigned long estimatedTime)
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

    void forbidTaskRunning(unsigned long time)
    {
        _isRunningTaskForbidden = true;
        _timer->in(time, finishTask, this);
    }

    bool isRunningTask()
    {
        return _isRunningTask;
    }

    String getRunningTaskTitle()
    {
        switch (_runningTaskType)
        {
        case TaskType::POWER_ON:
            return "Power on";
        case TaskType::POWER_OFF:
            return "Power off";
        case TaskType::TURN_ON_SLEEP_MODE:
            return "Turn on sleep mode";
        case TaskType::TURN_OFF_SLEEP_MODE:
            return "Turn off sleep mode";
        case TaskType::TURN_OFF_VU:
            return "Turn off VU";
        default:
            return "No active tasks";
        }
    }

    void runFastTask(std::function < void() > action)
    {
        forbidTaskRunning(500);

        if (checkRunningTask())
            return;

        _isRunningTaskForbidden = true;

        _timer->in(500, finishTask, this);

        action();
    }
};