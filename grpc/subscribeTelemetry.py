from __future__ import print_function

from concurrent import futures
import time

import grpc
import time

#import telemetry_pb2
#import telemetry_pb2_grpc
import OCtelemetry_pb2
import OCtelemetry_pb2_grpc
import argparse

_ONE_DAY_IN_SECONDS = 60 * 60 * 24

subscription_id=0
serverIP="10.30.2.37"
localIP="10.30.2.24"
def run():
  parser = argparse.ArgumentParser(description='Simple script to subscribe to telemetry service')
  parser.add_argument("s", type=str, help="list of ip:port splitted by # \n 10.30.2.24:50082#10.30.2.24:50083 ")
  parser.add_argument("k", type=str, help="list of paths splitted by # \n /terminal-device/logical-channels/channel[index=11811]/otn/state/pre-fec-ber/instant")
  parser.add_argument("l", type=str, help="Telemetry server IP:port \n  10.30.2.37:50051")
  parser.add_argument("c", type=int, help="1=avoid duplicated data")
  parser.add_argument("i", type=int, help="interval in seconds")
  parser.add_argument("d", type=int, help="duration in seconds")
  args = parser.parse_args()
  socks=args.s.split('#')
  paths=args.k.split('#')
  intvl=int(args.i)
  duration=int(args.d)
  if args.c==1:
    check=True
  else:
    check=False
  colls=[]
  keys=[]
  #print(socks)
  #print(paths)
  for sock in socks:
    addr, prt= sock.split(':')
    colls.append(OCtelemetry_pb2.Collector(ip_address=addr,port=int(prt)))
  #print(colls)
  for path in paths:
    keys.append(OCtelemetry_pb2.Resource(path=OCtelemetry_pb2.Path(path=path)))
  #print(keys)
  channel = grpc.insecure_channel(args.l)
  stub = OCtelemetry_pb2_grpc.OCTelemetryStub(channel)
  response = stub.telemetrySubscribe(
  OCtelemetry_pb2.SubscriptionRequest(suppression=check,interval=intvl,collectors=colls,resources=keys)
  #OCtelemetry_pb2.SubscriptionRequest(
    #collectors=[telemetry_pb2.Collector(ip_address=localIP,port=50082),
    #            telemetry_pb2.Collector(ip_address=localIP,port=50083)],
    #resources=[telemetry_pb2.Resource(path=telemetry_pb2.Path(path="/terminal-device/logical-channels/channel[index=11811]/otn/state/pre-fec-ber/instant")),
    #           telemetry_pb2.Resource(path=telemetry_pb2.Path(path="/terminal-device/logical-channels/channel[index=11811]/otn/state/pre-fec-ber/avg"))]
    #)
  )
  global subscription_id
  subscription_id=response.id.id
  #for collector in response.actualSubscription.collectors:
  #print("Subscription response received: " + str(response.id)+ " "+  response.actualSubscription.collectors.ip_address)
  #print("Subscription response received: " + str(response.id)+ " "+  collector.ip_address)
  time.sleep(duration)
  response = stub.cancelTelemetrySubscription(OCtelemetry_pb2.SubscriptionId(id=int(subscription_id)))



  
if __name__ == '__main__':
  run()

