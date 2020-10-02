from __future__ import print_function

from concurrent import futures
import time

import grpc
import time

import OCtelemetry_pb2
import OCtelemetry_pb2_grpc

_ONE_DAY_IN_SECONDS = 60 * 60 * 24

localPort=50083
class ClientTelemetry(OCtelemetry_pb2_grpc.OCReplyServicer):

  def StreamData(self, request, context):
      #print("Subscription request received: " + response.message)
      for data in request:
        for kv in data.kv:
           print(str(data.timestamp)+" "+str(data.subscription_id)+" "+str(data.last_sample)+" "+str(data.sequence_number)+" "+kv.key+"="+kv.str_value )
      return OCtelemetry_pb2.NoMessage()


def run():
  server = grpc.server(futures.ThreadPoolExecutor(max_workers=10))
  OCtelemetry_pb2_grpc.add_OCReplyServicer_to_server(ClientTelemetry(), server)
  server.add_insecure_port('[::]:'+str(localPort))
  print("Client listening on port "+str(localPort))
  server.start()
  try:
    while True:
      time.sleep(_ONE_DAY_IN_SECONDS)
  except KeyboardInterrupt:
    server.stop(0)

  
if __name__ == '__main__':
  run()
