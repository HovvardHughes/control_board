#include <communicator/communicator.h>
#include <TaskController.h>

Timer<> timer = timer_create_default();

Buzzer buzzer = Buzzer(&timer);

OneButton powerButton(POWER_BUTTON_PIN);
OneButton inputSelectorButton(INPUT_SELECTOR_BUTTON_PIN);
OneButton mainPowerOnButton(MAIN_POWER_ON_PIN);

InputSelector inputSelector = InputSelector();

Led inputSelectorLed = Led(INPUT_SELECTOR_BUTTON_LED_CHANNEL, INPUT_SELECTOR_BUTTON_LED_PIN, &timer);

PowerController powerController = PowerController(&timer, &inputSelector, &inputSelectorLed, &buzzer);

TaskController taskController = TaskController(&timer, &powerController);

void setup()
{
  Serial.begin(COM_PORT_SPEED);
  Serial.println("OneButton Starting...");

  // Setup PULLUPS: INPUT_PULLUP - means pushbutton connected to VCC, INPUT_PULLDOWN - means pushbutton connected to GND
  pinMode(POWER_BUTTON_PIN, INPUT_PULLUP);
  pinMode(INPUT_SELECTOR_BUTTON_PIN, INPUT_PULLUP);
  pinMode(MAIN_POWER_ON_PIN, INPUT_PULLDOWN);

  inputSelectorLed.setup();

  buzzer.setup();

  inputSelector.setup();

  powerController.setup();

  powerButton.attachClick([]()
                          { taskController.runTask(onClickPowerButton, "Handle power button click", 2000); });
  powerButton.attachDoubleClick([]()
                                { taskController.runTaskWithPowerCheck(onDoubleClickPowerButton, "Handle power button double click", 2000); });
  powerButton.attachLongPressStart([]()
                                   { taskController.runTaskWithPowerCheck(onLongPressPowerButtonStart, "Handle power button long press start", 2000); });

  inputSelectorButton.attachClick([]()
                                  { taskController.runTaskWithPowerCheck(onClickInputSelectorButton, "Handle input selector button click", 200 + 30); });

  inputSelectorButton.attachDoubleClick([]()
                                        { taskController.runTaskWithPowerCheck(onDoubleClickInputSelectorButton, "Handle input selector button double click", 2000); });

  inputSelectorButton.attachLongPressStart([]()
                                           { taskController.runTaskWithPowerCheck(onLongPressInputSelectorButtonStart, "Handle input selector button long press start", 2000); });

  mainPowerOnButton.attachLongPressStart([]()
                                         { taskController.runTask(onLongPressMainPowerOnButtonStart, "Handle main power on button long press start ", 2000); });
  mainPowerOnButton.attachLongPressStop([]()
                                        { taskController.runTask(onLongPressMainPowerOnButtonStop, "Handle main power on button long press stop", 2000); });

  setupCommunicator();
}

void loop()
{
  timer.tick<void>();

  powerButton.tick();
  inputSelectorButton.tick();
  mainPowerOnButton.tick();

  tickCommunicator();
}
