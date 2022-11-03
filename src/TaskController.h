enum LongTaskType
{
    NONE,
    POWER_ON,
    POWER_OFF,
    TURN_ON_SLEEP_MODE,
    TURN_OFF_SLEEP_MODE,
    TURN_ON_VU,
    TURN_OFF_VU,
};

class TaskController
{
private:
    bool _isFastTaskRunning;
    bool _isLongTaskRunning;
    LongTaskType _runningLongTaskType;

    Timer<> *_timer;

    void (*_prePostTaskAction)();

    static bool completeTask(void *p)
    {
        TaskController *ptr = (TaskController *)p;

        ptr->_runningLongTaskType = LongTaskType::NONE;
        ptr->_isLongTaskRunning = false;
        ptr->_isFastTaskRunning = false;
        ptr->_prePostTaskAction();

        return false;
    }

    bool checkRunningTask()
    {
        Serial.println("Check that previous task is completed to run new");

        if (_isLongTaskRunning || _isFastTaskRunning)
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

    void runLongTask(std::function<void()> action, LongTaskType taskType, unsigned long estimatedTime)
    {
        if (checkRunningTask())
            return;

        _runningLongTaskType = taskType;
        _isLongTaskRunning = true;

        _prePostTaskAction();

        action();

        _timer->in(estimatedTime, completeTask, this);
    }

    void runFastTask(std::function<void()> action)
    {
        if (checkRunningTask())
            return;

        _isFastTaskRunning = true;
        _timer->in(FAST_TASK_RUNTIME, completeTask, this);

        action();
    }

    bool isLongTaskRunning()
    {
        return _isLongTaskRunning;
    }

    LongTaskType getRunningLongTaskType()
    {
        return _runningLongTaskType;
    }
};