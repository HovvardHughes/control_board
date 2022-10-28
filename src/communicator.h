#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <AsyncTCP.h>
#include <fileUtils.h>
#include <arduino-timer.h>
#include <communicatorCommands.h>
#include <handlers.h>

extern TaskController taskController;

AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

// Search for parameter in HTTP POST request
const char *PARAM_INPUT_1 = "ssid";
const char *PARAM_INPUT_2 = "pass";
const char *PARAM_INPUT_3 = "ip";
const char *PARAM_INPUT_4 = "gateway";

// Variables to save values from HTML form
String ssid;
String pass;
String ip;
String gateway;

// File paths to save input values permanently
const char *ssidPath = "/ssid.txt";
const char *passPath = "/pass.txt";
const char *ipPath = "/ip.txt";
const char *gatewayPath = "/gateway.txt";

IPAddress localIP;
IPAddress localGateway;
IPAddress subnet(255, 255, 0, 0);

unsigned long previousMillis = 0;
const long interval = 10000;

extern PowerController powerController;

void initSPIFFS()
{
  if (!SPIFFS.begin(true))
  {
    Serial.println("An error has occurred while mounting SPIFFS");
  }
  Serial.println("SPIFFS mounted successfully");
}

// Initialize WiFi
bool initWiFi()
{
  if (ssid == "" || ip == "")
  {
    Serial.println("Undefined SSID or IP address.");
    return false;
  }

  WiFi.mode(WIFI_STA);
  localIP.fromString(ip.c_str());
  localGateway.fromString(gateway.c_str());

  if (!WiFi.config(localIP, localGateway, subnet))
  {
    Serial.println("STA Failed to configure");
    return false;
  }
  WiFi.begin(ssid.c_str(), pass.c_str());
  Serial.println("Connecting to WiFi...");

  unsigned long currentMillis = millis();
  previousMillis = currentMillis;

  while (WiFi.status() != WL_CONNECTED)
  {
    currentMillis = millis();
    if (currentMillis - previousMillis >= interval)
    {
      Serial.println("Failed to connect.");
      return false;
    }
  }

  Serial.println(WiFi.localIP());
  return true;
}
void textStateAll()
{
  int state = 0;

  state |= powerController.isPowerOn() << 0;
  state |= powerController.isSleepModeOn() << 1;
  state |= powerController.isPowerVUOn() << 2;
  state |= inputSelector.readRelay(MAIN_INPUT_RELAY_IO_NUMBER) << 3;
  state |= inputSelector.readRelay(SECONDARY_INPUT_RELAY_IO_NUMBER) << 4;
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

    if (strcmp((char *)data, POWER_OFF_VU) == 0)
      onLongPressPowerButtonStart();

    if (strcmp((char *)data, TURN_ON_MAIN_RELAY) == 0)
      onClickInputSelectorCheckbox(MAIN_INPUT_RELAY_IO_NUMBER, HIGH);

    if (strcmp((char *)data, TURN_OFF_MAIN_RELAY) == 0)
      onClickInputSelectorCheckbox(MAIN_INPUT_RELAY_IO_NUMBER, LOW);

    if (strcmp((char *)data, TURN_ON_SECONDARY_RELAY) == 0)
      onClickInputSelectorCheckbox(SECONDARY_INPUT_RELAY_IO_NUMBER, HIGH);

    if (strcmp((char *)data, TURN_OFF_SECONDARY_RELAY) == 0)
      onClickInputSelectorCheckbox(SECONDARY_INPUT_RELAY_IO_NUMBER, LOW);

    if (strcmp((char *)data, TURN_OFF_VOLUME_PWM) == 0 ||
        strcmp((char *)data, REVERSE_LOW_VOLUME_PWM) == 0 ||
        strcmp((char *)data, REVERSE_HIGH_VOLUME_PWM) == 0 ||
        strcmp((char *)data, FORWARD_LOW_VOLUME_PWM) == 0 ||
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
  case WS_EVT_DISCONNECT:
    break;
  case WS_EVT_DATA:
    handleWebSocketMessage(arg, data, len);
    break;
  case WS_EVT_PONG:
  case WS_EVT_ERROR:
    break;
  }
}

void initWebSocket()
{
  ws.onEvent(onEvent);
  server.addHandler(&ws);
}

void setupCommunicator()
{
  initSPIFFS();

  // Load values saved in SPIFFS
  // ssid = readFile(SPIFFS, ssidPath);
  // pass = readFile(SPIFFS, passPath);
  // ip = readFile(SPIFFS, ipPath);
  // gateway = readFile(SPIFFS, gatewayPath);

  ssid = "Begemot";
  pass = "19411945";
  ip = "192.168.1.200";
  gateway = "192.168.1.1";
  Serial.println(ssid);
  Serial.println(pass);
  Serial.println(ip);
  Serial.println(gateway);

  if (initWiFi())
  {
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
              { request->send(SPIFFS, "/control-board.html", "text/html", false); });

    server.on("/wi-fi-settings", HTTP_GET, [](AsyncWebServerRequest *request)
              { request->send(SPIFFS, "/wi-fi-settings.html", "text/html"); });

    server.serveStatic("/", SPIFFS, "/");
    initWebSocket();
    server.begin();
  }
  else
  {
    WiFi.softAP("ESP-WIFI-MANAGER", NULL);

    IPAddress IP = WiFi.softAPIP();

    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
              { request->send(SPIFFS, "/wi-fi-settings.html", "text/html"); });

    server.on("/", HTTP_POST, [](AsyncWebServerRequest *request)
              {
      int params = request->params();
      for(int i=0;i<params;i++){
        AsyncWebParameter* p = request->getParam(i);
        if(p->isPost()){
          if (p->name() == PARAM_INPUT_1) {
            ssid = p->value().c_str();
            writeFile(SPIFFS, ssidPath, ssid.c_str());
          }
          if (p->name() == PARAM_INPUT_2) {
            pass = p->value().c_str();
            writeFile(SPIFFS, passPath, pass.c_str());
          }
          if (p->name() == PARAM_INPUT_3) {
            ip = p->value().c_str();
            writeFile(SPIFFS, ipPath, ip.c_str());
          }
          if (p->name() == PARAM_INPUT_4) {
            gateway = p->value().c_str();
            writeFile(SPIFFS, gatewayPath, gateway.c_str());
          }
        }
      }
      request->send(200, "text/plain", "Done. ESP will restart, connect to your router and go to IP address: " + ip);
      delay(3000);
      ESP.restart(); });
  }
}

void tickCommunicator()
{
  ws.cleanupClients();
}
