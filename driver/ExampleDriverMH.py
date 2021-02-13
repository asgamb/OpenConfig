import string
from socket import socket, AF_INET, SOCK_STREAM
import time 
from threading import *
import threading
import thread
import socket as sock

#Monitoring Socket: IP port of remote Netconf Agent
IP = "10.30.2.190"
PORT = 12346
ADS = (IP, PORT)
#monitoring period
PERIOD=5

#Local server socket for the configuration
confIP='0.0.0.0' 
confPORT=16001 

#Global parameters
screen_lock = Semaphore(value=1) #threading access to the cli

toConnect=1 #enable the connection to the Netconf server for monitoring
enableConfig=1 #enable the configuration feature

FinalUser="SSSA" #supported option SSSA 

#supported monitored parameters
keys=["PRE-FEC-BER","ESNR","Q-VALUE", "FREQUENCY", "CHROMATIC-DISPERSION",]

class requester(threading.Thread):

    def __init__(self):
        super(requester, self).__init__()
        self._stop = threading.Event()
        self.connected = 0
        self.fileConfig='Config.txt'
        self.check_config='checkconfig.dat'
        #socket used fot monitoring purpose
        self.netconfsocket = socket(AF_INET, SOCK_STREAM)
        #socket used for the configuration
        self.configsocket = socket(AF_INET, SOCK_STREAM)
        self.configs=[]
        self.confs=[]
        self.threads=[]
        self.NetconfConnected=0
        self.msgCounter=0
        self.logMessages={}
        self.exit=0
        self.counter=0
        self.check=0
        self.adminstate=""
        self.targetpower=""
        self.frequency=""
        self.operationalmode=""
        self.configured=0
    
    def main(self):
        if toConnect==1:
            t1 = threading.Thread(target=self.connectNetconf, args=())
            t1.start()
            self.threads.append(t1)
        if enableConfig==1:
            t2 = threading.Thread(target=self.receivingConfig, args=())
            t2.start()
            self.threads.append(t2)

    def connectNetconf (self):
        if toConnect==1:
            while(1):
                print "Waiting for Requester to connect... \n"
                while not self.connected:
                    try:
                        self.netconfsocket.connect(ADS)
                        self.connected = 1
                        screen_lock.acquire()
                        print "Connection established"
                        screen_lock.release()
                    except Exception:
                        print "Connection not established"
                        time.sleep(5)
                        self.connected = 0
                break
        while(self.configured==0):
            time.sleep(1)
        for key in keys:
            thread.start_new_thread ( self.loadByKey, (key,) )
        
    def loadByKey(self,key):
        #keys=["PRE-FEC-BER","ESNR","Q-VALUE", "FREQUENCY", "CHROMATIC-DISPERSION",]
        # the syntax is (### is the adopted separator):
        #    - logical channel id or component name +###
        #    - key reported
        #    - instant value +###
        #    - average value id +###
        #    - min value id +###
        #    - Max value id
        #    - && (to close the string)
        while 1:
            if key=="PRE-FEC-BER":
               #    log_channel            key         instant              average            min             Max
               #complete communication
               conn = "11511"    +   "###"+key+"###" + "0.0002"  +  "###"+ "0.0003" + "###" + "0.0001"+"###"+"0.0004"+ "&&"
               #short communication with computation demanded to the agent
               #conn = "11511"    +   "###"+key+"###" + "0.0002"  +  "&&"
               
               print conn
               if toConnect:
                  self.netconfsocket.sendall(conn)
            elif key=="ESNR":
               #    log_channel            key         instant           average          min          Max
               #complete communication
               conn = "10101"    +   "###"+key+"###" + "10.0"  +  "###"+ "10.3" + "###" + "9.8"+"###"+"11.0"+ "&&"
               #short communication with computation demanded to the agent
               #conn = "11511"    +   "###"+key+"###" + "10.0"  + "&&"
               print conn
               if toConnect:
                  self.netconfsocket.sendall(conn)
            elif key=="Q-VALUE":
               #    log_channel            key         instant           average          min           Max
               #complete communication
               conn = "10101"    +   "###"+key+"###" + "10.0"  +  "###"+ "10.3" + "###" + "9.8"+"###"+"11.0"+ "&&"
               #short communication with computation demanded to the agent
               #conn = "11511"    +   "###"+key+"###" + "10.0"  + "&&"
               print conn
               if toConnect:
                  self.netconfsocket.sendall(conn)
            elif key=="CHROMATIC-DISPERSION":
               #    component name              key         instant          average            min          Max
               #complete communication
               conn = "10101"    +   "###"+key+"###" + "10.0"  +  "###"+ "10.3" + "###" + "9.8"+"###"+"11.0"+ "&&"
               #short communication with computation demanded to the agent
               #conn = "11511"    +   "###"+key+"###" + "10.0"  + "&&"
               print conn
               if toConnect:
                  self.netconfsocket.sendall(conn)
            elif key=="FREQUENCY":
               #    component name              key         freq in MHz
               #complete communication
               conn = "10101"    +   "###"+key+"###" + str(self.frequency)+ "&&"
               print conn
               if toConnect:
                  self.netconfsocket.sendall(conn)
                
            time.sleep(PERIOD)            
    
    
    def receivingConfig(self):
      addr = (confIP, int(confPORT))
      try:
            self.configsocket.setsockopt(sock.SOL_SOCKET, sock.SO_REUSEADDR, 1)
            self.configsocket.bind(addr)
      except sock.error as msg:
            self.configsocket.close()
      t3 = threading.Thread(target=self.receiver, args=())
      t3.start()

    def receiver(self):
        screen_lock.acquire()
        print "Running the Netconf Config server\n"
        screen_lock.release()
        self.configsocket.listen(10)
        while 1:
            clientsocket1, clientaddr1 = self.configsocket.accept()
            if (clientsocket1 and clientaddr1):
                self.NetconfConnected=1
                t4 = threading.Thread(target=self.listenerConf, args=(clientsocket1,clientaddr1,))
                t4.start()
            time.sleep(1)
        self.configsocket.close()   

    def listenerConf(self, clientsocket,clientaddr1):
      screen_lock.acquire()
      print "Netconf Connected\n"
      screen_lock.release()       
      while 1:
         try :  
              stream = clientsocket.recv(2048)
              aa=stream.split('$$$')
              aa.pop()
              self.configs.extend(aa)
              while (len(self.configs)>0):
                   data=self.configs.pop(0)
                   self.confs.append(data)
                   screen_lock.release() 
                   [tipo, data1]=data.split('###',1)
                   if (tipo=="CONFIGOC"):
                         [idx, command, data2]=data1.split('###',2)
                         if  command=="ADMINSTATE":
                           self.counter=self.counter+1
                           screen_lock.acquire()
                           print "received AdminState configuration: "+ data1
                           screen_lock.release()
                           self.adminstate=data2
                           if self.check==0:
                               threading.Timer(3,self.write).start()
                               self.check=1
                         if command=="TARGETPOWER":
                           self.counter=self.counter+1
                           screen_lock.acquire()
                           print "received TargetPower configuration: "+ data1
                           screen_lock.release()
                           self.targetpower=data2
                           if self.check==0:
                               threading.Timer(3,self.write).start()
                               self.check=1
                         if command=="FREQUENCY":
                           self.counter=self.counter+1
                           screen_lock.acquire()
                           print "received Frequency configuration: "+ data1
                           screen_lock.release()
                           self.frequency=data2
                           if self.check==0:
                               threading.Timer(3,self.write).start()
                               self.check=1
                         if command=="OPERATIONALMODE":
                           self.counter=self.counter+1
                           screen_lock.acquire()
                           print "received OperationalMode configuration: "+ data1
                           screen_lock.release()
                           self.operationalmode=data2
                           if self.check==0:
                               threading.Timer(3,self.write).start()
                               self.check=1
         except Exception, e:
            print "Requester Disconnected "
            break


    def write(self):
       if (FinalUser=="SSSA"):
           self.configured=1
           print "saving configuration"
           input_file = open(self.fileConfig,"w")
           input_file.write(self.adminstate+"\t% AdminState\n")
           input_file.write(self.targetpower+"\t% TargetPower[dBm]\n")
           input_file.write(self.frequency+"\t% % Frequency [MHz]\n")
           input_file.write(self.operationalmode+"\t% OperationalMode 0:not active; 1 active; 2: maintenance\n")
           input_file.close()
           out_file = open(self.check_config,"w")
           out_file.write("1\n")
           out_file.close()
           self.check=0

z = requester()
z.main()


