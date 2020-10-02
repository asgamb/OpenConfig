from __future__ import print_function

from concurrent import futures
import time
import os
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
#docker environment
pythonNetconf="/confd/bin/netconf-console"
#development environment
#pythonNetconf="/home/andrea/confd/bin/netconf-console"

def run():
  parser = argparse.ArgumentParser(description='Simple script to subscribe to telemetry service')
  parser.add_argument("s", type=str, help="list of ip:port splitted by # \n 10.30.2.109:50083#10.30.2.24:50084 ")
  parser.add_argument("k", type=str, help="list of paths splitted by # \n /terminal-device/logical-channels/channel[index=10101]/otn/state/pre-fec-ber/instant")
  parser.add_argument("l", type=str, help="Telemetry server IP:port \n  10.30.2.109:50051")
  parser.add_argument("c", type=int, help="1=avoid duplicated data")
  parser.add_argument("i", type=int, help="interval in milliseconds")
  parser.add_argument("d", type=int, help="duration in milliseconds")
  parser.add_argument("b", type=int, help="the subscription_id")
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
  sub=int(args.b)
  channel = grpc.insecure_channel(args.l)
  stub = OCtelemetry_pb2_grpc.OCTelemetryStub(channel)
  response = stub.telemetrySubscribe(
  OCtelemetry_pb2.SubscriptionRequest(suppression=check, interval=intvl,
                                      collectors=colls, resources=keys,
                                      subscription_id=sub, duration=duration))
  #OCtelemetry_pb2.SubscriptionRequest(observation_point="10.30.22.22",template_id=11, duration=dur, suppression=check,interval=intvl,collectors=colls,resources=keys)
  #global subscription_id
  #time.sleep(duration)
  subscription_id=response.id.id
  if duration!=0:
     filex = open('/delete.xml', 'w')
     filex.write("""
<telemetry-system xmlns="http://openconfig.net/yang/telemetry">
    <subscriptions>
     <dynamic-subscriptions>
       <dynamic-subscription operation="delete">
        <id>"""+str(sub)+"""</id>
       </dynamic-subscription>
     </dynamic-subscriptions>
    </subscriptions>
</telemetry-system>""")
     filex.close()
     options=" --proto=tcp --port=2023 --edit-config /delete.xml "
     command="python "+pythonNetconf+options
     time.sleep((duration/1000)+5)
     out=os.popen(command).read()
     response = stub.cancelTelemetrySubscription(OCtelemetry_pb2.SubscriptionId(id=int(subscription_id)))

if __name__ == '__main__':
  run()

