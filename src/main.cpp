#include <ESP8266WiFi.h>

#define SERVO_PIN 0
#define LED_PIN 3
#define ENGINE_DIRECTION_PIN 2
#define ENGINE_POWER_PIN 1
#define DEBUG 0
#define WIFI_AP_MODE 1
#define CONTROLLER_CONNECTED_LED_TIME 500


#if DEBUG
#define debug(x) Serial.print(x)
#define debugln(x) Serial.println(x)
#else
#define debug(x) 
#define debugln(x) 
#endif

int port = 65432;  //Port number
WiFiServer server(port);

#if WIFI_AP_MODE
const char *ssid = "ESPESP";
const char *password = "g0T$0m3toD0?";
#else
const char *ssid = "";  //wifi SSID
const char *password = "";  //wifi Password
IPAddress staticIP(192, 168, 1, 110); //ESP static ip
IPAddress gateway(192, 168, 1, 1);   //IP Address of WiFi Router (Gateway)
IPAddress subnet(255, 255, 255, 0);  //Subnet mask
#endif

String setts = "";

struct Pole
{
  unsigned char boatDirection : 5;
  unsigned char motorDriveDirection : 1;
  unsigned char motorDriveSpeed : 1;
  unsigned char feederState : 1;
};

union Unia
{
    unsigned char hexVal;
    Pole vars;
};

Unia vehicleSettings;

int msgTime = 0;
int msgInterval = 5000;

//=======================================================================
//                    Power on setup
//=======================================================================
void setup() 
{
  pinMode(ENGINE_DIRECTION_PIN, OUTPUT);
  digitalWrite(ENGINE_DIRECTION_PIN, HIGH);
  
  pinMode(ENGINE_POWER_PIN, FUNCTION_3);
  pinMode(ENGINE_POWER_PIN, OUTPUT);
  digitalWrite(ENGINE_POWER_PIN, LOW);
  
  pinMode(LED_PIN, FUNCTION_3);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, HIGH);

  pinMode(SERVO_PIN, OUTPUT);
  analogWriteFreq(50);
  analogWrite(SERVO_PIN, 19); //center pos

  #if DEBUG
    Serial.begin(115200);
  #endif

  WiFi.setOutputPower(20.5);

  #if WIFI_AP_MODE
    WiFi.mode(WIFI_AP);
    WiFi.softAP(ssid, password);
  #else
    WiFi.config(staticIP, gateway, subnet);
    WiFi.mode(WIFI_STA);
    //Connect to wifi
    WiFi.begin(ssid, password);

    // Wait for connection  
    debugln("Connecting to Wifi");
    while (WiFi.status() != WL_CONNECTED)
    {   
      delay(500);
      debug(".");
      delay(500);
    }
    debugln("");
    debug("Connected to ");
    debugln(ssid);
  #endif

  server.begin();

  debug("IP: ");
  debug(WiFi.localIP());
  debug(" port: ");
  debugln(port);
}
//=======================================================================
//                    Loop
//=======================================================================

void loop() 
{
  WiFiClient client = server.available();
  
  if (client)
  {
    if(client.connected())
    {
      debugln("Client Connected");
      digitalWrite(LED_PIN, LOW);
      delay(CONTROLLER_CONNECTED_LED_TIME);
      digitalWrite(LED_PIN, HIGH);
      msgTime = millis();
    }
    
    while(client.connected())
    { 
      if(client.available())
      {
        // read data from the connected client
        setts = client.readStringUntil('h');
        debug(setts);

        vehicleSettings.hexVal = strtoul(setts.c_str(), NULL, 16);
        
        //digitalWrite(LED_PIN, !(vehicleSettings.vars.lightsState));
        analogWrite(SERVO_PIN, vehicleSettings.vars.boatDirection);
        digitalWrite(ENGINE_DIRECTION_PIN, vehicleSettings.vars.motorDriveDirection);
        digitalWrite(ENGINE_POWER_PIN, vehicleSettings.vars.motorDriveSpeed);
          
        msgTime = millis();
      }
        
      if(millis() >= msgTime + msgInterval)
      {
        digitalWrite(LED_PIN, 1);
        analogWrite(SERVO_PIN, 19);
        digitalWrite(ENGINE_DIRECTION_PIN, 1);
        digitalWrite(ENGINE_POWER_PIN, 0);
        
        digitalWrite(LED_PIN, LOW);
        delay(CONTROLLER_CONNECTED_LED_TIME/2);
        digitalWrite(LED_PIN, HIGH);
        delay(CONTROLLER_CONNECTED_LED_TIME/2);
        digitalWrite(LED_PIN, LOW);
        delay(CONTROLLER_CONNECTED_LED_TIME/2);
        digitalWrite(LED_PIN, HIGH);

        client.stop();
      }
    }
    client.stop();
    debugln("Client disconnected");    
  }
}
