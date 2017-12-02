/*
 * This sketch receives the UDP packets sent by the sender over LAN
 * and prints the contents of the packets and turns an LED on/off 
 * depending on the content(1 = ON, 2 = OFF)
 * 
 * Board tested on - ESP32 Wemos Lolin32 Lite, ESP32 Dev Module
 * 
 * This should also work in ESP8266
 * 
 * Created on 12/1/2017 by RammaK
 */

#include <WiFi.h>
#include <WiFiUdp.h>

/* WiFi network name and password */
const char * ssid = "********";
const char * pwd = "********";

// IP address to send UDP data to.
// it can be ip address of the server or 
// a network broadcast address
// here is broadcast address
//const char * udpAddress = "192.168.1.100";
const int udpPort = 44444;

//create UDP instance
WiFiUDP udp;

void setup(){
  Serial.begin(115200);

  //LED pin initialization
  pinMode(22, OUTPUT);
  digitalWrite(22, LOW);
  
  //Connect to the WiFi network
   WiFi.begin(ssid, pwd);
  Serial.println("");

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  
  //This initializes udp and transfer buffer
  udp.begin(udpPort);
}

void loop(){
  
  //data will be sent to server
  uint8_t buffer[50];
  memset(buffer, 0, 50);
  
  //processing incoming packet, must be called before reading the buffer
  udp.parsePacket();
  
  //receive response from server, it will be HELLO WORLD
  if(udp.read(buffer, 50) > 0){
    Serial.print("Server to client: ");
    Serial.println((char *)buffer);

    //turn on/off led based on the content
    if(buffer[0] == '1') {
      digitalWrite(22, HIGH);
    }
    else
      digitalWrite(22, LOW);
  }
  //Wait for 1 second
  delay(1000);
}
