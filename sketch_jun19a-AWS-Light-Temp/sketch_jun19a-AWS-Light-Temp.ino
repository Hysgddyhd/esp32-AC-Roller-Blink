#include <DHT.h>
#include <Arduino.h>
#include <IRremote.hpp>

#define DHTPIN 4        
#define DHTTYPE DHT11   
#define ledPin 2
#define ledPin1 19
 //brightness
#define DO_PIN 14
#define IR_SEND_PIN 25
/*
   BYJ48 Stepper motor code
   Connect :
   IN1 >> 32
   IN2 >> 33
   IN3 >> 25
   IN4 >> 26
   VCC ... 5V Prefer to use external 5V Source
   Gnd
   written By :Mohannad Rawashdeh
  https://www.instructables.com/member/Mohannad+Rawashdeh/
     28/9/2013
  */

#define IN1  32
#define IN2  33
#define IN3  25
#define IN4  26

//aws code library
#include "secrets.h"
#include <WiFiClientSecure.h>
#include <MQTTClient.h>
#include <ArduinoJson.h>
#include "WiFi.h"
// The MQTT topics that this device should publish/subscribe
#define AWS_IOT_PUBLISH_TOPIC "esp32/esp32-to-aws"
String topic_1 = "esp32/AC";

String tag = "LED";
String tag1 = "AC";

#define PUBLISH_INTERVAL 4000  // 4 seconds
float humidity  = 0.0;
float temperature  = 0.0;
int lightState = 0;

WiFiClientSecure net = WiFiClientSecure();
MQTTClient client = MQTTClient(256);
unsigned long lastPublishTime = 0;
DHT dht(DHTPIN, DHTTYPE);
void setup() {
  Serial.begin(9600);
  
  pinMode(ledPin, OUTPUT);
  pinMode(ledPin1, OUTPUT);
  pinMode(DO_PIN, INPUT);
  pinMode(IN1, OUTPUT); 
  pinMode(IN2, OUTPUT); 
  pinMode(IN3, OUTPUT); 
  pinMode(IN4, OUTPUT); 
  // set the ADC attenuation to 11 dB (up to ~3.3V input)
  analogSetAttenuation(ADC_11db);
  IrSender.begin(IR_SEND_PIN);
  disableLEDFeedback(); // Disable feedback LED at default feedback LED pin 
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.println("ESP32 connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  connectToAWS();
  dht.begin();  
  Serial.println("DHT11 temperature, humidity and brightness monitor start.");
}
void loop() {
  client.loop();
  if (millis() - lastPublishTime > PUBLISH_INTERVAL) {
    sendToAWS();
    messageHandler(topic_1, tag);
    lastPublishTime = millis();
  }
}
void connectToAWS() {
  // Configure WiFiClientSecure to use the AWS IoT device credentials
  net.setCACert(AWS_CERT_CA);
  net.setCertificate(AWS_CERT_CRT);
  net.setPrivateKey(AWS_CERT_PRIVATE);
  // Connect to the MQTT broker on the AWS endpoint we defined earlier
  client.begin(AWS_IOT_ENDPOINT, 8883, net);
  // Create a handler for incoming messages
  client.onMessage(messageHandler);
  Serial.print("ESP32 connecting to AWS IOT");
  while (!client.connect(THINGNAME)) {
    Serial.print(".");
    delay(100);
  }
  Serial.println();
  if (!client.connected()) {
    Serial.println("ESP32 - AWS IoT Timeout!");
    return;
  }
  // Subscribe to a topic, the incoming messages are processed by messageHandler() function
  client.subscribe(topic_1);
  Serial.println("ESP32  - AWS IoT Connected!");
}
void sendToAWS() {
  digitalWrite(ledPin, HIGH);
  humidity = dht.readHumidity();
  temperature = dht.readTemperature(); 
  lightState = digitalRead(DO_PIN);
  StaticJsonDocument<200> message;
  message["timestamp"] = millis();
  message["temperature"] = temperature; // Or you can read data from other sensors
  message["humidity"] = humidity; // Or you can read data from other sensors
  message["lightState"] = lightState ? "It's dark" : "It's bright";
  char messageBuffer[512];
  serializeJson(message, messageBuffer);  // print to client
  client.publish(AWS_IOT_PUBLISH_TOPIC, messageBuffer);
//  Serial.println("sent:");
//  Serial.print("- topic: ");
//  Serial.println(AWS_IOT_PUBLISH_TOPIC);
//  Serial.print("- payload:"); 
//  Serial.println(messageBuffer);
  digitalWrite(ledPin, LOW);
}
void messageHandler(String &topic, String &payload) {
  Serial.println("received:");
  Serial.println("- topic: " + topic);
  Serial.println("- payload:");
  Serial.println(payload);
  // You can process the incoming data as json object, then control something
  StaticJsonDocument<200> doc;
  deserializeJson(doc, payload);
  String led = doc[tag];
  if (led == "ON"){
    digitalWrite(ledPin1, HIGH);
  }else if (led == "OFF"){
    digitalWrite(ledPin1, LOW);
  }
  else if (led == "BLINK"){
    digitalWrite(ledPin1, HIGH);   // turn the LED on (HIGH is the voltage level)
    delay(150);                       // wait for a second
    digitalWrite(ledPin1, LOW);    // turn the LED off by making the voltage LOW
    delay(150); 
    digitalWrite(ledPin1, HIGH);   // turn the LED on (HIGH is the voltage level)
    delay(150);                       // wait for a second
    digitalWrite(ledPin1, LOW);    // turn the LED off by making the voltage LOW
    delay(150); 
  }
  String command = doc[tag1];
  //Serial.println(command);
  sendIRCommand(command);
}

void sendIRCommand(String command){
    if(command[0]=='A' and command[1]=='C'){
        switch (command[2]){
          case '0':
            if(command[3]=='0' && command[4]=='0'){
              IrSender.sendPulseDistanceWidth(38, 4600, 2500, 350, 950, 350, 400, 0xEC26101012048216, 64, PROTOCOL_IS_LSB_FIRST, 0, 0);
              Serial.println("Send IR command: ON/OFF");
            }
            break;
          case '1':
              switch (command[3]){
                  case '1':
                      switch (command[4]){
                          case '1':
                          IrSender.sendPulseDistanceWidth(38, 4650, 2450, 450, 900, 450, 250, 0x4417101012118216, 64, PROTOCOL_IS_LSB_FIRST, 0, 0);
                          break;
                          case '2':
                          IrSender.sendPulseDistanceWidth(38, 4600, 2500, 400, 950, 400, 350, 0x417101012114216, 64, PROTOCOL_IS_LSB_FIRST, 0, 0); 
                          break;
                          case '3':
                          IrSender.sendPulseDistanceWidth(38, 4600, 2500, 400, 950, 400, 350, 0xE417101012112216, 64, PROTOCOL_IS_LSB_FIRST, 0, 0);
                          break;
                          default:
                          Serial.println("Incorrect code");
                      }
                      Serial.print("Send IR command: ");
                      Serial.println(command);
                  break;
                  case '2':
                      switch (command[4]){
                          case '1':
                          IrSender.sendPulseDistanceWidth(38, 4650, 2450, 450, 950, 450, 250, 0x6426101012138216, 64, PROTOCOL_IS_LSB_FIRST, 0, 0); 
                          break;
                          case '2':
                          IrSender.sendPulseDistanceWidth(38, 4600, 2500, 400, 950, 400, 350, 0x2426101012134216, 64, PROTOCOL_IS_LSB_FIRST, 0, 0);
                          break;
                          case '3':
                          IrSender.sendPulseDistanceWidth(38, 4600, 2500, 400, 950, 400, 300, 0x426101012132216, 64, PROTOCOL_IS_LSB_FIRST, 0, 0); 
                          break;
                          default:
                          Serial.println("Incorrect code");
                      }
                      Serial.print("Send IR command: ");
                          Serial.println(command);
                  break;
                  case '3':
                  switch (command[4]){
                          case '1':
                          IrSender.sendPulseDistanceWidth(38, 4650, 2450, 450, 900, 450, 250, 0x3430101012158216, 64, PROTOCOL_IS_LSB_FIRST, 0, 0);
                          break;
                          case '2':
                          IrSender.sendPulseDistanceWidth(38, 4650, 2450, 400, 900, 400, 350, 0xF430101012154216, 64, PROTOCOL_IS_LSB_FIRST, 0, 0);
                          break;
                          case '3':
                          IrSender.sendPulseDistanceWidth(38, 4600, 2500, 400, 900, 400, 350, 0xD430101012152216, 64, PROTOCOL_IS_LSB_FIRST, 0, 0);
                          break;
                          default:
                          Serial.println("Incorrect code");
                      }
                      Serial.print("Send IR command: ");
                          Serial.println(command);
                  break;
                  default:
                  Serial.println("Incorrect code");

                  
              }
          break;
          case '2':
              switch (command[3]){
                      case '1':
                      IrSender.sendPulseDistanceWidth(38, 4600, 2500, 400, 950, 400, 300, 0x5417101012192116, 64, PROTOCOL_IS_LSB_FIRST, 0, 0);
                      break;
                      case '2':
                      IrSender.sendPulseDistanceWidth(38, 4600, 2500, 450, 900, 450, 250, 0x5426101012192116, 64, PROTOCOL_IS_LSB_FIRST, 0, 0);
                      break;
                      case '3':
                      IrSender.sendPulseDistanceWidth(38, 4600, 2500, 400, 950, 400, 300, 0x430101012192116, 64, PROTOCOL_IS_LSB_FIRST, 0, 0);
                      break;
                      default:
                      Serial.println("Incorrect code");
                  }
                  Serial.print("Send IR command: ");
                  Serial.println(command);
          break;
          case '3':
              switch (command[4]){
                      case '1':
                      IrSender.sendPulseDistanceWidth(38, 4650, 2450, 450, 900, 450, 250, 0x1430101012208416, 64, PROTOCOL_IS_LSB_FIRST, 0, 0);
                      break;
                      case '2':
                      IrSender.sendPulseDistanceWidth(38, 4600, 2500, 400, 950, 400, 350, 0xD430101012204416, 64, PROTOCOL_IS_LSB_FIRST, 0, 0);
                      break;
                      case '3':
                      IrSender.sendPulseDistanceWidth(38, 4600, 2500, 400, 950, 400, 300, 0xB430101012202416, 64, PROTOCOL_IS_LSB_FIRST, 0, 0);
                      break;
                      default:
                      Serial.println("Incorrect code");
                  }
                  Serial.print("Send IR command: ");
                  Serial.println(command);
          break;
          default:
          Serial.println("Incorrect code");
        }
    }else{
      Serial.println("No command detected");
    }  
}
