# Copyright 2015, Google Inc.
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are
# met:
#
#     * Redistributions of source code must retain the above copyright
# notice, this list of conditions and the following disclaimer.
#     * Redistributions in binary form must reproduce the above
# copyright notice, this list of conditions and the following disclaimer
# in the documentation and/or other materials provided with the
# distribution.
#     * Neither the name of Google Inc. nor the names of its
# contributors may be used to endorse or promote products derived from
# this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
# "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
# LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
# A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
# OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
# SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
# LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
# DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
# THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.


from concurrent import futures
import time
import os

import grpc
import OCtelemetry_pb2
import OCtelemetry_pb2_grpc
#import telemetry_pb2
#import telemetry_pb2_grpc
import threading
import thread
import random
import argparse

import xml.etree.ElementTree as ET
import urllib2

_ONE_DAY_IN_SECONDS = 60 * 60 * 24
#counter=0
status={}
#development agents
#pythonNetconf="/home/andrea/netconf-agent/confd/bin/netconf-console"
#docker agents
pythonNetconf="/confd/bin/netconf-console"
#check=0
localPort=50051
ipAddr=""

class Telemetry(OCtelemetry_pb2_grpc.OCTelemetryServicer):

  def telemetrySubscribe(self, request, context):
      #print("Subscription request received: " + response.message)
      print("GRPC subscription request "+str(request.subscription_id)+" received!")
      global counter
      #counter=counter+1
      #status[counter]=1
      status[request.subscription_id]=1
      #print status
      #executor=futures.ThreadPoolExecutor(max_workers=2)
      #executor.submit(start_stream())
      #temp_counter=counter
         
      for collector in request.collectors:
        print collector
        try:
           #thread.start_new_thread( start_stream, (temp_counter,collector,request.observation_point, request.resources,request.suppression, request.interval, request.duration, ) )
           thread.start_new_thread( start_stream, (collector, request.subscription_id, request.resources, request.suppression, request.interval, request.duration, ) )
        except:
          print "Error: unable to start thread"
      return OCtelemetry_pb2.SubscriptionResponse(
	id=OCtelemetry_pb2.SubscriptionId(id=int(request.subscription_id)),
	actualSubscription=OCtelemetry_pb2.SubscriptionRequest(collectors=request.collectors,resources=request.resources))


  def cancelTelemetrySubscription(self, request, context):
    print("GRPC Unsubscription received!")
    counter=request.id
    status[counter]=0
    #print status
    del status[counter]
    #print status
    return OCtelemetry_pb2.NoMessage()
     

def parser_data(reply,tags,target):
    tree =ET.ElementTree(ET.fromstring(reply))
    root = tree.getroot()
    for child in root.iter():
       #print child.tag, child.attrib
       if target in child.tag:
          return child.text

#def generate_stream(counterx,resources, check, interval, template, duration, opoint):   
def generate_stream(resources, check, interval, duration, sub):
    #global check
    global ipAddr
    count=1
    options=" --proto=tcp --port=2023 --get-config -x "
    old_vals=[]
    last_count=int(duration)/int(interval)
    #print(str(duration)+" "+str(sub))
    if duration==0:     
      #while (status[counterx]==1):
      while (status[sub]==1):
        temp=[]
        valx=[]
        j=0
        t0=int(round(time.time()*1000))
        for resource in resources:
           path=resource.path.path
           command="python "+pythonNetconf+options+path
           tags=path.split("/")
           target=tags[len(tags)-1]
           out=os.popen(command).read()
           val=parser_data(out,tags,target)
           valx.insert(j, val)
           temp.append(OCtelemetry_pb2.KeyValue(key=path,str_value=val))
           j=j+1
        eq=0
        #if check==1:
        millis = int(round(time.time() * 1000))
        if check==True:
          if len(old_vals)!=0:
             for k in range(len(valx)):
                if valx[k]!=old_vals[k]:
                   eq=1
                   break
             if eq==1:
                #datax1=OCtelemetry_pb2.TelemetryData(system_id=str(count), timestamp=ts, kv=temp)
                if(count==last_count):
                  data=OCtelemetry_pb2.TelemetryData(subscription_id=sub, observation_point=ipAddr, sequence_number=count, last_sample=True, kv=temp, timestamp=millis)
                else: 
                  data=OCtelemetry_pb2.TelemetryData(subscription_id=sub, observation_point=ipAddr, sequence_number=count, last_sample=False, kv=temp, timestamp=millis)
                yield data
          else:
             #datax1=OCtelemetry_pb2.TelemetryData(system_id=str(count), timestamp=ts, kv=temp )
             if(count==last_count):
                  data=OCtelemetry_pb2.TelemetryData(subscription_id=sub, observation_point=ipAddr, sequence_number=count, last_sample=True, kv=temp, timestamp=millis)
             else: 
                  data=OCtelemetry_pb2.TelemetryData(subscription_id=sub, observation_point=ipAddr, sequence_number=count, last_sample=False, kv=temp, timestamp=millis)
             old_vals=valx
             yield data
        else:
            #datax1=OCtelemetry_pb2.TelemetryData(system_id=str(count), timestamp=ts, kv=temp)
            #print "11"
            if(count==last_count):
              #data=OCtelemetry_pb2.TelemetryData(observation_point=str(opoint), sequnce_number=count, template_id=template, last_sample=True, kv=temp)
              data=OCtelemetry_pb2.TelemetryData(subscription_id=sub, last_sample=True, observation_point=ipAddr, sequence_number=count, kv=temp, timestamp=millis)
            else: 
              data=OCtelemetry_pb2.TelemetryData(subscription_id=sub, last_sample=False, observation_point=ipAddr, sequence_number=count, kv=temp, timestamp=millis)
              #data=OCtelemetry_pb2.TelemetryData(observation_point=str(opoint), sequnce_number=count, template_id=template, last_sample=False, kv=temp)
            #print str(data)
            yield data
        count+=1
        #print str(interval)
        t1=int(round(time.time()*1000))
        remaining=int(interval)-(t1-t0)
        rem=float(remaining)/1000.0
        #print str(rem)
        if (rem<0.0):
           continue
        else:
           #time.sleep(int(interval))
           time.sleep(rem)
        #time.sleep(random.uniform(0.5, 1.5))
    else:
      while (count-1!=last_count):
        temp=[]
        valx=[]
        j=0
        t0=int(round(time.time()*1000))
        for resource in resources:
           path=resource.path.path
           command="python "+pythonNetconf+options+path
           tags=path.split("/")
           target=tags[len(tags)-1]
           out=os.popen(command).read()
           val=parser_data(out,tags,target)
           valx.insert(j, val)
           temp.append(OCtelemetry_pb2.KeyValue(key=path,str_value=val))
           #temp.append(OCtelemetry_pb2.KeyValue(key="pre-FecBER"+str(i),str_value="1E-3"+str(count)))
           j=j+1
        eq=0
        #if check==1:
        millis = int(round(time.time() * 1000))
        if check==True:
          if len(old_vals)!=0:
             for k in range(len(valx)):
                if valx[k]!=old_vals[k]:
                   eq=1
                   break
             if eq==1:
                #datax1=OCtelemetry_pb2.TelemetryData(system_id=str(count), timestamp=ts, kv=temp)
                if(count==last_count):
                  #data=OCtelemetry_pb2.TelemetryData(observation_point=str(opoint), sequence_number=count, template_id=template, last_sample=True, kv=temp, timestamp=millis)
                  data=OCtelemetry_pb2.TelemetryData(subscription_id=sub, observation_point=ipAddr, sequence_number=count, last_sample=True, kv=temp, timestamp=millis)
                else: 
                  #data=OCtelemetry_pb2.TelemetryData(observation_point=str(opoint), sequence_number=count, template_id=template, last_sample=False, kv=temp, timestamp=millis)
                  data=OCtelemetry_pb2.TelemetryData(subscription_id=sub, observation_point=ipAddr, sequence_number=count, last_sample=False, kv=temp, timestamp=millis)
                yield data
          else:
             #datax1=OCtelemetry_pb2.TelemetryData(system_id=str(count), timestamp=ts, kv=temp )
             if(count==last_count):
                  #data=OCtelemetry_pb2.TelemetryData(observation_point=str(opoint), sequence_number=count, template_id=template, last_sample=True, kv=temp, timestamp=millis)
                  data=OCtelemetry_pb2.TelemetryData(subscription_id=sub, observation_point=ipAddr, sequence_number=count, last_sample=True, kv=temp, timestamp=millis)
             else: 
                  #data=OCtelemetry_pb2.TelemetryData(observation_point=str(opoint), sequence_number=count, template_id=template, last_sample=False, kv=temp, timestamp=millis)
                  data=OCtelemetry_pb2.TelemetryData(subscription_id=sub, observation_point=ipAddr, sequence_number=count, last_sample=False, kv=temp, timestamp=millis)
             old_vals=valx
             yield data
        else:
            #datax1=OCtelemetry_pb2.TelemetryData(system_id=str(count), timestamp=ts, kv=temp)
            #print "11"
            if(count==last_count):
              #data=OCtelemetry_pb2.TelemetryData(observation_point=str(opoint), sequnce_number=count, template_id=template, last_sample=True, kv=temp)
              data=OCtelemetry_pb2.TelemetryData(subscription_id=sub, observation_point=ipAddr, last_sample=True, sequence_number=count, kv=temp, timestamp=millis)
            else: 
              #data=OCtelemetry_pb2.TelemetryData(observation_point=str(opoint), last_sample=False, sequence_number=count, template_id=template, kv=temp, timestamp=millis)
              data=OCtelemetry_pb2.TelemetryData(subscription_id=sub, observation_point=ipAddr, last_sample=False, sequence_number=count, kv=temp, timestamp=millis)
              #data=OCtelemetry_pb2.TelemetryData(observation_point=str(opoint), sequnce_number=count, template_id=template, last_sample=False, kv=temp)
            #print str(data)
            yield data
        count+=1
        #print str(interval)
        t1=int(round(time.time()*1000))
        remaining=int(interval)-(t1-t0)
        rem=float(remaining)/1000.0
        #print str(rem)
        if (rem<0.0):
           continue
        else:
           #time.sleep(int(interval))
           time.sleep(rem)
        #time.sleep(random.uniform(0.5, 1.5))



#def send_stream(stub, counter,resources,check, interval, template, duration, opoint):
def send_stream(stub, resources,check, interval, duration, sub):
    #data_iterator = generate_stream(counter,resources,check, interval, template, duration, opoint)
    data_iterator = generate_stream(resources,check, interval, duration, sub)
    data_response = stub.StreamData(data_iterator)

#def start_stream(counter,collector, opoint, resources,check, interval, template, duration):
def start_stream(collector, sub, resources, check, interval, duration):
    ip=collector.ip_address
    port=collector.port
    channel = grpc.insecure_channel(ip+":"+str(port))  
    stub = OCtelemetry_pb2_grpc.OCReplyStub(channel)
    #send_stream(stub,counter,resources,check, interval, template, duration, opoint)
    send_stream(stub,resources,check, interval, duration, sub)

def serve():
  global ipAddr
  parser = argparse.ArgumentParser(description='Simple Telemetry server')
  parser.add_argument("p", type=str, help="GRPC server port ")
  parser.add_argument("i", type=str, help="GRPC server ip address ")
  args = parser.parse_args()
  #global check
  #if int(args.c)==1:
  #  check=1
  localPort=args.p
  ipAddr=args.i
  server = grpc.server(futures.ThreadPoolExecutor(max_workers=10))
  OCtelemetry_pb2_grpc.add_OCTelemetryServicer_to_server(Telemetry(), server)
  server.add_insecure_port('[::]:'+str(localPort))
  server.start()
  
  while len(status)==0:
    time.sleep(2) 
  #print "It works"
  '''
  try:
    while True:
      stopping=1
      for state in status.keys():
        #print status[state]
        if status[state]==1:
          stopping=0
          break
      if stopping==0:
         #print "stopping =0"
         time.sleep(5)
      else:
         #print "stopping =1"
         print "All subscriptions are satisfied"
         break
  except KeyboardInterrupt:
    server.stop(0)
  '''
  try:
    while True:
      time.sleep(_ONE_DAY_IN_SECONDS)
  except KeyboardInterrupt:
    server.stop(0)
  

if __name__ == '__main__':
  serve()
