#include <communicator.h>

Timer<> timer = timer_create_default();

Buzzer buzzer = Buzzer(&timer);

OneButton powerButton(POWER_BUTTON_PIN);
OneButton inputSelectorButton(INPUT_SELECTOR_BUTTON_PIN);

InputSelector inputSelector = InputSelector();

Led inputSelectorLed = Led(INPUT_SELECTOR_BUTTON_LED_CHANNEL, INPUT_SELECTOR_BUTTON_LED_PIN, &timer);

VolumeEngine volumeEngine = VolumeEngine();

PowerController powerController = PowerController(&timer, &inputSelector, &inputSelectorLed, &buzzer, &volumeEngine);

TaskController taskController = TaskController(&timer, textStateAll);

void setup()
{
  Serial.begin(COM_PORT_SPEED);
  Serial.println("OneButton Starting...");

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

  initSPIFFS();
  setupCommunicator();
}

void loop()
{
  timer.tick<void>();

  powerButton.tick();
  inputSelectorButton.tick();
  volumeEngine.turnOffIfActiveForLongTime();

  tickCommunicator();
}
