enum DisplayMessageType
{
    EMPTY,
    POWER_ON,
    POWER_OFF,
    TURN_ON_SLEEP_MODE,
    TURN_OFF_SLEEP_MODE,
    TURN_ON_VU,
    TURN_OFF_VU,
    EMERGENCY_POWER_OFF_BECAUSE_OF_CURRENTS,
    EMERGENCY_POWER_OFF_BECAUSE_OF_TEMPERATURE,
};

class TaskController
{
private:
    bool _isFastTaskRunning;
    bool _isLongTaskRunning;
    DisplayMessageType _displayMessageType;

    Timer<> *_timer;

    void (*_prePostTaskAction)();

    static bool completeTask(void *p)
    {
        TaskController *ptr = (TaskController *)p;

        ptr->_displayMessageType = DisplayMessageType::EMPTY;
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

    bool isFastTaskRunning()
    {
        return _isFastTaskRunning;
    }

public:
    TaskController(Timer<> *timer, void (*prePostTaskAction)())
    {
        _timer = timer;
        _prePostTaskAction = prePostTaskAction;
    }

    void runLongTask(std::function<void()> action, DisplayMessageType displayMessageType, unsigned long estimatedTime)
    {
        if (checkRunningTask())
            return;

        _displayMessageType = displayMessageType;
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

    void setDisplayMessageType(DisplayMessageType displayMessageType)
    {
        _displayMessageType = displayMessageType;
        _prePostTaskAction();
    }

    DisplayMessageType getDisplayMessageType()
    {
        return _displayMessageType;
    }

    bool isLongTaskRunning()
    {
        return _isLongTaskRunning;
    }

    bool isRunningTask()
    {
        return _isLongTaskRunning || _isFastTaskRunning;
    }
};