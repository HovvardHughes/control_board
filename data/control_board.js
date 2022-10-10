const Command = {
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

const TaskType = {
  NONE: 0,
  POWER_ON: 1,
  POWER_OFF: 2,
  TURN_ON_SLEEP_MODE: 3,
  TURN_OFF_SLEEP_MODE : 4,
  TURN_OFF_VU: 5,
}

var gateway = `ws://${window.location.hostname}/ws`;
var websocket;

var areAllControlsDisabled = true;

window.addEventListener('load', onLoad);

function initWebSocket() {
  websocket = new WebSocket(gateway);
  websocket.onclose = onClose;
  websocket.onmessage = onMessage;
}

function onClose(event) {
  setTimeout(initWebSocket, 2000);
}

const getBit = (n, bitPosition) => (n >> bitPosition) & 1

function onMessage(event) {
  const splitData = event.data.split("|");

  const state = parseInt(splitData[0]);
  const runningTaskType = parseInt(splitData[1]);

  const isPowerOn = getBit(state, 0);
  const isSleepModeOn = getBit(state, 1);
  const isVUOn = getBit(state, 2);
  const mainInputRelay = getBit(state, 3);
  const secondaryInputRelay = getBit(state, 4);
  const isRunningTask = getBit(state, 5);

  const logMessage = document.getElementById("log-message")
  logMessage.innerText = getRunningTaskTitle(runningTaskType);

  if(isRunningTask)
    logMessage.classList.add("progress-ellipsis")
  else 
    logMessage.classList.remove("progress-ellipsis")

  const powerButton = document.getElementById('power-button');
  const sleepModeButton = document.getElementById('sleep-mode-button');
  const powerOffButtonVUButton = document.getElementById('power-off-vu-button');
  powerButton.disabled = isRunningTask || isSleepModeOn;
  sleepModeButton.disabled = isRunningTask || !isPowerOn;
  powerOffButtonVUButton.disabled = isRunningTask || !isPowerOn || isSleepModeOn | !isVUOn;

  if(runningTaskType === TaskType.POWER_ON || runningTaskType === TaskType.POWER_OFF)
    powerButton.classList.add("progress")
  else 
    powerButton.classList.remove("progress")

  if(runningTaskType === TaskType.TURN_ON_SLEEP_MODE || runningTaskType === TaskType.TURN_OFF_SLEEP_MODE)
    sleepModeButton.classList.add("progress")
  else 
    sleepModeButton.classList.remove("progress")

  if(runningTaskType === TaskType.TURN_OFF_VU)
    powerOffButtonVUButton.classList.add("progress")
  else
    powerOffButtonVUButton.classList.remove("progress")


    if(isPowerOn)
    powerButton.classList.add("enabled")
  else 
    powerButton.classList.remove("enabled")

  if(isSleepModeOn)
    sleepModeButton.classList.add("enabled")
  else 
    sleepModeButton.classList.remove("enabled")

  if(isVUOn)
    powerOffButtonVUButton.classList.add("enabled")
  else
    powerOffButtonVUButton.classList.remove("enabled")

  document.getElementById('volume-slider').disabled = isRunningTask || !isPowerOn;

  const mainRelayInput = document.getElementById('main-relay');
  const secondaryRelayInput = document.getElementById('secondary-relay');
  
  mainRelayInput.checked = mainInputRelay;
  secondaryRelayInput.checked = secondaryInputRelay;

  mainRelayInput.disabled = isRunningTask || !isPowerOn;
  secondaryRelayInput.disabled = isRunningTask || !isPowerOn;
}

function onLoad(event) {
  initWebSocket();
  initButtons();
}

function initButtons() {
  document.getElementById('power-button').addEventListener('click', switchPower);
  document.getElementById('sleep-mode-button').addEventListener('click', switchSleepMode);
  document.getElementById('power-off-vu-button').addEventListener('click', powerOffVU);
  document.getElementById('main-relay').addEventListener('click', handleInputSwitchClicked);
  document.getElementById('secondary-relay').addEventListener('click', handleInputSwitchClicked);
  const volumeSlider = document.getElementById("volume-slider");
  volumeSlider.addEventListener("mouseup", handleVolumeSliderMouseUpped);
  volumeSlider.addEventListener("input", handleVolumeSliderChanged);
}

function switchPower() {
  websocket.send(Command.SWITCH_POWER);
}

function switchSleepMode() {
  websocket.send(Command.SWITCH_SLEEP_MODE);
}

function powerOffVU() {
  websocket.send(Command.POWER_OFF_VU);
}

function handleInputSwitchClicked(event) {
  const {value, checked} = event.target;
  if (checked)
    websocket.send(value === "0" ? Command.TURN_ON_MAIN_RELAY : Command.TURN_ON_SECONDARY_RELAY);
  else
    websocket.send(value === "0" ? Command.TURN_OFF_MAIN_RELAY : Command.TURN_OFF_SECONDARY_RELAY);
}


function handleVolumeSliderChanged(event) {
  const {value} = event.target;
  if(value <= 16) 
    websocket.send(Command.REVERSE_HIGH_VOLUME_PWM);

  if(value > 16 && value <= 32)
    websocket.send(Command.REVERSE_MEDIUM_VOLUME_PWM);

  if(value > 32 && value <= 50)
    websocket.send(Command.REVERSE_LOW_VOLUME_PWM);

  if (value > 50 && value <= 66) 
    websocket.send(Command.FORWARD_LOW_VOLUME_PWM);

  if (value > 66 && value <= 82)
    websocket.send(Command.FORWARD_MEDIUM_VOLUME_PWM);

  if (value > 82 && value <= 100)
    websocket.send(Command.FORWARD_HIGH_VOLUME_PWM);
  
}

function handleVolumeSliderMouseUpped() {
  document.getElementById("volume-slider").value = 50;
  websocket.send(Command.TURN_OFF_VOLUME_PWM);
}

function getRunningTaskTitle(taskType) {  
  switch (taskType) {
    case TaskType.NONE:
      return "No active tasks";
    case TaskType.POWER_ON:
      return "Power on";
    case TaskType.POWER_OFF:
      return "Power off";
    case TaskType.TURN_ON_SLEEP_MODE:
      return "Turn on sleep mode";
    case TaskType.TURN_OFF_SLEEP_MODE:
      return "Turn off sleep mode";
    case TaskType.TURN_OFF_VU:
      return "Turn off VU";
    }
}