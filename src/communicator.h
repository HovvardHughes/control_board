#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <AsyncTCP.h>
#include <fileSystemUtils.h>
#include <communicatorCommands.h>
#include <handlers.h>
#include <WiFiSettings.h>

#define TIME_TO_TRY_CONNECTING_WIFI 10000

WiFiSettings wiFiSettings;

AsyncWebServer server(80);
AsyncWebSocket webSocket("/ws");

const char *SSID_PARAM = "ssid";
const char *PASSWORD_PARAM = "password";
const char *IP_PARAM = "ip";
const char *GATEWAY_PARAM = "gateway";

IPAddress localIP;
IPAddress localGateway;
IPAddress subnet(255, 255, 0, 0);

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
  unsigned long previousMillis = currentMillis;

  while (WiFi.status() != WL_CONNECTED)
  {
    currentMillis = millis();
    if (currentMillis - previousMillis >= TIME_TO_TRY_CONNECTING_WIFI)
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
  state |= taskController.isLongTaskRunning() << 5;

  webSocket.textAll(String(state) + "|" + taskController.getRunningLongTaskType());
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
  if (type == WS_EVT_DATA)
    handleWebSocketMessage(arg, data, len);
}

void initWebSocket()
{
  webSocket.onEvent(onEvent);
  server.addHandler(&webSocket);
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
void handleWiFiSettingsChanged(AsyncWebServerRequest *request)
{
  if (taskController.isLongTaskRunning())
  {
    request->send(503, "text/plain", "Task is running. Please, try again after some time");
    return;
  }

  int params = request->params();
  for (int i = 0; i < params; i++)
  {
    AsyncWebParameter *p = request->getParam(i);
    Serial.println(p->name());
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
  wiFiSettings = WiFiSettings();
  wiFiSettings.readSettings();

  if (initWiFi())
  {
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
              { request->send(SPIFFS, "/control-board.html", "text/html", false); });
    server.on("/wi-fi-settings", HTTP_GET, [](AsyncWebServerRequest *request)
              { request->send(SPIFFS, "/wi-fi-settings.html", "text/html", false, wiFiSettingsProcessor); });
    server.on("/wi-fi-settings", HTTP_POST, handleWiFiSettingsChanged);

    initWebSocket();
  }
  else
  {
    WiFi.softAP("ESP32 access point", NULL);
    IPAddress IP = WiFi.softAPIP();
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
              { request->send(SPIFFS, "/wi-fi-settings.html", "text/html", false, wiFiSettingsProcessor); });
    server.on("/wi-fi-settings", HTTP_POST, handleWiFiSettingsChanged);
  }

  server.serveStatic("/", SPIFFS, "/");
  server.begin();
}

void tickCommunicator()
{
  webSocket.cleanupClients();
}
