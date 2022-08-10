#include "heltec.h"
#include "protocol.h"

uint32_t chipId = 0;

#define BAND    915E6  //you can set band here directly,e.g. 868E6,915E6

int interval = 2*1000;          // interval between sends
long lastPacket = 0;        // time of last packet seen


void setup() {
    //WIFI Kit series V1 not support Vext control
  Heltec.begin(true /*DisplayEnable Enable*/, true /*Heltec.LoRa Disable*/, true /*Serial Enable*/, true /*PABOOST Enable*/, BAND /*long BAND*/);

  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }


  for(int i=0; i<17; i=i+8) {
    chipId |= ((ESP.getEfuseMac() >> (40 - i)) & 0xff) << i;
  }

  Serial.print("I am ");
  Serial.println(chipId);
}

boolean onReceive(int packetSize)
{
  yield();
  if (packetSize == 0) return false;          // if there's no packet, return

  digitalWrite(25, HIGH);   // turn the LED on (HIGH is the voltage level)
  

  String incoming = "";
  while (LoRa.available()) {
    char p = (char)LoRa.read();
    incoming += p;
  }


  Heltec.display->clear();
  Heltec.display->drawRect(5, 13, 105, 6);
  Heltec.display->drawRect(6, 14, 106 + LoRa.packetRssi(), 2);
  Heltec.display->drawRect(6, 16, 106 - LoRa.packetSnr(), 2);
  Heltec.display->drawString(1, 0, String(LoRa.packetRssi()));
  Heltec.display->drawString(64, 0, String(LoRa.packetSnr()));
  
  // Check if byte 0 is of type MessageType::DATA
  if (incoming[0] == (uint8_t)MessageType::DATA){
    message_t msg = parseData(incoming);

    union msg_buffer {
      byte b[11];
      message_t msg;
    } buf;

    buf.msg = { MessageType::DATA , LoRa.packetRssi(), LoRa.packetSnr(), msg.latitude, msg.longitude };
    Serial.write(buf.b, sizeof(msg));
    Serial.write('\0');

    Heltec.display->drawString(1, 30, " < DATA");
    Heltec.display->drawString(1, 40, " < " + String(msg.latitude,8) + "/" + String(msg.longitude));
    msg = { MessageType::ACK, 0, 0, 0, 0};
    sendPacket(msg);

  } else if (incoming[0] == (uint8_t)MessageType::ACK){
    Heltec.display->drawString(1, 30, " < ACK");
    Serial.println("ACK received");
  } else if (incoming[0] == (uint8_t)MessageType::HEARTBEAT){
    Heltec.display->drawString(1, 30, " < HEARTBEAT");
    message_t msg;
    msg = { MessageType::ACK, 0, 0, 0, 0};
    sendPacket(msg);

  } else {
    Heltec.display->drawString(1, 30, " < UNKNOWN");
    // Display incoming[0] as hex
    Heltec.display->drawString(100, 30, String(incoming[0], HEX));
  }

  Heltec.display->display();
  digitalWrite(25, LOW);    // turn the LED off by making the voltage LOW
}

void sendPacket(message_t msg){

  union msg_buffer {
    byte b[11];
    message_t msg;
  } buf;

  /*
  Heltec.display->clear();
  Heltec.display->drawRect(5, 15, 105, 4);
  Heltec.display->drawRect(6, 16, 106 + LoRa.packetRssi(), 2);
  Heltec.display->drawString(1, 0, String(LoRa.packetRssi()));
  Heltec.display->drawString(64, 0, String(LoRa.packetSnr()));
  */

  buf.msg = msg;

  if (msg.type == MessageType::DATA){
    Heltec.display->drawString(1, 20, " > DATA");
  } else if (msg.type == MessageType::ACK){
    Heltec.display->drawString(1, 20, " > ACK");
  } else if (msg.type == MessageType::HEARTBEAT){ 
    Heltec.display->drawString(1, 20, " > HB");
  } else {
    Heltec.display->drawString(1, 20, " > UNKNOWN");
  }
  Heltec.display->display();

  digitalWrite(25, HIGH);   // turn the LED on (HIGH is the voltage level)
  LoRa.beginPacket();
  LoRa.write(buf.b, sizeof(msg));
  LoRa.endPacket();
  digitalWrite(25, LOW);    // turn the LED off by making the voltage LOW

  lastPacket = millis(); 
  // delayMicroseconds(100);
}

message_t parseData(String incoming){

  // Decode the incoming message 
  union u_tag {
      byte b[4];
      float fval;
  } u;    
  
  u.b[0] = incoming[3];
  u.b[1] = incoming[4];
  u.b[2] = incoming[5];
  u.b[3] = incoming[6];
  float lat;
  lat = u.fval;

  u.b[0] = incoming[7];
  u.b[1] = incoming[8];
  u.b[2] = incoming[9];
  u.b[3] = incoming[10];
  float lon;
  lon = u.fval;

  message_t msg = { MessageType::DATA , LoRa.packetRssi(), LoRa.packetSnr(), lat, lon};
  return msg;
}

void sendHeartbeat(){
  Serial.println("Sending heartbeat");

  message_t msg;
  msg = { MessageType::HEARTBEAT, 0, 0, 0, 0};
  sendPacket(msg);  
}

void forward(String incoming){
  // Serial.println("Forwarding data");
  message_t msg = parseData(incoming);
  Heltec.display->drawString(1, 40, " > lat: " + String(msg.latitude,8));
  sendPacket(msg);
}

void loop() {

  onReceive(LoRa.parsePacket());

  if (millis() - lastPacket > interval && chipId != 10427812)
  {
    sendHeartbeat();
  }  
  
  while (Serial.available()) {    
    String input;
    input = Serial.readString();
    forward(input);
    //    wait_for_ack();
  }    
}
