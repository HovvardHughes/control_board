const Commands = {
  GET_STATE: 0,
  SWITCH_POWER: 1,
  SWITCH_SLEEP_MODE: 2,
  POWER_OFF_VU: 3,
  TURN_ON_MAIN_RELAY: 4,
  TURN_OFF_MAIN_RELAY: 5,
  TURN_ON_SECONDARY_RELAY: 6,
  TURN_OFF_SECONDARY_RELAY: 7,
  TURN_OFF_VOLUME_PWM: 8,
  REVERSE_LOW_VOLUME_PWM: 9,
  REVERSE_MEDIUM_VOLUME_PWM: 10,
  REVERSE_HIGH_VOLUME_PWM: 11,
  FORWARD_LOW_VOLUME_PWM: 12,
  FORWARD_MEDIUM_VOLUME_PWM: 13,
  FORWARD_HIGH_VOLUME_PWM: 14,
}

const TaskTypes = {
  NONE: 0,
  POWER_ON: 1,
  POWER_OFF: 2,
  TURN_ON_SLEEP_MODE: 3,
  TURN_OFF_SLEEP_MODE : 4,
  TURN_OFF_VU: 5,
}

const TaskTypesTitles = {
  [TaskTypes.NONE]: "No active tasks",
  [TaskTypes.POWER_ON]: "Power on",
  [TaskTypes.POWER_OFF]: "Power off",
  [TaskTypes.TURN_ON_SLEEP_MODE]: "Turn on sleep mode",
  [TaskTypes.TURN_OFF_SLEEP_MODE] : "Turn off sleep mode",
  [TaskTypes.TURN_OFF_VU]: "Turn off VU",
}

const IDs = {
  POWER_BUTTON: "power-button",
  SLEEP_MODE_BUTTON: "sleep-mode-button",
  POWER_OFF_VU__BUTTON: "power-off-vu-button",
  VOLUME_SLIDER_CONTAINER: "volume-slider-container",
  VOLUME_SLIDER: "volume-slider",
  INPUT_SELECTOR: "input-selector",
  MAIN_INPUT: "main-relay",
  SECONDARY_INPUT: "secondary-relay",
  DISPLAY_MESSAGE: "log-message"
}

var websocket;

window.addEventListener('load', onLoad);

function initWebSocket() {
  websocket = new WebSocket(`ws://${window.location.hostname}/ws`);
  websocket.onclose = onClose;
  websocket.onmessage = onMessage;
}

function onClose(event) {
  setTimeout(initWebSocket, 2000);
}

function onMessage(event) {
  const getBit = (n, bitPosition) => (n >> bitPosition) & 1

  const splitData = event.data.split("|");

  const state = parseInt(splitData[0]);
  const runningTaskType = parseInt(splitData[1]);

  const isPowerOn = getBit(state, 0);
  const isSleepModeOn = getBit(state, 1);
  const isVUOn = getBit(state, 2);
  const mainInputRelay = getBit(state, 3);
  const secondaryInputRelay = getBit(state, 4);
  const isRunningTask = getBit(state, 5);

  displayTaskType(runningTaskType);

  addOrRemoveClass(IDs.DISPLAY_MESSAGE, "progress-ellipsis", isRunningTask);

  addOrRemoveClass(IDs.POWER_BUTTON, "progress", runningTaskType === TaskTypes.POWER_ON || runningTaskType === TaskTypes.POWER_OFF);
  addOrRemoveClass(IDs.SLEEP_MODE_BUTTON, "progress", runningTaskType === TaskTypes.TURN_ON_SLEEP_MODE || runningTaskType === TaskTypes.TURN_OFF_SLEEP_MODE)
  addOrRemoveClass(IDs.POWER_OFF_VU__BUTTON, "progress", runningTaskType === TaskTypes.TURN_OFF_VU)

  addOrRemoveClass(IDs.POWER_BUTTON, "enabled-power-button", isPowerOn);
  addOrRemoveClass(IDs.SLEEP_MODE_BUTTON, "enabled-sleep-mode-button", isSleepModeOn);
  addOrRemoveClass(IDs.POWER_OFF_VU__BUTTON, "enabled-vu-button", isVUOn);

  setAtrribute("checked", {
    [IDs.MAIN_INPUT]: mainInputRelay,
    [IDs.SECONDARY_INPUT]: secondaryInputRelay,
  });

  const isRunningTaskOrPowerTurndedOff = isRunningTask || !isPowerOn;

  setAtrribute("disabled", {
    [IDs.POWER_BUTTON]: isRunningTask || isSleepModeOn,
    [IDs.SLEEP_MODE_BUTTON]: isRunningTaskOrPowerTurndedOff,
    [IDs.POWER_OFF_VU__BUTTON]: isRunningTaskOrPowerTurndedOff || isSleepModeOn | !isVUOn,
    [IDs.MAIN_INPUT]: isRunningTaskOrPowerTurndedOff,
    [IDs.SECONDARY_INPUT]:  isRunningTaskOrPowerTurndedOff,
    [IDs.VOLUME_SLIDER]: isRunningTaskOrPowerTurndedOff
  });

  addOrRemoveClass(IDs.INPUT_SELECTOR, "disabled", isRunningTaskOrPowerTurndedOff);
  addOrRemoveClass(IDs.VOLUME_SLIDER_CONTAINER, "disabled", isRunningTaskOrPowerTurndedOff);
}

function displayTaskType(taksType) {
  const logMessage = document.getElementById("log-message")
  logMessage.innerText = TaskTypesTitles[taksType];
}

function addOrRemoveClass(id, className, addClass) {
  const element = document.getElementById(id);
  if(addClass)
    element.classList.add(className)
  else
    element.classList.remove(className)
}

function setAtrribute(attributeName, idsWithAttributeValue) {
  Object.entries(idsWithAttributeValue).forEach(([id, attributeValue]) => document.getElementById(id)[attributeName] = attributeValue);
}

function onLoad(event) {
  initWebSocket();
  initButtons();
  displayTaskType(TaskTypes.NONE);
  setAtrribute("disabled", Object.fromEntries(Object.values(IDs).map(id => [id, false])));
  addOrRemoveClass(IDs.INPUT_SELECTOR, "disabled", false);
  addOrRemoveClass(IDs.VOLUME_SLIDER_CONTAINER, "disabled", false);
}

function initButtons() {
  document.getElementById('power-button').addEventListener('click', switchPower);
  document.getElementById('sleep-mode-button').addEventListener('click', switchSleepMode);
  document.getElementById('power-off-vu-button').addEventListener('click', powerOffVU);
  document.getElementById('main-relay').addEventListener('click', handleInputSwitchClicked);
  document.getElementById('secondary-relay').addEventListener('click', handleInputSwitchClicked);
  const volumeSlider = document.getElementById("volume-slider");
  volumeSlider.addEventListener("mouseup", handleVolumeSliderMouseUp);
  volumeSlider.addEventListener("input", handleVolumeSliderChanged);
  volumeSlider.addEventListener("keydown", handleVolumeSliderKeyDown);
}

function switchPower() {
  websocket.send(Commands.SWITCH_POWER);
}

function switchSleepMode() {
  websocket.send(Commands.SWITCH_SLEEP_MODE);
}

function powerOffVU() {
  websocket.send(Commands.POWER_OFF_VU);
}

function handleInputSwitchClicked(event) {
  const {value, checked} = event.target;
  if (checked)
    websocket.send(value === "0" ? Commands.TURN_ON_MAIN_RELAY : Commands.TURN_ON_SECONDARY_RELAY);
  else
    websocket.send(value === "0" ? Commands.TURN_OFF_MAIN_RELAY : Commands.TURN_OFF_SECONDARY_RELAY);
}

function handleVolumeSliderChanged(event) {
  const value = parseInt(event.target.value);
  console.log(value);
  console.log(value === 50);

 

  if(value === 99) {
    console.log("REVERSE :: HIGH")
    websocket.send(Commands.FORWARD_HIGH_VOLUME_PWM);
  }

  if(value === 83) {
    console.log("REVERSE :: MEDIUM")
    websocket.send(Commands.FORWARD_MEDIUM_VOLUME_PWM);
  }

  if(value === 66){
    console.log("REVERSE :: LOW")
    websocket.send(Commands.FORWARD_LOW_VOLUME_PWM);
  }

  if (value === 50){
    console.log("TURN_OFF")
    websocket.send(Commands.TURN_OFF_VOLUME_PWM);
  }

  if (value === 33){
    console.log("FORWARD :: LOW")
    websocket.send(Commands.REVERSE_LOW_VOLUME_PWM);
  }

  if (value === 16){
    console.log("FORWARD :: MEDIUM")
    websocket.send(Commands.REVERSE_MEDIUM_VOLUME_PWM);
  }

  if (value === 0) {
    console.log("FORWARD :: HIGH")
    websocket.send(Commands.REVERSE_HIGH_VOLUME_PWM);
  }

}

function handleVolumeSliderMouseUp() {
  document.getElementById("volume-slider").value = 50;
  websocket.send(Commands.TURN_OFF_VOLUME_PWM);
}

function handleVolumeSliderKeyDown(event) {
  event.preventDefault();
}