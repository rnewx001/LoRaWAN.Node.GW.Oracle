/**********************************************************************
 * THIS IS THE WORKING VERSION
 * LG01-P LoRaWAN Gateway Sketch
 * 
 * 
 **********************************************************************/

#include <SPI.h>      //necessary for pinmap
#include <LoRa.h>     //necessary for LoRa calls
#include <Console.h>  
#include <time.h>     
#include <Process.h>  
#include <lmic.h>     //necessary for os_getTime()
#include <FileIO.h>   //necessary for FileSystem. calls

//#define DEBUG 1
#define PROTOCOL_VERSION 1
#define PKT_PUSH_DATA 0
#define PKT_PUSH_ACK  1
#define PKT_PULL_DATA 2
#define PKT_PULL_RESP 3
#define PKT_PULL_ACK  4
#define TX_BUFF_SIZE  2048
#define STATUS_SIZE  1024
#define LORAMAC_PHY_MAXPAYLOAD 255
#define UPLINK 0
#define DOWNLINK 1
static uint16_t datasize = 10;

//LoRaWAN RXPK formatting handled in the python script
//gwstat.py found under /mnt/mtdblock3/

// define TTN server & port 
char SERVER1[] = "router.au.thethings.network"; // The Things Network (This router works)
//char SERVER1[] = "us-west.thethings.network"; // The Things Network (This router does not!!!!!!)
unsigned int ttnPort = 1700;//ttn
// Set center frequency
//uint32_t freq = 923200000;
uint32_t freq = 915000000; //US uses 915E6 //THIS WORKS WITH LORASIMPLENODE SKETCH
//uint32_t freq = 924000000; //either sub-band 6 or 7?

//gateway location info (hardcoded)
//unimportant for ttn and our project for now
float lat = 36.88646235;
float lon = -76.30490227;
int alt = 0;

//value to be used in loop()
long lasttime;

/**********************************************************************
* NAME:
* Function:
* Returns:
**********************************************************************/

void sendudp(char *rssi, char *packetSize, char *freq) {
  Process p;
  String strRssi = rssi;
  String strPacketSize = packetSize;
  String strFreq = freq;

  delay(3000);
  p.begin("python");
  p.addParameter("/mnt/mtdblock3/gwstat.py");
  p.addParameter(String(SERVER1));
  p.addParameter(String(ttnPort));
  p.addParameter(strRssi);
  p.addParameter(strPacketSize);
  p.addParameter(strFreq);
  p.run();
  while(p.running());
  while (p.available()) {
    char c = p.read();
    Console.print(c);
    Serial.print(c);
  }
  Console.flush();
}

/**********************************************************************
* NAME:
* Function:
* Returns:
**********************************************************************/
void sendstat() {
  // Gateway status sent using python script located @ /mnt/mtdblock3/
  sendudp("stat","",String((double)freq/1000000).c_str());
}

/**********************************************************************
* NAME:
* Function:
* Returns:
**********************************************************************/
void receivepacket() {
  // try to parse packet
  int packetSize = LoRa.parsePacket();

  if (packetSize) {
    // received a packet
    
    #ifdef DEBUG
    Console.print("packetSize : ");
    Console.println(packetSize);
    #endif
    
    int rssicorr = 157;
    
    // read packet
    int i = 0;
    char message[256];

    /*
    while (LoRa.available() && i < 256) {
      message[i]=LoRa.read();
      i++;
    }*/
    while (LoRa.available() && i < packetSize) {
      message[i]=LoRa.read();
      i++;
    }
    
    #ifdef DEBUG
    Console.print("message : ");
    Console.println(message);
    #endif
    
////////////////////////////////////////////////////////////////////////////
//  int whaval = LoRa.wha();
//    Console.print("LoRa whaval : ");
//    Console.println(whaval);

  /**********************************************************************
  * FYI- UNDERSTANDING Rssi (recieved signal strength indicator)
  * -30 dBm  Amazing Max achievable signal strength. The client can only be a few feet from the AP to achieve this. Not typical or desirable in the real world.
  * -67 dBm Very Good Minimum signal strength for applications that require very reliable, timely delivery of data packets. VoIP/VoWiFi, streaming video
  * -70 dBm Okay  Minimum signal strength for reliable packet delivery. Email, web
  * -80 dBm Not Good  Minimum signal strength for basic connectivity. Packet delivery may be unreliable.
  **********************************************************************/
    
    #ifdef DEBUG 
    Console.print("LoRa.packetRssi()).c_str() ");
    Console.println(String(LoRa.packetRssi()).c_str());
    
    Console.print("String(packetSize).c_str() ");
    Console.println(String(packetSize).c_str());
    
    Console.print("String((double)freq/1000000).c_str() ");
    Console.println(String((double)freq/1000000).c_str());
    #endif
    
    
////////////////////////////////////////////////////////////////////////////

    //Write to three locations
    //Location 1: "/mnt/mtdblock3/data/bin" (used by the packet sender as its pickup location) 
                  //data does not persist after each sendup
    //Location 2: "/var/iot/data" (used by the Dragino LG01-P UI interface to view data) 
                  //data does not persist after each sendup
    //Location 3: "/mnt/mtdblock3/data/oracleLog" (used by the Dragino LG01-P's oracle program) 
                  //data does persist for as long as it take for the cronjob to run the oracle program 

    //NOTE that anythig written under /var is written to dynamic memory and released after reboot
    
    FileSystem.begin();
    File dataFile = FileSystem.open("/mnt/mtdblock3/data/bin", FILE_WRITE);
    File dataFile2 = FileSystem.open("/var/iot/data", FILE_WRITE);
    File dataFile3 = FileSystem.open("/mnt/mtdblock3/data/oracleLog", FILE_APPEND);

    if(!dataFile){  Console.print("Didn't open /mnt/mtdblock3/data/bin"); }
    if(!dataFile2){  Console.print("Didn't open /var/iot/data"); }
    if(!dataFile3){  Console.print("/mnt/mtdblock3/data/oracleLog"); }
    
    for(int j=0;j<i;j++)
    {
        //dataFile.print(message[j]); //this wasn't resulting in a write to the bin file
        dataFile.write(message[j]);
        dataFile2.write(message[j]);
        dataFile3.write(message[j]);
    }
    dataFile3.write('\n'); //add a newline 
    dataFile.close();
    dataFile2.close();
    dataFile3.close();
    delay(1000);

    //send the messages
    sendudp(String(LoRa.packetRssi()).c_str(), String(packetSize).c_str(),String((double)freq/1000000).c_str());
  }
}

/**********************************************************************
* NAME:
* Function:
* Returns:
**********************************************************************/
void setup() {
  Bridge.begin(115200);
  Console.begin();
  while(!Console);
  // put your setup code here, to run once:
  Console.println("LoRa Receiver");

  if (!LoRa.begin(freq)) {
    Console.println("Starting LoRa failed!");
    while (1);
  }
  LoRa.setSyncWord(0x34); //synchonization value
  LoRa.receive(0);        //receive status set
}

/**********************************************************************
* NAME:
* Function:
* Returns:
**********************************************************************/
void loop() {
  receivepacket();
  //60 second interval
  long nowseconds = os_getTime();
  long waitTime = 60000000; //default = 6000000

  if( (nowseconds-lasttime) > waitTime){
      
      #ifdef DEBUG
      Console.print("lasttime = ");
      Console.println(lasttime);
      Console.print("nowseconds = ");
      Console.println(nowseconds);
      Console.print("Is this value larger than 60,000,000? ");
      Console.print(nowseconds-lasttime);
      Console.print(" > ");
      Console.println(waitTime);
      #endif
      
      Console.println("send stat\n");
      lasttime = nowseconds;
      sendstat();   
  }
  
}
