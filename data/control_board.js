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

  console.log("Main :: " + mainInputRelay);
  console.log("Secondary :: " + secondaryInputRelay);

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

const VOLUME_PWM_0 = "8";
const VOLUME_PWM_33 = "9";
const VOLUME_PWM_66 = "10";
const VOLUME_PWM_100 = "11";

function handleInputSwitchClicked(element) {
  const {value, checked} = element;

  if (checked)
    websocket.send(value === "0" ? TURN_ON_MAIN_RELAY : TURN_ON_SECONDARY_RELAY);
  else
    websocket.send(value === "0" ? TURN_OFF_MAIN_RELAY : TURN_OFF_SECONDARY_RELAY);
}


function handleVolumeSliderChanged(element) {
  const {value} = element;
  if(value <= 33) 
    websocket.send(VOLUME_PWM_33);
  else if(value <= 66) 
    websocket.send(VOLUME_PWM_66);
  else 
    websocket.send(VOLUME_PWM_100);
}

function handleVolumeSliderMouseUp() {
  document.getElementById("volume-slider-input").value = 0;
  websocket.send(VOLUME_PWM_0);
}