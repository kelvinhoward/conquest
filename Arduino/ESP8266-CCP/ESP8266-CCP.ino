/*
LaserTag Conquest Control Point (CCP)
Players attempt to control CCP's on the course  
Once a CCP has been taken it blinks, changes to the teams color and 
sends a message to the MMQT Broker
*/
//+=============================================================================
// Define Pins for RGB LED
//

#define BLUE 14
#define GREEN 12
#define RED 13

//+=============================================================================
//Setup IR Sensor https://github.com/z3t0/Arduino-IRremote
//

#include <IRremoteESP8266.h>
#include <IRrecv.h>
#include <IRsend.h>
byte RECV_PIN = 4;
IRrecv irrecv(RECV_PIN);
decode_results results;

//+=============================================================================
//Include Wifi and MQTT Libraries
//For more info on MMQT visit mqtt.org

#include <ESP8266WiFi.h> // Enables the ESP8266 to connect to the local network (via WiFi)
//https://github.com/esp8266/Arduino
#include <PubSubClient.h> // Allows us to connect to, and publish to the MQTT broker
//https://github.com/knolleary/pubsubclient


//+=============================================================================
// Setup WiFi & MQTT Connections 
//
/*  Uncomment the following lines and fill with your values then comment out the include

const char * ssid = "wifi_network";
const char * password = "password";
const char * mqtt_server = "server hostname or ip";
const char * mqtt_topic = "conquest";
const char * mqtt_username = "un";
const char * mqtt_password = "pw";
*/

#include "network.h" //Loading my specific values from another file

WiFiClient espClient;
PubSubClient client(espClient); 

//+=============================================================================
// Define variables and set defaults
//

byte redValue, greenValue, blueValue = 0;
byte redScore, greenScore, blueScore = 0;
String controlledBy = "None";

//+=============================================================================
// Adjust gameplay with following variables 
//

byte hit = 60;        // damage per hit
byte threshold = 240;  // damage neccessary to change ownership
byte blinkCount = 3;  // how many times will the led blink when point is scored
int timer = 400;      // how long led is on and off


//+=============================================================================
//

void setup_wifi() {
  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  randomSeed(micros());
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP()); 
}

//+=============================================================================
//

void callback(char *topic, byte *payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
}

//+=============================================================================
//

boolean reconnect() {
 // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    //Assign Mac Address to ClientId
    String clientId = WiFi.macAddress().c_str();
    // Attempt to connect
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      client.publish("register", clientId.c_str());
      Serial.println(clientId.c_str());
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

//+=============================================================================
//Conquest Control Point Taken By
//

void controlPointTakenBy(char *team) { 
  //Send MMQT Server message indicating which team is in controll of this station
  String clientId = WiFi.macAddress();
  if (team == "red" && controlledBy !="red") {
    client.publish(clientId.c_str(), team);
    for (int i = 0; i < blinkCount; i++) {
      analogWrite(RED, 240);
      delay(timer);
      analogWrite(RED, 0);
      delay(timer);
      }
      analogWrite(RED, 240);
   }
   if (team == "green" && controlledBy !="green") {
    client.publish(clientId.c_str(), team);
    for (int i = 0; i < blinkCount; i++) {
      analogWrite(GREEN, 240);
      delay(timer);
      analogWrite(GREEN, 0);
      delay(timer);
      }
      analogWrite(GREEN, 240);
    }
    if (team == "blue" && controlledBy !="blue") {
    client.publish(clientId.c_str(), team);
    for (int i = 0; i < blinkCount; i++) {
      analogWrite(BLUE, 240);
      delay(timer);
      analogWrite(BLUE, 0);
      delay(timer);
      }
      analogWrite(BLUE, 240);
    }
    controlledBy = team;
}

//+=============================================================================
// Setup

void setup()
{
  pinMode(RED, OUTPUT);
  pinMode(GREEN, OUTPUT);
  pinMode(BLUE, OUTPUT);
  //Turn on RGB LED 
  digitalWrite(RED, HIGH);
  digitalWrite(GREEN, HIGH);
  digitalWrite(BLUE, HIGH);
  Serial.println("Initializing. . . ");
  Serial.begin(115200);
  Serial.println("Enabling IRin");
  irrecv.enableIRIn(); // Start the receiver
  Serial.println("Enabled IRin");
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}

//+=============================================================================
// Loop

void loop() {
  analogWrite(RED, redValue);
  analogWrite(GREEN, greenValue);
  analogWrite(BLUE, blueValue);
//
// IR decode values for laser tag guns https://shop.dynastytoys.com/collections/lasertag
// Not 100% accurate does not always detect used an IR sketch to come up w/ these values 
// 3 values for each color for 3 different possible gun types Pistol, Shotgun & SMG which 
// use the same IR value and the Rocket Launcher
// Could adjust hitpoints based on which gun is used
// Also, currently using White team as Referree to restart game
//

  #define BLUE1 0x5BC9BC68
  #define BLUE2 0x6F807CDD
  #define BLUE3 0xDC6082E7
  #define GREEN1 0xB3AAF2D7
  #define GREEN2 0xB7C42C00
  #define GREEN3 0x5AA7A3A5
  #define RED1 0xE4ED6F15
  #define RED2 0x4550DD55
  #define RED3 0x65843590
  #define WHITE1 0x56A0096A
  #define WHITE2 0X6B56CB6D
  #define WHITE3 0X6487A7B1

  if(irrecv.decode(&results)) {
    switch(results.value) {
      case WHITE1:      redValue=0, blueValue=0, greenValue=0; break;
      case RED1:      if (redValue<threshold){redValue+=hit; if (greenValue>0){greenValue-=hit;}; if (blueValue>0){blueValue-=hit;};} else controlPointTakenBy("red"); break;
      case GREEN1:      if (greenValue<threshold){greenValue+=hit; if (redValue>0){redValue-=hit;}; if (blueValue>0){blueValue-=hit;};} else controlPointTakenBy("green"); break;
      case BLUE1:      if (blueValue<threshold){blueValue+=hit; if (redValue>0){redValue-=hit;}; if (greenValue>0){greenValue-=hit;};} else controlPointTakenBy("blue"); break;
      case RED2:      if (redValue<threshold){redValue+=hit; if (greenValue>0){greenValue-=hit;}; if (blueValue>0){blueValue-=hit;};} else controlPointTakenBy("red"); break;
      case GREEN2:      if (greenValue<threshold){greenValue+=hit; if (redValue>0){redValue-=hit;}; if (blueValue>0){blueValue-=hit;};} else controlPointTakenBy("green"); break;
      case BLUE2:      if (blueValue<threshold){blueValue+=hit; if (redValue>0){redValue-=hit;}; if (greenValue>0){greenValue-=hit;};} else controlPointTakenBy("blue"); break;
      case RED3:      if (redValue<threshold){redValue+=hit; if (greenValue>0){greenValue-=hit;}; if (blueValue>0){blueValue-=hit;};} else controlPointTakenBy("red"); break;
      case GREEN3:      if (greenValue<threshold){greenValue+=hit; if (redValue>0){redValue-=hit;}; if (blueValue>0){blueValue-=hit;};} else controlPointTakenBy("green"); break;
      case BLUE3:      if (blueValue<threshold){blueValue+=hit; if (redValue>0){redValue-=hit;}; if (greenValue>0){greenValue-=hit;};} else controlPointTakenBy("blue"); break;
      }
    Serial.println(redValue);
    Serial.println(greenValue);
    Serial.println(blueValue);
    
    if (!client.connected()) {
      reconnect();
      }
    
    client.loop();
    };
    
  analogWrite(RED, redValue);
  analogWrite(GREEN, greenValue);
  analogWrite(BLUE, blueValue);
  irrecv.resume(); 
  delay(100);
}
