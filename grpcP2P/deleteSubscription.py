from __future__ import print_function

import grpc
import OCtelemetry_pb2_grpc
import OCtelemetry_pb2
#import FGrTelemetry_pb2
#import FGrTelemetry_pb2_grpc
import argparse

serverIP = "10.10.255.101"
serverPrt = 50051
subscription_id = 0


def run():
    global serverIP
    global serverPrt
    global subscription_id

    parser = argparse.ArgumentParser(description='Simple Telemetry subscriber')
    parser.add_argument("-s", "--server", type=str, help="GRPC server ip")
    parser.add_argument("-l", "--listen", type=int, help="GRPC server port listen")
    parser.add_argument("-i", "--id", type=str, help="the subscription id to be deleted")
    args = parser.parse_args()
    if args.listen:
        serverPrt = int(args.listen)
    if args.server:
        serverIP = args.server
    if args.id:
        subscription_id = args.id

    channel = grpc.insecure_channel(serverIP + ":" + str(serverPrt))

    #stub = FGrTelemetry_pb2_grpc.FGrTelemetryStub(channel)
    stub = OCtelemetry_pb2_grpc.OCTelemetryStub(channel)
    response = stub.cancelTelemetrySubscription(OCtelemetry_pb2.SubscriptionId(id=int(subscription_id)))
    #response = stub.cancelTelemetrySubscription(FGrTelemetry_pb2.SubscriptionId(id=int(subscription_id)))


if __name__ == '__main__':
    run()
