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
import grpc
import OCtelemetry_pb2
import OCtelemetry_pb2_grpc
import threading
import random
import argparse
import xml.etree.ElementTree as ET

from lumentum import Lumentum
import json
import requests

lumentum = 1
spo = 1
_ONE_DAY_IN_SECONDS = 60 * 60 * 24
status = {}
# development agents
pythonNetconf = "/home/andrea/netconf-agent/confd/bin/netconf-console"
# docker agents
# pythonNetconf="/confd/bin/netconf-console"
# check=0
lport = 50051
ipAddr = ""
head = {"Content-Type": "application/json"}


class Telemetry(OCtelemetry_pb2_grpc.OCTelemetryServicer):

    def telemetrySubscribe(self, request, context):
        print("GRPC subscription request " + str(request.subscription_id) + " received!")
        global counter
        status[request.subscription_id] = 1

        for collector in request.collectors:
            # print collector
            try:
                # thread.start_new_thread( start_stream, (collector, request.subscription_id, request.resources, request.suppression, request.interval, request.duration, request.template_id, request.observation_point, ) )
                threading.Thread(target=start_stream, args=(collector, request,)).start()
                # thread.start_new_thread( start_stream, (collector, request, ) )
            except:
                print("Error: unable to start thread")
        return OCtelemetry_pb2.SubscriptionResponse(
            id=OCtelemetry_pb2.SubscriptionId(id=int(request.subscription_id)),
            actualSubscription=OCtelemetry_pb2.SubscriptionRequest(collectors=request.collectors,
                                                                   resources=request.resources))

    def cancelTelemetrySubscription(self, request, context):
        print("GRPC Unsubscription received!")
        counter = request.id
        status[counter] = 0
        # print status
        del status[counter]
        # print status
        return OCtelemetry_pb2.NoMessage()


def parser_data(reply, tags, target):
    tree = ET.ElementTree(ET.fromstring(reply))
    root = tree.getroot()
    for child in root.iter():
        # print child.tag, child.attrib
        if target in child.tag:
            return child.text


# def generate_stream(resources, check, interval, duration, sub, template, opoint):
def generate_stream(request):
    global lumentum
    global spo
    global head
    NorMaxOSNR = 21.5
    NorMinOSNR = 21.1
    MinInPower = 17.2
    MaxInPower = 16.8
    MinOutPower = 0.4
    MaxOutPower = 0.3
    MaxBER = 0.000007
    MinBER = 0.000003
    count = 1
    # Get values from request
    resources = request.resources
    check = request.suppression
    interval = request.interval
    duration = request.duration
    sub = request.subscription_id
    template = request.template_id
    opoint = request.observation_point
    correlation = request.correlation
    position = request.positioning
    print(correlation)
    print(position)
    old_vals = []
    last_count = int(duration) / int(interval)
    inittime = int(round(time.time() * 1000))
    isLast = False

    l1 = Lumentum(0, '10.10.10.30')
    if duration == 0:
        while (not isLast):
            if status[sub] == 1:
                isLast = True
            temp = []
            valx = []
            j = 0
            t0 = int(round(time.time() * 1000))

            for resource in resources:
                path = resource.path.path
                if "lumentum" in path or "Lumentum" in path:
                    inpow = 0
                    outpow = 0
                    if lumentum == 1:
                        if l1.connect():
                            a = l1.getValues()
                            inpow = a['10']['input']
                            outpow = a['11']['output']
                    else:
                        inpow = round(random.uniform(MaxInPower, MinInPower), 2)
                        outpow = round(random.uniform(MaxOutPower, MinOutPower), 1)
                    temp.append(OCtelemetry_pb2.KeyValue(key=path + "_input", str_value=str(inpow)))
                    valx.insert(j, inpow)
                    j = j + 1
                    temp.append(OCtelemetry_pb2.KeyValue(key=path + "_output", str_value=str(outpow)))
                    valx.insert(j, outpow)
                    j = j + 1
                elif "spo" in path or "SPO" in path:
                    val1 = 0
                    val2 = 0
                    if spo == 1:
                        r = requests.get('http://10.30.2.24:5000/Monitoring/GetPortStats/SPO2/18/11', headers=head)
                        # print r.text
                        out = json.loads(r.text)
                        val1 = out["ber"]
                        val2 = out["osnr"]
                    else:
                        val1 = round(random.uniform(MaxBER, MinBER), 6)
                        val2 = round(random.uniform(NorMaxOSNR, NorMinOSNR), 2)
                    temp.append(OCtelemetry_pb2.KeyValue(key=path + "_BER", str_value=str(val1)))
                    valx.insert(j, val1)
                    j = j + 1
                    temp.append(OCtelemetry_pb2.KeyValue(key=path + "_OSNR", str_value=str(val2)))
                    valx.insert(j, val2)
                    j = j + 1

                elif "input" in path or "Input" in path:
                    inpow = round(random.uniform(MaxInPower, MinInPower), 2)
                    temp.append(OCtelemetry_pb2.KeyValue(key=path, str_value="-" + str(inpow)))
                    valx.insert(j, inpow)
                    j = j + 1
                elif "output" in path or "Output" in path:
                    outpow = round(random.uniform(MaxOutPower, MinOutPower), 1)
                    temp.append(OCtelemetry_pb2.KeyValue(key=path, str_value=str(outpow)))
                    valx.insert(j, outpow)
                    j = j + 1
                elif "osnr" in path or "OSNR" in path:
                    val = round(random.uniform(NorMaxOSNR, NorMinOSNR), 2)
                    temp.append(OCtelemetry_pb2.KeyValue(key=path, str_value=str(val)))
                    valx.insert(j, val)
                    j = j + 1
                elif "ber" in path or "BER" in path:
                    # val=round((random.randint(MinBER, MaxBER)/1000000),5)
                    val = round(random.uniform(MaxBER, MinBER), 6)
                    temp.append(OCtelemetry_pb2.KeyValue(key=path, str_value=str(val)))
                    valx.insert(j, val)
                    j = j + 1
                else:
                    val = round(random.randint(11, 20))
                    temp.append(OCtelemetry_pb2.KeyValue(key=path, str_value=str(val)))
                    valx.insert(j, val)
                    j = j + 1

            eq = 0
            # if check==1:
            millis = int(round(time.time() * 1000))
            if check == True:
                if len(old_vals) != 0:
                    for k in range(len(valx)):
                        if valx[k] != old_vals[k]:
                            eq = 1
                            break
                    if eq == 1:
                        data = OCtelemetry_pb2.TelemetryData(observation_point=str(opoint), sequence_number=count,
                                                             template_id=template, last_sample=isLast, kv=temp,
                                                             timestamp=millis, subscription_id=sub,
                                                             correlation=correlation, positioning=position)
                        old_vals = valx
                        yield data
                else:
                    data = OCtelemetry_pb2.TelemetryData(observation_point=str(opoint), sequence_number=count,
                                                         template_id=template, last_sample=isLast, kv=temp,
                                                         timestamp=millis, subscription_id=sub, correlation=correlation,
                                                         positioning=position)
                    old_vals = valx
                    yield data
            else:
                data = OCtelemetry_pb2.TelemetryData(observation_point=str(opoint), last_sample=isLast,
                                                     sequence_number=count, template_id=template, kv=temp,
                                                     timestamp=millis, subscription_id=sub, correlation=correlation,
                                                     positioning=position)
                yield data
            count += 1
            t1 = int(round(time.time() * 1000))
            remaining = int(interval) - (t1 - t0)
            rem = float(remaining) / 1000.0
            # print str(rem)
            if (rem < 0.0):
                continue
            else:
                time.sleep(rem)
    else:
        while (not isLast):
            temp = []
            valx = []
            j = 0
            t0 = int(round(time.time() * 1000))
            elapsed_time = t0 - inittime
            if duration - elapsed_time - interval <= 0.0:
                isLast = True
            if (count == last_count):
                isLast = True
            for resource in resources:
                path = resource.path.path
                if "lumentum" in path or "Lumentum" in path:
                    inpow = 0
                    outpow = 0
                    if lumentum == 1:
                        if l1.connect():
                            a = l1.getValues()
                            inpow = a['10']['input']
                            outpow = a['11']['output']
                    else:
                        inpow = round(random.uniform(MaxInPower, MinInPower), 2)
                        outpow = round(random.uniform(MaxOutPower, MinOutPower), 1)
                    temp.append(OCtelemetry_pb2.KeyValue(key=path + "input", str_value=str(inpow)))
                    valx.insert(j, inpow)
                    j = j + 1
                    temp.append(OCtelemetry_pb2.KeyValue(key=path + "output", str_value=str(outpow)))
                    valx.insert(j, outpow)
                    j = j + 1
                elif "spo" in path or "SPO" in path:
                    val1 = 0
                    val2 = 0
                    if spo == 1:
                        r = requests.get('http://10.30.2.24:5000/Monitoring/GetPortStats/SPO2/18/11', headers=head)
                        out = json.loads(r.text)
                        val1 = out["ber"]
                        val2 = out["osnr"]
                    else:
                        val1 = round(random.uniform(MaxBER, MinBER), 6)
                        val2 = round(random.uniform(NorMaxOSNR, NorMinOSNR), 2)
                    temp.append(OCtelemetry_pb2.KeyValue(key=path + "_BER", str_value=str(val1)))
                    valx.insert(j, val1)
                    j = j + 1
                    temp.append(OCtelemetry_pb2.KeyValue(key=path + "_OSNR", str_value=str(val2)))
                    valx.insert(j, val2)
                    j = j + 1
                elif "input" in path or "Input" in path:
                    inpow = round(random.uniform(MaxInPower, MinInPower), 2)
                    temp.append(OCtelemetry_pb2.KeyValue(key=path, str_value="-" + str(inpow)))
                    valx.insert(j, inpow)
                    j = j + 1
                elif "output" in path or "Output" in path:
                    outpow = round(random.uniform(MaxOutPower, MinOutPower), 1)
                    temp.append(OCtelemetry_pb2.KeyValue(key=path, str_value=str(outpow)))
                    valx.insert(j, outpow)
                    j = j + 1
                elif "osnr" in path or "OSNR" in path:
                    val = round(random.uniform(NorMaxOSNR, NorMinOSNR), 2)
                    temp.append(OCtelemetry_pb2.KeyValue(key=path, str_value=str(val)))
                    valx.insert(j, val)
                    j = j + 1
                elif "ber" in path or "BER" in path:
                    # val=round((random.randint(MinBER, MaxBER)/1000000),5)
                    val = round(random.uniform(MaxBER, MinBER), 6)
                    temp.append(OCtelemetry_pb2.KeyValue(key=path, str_value=str(val)))
                    valx.insert(j, val)
                    j = j + 1
                else:
                    val = round(random.randint(11, 20))
                    temp.append(OCtelemetry_pb2.KeyValue(key=path, str_value=str(val)))
                    valx.insert(j, val)
                    j = j + 1
            # print(str(valx))
            eq = 0
            millis = int(round(time.time() * 1000))
            print(correlation)
            if check == True:
                if len(old_vals) != 0:
                    for k in range(len(valx)):
                        if valx[k] != old_vals[k]:
                            eq = 1
                            break
                    if eq == 1:
                        data = OCtelemetry_pb2.TelemetryData(observation_point=str(opoint), sequence_number=count,
                                                             template_id=template, last_sample=isLast, kv=temp,
                                                             timestamp=millis, subscription_id=sub,
                                                             correlation=correlation, positioning=position)
                        old_vals = valx
                        yield data
                else:
                    data = OCtelemetry_pb2.TelemetryData(observation_point=str(opoint), sequence_number=count,
                                                         template_id=template, last_sample=isLast, kv=temp,
                                                         timestamp=millis, subscription_id=sub, correlation=correlation,
                                                         positioning=position)
                    old_vals = valx
                    yield data
            else:
                data = OCtelemetry_pb2.TelemetryData(observation_point=str(opoint), last_sample=isLast,
                                                     sequence_number=count, template_id=template, kv=temp,
                                                     timestamp=millis, subscription_id=sub, correlation=correlation,
                                                     positioning=position)
                yield data
            count += 1
            t1 = int(round(time.time() * 1000))
            remaining = int(interval) - (t1 - t0)
            rem = float(remaining) / 1000.0
            # print str(rem)
            if (rem < 0.0):
                continue
            else:
                time.sleep(rem)


# def send_stream(stub, resources,check, interval, duration, sub, template, opoint):
def send_stream(stub, request):
    # data_iterator= generate_stream2(resources, check, interval, duration, sub, template, opoint)
    data_iterator = generate_stream(request)
    data_response = stub.StreamData(data_iterator)


# def start_stream(collector, sub, resources, check, interval, duration, template, opoint):
def start_stream(collector, request):
    # sub, resources, check, interval, duration, template, opoint):
    ip = collector.ip_address
    port = collector.port
    channel = grpc.insecure_channel(ip + ":" + str(port))
    stub = OCtelemetry_pb2_grpc.OCReplyStub(channel)
    # send_stream(stub,resources,check, interval, duration, sub, template, opoint)
    send_stream(stub, request)


def serve():
    global ipAddr
    parser = argparse.ArgumentParser(description='Simple Telemetry server')
    parser.add_argument("p", type=str, default=str(lport), help="GRPC server port ", nargs='?')
    parser.add_argument("i", type=str, default="127.0.0.1", help="GRPC server ip address ", nargs='?')
    args = parser.parse_args()
    localPort = args.p
    ipAddr = args.i
    print("GRPC server running on port " + localPort)
    server = grpc.server(futures.ThreadPoolExecutor(max_workers=10))
    OCtelemetry_pb2_grpc.add_OCTelemetryServicer_to_server(Telemetry(), server)
    server.add_insecure_port('[::]:' + str(localPort))
    server.start()

    while len(status) == 0:
        time.sleep(2)
    try:
        while True:
            time.sleep(_ONE_DAY_IN_SECONDS)
    except KeyboardInterrupt:
        server.stop(0)


if __name__ == '__main__':
    serve()
