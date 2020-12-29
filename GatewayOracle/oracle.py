#! /usr/bin/env python

import timeit 			#necessary for start/stop time
import os				#necessary for os.system calls
import subprocess		#necessary for subprocess.calls (this is to be used INSTEAD of os.system if chosen)
from time import sleep
from json import dumps
from kafka import KafkaProducer #necessary for kafka calls

#start = timeit.default_timer()

#CONFIGURE THE IP:PORT SETTINGS FOR THE KAFKA CLUSTER HERE
KafkaServerIPandPORT = '10.130.1.239:9092'

#copy the logfile intended for oracle use from /mnt location to pwd
#subprocess.call(["cp", "/mnt/mtdblock3/data/oracleLog", "oracleLog.lock"])
os.system("cp /mnt/mtdblock3/data/oracleLog oracleLog.lock")
#os.system("cp /mnt/mtdblock3/data/oracleLog.bak2 oracleLog.lock") #just for now so we don't need a constant LoRaWAN feed

#zero out the contents of the log since we've made our copy  
open("/mnt/mtdblock3/data/oracleLog", 'w').close()

#open the locked version of oracleLog in read mode
F = open("oracleLog.lock","r") 

#assign list the contents of the locked log file
list = F.read()

#strip leading/trailing whitesplace and split each element based on newline delimiter
list = list.strip().split('\n')

#create a list for the forthcoming JSON objects
JSON = []

#define a function to switch byte order from little endian to big endian
#needed for DevAddr and FCnt
def bigEndian(field):
	index = (len(field) - 1)
	list = []
	for i in range( 0, (len(field)/2) ):
		list.append(field[index-1])
		list.append(field[index])
		index -= 2
	return ''.join(list)

#parse the ASCII-endcoded, HEX representation of the PHYPayload
for line in list:
	MHDR = line[0:2]
	MIC = line[-8:]
	MACPayload = line[2:-8]
	FHDR = MACPayload[0:14] 
	FPort = MACPayload[14:16]
	FRMPayload = MACPayload[16:len(MACPayload)]
	DevAddr = FHDR[0:8] #reverse order
	#reverse DevAddr byte order (BIG ENDIAN)
	DevAddr = bigEndian(DevAddr)
	FCtrl = FHDR[8:10]
	FCnt = FHDR[10:14]
	#reverse FCnt byte order (BIG ENDIAN)
	FCnt = bigEndian(FCnt)
	FOpts = FHDR[14:len(FHDR)]
	
	print "MHDR: ", MHDR
	print "MIC: ", MIC
	print "MACPayload: ", MACPayload 
	print "FHDR: ", FHDR
	print "FPort: ", FPort
	print "FRMPayload: ", FRMPayload
	print "DevAddr: ", DevAddr
	print "FCtrl: ", FCtrl
	print "FCnt: ", FCnt
	print "FOpts: ",FOpts
	print "\n"

	rxpk = "{\"rxpk\":[{" \
      	+ "\"tmst\":" + subprocess.check_output(["date", "+%s"])[0:10] + ",".replace('\n','') \
      	+ "\"MHDR\":" + MHDR + "," \
      	+ "\"MIC\":" + MIC + "," \
      	+ "\"MACPayload\":" + MACPayload + "," \
      	+ "\"FHDR\":" + FHDR + "," \
      	+ "\"FPort\":" + FPort + "," \
      	+ "\"FRMPayload\":" + FRMPayload + "," \
      	+ "\"DevAddr\":" + DevAddr + ","\
      	+ "\"FCtrl\":" + FCtrl + "," \
      	+ "\"FCnt\":" + FCnt + "," \
      	+ "\"FOpts\":" + FOpts + "," \
    	+ "\"}]}"

	JSON.append(rxpk)

#copy the logfile intended for oracle use from /mnt location to pwd                  
#subprocess.call(["rm", "oracleLog.lock"]) 
os.system("rm oracleLog.lock")


#need to have a kafka server cluster defined                  
#producer = KafkaProducer(bootstrap_servers=['localhost:9092'], value_serializer=lambda x: dumps(x).encode('utf-8'))
#producer = KafkaProducer(bootstrap_servers=['10.130.1.239:9092'], value_serializer=lambda x: dumps(x).encode('utf-8'))
producer = KafkaProducer(bootstrap_servers=[KafkaServerIPandPORT], value_serializer=lambda x: dumps(x).encode('utf-8'))

                                                                                                                      
#print(len(JSON))                                                                                                      
                                                                                                                      
for i in JSON:                                                                                                        
    producer.send('lorawan2', value=i)                                                                                
    sleep(1)                                                                                                          
                                                                                                                      
                                                                                                                      
#stop = timeit.default_timer()                                                                                         
#print('Time: ', stop - start) 

#print out the contents of the JSON list for validation
#for i in JSON:
#	print i
#	print "\n"
