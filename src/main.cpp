#include <communicator.h>

Timer<> timer = timer_create_default();

Buzzer buzzer = Buzzer(&timer);

OneButton powerButton(POWER_BUTTON_PIN);
OneButton inputSelectorButton(INPUT_SELECTOR_BUTTON_PIN);

ControlBoardEEPROM eeprom = ControlBoardEEPROM();

InputSelector inputSelector = InputSelector(&eeprom);

Led inputSelectorLed = Led(INPUT_SELECTOR_BUTTON_LED_CHANNEL, INPUT_SELECTOR_BUTTON_LED_PIN, &timer);

VolumeEngine volumeEngine = VolumeEngine();

PowerController powerController = PowerController(&timer, &inputSelector, &inputSelectorLed, &buzzer, &volumeEngine, &eeprom);

TaskController taskController = TaskController(&timer, textStateAll);

TemperatureMeasurer temperatureMeasurer = TemperatureMeasurer(
    &timer,
    &taskController,
    []()
    {
      powerOffEmergency(EMERGENCY_POWER_OFF_BECAUSE_OF_TEMPERATURE);
    },
    []()
    { return powerController.anyPowerRelayTurnedOn(); });

CurrentMeasurer currentMeasurer = CurrentMeasurer(
    []()
    {
      powerOffEmergency(EMERGENCY_POWER_OFF_BECAUSE_OF_CURRENTS);
    },
    []()
    { return powerController.anyPowerRelayTurnedOn() && !taskController.isRunningTask(); });

void setup()
{
  Serial.begin(COM_PORT_SPEED);
  Serial.println("Starting...");

  eeprom.setup();

  pinMode(POWER_BUTTON_PIN, INPUT_PULLUP);
  pinMode(INPUT_SELECTOR_BUTTON_PIN, INPUT_PULLUP);

  inputSelectorLed.setup();

  buzzer.setup();

  inputSelector.setup();

  powerController.setup();

  powerButton.attachClick(onClickPowerButton);
  powerButton.attachDoubleClick(onDoubleClickPowerButton);
  powerButton.attachLongPressStart(onLongPressPowerButtonStart);

  inputSelectorButton.attachClick(onClickInputSelectorButton);
  inputSelectorButton.attachDoubleClick(onDoubleClickInputSelectorButton);
  inputSelectorButton.attachLongPressStart(onLongPressInputSelectorButtonStart);

  volumeEngine.setup();

  currentMeasurer.setup();
  temperatureMeasurer.setup();

  initSPIFFS();
  setupCommunicator();

//UNCOMMENT FOR DEBUG INFO:

  // timer.every(1000, [](void *)
  //             {
  // currentMeasurer.printDebugInfo();
  // return true; });
  
}

void loop()
{
  timer.tick<void>();

  powerButton.tick();
  inputSelectorButton.tick();
  volumeEngine.check();

  processMainPowerSource();

  currentMeasurer.process();

  tickCommunicator();
}
