/*
 * Example to demonstrate ESP32's ability to act as a wifi station and
 * BLE server simultaneously (while running MQTT!)
 *
 * Board tested on - ESP32 Wemos Lolin32 Lite
 *
 * Created on 12/2/2017 by legendary6
 */

#include <WiFi.h>
#include <PubSubClient.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>


// Update these with values suitable for your network.
const char* ssid = "********";
const char* password = "********";
const char* mqtt_server = "********";

// See the following for generating UUIDs:
// https://www.uuidgenerator.net/

#define SERVICE_UUID        "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"


WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
char msg[100];
int failed = 0;

//callback for BLE characteristic
class MyCallbacks: public BLECharacteristicCallbacks {
  
    //called when a value is written by the master
    void onWrite(BLECharacteristic *pCharacteristic) {
      std::string value = pCharacteristic->getValue();    //gets the written value from the characteristic

      char arr[value.length()+1];                         //array to store the value
      arr[value.length()] = '\0';                         //making the array a valid UTF8 string

      //print the value on serial and copy value data into arr 
      if (value.length() > 0) {
        Serial.println("*********");
        Serial.print("New value: ");
        for (int i = 0; i < value.length(); i++){
          Serial.print(value[i]);
          arr[i] = value[i];
        }
        Serial.println();
        Serial.println("*********");
      }
      
      client.publish("test_feed_2", arr);                 //publish the message to a feed
    }
};

void setup() {

  pinMode(22, OUTPUT);                                    //builtin led init
  digitalWrite(22, LOW);
  Serial.begin(115200);
  
  setup_wifi();                                           //set up wifi

  BLEDevice::init("MyESP32");                             //set BLE device name
  BLEServer *pServer = BLEDevice::createServer();         //create ble server

  //create a service
  BLEService *pService = pServer->createService(SERVICE_UUID);

  //create a characteristic
  BLECharacteristic *pCharacteristic = pService->createCharacteristic(
                                         CHARACTERISTIC_UUID,
                                         BLECharacteristic::PROPERTY_READ |
                                         BLECharacteristic::PROPERTY_WRITE
                                       );

  //set callback for the characteristic
  pCharacteristic->setCallbacks(new MyCallbacks());

  //set value for reading
  pCharacteristic->setValue("Hello World");
  pService->start();

  //start advertising
  BLEAdvertising *pAdvertising = pServer->getAdvertising();
  pAdvertising->start();
  
}

//wifi set up code
void setup_wifi() {

  delay(10);
  int i = 0;
  do{
    Serial.print("attempting wifi : ");
    Serial.println(i);
    i++;
    Serial.println(WiFi.status());
    WiFi.begin(ssid, password);
    delay(2500);
  }while (WiFi.status() != WL_CONNECTED);

  
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  //MQTT server setup
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);

  //connect to MQTT server
  reconnect();
}

//MQTT callback
void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
    for (int i = 0; i < length; i++) {
      Serial.print((char)payload[i]);
    }
  Serial.println();
}

//MQTT connect function
void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    //Attempt to connect
    if (client.connect("ESP32Client", "root", "root")) {
      Serial.println("connected");
      
      // Once connected, publish an announcement...
      //client.publish("notify_all", "hello world");
      // ... and resubscribe
      client.subscribe("test_feed_1");
      digitalWrite(2, HIGH);
    } else {
      digitalWrite(2, LOW);
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      failed = 100;
      // Wait 5 seconds before retrying
      //delay(5000);
    }
  }
}

void loop() {

  if(WiFi.status() != WL_CONNECTED) {
    Serial.print("wifi disconnected");
    digitalWrite(22 , LOW);
    setup_wifi();
  }
  
  if (!client.connected() && failed == 0) {
    digitalWrite(22, LOW);
    reconnect();
  }
  else {
    failed--;
  }
  client.loop();
}
