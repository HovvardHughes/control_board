#define SSID_PATH "/ssid.txt"
#define PASSWORD_PATH "/password.txt"
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

  void readSettings()
  {
    _ssid = "Begemot";
    _password = "19411945";
    _ip = "192.168.1.200";
    _gateway = "192.168.4.1";
    // _ssid = readFile(SSID_PATH);
    // _password = readFile(PASSWORD_PATH);
    // _ip = readFile(IP_PATH);
    // _gateway = readFile(GATEWAY_PATH);
  }

  String getSSID()
  {
    return _ssid;
  }

  void setSSID(String ssid)
  {
    _ssid = ssid;
    writeFile(SSID_PATH, ssid.c_str());
  }

  String getPassword()
  {
    return _password;
  }

  void setPassword(String password)
  {
    _password = password;
    writeFile(PASSWORD_PATH, password.c_str());
  }

  String getIP()
  {
    return _ip;
  }

  void setIP(String ip)
  {
    _ip = ip;
    writeFile(IP_PATH, ip.c_str());
  }

  String getGateway()
  {
    return _gateway;
  }

  void setGateway(String gateway)
  {
    _gateway = gateway;
    writeFile(GATEWAY_PATH, gateway.c_str());
  }
};