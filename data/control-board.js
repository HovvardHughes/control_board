const NON_BREAKING_SPACE = '\xa0'
const MAX_NORMAL_TEMPERATURE_DEGREES_CELSIUS = 65.0
const MAX_NORMAL_CURRENT_MA = 85.0

const Commands = {
    GET_STATE: 0,
    SWITCH_POWER: 1,
    SWITCH_SLEEP_MODE: 2,
    SWITCH_VU: 3,
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
    FORWARD_HIGH_VOLUME_PWM: 14
}

const DisplayMessageTypes = {
    EMPTY: 0,
    POWER_ON: 1,
    POWER_OFF: 2,
    TURN_ON_SLEEP_MODE: 3,
    TURN_OFF_SLEEP_MODE: 4,
    TURN_ON_VU: 5,
    TURN_OFF_VU: 6,
    EMERGENCY_POWER_OFF_BECAUSE_OF_CURRENTS: 7,
    EMERGENCY_POWER_OFF_BECAUSE_OF_TEMPERATURE: 8
}

const DisplayTitles = {
    [DisplayMessageTypes.EMPTY]: NON_BREAKING_SPACE,
    [DisplayMessageTypes.POWER_ON]: 'Turn on power',
    [DisplayMessageTypes.POWER_OFF]: 'Turn off power',
    [DisplayMessageTypes.TURN_ON_SLEEP_MODE]: 'Turn on sleep mode',
    [DisplayMessageTypes.TURN_OFF_SLEEP_MODE]: 'Turn off sleep mode',
    [DisplayMessageTypes.TURN_ON_VU]: 'Turn on VU',
    [DisplayMessageTypes.TURN_OFF_VU]: 'Turn off VU',
    [DisplayMessageTypes.EMERGENCY_POWER_OFF_BECAUSE_OF_CURRENTS]: 'Emergency power off because of high currents',
    [DisplayMessageTypes.EMERGENCY_POWER_OFF_BECAUSE_OF_TEMPERATURE]: 'Emergency power off because of high temperature'
}

const IDs = {
    POWER_BUTTON: 'power-button',
    SLEEP_MODE_BUTTON: 'sleep-mode-button',
    VU_BUTTON: 'vu-button',
    VOLUME_SLIDER_CONTAINER: 'volume-slider-container',
    VOLUME_SLIDER: 'volume-slider',
    INPUT_SELECTOR: 'input-selector',
    MAIN_INPUT: 'main-relay',
    SECONDARY_INPUT: 'secondary-relay',
    DISPLAY_MESSAGE_CONTAINER: 'message-container',
    DISPLAY_MESSAGE: 'message',
    TEMPERATURE_CONTAINER: 'temperature-container',
    TEMPERATURE: 'temperature',
    VOLTAGE: 'voltage',
    CURRENTS: 'currents'
}

const Elements = Object.fromEntries(Object.values(IDs).map((id) => [id, document.getElementById(id)]))

const Dots = document.getElementsByClassName('blinking-dot')

let websocket
let webSocketErrorCount = 0

let disableAllControlsId

let ignoreOnceInputSelectorSwitchUpdateWhenMessageRecieved

window.addEventListener('load', handleLoad)

function handleLoad() {
    deleteHoverEffectsOnMobile()
    initWebSocket()
    initControlHandlers()
    displayMessage(NON_BREAKING_SPACE, false, false)
    disableAllControlsId = setTimeout(disableAllControls, 1000)
}

function initWebSocket() {
    websocket = new WebSocket(`ws://${window.location.hostname}/ws`)
    websocket.onopen = handleOpen
    websocket.onclose = handleClose
    websocket.onerror = handleError
    websocket.onmessage = handleMessage
}

function handleOpen() {
    webSocketErrorCount = 0
    clearInterval(disableAllControlsId)
}

function handleClose() {
    setTimeout(initWebSocket, 2000)
}

function handleError() {
    webSocketErrorCount++
    if (webSocketErrorCount > 5) {
        displayMessage('Connection is not established', true, false)
        disableAllControls()
    }
}

function handleMessage(event) {
    const getBit = (number, bitPosition) => (number >> bitPosition) & 1

    const splitData = event.data.split('|')

    const state = parseInt(splitData[0])
    const displayMessageType = parseInt(splitData[1])
    const temperature = parseFloat(splitData[2])
    const voltage = parseFloat(splitData[3])
    const currents = splitData[4].split(':').map(parseFloat)

    const isPowerOn = getBit(state, 0)
    const isSleepModeOn = getBit(state, 1)
    const isVUOn = getBit(state, 2)
    const mainInputRelay = getBit(state, 3)
    const secondaryInputRelay = getBit(state, 4)
    const isRunningTask = getBit(state, 5)

    displayMessage(
        DisplayTitles[displayMessageType],
        displayMessageType === DisplayMessageTypes.EMERGENCY_POWER_OFF_BECAUSE_OF_TEMPERATURE ||
            displayMessageType === DisplayMessageTypes.EMERGENCY_POWER_OFF_BECAUSE_OF_CURRENTS,
        isRunningTask
    )

    addOrRemoveClass(
        Elements[IDs.POWER_BUTTON],
        'progress',
        displayMessageType === DisplayMessageTypes.POWER_ON || displayMessageType === DisplayMessageTypes.POWER_OFF
    )
    addOrRemoveClass(
        Elements[IDs.SLEEP_MODE_BUTTON],
        'progress',
        displayMessageType === DisplayMessageTypes.TURN_ON_SLEEP_MODE ||
            displayMessageType === DisplayMessageTypes.TURN_OFF_SLEEP_MODE
    )
    addOrRemoveClass(Elements[IDs.VU_BUTTON], 'progress', displayMessageType === DisplayMessageTypes.TURN_OFF_VU)

    addOrRemoveClass(Elements[IDs.POWER_BUTTON], 'enabled-power-button', isPowerOn)
    addOrRemoveClass(Elements[IDs.SLEEP_MODE_BUTTON], 'enabled-sleep-mode-button', isSleepModeOn)
    addOrRemoveClass(Elements[IDs.VU_BUTTON], 'enabled-vu-button', isVUOn)

    if (!ignoreOnceInputSelectorSwitchUpdateWhenMessageRecieved) {
        setAtrribute('checked', {
            [IDs.MAIN_INPUT]: mainInputRelay,
            [IDs.SECONDARY_INPUT]: secondaryInputRelay
        })
    }
    ignoreOnceInputSelectorSwitchUpdateWhenMessageRecieved = false

    const isRunningTaskOrPowerTurnedOff = isRunningTask || !isPowerOn

    setAtrribute('disabled', {
        [IDs.POWER_BUTTON]: isRunningTask || isSleepModeOn,
        [IDs.SLEEP_MODE_BUTTON]: isRunningTaskOrPowerTurnedOff,
        [IDs.VU_BUTTON]: isRunningTask,
        [IDs.MAIN_INPUT]: isRunningTaskOrPowerTurnedOff,
        [IDs.SECONDARY_INPUT]: isRunningTaskOrPowerTurnedOff,
        [IDs.VOLUME_SLIDER]: isRunningTaskOrPowerTurnedOff
    })

    addOrRemoveClass(Elements[IDs.INPUT_SELECTOR], 'disabled', isRunningTaskOrPowerTurnedOff)
    addOrRemoveClass(Elements[IDs.VOLUME_SLIDER_CONTAINER], 'disabled', isRunningTaskOrPowerTurnedOff)

    Elements[IDs.TEMPERATURE].innerText = temperature
    addOrRemoveClass(Elements[IDs.TEMPERATURE_CONTAINER], 'error', temperature > MAX_NORMAL_TEMPERATURE_DEGREES_CELSIUS)

    Elements[IDs.VOLTAGE].innerText = voltage

    const currentValues = Elements[IDs.CURRENTS].querySelectorAll('* > .current-value')
    const currentUnits = Elements[IDs.CURRENTS].querySelectorAll('* > .measurement-units')
    currents.forEach((current, currentIndex) => {
        const value = currentValues[currentIndex]
        const units = currentUnits[currentIndex]

        value.innerHTML = current

        const addErrorClass = current > MAX_NORMAL_CURRENT_MA
        addOrRemoveClass(value, 'error', addErrorClass)
        addOrRemoveClass(units, 'error', addErrorClass)
    })
}

function addOrRemoveClass(element, className, addClass) {
    if (addClass) element.classList.add(className)
    else element.classList.remove(className)
}

function setAtrribute(attributeName, idsWithAttributeValue) {
    Object.entries(idsWithAttributeValue).forEach(
        ([id, attributeValue]) => (Elements[id][attributeName] = attributeValue)
    )
}

function initControlHandlers() {
    Elements[IDs.POWER_BUTTON].addEventListener('click', switchPower)
    Elements[IDs.SLEEP_MODE_BUTTON].addEventListener('click', switchSleepMode)
    Elements[IDs.VU_BUTTON].addEventListener('click', switchVU)
    Elements[IDs.MAIN_INPUT].addEventListener('click', handleInputSwitchClicked)
    Elements[IDs.SECONDARY_INPUT].addEventListener('click', handleInputSwitchClicked)

    const volumeSlider = Elements[IDs.VOLUME_SLIDER]
    volumeSlider.addEventListener('change', handleVolumeSliderChanged)
    volumeSlider.addEventListener('input', handleVolumeSliderInput)
    volumeSlider.addEventListener('keydown', handleVolumeSliderKeyDown)
}

function switchPower() {
    websocket.send(Commands.SWITCH_POWER)
}

function switchSleepMode() {
    websocket.send(Commands.SWITCH_SLEEP_MODE)
}

function switchVU() {
    websocket.send(Commands.SWITCH_VU)
}

function handleInputSwitchClicked(event) {
    const { value, checked } = event.target
    if (checked) websocket.send(value === '0' ? Commands.TURN_ON_MAIN_RELAY : Commands.TURN_ON_SECONDARY_RELAY)
    else websocket.send(value === '0' ? Commands.TURN_OFF_MAIN_RELAY : Commands.TURN_OFF_SECONDARY_RELAY)
    ignoreOnceInputSelectorSwitchUpdateWhenMessageRecieved = true
}

function handleVolumeSliderInput(event) {
    const value = parseInt(event.target.value)

    if (value === 99) websocket.send(Commands.FORWARD_HIGH_VOLUME_PWM)

    if (value === 83) websocket.send(Commands.FORWARD_MEDIUM_VOLUME_PWM)

    if (value === 66) websocket.send(Commands.FORWARD_LOW_VOLUME_PWM)

    if (value === 50) websocket.send(Commands.TURN_OFF_VOLUME_PWM)

    if (value === 33) websocket.send(Commands.REVERSE_LOW_VOLUME_PWM)

    if (value === 16) websocket.send(Commands.REVERSE_MEDIUM_VOLUME_PWM)

    if (value === 0) websocket.send(Commands.REVERSE_HIGH_VOLUME_PWM)
}

function handleVolumeSliderChanged() {
    Elements[IDs.VOLUME_SLIDER].value = 50
    websocket.send(Commands.TURN_OFF_VOLUME_PWM)
}

function handleVolumeSliderKeyDown(event) {
    event.preventDefault()
}

function displayMessage(message, error, ellipsisAnimation) {
    Elements[IDs.DISPLAY_MESSAGE].innerText = message
    addOrRemoveClass(Elements[IDs.DISPLAY_MESSAGE_CONTAINER], 'error', error)
    for (let dot of Dots) dot.style.display = ellipsisAnimation ? 'inline' : 'none'
}

function disableAllControls() {
    setAtrribute('disabled', Object.fromEntries(Object.values(IDs).map((id) => [id, true])))
    addOrRemoveClass(Elements[IDs.INPUT_SELECTOR], 'disabled', true)
    addOrRemoveClass(Elements[IDs.VOLUME_SLIDER_CONTAINER], 'disabled', true)
}

function deleteHoverEffectsOnMobile() {
    const hasTouch =
        'ontouchstart' in document.documentElement || navigator.maxTouchPoints > 0 || navigator.msMaxTouchPoints > 0

    if (hasTouch)
        try {
            for (let si in document.styleSheets) {
                let styleSheet = document.styleSheets[si]

                if (!styleSheet.cssRules) continue

                for (let ri = styleSheet.cssRules.length - 1; ri >= 0; ri--) {
                    if (!styleSheet.cssRules[ri].selectorText) continue

                    if (styleSheet.cssRules[ri].selectorText.match(':hover')) {
                        styleSheet.deleteRule(ri)
                    }
                }
            }
        } catch (ignored) {}
}
