#include <ESP8266WiFi.h>
#include <stdint.h>

#define SERVO_PIN 0
#define LED_PIN 3
#define MOTOR_DRIVE_DIRECTION_PIN 2
#define MOTOR_POWER_PIN 1
#define DEBUG 0
#define WIFI_AP_MODE 1
#define CONTROLLER_CONNECTED_LED_TIME 500

#define BOAT_DIRECTION_STRAIGHT 19
#define BOAT_DIRECTION_LEFT 29
#define BOAT_DIRECTION_RIGHT 9
#define MOTOR_DRIVE_DIRECTION_FORWARD 1
#define MOTOR_DRIVE_DIRECTION_BACKWARD 0
#define MOTOR_RUN 1
#define MOTOR_STOP 0
#define FEEDER_OPEN 1
#define FEEDER_CLOSED 0
#define LIGHTS_ON 1
#define LIGHTS_OFF 0
#define GPS_ON 1
#define GPS_OFF 0

#define MESSAGE_INTERVAL 5000UL

#if DEBUG
#define debug(x) Serial.print(x)
#define debugln(x) Serial.println(x)
#else
#define debug(x) 
#define debugln(x) 
#endif

uint16_t port = 65432;  //Port number
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

union ControlRegister
{
    uint8_t hexValue;
    struct
    {
      uint8_t boatDirection : 5;
      uint8_t motorDriveDirection : 1;
      uint8_t motorDriveSpeed : 1;
      uint8_t feederState : 1;
    };
};

ControlRegister vehicleSettings;

uint32_t msgTime = 0;

//=======================================================================
//                    Power on setup
//=======================================================================
void setup() 
{
  pinMode(MOTOR_DRIVE_DIRECTION_PIN, OUTPUT);
  digitalWrite(MOTOR_DRIVE_DIRECTION_PIN, MOTOR_DRIVE_DIRECTION_FORWARD);
  
  pinMode(MOTOR_POWER_PIN, FUNCTION_3);
  pinMode(MOTOR_POWER_PIN, OUTPUT);
  digitalWrite(MOTOR_POWER_PIN, MOTOR_STOP);
  
  pinMode(LED_PIN, FUNCTION_3);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, HIGH);

  pinMode(SERVO_PIN, OUTPUT);
  analogWriteFreq(50);
  analogWrite(SERVO_PIN, BOAT_DIRECTION_STRAIGHT);

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
  WiFiClient client = server.accept();
  
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

        vehicleSettings.hexValue = strtoul(setts.c_str(), NULL, 16);
        
        analogWrite(SERVO_PIN, vehicleSettings.boatDirection);
        digitalWrite(MOTOR_DRIVE_DIRECTION_PIN, vehicleSettings.motorDriveDirection);
        digitalWrite(MOTOR_POWER_PIN, vehicleSettings.motorDriveSpeed);
          
        msgTime = millis();
      }
        
      if(millis() >= ((unsigned long)msgTime + MESSAGE_INTERVAL))
      {
        digitalWrite(LED_PIN, 1);
        analogWrite(SERVO_PIN, BOAT_DIRECTION_STRAIGHT);
        digitalWrite(MOTOR_DRIVE_DIRECTION_PIN, MOTOR_DRIVE_DIRECTION_FORWARD);
        digitalWrite(MOTOR_POWER_PIN, MOTOR_STOP);
        
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
