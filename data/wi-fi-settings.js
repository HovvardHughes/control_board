const IDs = {
    SAVE_BUTTON: "save-button",
    SSID_INPUT: "ssid",
    SSID_INPUT_HELPER_TEXT: "ssid-helper-text",
    PASSWORD_INPUT: "password",
    PASSWORD_INPUT_HELPER_TEXT: "password-helper-text",
    IP_INPUT: "ip",
    IP_INPUT_HELPER_TEXT: "ip-helper-text",
    GATEWAY_INPUT: "gateway",
    GATEWAY_INPUT_HELPER_TEXT: "gateway-helper-text",
    SAVE_RESULT: "save-result"
}

const InputHelperTexts = {
    [IDs.SSID_INPUT] : IDs.SSID_INPUT_HELPER_TEXT,
    [IDs.PASSWORD_INPUT] : IDs.PASSWORD_INPUT_HELPER_TEXT,
    [IDs.IP_INPUT] : IDs.IP_INPUT_HELPER_TEXT,
    [IDs.GATEWAY_INPUT] : IDs.GATEWAY_INPUT_HELPER_TEXT,
}

const InputTitles = {
    [IDs.SSID_INPUT] : "SSID",
    [IDs.PASSWORD_INPUT] : "Password",
    [IDs.IP_INPUT] : "IP address",
    [IDs.GATEWAY_INPUT] : "Gateway address",
}


window.addEventListener('load', onLoad);

function onLoad(event) {
    document.getElementById(IDs.SAVE_BUTTON).addEventListener('click', handleSaveButtonClicked);
    document.getElementById(IDs.IP_INPUT).addEventListener('input', handleInputWithIpChanged);
    document.getElementById(IDs.GATEWAY_INPUT).addEventListener('input', handleInputWithIpChanged);
}

function handleInputWithIpChanged(event) {
    const {id, value} = event.target;
    validateInputWithIp(event.target);
}

function validateInputWithIp(input) {
    const {id: inputId, value: inputValue} = input;
    const inputPlaceholder = input.placeholder;

    const inputHelperText = document.getElementById(InputHelperTexts[inputId]);
    const inputTitle = InputTitles[inputId]; 

    let isError = false;

    if(!inputValue) {
        inputHelperText.innerText =`${inputTitle} cannot be empty`;
        isError = true;
    } else if(!validateIp(inputValue)) {
        inputHelperText.innerText = `${inputTitle} should match pattern ${inputPlaceholder}`;
        isError = true;
    }

    if(isError) {
        input.classList.add("error");
        inputHelperText.classList.add("error");
    } else {
        input.classList.remove("error");
        inputHelperText.innerHTML = "&nbsp";
        inputHelperText.classList.remove("error");
    }

    document.getElementById(IDs.SAVE_BUTTON).disabled = isError;
}

function validateIp(ip) 
{
 return /^(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\.(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\.(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\.(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)$/.test(ip);
}

async function handleSaveButtonClicked() {
    const ipElement =  document.getElementById(IDs.IP_INPUT);
    const gatewayElement =  document.getElementById(IDs.GATEWAY_INPUT);
    
    if(!validateInputWithIp(ipElement) || !validateInputWithIp(gatewayElement))
        return;
    
    const ssid = document.getElementById(IDs.SSID_INPUT).value;
    const password =  document.getElementById(IDs.PASSWORD_INPUT).value;
    const ip = ipElement.value;
    const gateway =  gatewayElement.value;

    let saveSettingsResult =  {
        text: "",
        isError: false
    }

    try {     
        const response = await fetch('/wi-fi-settings', {
          method: 'post',
          body: {
            ssid,
            password,
            ip,
            gateway
          }
        });

        saveSettingsResult = {
            text: await response.text(),
            isError: response.status !== 200
        };

        console.log(response);
      } catch(err) {
        saveSettingsResult = {
            text: "Something's gone wrong. Please, reload page and send request again.",
            isError: true
        };
      }

      const saveResult = document.getElementById(IDs.SAVE_RESULT);
      saveResult.innerText = saveSettingsResult.text;
      if(saveSettingsResult.isError){
        saveResult.classList.add("error");
        saveResult.classList.remove("success");
    }
      else {
        saveResult.classList.add("success");
        saveResult.classList.remove("error");
    }
  }

