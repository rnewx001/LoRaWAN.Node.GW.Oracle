# LoRaWAN.Node.GW.Oracle
This repo houses the code for the LoRaWAN endnode and gateway/oracle
LoRaWAN Proof-of-Concept Oracle Readme


[  How does it work?  ]
The LG01-P serves several roles--chief among them are as a LoRaWAN router to The Things Network and as an oracle to the Tikiri blockchain.
The LG01-P has an Arduino IDE sketch (LiteLoRaWanGW.ino) which allows it to receive LoRaWAN transmissions from a LoRaWAN endnode.
The Arduino Sketch receives the LoRaWAN transmission and writes that data to 3 locations:
	//Writes to three locations:
    	//Location 1: "/mnt/mtdblock3/data/bin" (used by the packet sender as its pickup location) 
     	              //data does not persist after each sendup
    	//Location 2: "/var/iot/data" (used by the Dragino LG01-P Admin UI interface to view data) 
    	              //data does not persist after each sendup
    	//Location 3: "/mnt/mtdblock3/data/oracleLog" (used by the Dragino LG01-P's oracle script) 
         	          //data does persist for as long as it take for the cronjob to run the oracle script 

The sketch also calls a python script (gwstat.py) which sends the LoRaWAN packet to the TTN (The Things Network).
It requires TTN configuration (see TTN configuration section below).

Furthermore, the LG01-P acts as an oracle to the Tikiri blockchain. 
In order to accomplish this, the LG01-P has a python script (oracle.py) which runs according to its crontab entry.
The oracle.py script gathers the transmissions from the oracleLog, parses them, and sends them to the Kafka broker.
It requires Kafka server configuration (See Kafka configuration section below).


[  LoRaWAN LG01-P Gateway SECTION  ]

*** Accessing the LG01-P Gateway: SSH/FTP *** 
1) The LG01-P LoRaWAN Gateway broadcasts as a wifi connection. EX: SSID might look like this "dragino-1c9d54"
WIFI PASSWORD for LG01-P is "dragino".
2) Once you are connected to the LG01-P's wifi, you can directly access the LG01-P gateway.
3) The LG01-P has a static IP of 10.130.1.1. With it, you can SSH and FTP into LG01-P using ROOT credentials (user: root, password: dragino)
EX: ssh root@10.130.1.1 
EX: ftp root@10.130.1.1 


*** Accessing the LG01-P Gateway: Arduino IDE *** 
1) You can also compile and upload sketches to the LG01-P gateway using the Arduino IDE app. 
2) First, ensure you are connected to the LG01-P's WIFI before accessing it via the IDE.
3) In the Arduino IDE, under Tools > Board, ensure "Draino Yun + UNO or LG01/OLG01" is selected.
Under Port: ensure the wifi connected is selected (i.e. "dragino-1c9d54 at 172.31.255.254 (Arduino Yun)").
4) Now you can load, compile, and send sketch updates to the LG01-P--as well as use the Serial Monitor to view sketch results.


*** Accessing the LG01-P Gateway: LG01-P Administrative UI *** 
1) First, ensure you are connected to the LG01-P's WIFI before accessing it via the IDE.
2) Enter this url: http://10.130.1.1/cgi-bin/luci/admin    //this assumes the default IP has not been changed on the LG01-P
3) Log in using the user/password credentials
user: root
password: dragino


*** LG01-P Gateway: STRUCTURE *** 
On the LG01-P gateway, there needs to be the following file structure:

/mnt/mtdblock3/
		base64.py
		gwstat.py
		oracle.py
		data/
		    bin
		    oracleLog

gwstat.py is called by the Arduino Sketch and creates a socket and sends lorawan packets as DGRAMS to TTN
base64.py is called by gwstat.py and b64 encodes the data
oracle.py is called by crontab and is responsible for sending the LoRaWAN transmissions to a Kafka broker (i.e. Tikiri)


*** What was installed on the LG01-P ***
In addition to the sketches (as well as the libs the sketches need--installed by the Arduino IDE), there was an additional installation.
kafka-python-2.0.1.tar.gz was installed on the gateway to facilitiate the kafka producer calls made by oracle.py.


[  About gwstat.py  ]

gwstat.py reaches out to TTN using the server address and port specified in the Arduino Sketch.
They are currently hardcoded in the sketch with the below values and have been tested to work against TTN.
	char SERVER1[] = "router.au.thethings.network"; 
	unsigned int ttnPort = 1700;

gwstat.py also has a section which requires the LG01-P's DevEUI be defined. 
This can be found on the LG01-P's housing. Note that FF FF were inserted in the middle per LoRaWAN spec.

	# change your gateway ID(from 0xA8 to 0x57)
	# our LG01-P's DevEUI is A8 40 41 FF FF 1C 9D 57 (FF inserted)
	head = chr(1) \
	+ chr(random.randint(0,255)) \
	+ chr(random.randint(0,255)) \
	+ chr(0) \
	+ chr(0xA8) \
	+ chr(0x40) \
	+ chr(0x41) \
	+ chr(0xFF) \
	+ chr(0xFF) \
	+ chr(0x1C) \
	+ chr(0x9D) \
	+ chr(0x57)

	NOTE - This MUST match the DevEUI that's defined for the gateway on the TTN or forwarded messsages to TTN from this gateway will be dropped.


[  About oracle.py  ]

oracle.py is responsible for sending LoRaWAN packets to a kafka broker.
Currently, the broker is defined within the script:
	#CONFIGURE THE IP:PORT SETTINGS FOR THE KAFKA CLUSTER HERE
	KafkaServerIPandPORT = '10.130.1.239:9092'
This variable will need to be changed to point to the kafka broker you're testing against.	

Furthermore, oracle.py is currently controlled by a cronjob entry.
cronjob for the oracle.py (set to run every minute)

	root@dragino-1c9d54:/mnt/mtdblock3/data# crontab -l
	# For details see man 4 crontabs

	# Example of job definition:
	# .---------------- minute (0 - 59)
	# | .------------- hour (0 - 23)
	# | | .---------- day of month (1 - 31)
	# | | | .------- month (1 - 12) OR jan,feb,mar,apr ...
	# | | | | .---- day of week (0 - 6) (Sunday=0 or 7) OR sun,mon,tue,wed,thu,fri,sat
	# | | | | |
	# * * * * * user-name command to be executed

	* * * * * python /mnt/mtdblock3/oracle.py



[  About TTN (The Things Network)  ]

The Things Network serves as the LoRaWAN server (and end destination for LoRaWAN packets).
To access TTN, use the below:

TTN TheThingsNetwork
	URL: https://console.thethingsnetwork.org/gateways/eui-a84041ffff1c9d57
	user: rnewx001
	useremail: rnewx001@odu.edu
	TTN password: CDEcseVFRrew432@

	Also remember that routing to TTN requires a definition within the Arduino Sketch: 
		// define TTN server & port 
		char SERVER1[] = "router.au.thethings.network"; // The Things Network (This router works)
		unsigned int ttnPort = 1700;//ttn



[  About the End Node Sketch  ]

The arduino LoRaWAN endnode sketch (LoRaSimpleNode_HardCodedMsg.ino) serves the LoRaWAN Gateway with a static transmission and repeats the transmission indefinitely. Simply broadcasting on the US LoRaWAN band, the gateway will pick up the end node's packets.

The required libs are downloaded using the Arduino IDE.
