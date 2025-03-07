/*
  Smart Parking System using ESP8266

  This program is designed to manage a smart parking system using an ESP8266 microcontroller.
  It integrates IR sensors, servo motors, and Adafruit IO MQTT for real-time data transmission
  and gate automation. Ensure you have the necessary libraries installed in your Arduino IDE
  (e.g., ESP8266WiFi, Servo, Adafruit_MQTT). Upload this code to your ESP8266 board using the
  Arduino IDE, ensuring correct pin mappings and WiFi credentials are provided in the setup
  section below.

  Board: ESP8266 (NodeMCU, Wemos D1 Mini, etc.)
  IDE: Arduino IDE
  Libraries: ESP8266WiFi, Servo, Adafruit_MQTT

*/

#include <ESP8266WiFi.h>
#include <Servo.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"
// const char *ssid =  "CHENAB";     // Enter your WiFi Name
// const char *pass =  "44zMf3QqdU&KC3Mv"; // Enter your WiFi Password




const char *ssid =  "";     // Enter your WiFi Name
const char *pass =  ""; // Enter your WiFi Password

#define MQTT_SERV "io.adafruit.com"
#define MQTT_PORT 1883
#define MQTT_NAME ""
#define MQTT_PASS ""
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", 19800,60000);
Servo myservo;                          //servo as gate
Servo myservos;


int entryTrigger = D0;
int entryEcho = D1;
int exitTrigger = D2;
int exitEcho = D3;                  //exi sensor
int slot3 = D7;                 
int slot2 = D6;
int slot1  = A0;
int count =0; 
int CLOSE_ANGLE = 180;  // The closing angle of the servo motor arm
int OPEN_ANGLE = 0;  // The opening angle of the servo motor arm              
int  hh, mm, ss;
int pos;
int pos1;

String h, m,EntryTimeSlot1,ExitTimeSlot1, EntryTimeSlot2,ExitTimeSlot2, EntryTimeSlot3,ExitTimeSlot3;
boolean entrysensor, exitsensor,s1,s2,s3;

boolean s1_occupied = false;
boolean s2_occupied = false;
boolean s3_occupied = false;

WiFiClient client;
Adafruit_MQTT_Client mqtt(&client, MQTT_SERV, MQTT_PORT, MQTT_NAME, MQTT_PASS);

//Set up the feed you're subscribing to
Adafruit_MQTT_Subscribe EntryGate = Adafruit_MQTT_Subscribe(&mqtt, MQTT_NAME "/f/EntryGate1");
Adafruit_MQTT_Subscribe ExitGate = Adafruit_MQTT_Subscribe(&mqtt, MQTT_NAME "/f/ExitGate");

//Set up the feed you're publishing to
Adafruit_MQTT_Publish CarsParked = Adafruit_MQTT_Publish(&mqtt,MQTT_NAME "/f/CarsParked");
Adafruit_MQTT_Publish EntrySlot1 = Adafruit_MQTT_Publish(&mqtt,MQTT_NAME "/f/EntrySlot1");
Adafruit_MQTT_Publish ExitSlot1 = Adafruit_MQTT_Publish(&mqtt,MQTT_NAME "/f/ExitSlot1");
Adafruit_MQTT_Publish EntrySlot2 = Adafruit_MQTT_Publish(&mqtt,MQTT_NAME "/f/EntrySlot2");
Adafruit_MQTT_Publish ExitSlot2 = Adafruit_MQTT_Publish(&mqtt,MQTT_NAME "/f/ExitSlot2");
Adafruit_MQTT_Publish EntrySlot3 = Adafruit_MQTT_Publish(&mqtt,MQTT_NAME "/f/EntrySlot3");
Adafruit_MQTT_Publish ExitSlot3 = Adafruit_MQTT_Publish(&mqtt,MQTT_NAME "/f/ExitSlot3");

void setup() {
  delay(1000);
  Serial.begin (9600); 
  mqtt.subscribe(&EntryGate);
  mqtt.subscribe(&ExitGate);
  timeClient.begin(); 
// here comes the predefined things
  myservo.attach(D4);      // servo pin to D6
  myservos.attach(D5);       // servo pin to D5
  pinMode(entryTrigger, OUTPUT);
  pinMode(entryEcho, INPUT);
  pinMode(exitTrigger, OUTPUT);
  pinMode(exitEcho, INPUT);
  pinMode(slot1, INPUT);
  pinMode(slot2, INPUT);
  pinMode(slot3, INPUT);
  WiFi.begin(ssid, pass);                                     //try to connect with wifi
  Serial.print("Connecting to ");
  Serial.print(ssid);                          // display ssid
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");                          // if not connected print this
    delay(500);
  }
  Serial.println();
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP Address is : ");
  Serial.println(WiFi.localIP());                                            //print local IP address
}

void loop() {

// keep looping till death
 MQTT_connect();
 timeClient.update();
 hh = timeClient.getHours();
 mm = timeClient.getMinutes();
 ss = timeClient.getSeconds();
 h= String(hh); 
 m= String(mm);
 h +" :" + m;
 
int entryDistance = measureDistance(entryTrigger, entryEcho);
int exitDistance = measureDistance(exitTrigger, exitEcho);
Serial.println(exitDistance);

 s1 = digitalRead(slot1);
 s2 = digitalRead(slot2);
 s3 = digitalRead(slot3);
 
  if (entryDistance > 0 && entryDistance < 20) { // If entry detected
    if (count < 3){
      count=  count+1;                             //increment count
      myservos.write(OPEN_ANGLE);
      delay(3000);
      myservos.write(CLOSE_ANGLE);
    }                  
   }
   
  if (exitDistance > 0 && exitDistance < 20) { // If entry detected

      count=  count-1;                             //increment count
      myservo.write(OPEN_ANGLE);
      delay(3000);
      myservo.write(CLOSE_ANGLE);
                
   }
  if (! CarsParked.publish(count)) {}
// seriously bruh?
  if (s1 == 0 && s1_occupied == false) {                     
        Serial.println("Occupied1 ");
        EntryTimeSlot1 =  h +" :" + m;
        //Serial.print("EntryTimeSlot1");
        //Serial.print(EntryTimeSlot1);
        
        s1_occupied = true;
        if (! EntrySlot1.publish((char*) EntryTimeSlot1.c_str())){}
    }
  if(s1 == 1 && s1_occupied == true) {
       Serial.println("Available1 ");
       ExitTimeSlot1 =  h +" :" + m;
       //Serial.print("ExitTimeSlot1");
       //Serial.print(ExitTimeSlot1);
     
       s1_occupied = false;
        if (! ExitSlot1.publish((char*) ExitTimeSlot1.c_str())){} 
}
  if (s2 == 0 && s2_occupied == false) {                     
      Serial.println("Occupied2 ");
      EntryTimeSlot2 =  h +" :" + m;
      //Serial.print("EntryTimeSlot2");
      //Serial.print(EntryTimeSlot2);
     
      s2_occupied = true;
      if (! EntrySlot2.publish((char*) EntryTimeSlot2.c_str())){}
    }
  if(s2 == 1 && s2_occupied == true) {
       Serial.println("Available2 ");
       ExitTimeSlot2 =  h +" :" + m;
       //Serial.print("ExitTimeSlot2");
       //Serial.print(ExitTimeSlot2);
       
       s2_occupied = false;
        if (! ExitSlot2.publish((char*) ExitTimeSlot2.c_str())){}
  }
  if (s3 == 0 && s3_occupied == false) {                     
      Serial.println("Occupied3 ");
      EntryTimeSlot3 =  h +" :" + m;
     //Serial.print("EntryTimeSlot3: ");
      //Serial.print(EntryTimeSlot3);
      s3_occupied = true;
       if (! EntrySlot3.publish((char*) EntryTimeSlot3.c_str())){}
    }
  if(s3 == 1  && s3_occupied == true) {
       Serial.println("Available3 ");
       ExitTimeSlot3 =  h +" :" + m;
       //Serial.print("ExitTimeSlot3: ");
       //Serial.print(ExitTimeSlot3);
       s3_occupied = false;
        if (! ExitSlot3.publish((char*) ExitTimeSlot3.c_str())){ }
  } 

  Adafruit_MQTT_Subscribe * subscription;
  while ((subscription = mqtt.readSubscription(5000)))
     {
    
   if (subscription == &EntryGate)
     {
      //Print the new value to the serial monitor
      Serial.println((char*) EntryGate.lastread);
     
   if (!strcmp((char*) EntryGate.lastread, "ON"))
      {
       myservos.write(OPEN_ANGLE);
       delay(3000);
       myservos.write(CLOSE_ANGLE);
    }
}
  if (subscription == &ExitGate)
     {
      //Print the new value to the serial monitor
      Serial.println((char*) EntryGate.lastread);
     
   if (!strcmp((char*) ExitGate.lastread, "ON"))
      {
       myservo.write(OPEN_ANGLE);
       delay(3000);
       myservo.write(CLOSE_ANGLE);
    }
}
}  
}
int measureDistance(int triggerPin, int echoPin) {
  digitalWrite(triggerPin, LOW);
  delayMicroseconds(2);
  digitalWrite(triggerPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(triggerPin, LOW);
  return pulseIn(echoPin, HIGH) / 58; // Convert duration to distance (cm)
}
void MQTT_connect() 
{
  int8_t ret;

  // Stop if already connected.
  if (mqtt.connected()) 
  {
    return;
  }

  uint8_t retries = 3;
  while ((ret = mqtt.connect()) != 0) // connect will return 0 for connected
  { 
       mqtt.disconnect();
       delay(5000);  // wait 5 seconds
       retries--;
       if (retries == 0) 
       {
         // basically die and wait for WDT to reset me
         while (1);
       }
  }
}
