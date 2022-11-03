#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <AsyncTCP.h>
#include <fileUtils.h>
#include <arduino-timer.h>
#include <communicatorCommands.h>
#include <handlers.h>
#include <WiFiSettings.h>

extern TaskController taskController;
WiFiSettings wiFiSettings = WiFiSettings();

AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

// Search for parameter in HTTP POST request
const char *SSID_PARAM = "ssid";
const char *PASSWORD_PARAM = "password";
const char *IP_PARAM = "ip";
const char *GATEWAY_PARAM = "gateway";

IPAddress localIP;
IPAddress localGateway;
IPAddress subnet(255, 255, 0, 0);

unsigned long previousMillis = 0;
const long interval = 10000;

extern PowerController powerController;

// Initialize WiFi
bool initWiFi()
{
  if (wiFiSettings.getSSID() == "" || wiFiSettings.getIP() == "")
    return false;

  WiFi.mode(WIFI_STA);
  localIP.fromString(wiFiSettings.getIP().c_str());
  localGateway.fromString(wiFiSettings.getGateway().c_str());

  if (!WiFi.config(localIP, localGateway, subnet))
    return false;

  WiFi.begin(wiFiSettings.getSSID().c_str(), wiFiSettings.getPassword().c_str());

  unsigned long currentMillis = millis();
  previousMillis = currentMillis;

  while (WiFi.status() != WL_CONNECTED)
  {
    currentMillis = millis();
    if (currentMillis - previousMillis >= interval)
      return false;
  }

  return true;
}

void textStateAll()
{
  int state = 0;

  state |= powerController.isPowerOn() << 0;
  state |= powerController.isSleepModeOn() << 1;
  state |= powerController.isVUOn() << 2;
  state |= inputSelector.readRelay(MAIN_INPUT_RELAY_PIN) << 3;
  state |= inputSelector.readRelay(SECONDARY_INPUT_RELAY_PIN) << 4;
  state |= taskController.isRunningTask() << 5;

  ws.textAll(String(state) + "|" + taskController.getRunningTaskType());
}

void handleWebSocketMessage(void *arg, uint8_t *data, size_t len)
{
  AwsFrameInfo *info = (AwsFrameInfo *)arg;
  if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT)
  {
    data[len] = 0;

    if (strcmp((char *)data, GET_STATE) == 0)
      textStateAll();

    if (strcmp((char *)data, SWITCH_POWER) == 0)
      onClickPowerButton();

    if (strcmp((char *)data, SWITCH_SLEEP_MODE) == 0)
      onDoubleClickPowerButton();

    if (strcmp((char *)data, SWITCH_VU) == 0)
      onLongPressPowerButtonStart();

    if (strcmp((char *)data, TURN_ON_MAIN_RELAY) == 0)
      onClickInputSelectorCheckbox(MAIN_INPUT_RELAY_PIN, HIGH);

    if (strcmp((char *)data, TURN_OFF_MAIN_RELAY) == 0)
      onClickInputSelectorCheckbox(MAIN_INPUT_RELAY_PIN, LOW);

    if (strcmp((char *)data, TURN_ON_SECONDARY_RELAY) == 0)
      onClickInputSelectorCheckbox(SECONDARY_INPUT_RELAY_PIN, HIGH);

    if (strcmp((char *)data, TURN_OFF_SECONDARY_RELAY) == 0)
      onClickInputSelectorCheckbox(SECONDARY_INPUT_RELAY_PIN, LOW);

    if (strcmp((char *)data, TURN_OFF_VOLUME_PWM) == 0 ||
        strcmp((char *)data, REVERSE_LOW_VOLUME_PWM) == 0 ||
        strcmp((char *)data, REVERSE_MEDIUM_VOLUME_PWM) == 0 ||
        strcmp((char *)data, REVERSE_HIGH_VOLUME_PWM) == 0 ||
        strcmp((char *)data, FORWARD_LOW_VOLUME_PWM) == 0 ||
        strcmp((char *)data, FORWARD_MEDIUM_VOLUME_PWM) == 0 ||
        strcmp((char *)data, FORWARD_HIGH_VOLUME_PWM) == 0)
      onVolumeChanged(data);
  }
}

void onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len)
{
  switch (type)
  {
  case WS_EVT_CONNECT:
    textStateAll();
    break;
  case WS_EVT_DATA:
    handleWebSocketMessage(arg, data, len);
    break;
  }
}

void initWebSocket()
{
  ws.onEvent(onEvent);
  server.addHandler(&ws);
}

String wiFiSettingsProcessor(const String &var)
{
  if (var == "ssid")
    return wiFiSettings.getSSID();

  if (var == "password")
    return wiFiSettings.getPassword();

  if (var == "ip")
    return wiFiSettings.getIP();

  if (var == "gateway")
    return wiFiSettings.getGateway();

  return String();
}
void handleWi(AsyncWebServerRequest *request)
{
  int params = request->params();
  for (int i = 0; i < params; i++)
  {
    AsyncWebParameter *p = request->getParam(i);
    if (p->isPost())
    {
      if (p->name() == SSID_PARAM)
        wiFiSettings.setSSID(p->value().c_str());
      if (p->name() == PASSWORD_PARAM)
        wiFiSettings.setPassword(p->value().c_str());
      if (p->name() == IP_PARAM)
        wiFiSettings.setIP(p->value().c_str());
      if (p->name() == GATEWAY_PARAM)
        wiFiSettings.setGateway(p->value().c_str());
    }
  }
  request->send(200, "text/plain", "Done. ESP will restart, connect to your router and go to IP address: " + wiFiSettings.getIP());
  delay(3000);
  ESP.restart();
};

void setupCommunicator()
{
  SPIFFS.begin(true);

  if (initWiFi())
  {
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
              { request->send(SPIFFS, "/control-board.html", "text/html", false); });
    server.on("/wi-fi-settings", HTTP_GET, [](AsyncWebServerRequest *request)
              { request->send(SPIFFS, "/wi-fi-settings.html", "text/html", false, wiFiSettingsProcessor); });

    initWebSocket();
  }
  else
  {
    WiFi.softAP("ESP-WIFI-MANAGER", NULL);
    IPAddress IP = WiFi.softAPIP();

    server.on("/", HTTP_POST, handleWi);
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
              { request->send(SPIFFS, "/wi-fi-settings.html", "text/html", false, wiFiSettingsProcessor); });
  }

  server.serveStatic("/", SPIFFS, "/");
  server.begin();
}

void tickCommunicator()
{
  ws.cleanupClients();
}
