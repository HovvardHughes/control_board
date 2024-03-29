const IDs = {
    SSID_INPUT_CONTAINER: 'ssid',
    PASSWORD_INPUT_CONTAINER: 'password',
    IP_INPUT_CONTAINER: 'ip',
    GATEWAY_INPUT_CONTAINER: 'gateway',
    SAVE_BUTTON: 'save-button',
    SAVE_RESULT: 'save-result'
}

const InputContainers = {
    [IDs.SSID_INPUT_CONTAINER]: document.getElementById(IDs.SSID_INPUT_CONTAINER),
    [IDs.PASSWORD_INPUT_CONTAINER]: document.getElementById(IDs.PASSWORD_INPUT_CONTAINER),
    [IDs.IP_INPUT_CONTAINER]: document.getElementById(IDs.IP_INPUT_CONTAINER),
    [IDs.GATEWAY_INPUT_CONTAINER]: document.getElementById(IDs.GATEWAY_INPUT_CONTAINER)
}

const Inputs = Object.fromEntries(
    Object.entries(InputContainers).map(([id, element]) => [id, element.querySelector('input')])
)

const InputHelperTexts = Object.fromEntries(
    Object.entries(InputContainers).map(([id, element]) => [id, element.querySelector('.helper-text')])
)

const InputTitles = {
    [IDs.SSID_INPUT_CONTAINER]: 'SSID',
    [IDs.PASSWORD_INPUT_CONTAINER]: 'Password',
    [IDs.IP_INPUT_CONTAINER]: 'IP address',
    [IDs.GATEWAY_INPUT_CONTAINER]: 'Gateway address'
}

const SaveButton = document.getElementById(IDs.SAVE_BUTTON)
const SaveResult = document.getElementById(IDs.SAVE_RESULT)

window.addEventListener('load', onLoad)

function onLoad() {
    deleteHoverEffectsOnMobile()

    SaveButton.addEventListener('click', handleSaveButtonClicked)

    const validateInput = (event) => validateInputWithIp(event.target.name, event.target.value)

    const ipInput = Inputs[IDs.IP_INPUT_CONTAINER]
    ipInput.addEventListener('input', validateInput)
    ipInput.addEventListener('blur', validateInput)

    const gatewayInput = Inputs[IDs.GATEWAY_INPUT_CONTAINER]
    gatewayInput.addEventListener('input', validateInput)
    gatewayInput.addEventListener('blur', validateInput)
}

function validateInputWithIp(inputContainerId, inputValue) {
    const inputHelperText = InputHelperTexts[inputContainerId]
    const inputTitle = InputTitles[inputContainerId]

    let isValid = true

    if (!inputValue) {
        inputHelperText.innerText = `${inputTitle} cannot be empty`
        isValid = false
    } else if (!isIpValid(inputValue)) {
        const input = Inputs[inputContainerId]
        inputHelperText.innerText = `${inputTitle} should match pattern ${input.placeholder}`
        isValid = false
    }

    const inputContainer = InputContainers[inputContainerId]
    if (!isValid) inputContainer.classList.add('error')
    else {
        inputContainer.classList.remove('error')
        inputHelperText.innerHTML = '&nbsp'
    }

    SaveButton.disabled = !isValid

    return isValid
}

function isIpValid(ip) {
    return /^(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\.(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\.(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\.(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)$/.test(
        ip
    )
}

async function handleSaveButtonClicked() {
    const validationResults = [
        validateInputWithIp(IDs.IP_INPUT_CONTAINER, Inputs[IDs.IP_INPUT_CONTAINER].value),
        validateInputWithIp(IDs.GATEWAY_INPUT_CONTAINER, Inputs[IDs.GATEWAY_INPUT_CONTAINER].value)
    ]

    if (validationResults.includes(false)) return

    let saveResult = {
        text: '',
        isError: false
    }

    const generalErrorMessage = "Something's gone wrong. Please, try again."

    try {
        const controller = new AbortController()
        const id = setTimeout(() => controller.abort(), 10000)

        const formData = new FormData()
        formData.append("ssid", Inputs[IDs.SSID_INPUT_CONTAINER].value)
        formData.append("password", Inputs[IDs.PASSWORD_INPUT_CONTAINER].value)
        formData.append("ip", Inputs[IDs.IP_INPUT_CONTAINER].value)
        formData.append("gateway", Inputs[IDs.GATEWAY_INPUT_CONTAINER].value)

        const response = await fetch('/wi-fi-settings', {
            method: 'post',
            body: formData,
            signal: controller.signal
        })

        clearTimeout(id)

        saveResult = {
            text: (await response.text()) || generalErrorMessage,
            isError: response.status !== 200
        }
    } catch (err) {
        saveResult = {
            text: generalErrorMessage,
            isError: true
        }
    }

    SaveResult.innerText = saveResult.text
    if (saveResult.isError) {
        SaveResult.classList.add('error')
        SaveResult.classList.remove('success')
    } else {
        SaveResult.classList.add('success')
        SaveResult.classList.remove('error')
    }
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
