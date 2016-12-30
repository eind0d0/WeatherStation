#define SSID "Your SSID"
#define PASSWORD "Your Key"

#define CITY "Your City"

#define DEBUG true

#define LED_WLAN 13

#include <SoftwareSerial.h>

int LED_GREEN=5;
int LED_BLUE=3;
int LED_RED=7;
int GND=4;

SoftwareSerial esp8266(11, 12); // RX, TX


void setup() {
  Serial.begin(19200);
  esp8266.begin(19200);

  if (!espConfig()) serialDebug();
  else digitalWrite(LED_WLAN, HIGH);

  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_BLUE, OUTPUT);
  pinMode(LED_RED, OUTPUT);
  pinMode(GND, OUTPUT);
  digitalWrite(GND, LOW);

}

void loop() {
  int temp  = getWeatherTemp(CITY);
  digitalWrite(LED_BLUE, LOW);
  digitalWrite(LED_GREEN, LOW);
  digitalWrite(LED_RED, LOW);
  rgbTemp(temp);
  delay(1800000);  
}

void rgbTemp(int val)
{
  if (val <= -10 & val >= -20)
    digitalWrite(LED_BLUE, HIGH);
  else
    digitalWrite(LED_BLUE, LOW);

  if (val >= -10 & val <=0)
    digitalWrite(LED_GREEN, HIGH);
  else
    digitalWrite(LED_GREEN, LOW);

  if (val >= 0 & val <=10)
    digitalWrite(LED_RED, HIGH);
  else
    digitalWrite(LED_RED, LOW);
}

float getWeatherTemp(String city)
{
  float temp;
  int humidity, clouds;
  String  Host = "temp.fkainka.de";
  String  Subpage = "/?city="+city;

  if (sendCom("AT+CIPSTART=\"TCP\",\"" + Host + "\",80", "OK"))
  {
    String getRequest = "GET " + Subpage + " HTTP/1.1\r\nHost:" + Host + "\r\n";
    if (sendCom("AT+CIPSEND=" + String(getRequest.length() + 2), ">"))
    {
      esp8266.println(getRequest);
      if (esp8266.find("+IPD"))
      {
        if (esp8266.find("\r\n\r\n"))
        {
          if (esp8266.find("Temp:"))
          {
            int vTemp = esp8266.parseInt();
            //String vTemp = esp8266.readStringUntil('\"');
            debug("Temp: " + String(vTemp) + "C");
            temp = vTemp;//.toFloat();
          }
          sendCom("AT+CIPCLOSE", "OK");
          return temp;
          Serial.println(temp);
        }
      }
    }

  }
}

String getTCP(String Host, String Subpage)
{
  boolean succes = true;

  succes &= sendCom("AT+CIPSTART=\"TCP\",\"" + Host + "\",80", "OK");
  String getRequest = "GET " + Subpage + " HTTP/1.1\r\nHost:" + Host + "\r\n\r\n";
  succes &= sendCom("AT+CIPSEND=" + String(getRequest.length() + 2), ">");

  return sendCom(getRequest);
}

//-----------------------------------------Config ESP8266------------------------------------

boolean espConfig()
{
  boolean succes = true;
  esp8266.setTimeout(12000);
  succes &= sendCom("AT+RST", "ready");
  esp8266.setTimeout(1000);
  if (configStation(SSID, PASSWORD)) {
    succes &= true;
    debug("WLAN Connected");
    debug("My IP is:");
    debug(sendCom("AT+CIFSR"));
  }
  else
  {
    succes &= false;
  }
  //shorter Timeout for faster wrong UPD-Comands handling
  succes &= sendCom("AT+CIPMODE=0", "OK");  //So rum scheit wichtig!
  succes &= sendCom("AT+CIPMUX=0", "OK");

  return succes;
}

boolean configTCPServer()
{
  boolean succes = true;

  succes &= (sendCom("AT+CIPMUX=1", "OK"));
  succes &= (sendCom("AT+CIPSERVER=1,80", "OK"));

  return succes;

}

boolean configTCPClient()
{
  boolean succes = true;

  succes &= (sendCom("AT+CIPMUX=0", "OK"));
  //succes &= (sendCom("AT+CIPSERVER=1,80", "OK"));

  return succes;

}


boolean configStation(String vSSID, String vPASSWORT)
{
  boolean succes = true;
  succes &= (sendCom("AT+CWMODE=1", "OK"));
  esp8266.setTimeout(20000);
  succes &= (sendCom("AT+CWJAP=\"" + String(vSSID) + "\",\"" + String(vPASSWORT) + "\"", "OK"));
  esp8266.setTimeout(1000);
  return succes;
}

boolean configAP()
{
  boolean succes = true;

  succes &= (sendCom("AT+CWMODE=2", "OK"));
  succes &= (sendCom("AT+CWSAP=\"NanoESP\",\"\",5,0", "OK"));

  return succes;
}

boolean configUDP()
{
  boolean succes = true;

  succes &= (sendCom("AT+CIPMODE=0", "OK"));
  succes &= (sendCom("AT+CIPMUX=0", "OK"));
  succes &= sendCom("AT+CIPSTART=\"UDP\",\"192.168.255.255\",90,91,2", "OK"); //Importand Boradcast...Reconnect IP
  return succes;
}

//-----------------------------------------------Controll ESP-----------------------------------------------------

boolean sendUDP(String Msg)
{
  boolean succes = true;

  succes &= sendCom("AT+CIPSEND=" + String(Msg.length() + 2), ">");    //+",\"192.168.4.2\",90", ">");
  if (succes)
  {
    succes &= sendCom(Msg, "OK");
  }
  return succes;
}


boolean sendCom(String command, char respond[])
{
  esp8266.println(command);
  if (esp8266.findUntil(respond, "ERROR"))
  {
    return true;
  }
  else
  {
    debug("ESP SEND ERROR: " + command);
    return false;
  }
}

String sendCom(String command)
{
  esp8266.println(command);
  return esp8266.readString();
}



//-------------------------------------------------Debug Functions------------------------------------------------------
void serialDebug() {
  while (true)
  {
    if (esp8266.available())
      Serial.write(esp8266.read());
    if (Serial.available())
      esp8266.write(Serial.read());
  }
}

void debug(String Msg)
{
  if (DEBUG)
  {
    Serial.println(Msg);
  }
}
