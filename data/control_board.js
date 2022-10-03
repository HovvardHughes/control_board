var gateway = `ws://${window.location.hostname}/ws`;
var websocket;

var areAllControlsDisabled = true;

window.addEventListener('load', onLoad);

function initWebSocket() {
  console.log('Trying to open a WebSocket connection...');
  websocket = new WebSocket(gateway);
  websocket.onopen = onOpen;
  websocket.onclose = onClose;
  websocket.onmessage = onMessage;
}

function onOpen(event) {
  console.log('Connection opened');
}

function onClose(event) {
  console.log('Connection closed');
  setTimeout(initWebSocket, 2000);
}

const getBit = (n, bitPosition) => (n >> bitPosition) & 1

function onMessage(event) {
  var state;
  console.log(event);

  const parsedNumber = Number(event.data);
  if (isNaN(parsedNumber))
    return;

  const power = getBit(parsedNumber, 0);
  const mainInputRelay = getBit(parsedNumber, 1);
  const secondaryInputRelay = getBit(parsedNumber, 2);
  const isRunningTask = getBit(parsedNumber, 3);


  const mainRelayInput = document.getElementById('main-relay');
  const secondaryRelayInput = document.getElementById('secondary-relay');

  mainRelayInput.checked = mainInputRelay;
  secondaryRelayInput.checked = secondaryInputRelay;

  // mainRelayInput.disabled = isRunningTask;
  // secondaryRelayInput.disabled = isRunningTask;
}


function onLoad(event) {
  initWebSocket();
  initButtons();
}

function initButtons() {
  document.getElementById('power-button').addEventListener('click', switchPower);
  document.getElementById('sleep-mode-button').addEventListener('click', switchSleepMode);
  document.getElementById('power-off-vu-button').addEventListener('click', powerOffVU);
}


function switchPower() {
  websocket.send(SWITCH_POWER);
}


function switchSleepMode() {
  websocket.send(SWITCH_SLEEP_MODE);
}

function powerOffVU() {
  websocket.send(POWER_OFF_VU);
}

const GET_STATE = "0";

const SWITCH_POWER = "1";
const SWITCH_SLEEP_MODE = "2";
const POWER_OFF_VU = "3";

const TURN_ON_MAIN_RELAY = "4";
const TURN_OFF_MAIN_RELAY = "5";

const TURN_ON_SECONDARY_RELAY = "6";
const TURN_OFF_SECONDARY_RELAY = "7";

const TURN_OFF_VOLUME_PWM = "8";
const REVERSE_LOW_VOLUME_PWM = "9";
const REVERSE_HIGH_VOLUME_PWM = "10";
const FORWARD_LOW_VOLUME_PWM = "11";
const FORWARD_HIGH_VOLUME_PWM = "12";

function handleInputSwitchClicked(element) {
  const {value, checked} = element;

  if (checked)
    websocket.send(value === "0" ? TURN_ON_MAIN_RELAY : TURN_ON_SECONDARY_RELAY);
  else
    websocket.send(value === "0" ? TURN_OFF_MAIN_RELAY : TURN_OFF_SECONDARY_RELAY);
}


function handleVolumeSliderChanged(element) {
  const {value} = element;

  if(value <= 25) {
    console.log("REVERSE_HIGH_VOLUME_PWM")
    websocket.send(REVERSE_HIGH_VOLUME_PWM);
  }
  else if(value > 25 && value <= 50) {
    console.log("REVERSE_LOW_VOLUME_PWM")
    websocket.send(REVERSE_LOW_VOLUME_PWM);
  }
  else if(value > 50 && value <= 75) {
    console.log("FORWARD_LOW_VOLUME_PWM")
    websocket.send(FORWARD_LOW_VOLUME_PWM);
  }
  else if(value > 75 && value <= 100) {
    console.log("FORWARD_HIGH_VOLUME_PWM")
    websocket.send(FORWARD_HIGH_VOLUME_PWM);
  }
}

function handleVolumeSliderMouseUp() {
  document.getElementById("volume-slider-input").value = 50;
  websocket.send(TURN_OFF_VOLUME_PWM);
}