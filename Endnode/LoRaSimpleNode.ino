/*
  This sketch's basis is formed from the sketch with credits below: 

  "LoRa Simple Gateway/Node Exemple

  This code uses InvertIQ function to create a simple Gateway/Node logic.

  Gateway - Sends messages with enableInvertIQ()
          - Receives messages with disableInvertIQ()

  Node    - Sends messages with disableInvertIQ()
          - Receives messages with enableInvertIQ()

  With this arrangement a Gateway never receive messages from another Gateway
  and a Node never receive message from another Node.
  Only Gateway to Node and vice versa.

  This code receives messages and sends a message every second.

  InvertIQ function basically invert the LoRa I and Q signals.

  See the Semtech datasheet, http://www.semtech.com/images/datasheet/sx1276.pdf
  for more on InvertIQ register 0x33.

  created 05 August 2018
  by Luiz H. Cassettari"
*/

#include <SPI.h>              
#include <LoRa.h>
#include "Encrypt.h"

const long frequency = 915E6;  // LoRa Frequency
//const long frequency = 9135E5;  // LoRa Frequency

const int csPin = 10;          // LoRa radio chip select
const int resetPin = 9;        // LoRa radio reset
const int irqPin = 2;          // change for your board; must be a hardware interrupt pin

////////////////////////////////FUNCTIONS////////////////////////////////

void setup() {
  Serial.begin(115200); //9600);                   // initialize serial
  while (!Serial);

  LoRa.setPins(csPin, resetPin, irqPin);

  if (!LoRa.begin(frequency)) {
    Serial.println("LoRa init failed. Check your connections.");
    while (true);                       // if failed, do nothing
  }

  Serial.println("LoRa init succeeded.");
  Serial.println();
  Serial.println("LoRa Simple Node");
  Serial.println("Only receive messages from gateways");
  Serial.println("Tx: invertIQ disable");
  Serial.println("Rx: invertIQ enable");
  Serial.println();

  LoRa.onReceive(onReceive);
  LoRa.onTxDone(onTxDone);
  LoRa_rxMode();
}


////////////////////////////////////////////////////
////////////////////////////////////////////////////

void loop() {
  if (runEvery(1000)) { // repeat every 1000 millis

    String message = "HeLoRa World! ";
    message += "I'm a Node! ";
    message += millis();

    encypt(message); //encrypt and send

    Serial.println("Send Message!");
  }
}

////////////////////////////////////////////////////
////////////////////////////////////////////////////

void LoRa_rxMode(){
  LoRa.enableInvertIQ();                // active invert I and Q signals
  LoRa.receive();                       // set receive mode
}

////////////////////////////////////////////////////
////////////////////////////////////////////////////

void LoRa_txMode(){
  LoRa.idle();                          // set standby mode
  LoRa.disableInvertIQ();               // normal mode
}

////////////////////////////////////////////////////
////////////////////////////////////////////////////

void LoRa_sendMessage(String message) {
  LoRa_txMode();                        // set tx mode
  LoRa.beginPacket();                   // start packet
  LoRa.print(message);                  // add payload
  LoRa.endPacket(true);                 // finish packet and send it
}

////////////////////////////////////////////////////
////////////////////////////////////////////////////

void onReceive(int packetSize) {
  String message = "";

  while (LoRa.available()) {
    message += (char)LoRa.read();
  }

  Serial.print("Node Receive: ");
  Serial.println(message);
}

////////////////////////////////////////////////////
////////////////////////////////////////////////////

void onTxDone() {
  Serial.println("TxDone");
  LoRa_rxMode();
}

////////////////////////////////////////////////////
////////////////////////////////////////////////////

boolean runEvery(unsigned long interval)
{
  static unsigned long previousMillis = 0;
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval)
  {
    previousMillis = currentMillis;
    return true;
  }
  return false;
}

////////////////////////////////////////////////////
////////////////////////////////////////////////////

void encypt(String message){

  sBuffer *Buffer;
  Buffer->Data = (unsigned char*)message.c_str();
  
  const char *devAddr = "526F4C65";
  const char *nwkSKey = "74CD80A28E6299C777D1A8AD4961C894"; //MSB
  //const char *nwkSKey = "94C86149ADA8D177C799628EA280CD74"; //LSB nwkSKey
  const char *appSKey = "E378681D1B3C9FF71A0ADE630CB3FA05";//MSB

  Serial.print("size of Buffer->Data: ");
  Serial.println(strlen((char*)Buffer->Data));
  Serial.println();

  sLoRa_Message loraMessage;
  loraMessage.DevAddr[0] = 0x52; 
  loraMessage.DevAddr[1] = 0x6F;
  loraMessage.DevAddr[2] = 0x4C;
  loraMessage.DevAddr[3] = 0x65;
  loraMessage.Direction = 0x00; //0 for uplink frames
  loraMessage.Frame_Counter = 1; 

  Encrypt_Payload(Buffer, nwkSKey, &loraMessage);

  int len = strlen((char*)Buffer->Data);
  char hex_str[(len*2)+1];

  for(int i = 0; i < len; i++)
  { 
      sprintf(hex_str+2*i, "%02x", Buffer->Data[i]); 
      if(i==len){hex_str[i] = '\0';}
  } 

  Serial.println(hex_str);
  Serial.println();
  
  LoRa_sendMessage(hex_str); // send a message
  
}
