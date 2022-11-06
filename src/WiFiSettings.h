#define SSID_PATH  "/ssid.txt"
#define PASSWORD_PATH  "/password.txt"
#define IP_PATH "/ip.txt"
#define GATEWAY_PATH "/gateway.txt"

class WiFiSettings
{
private:
  String _ssid;
  String _password;
  String _ip;
  String _gateway;

public:
  WiFiSettings()
  {
    // _ssid = readFile(SPIFFS, SSID_PATH);
    // _password = readFile(SPIFFS, PASSWORD_PATH);
    // _ip = readFile(SPIFFS, IP_PATH);
    // _gateway = readFile(SPIFFS, GATEWAY_PATH);

    _ssid = "Begemot";
    _password = "19411945";
    _ip = "192.168.1.200";
    _gateway = "192.168.4.1";
  }

  String getSSID()
  {
    return _ssid;
  }

  void setSSID(String ssid)
  {
    _ssid = ssid;
    writeFile(SPIFFS, SSID_PATH, ssid.c_str());
  }

  String getPassword()
  {
    return _password;
  }

  void setPassword(String password)
  {
    _password = password;
    writeFile(SPIFFS, PASSWORD_PATH, password.c_str());
  }

  String getIP()
  {
    return _ip;
  }

  void setIP(String ip)
  {
    _ip = ip;
    writeFile(SPIFFS, IP_PATH, ip.c_str());
  }

  String getGateway()
  {
    return _gateway;
  }

  void setGateway(String gateway)
  {
    _gateway = gateway;
    writeFile(SPIFFS, GATEWAY_PATH, gateway.c_str());
  }
};