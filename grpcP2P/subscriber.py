from __future__ import print_function

import grpc
import OCtelemetry_pb2
import OCtelemetry_pb2_grpc
import argparse

_ONE_DAY_IN_SECONDS = 60 * 60 * 24

subscription = 111
serverIP = "127.0.0.1"
serverPrt = 50051
collIP = "127.0.0.1"
collPrt = 50083
duration = 30000
obs = "xponder1"
template = 1


def run():
    global subscription
    global duration
    global collIP
    global collPrt
    global serverIP
    global serverPrt
    global check
    global obs
    global template
    colls = []
    keys = []
    paths = []
    intvl = 1000
    check = False

    parser = argparse.ArgumentParser(description='Simple Telemetry subscriber')
    parser.add_argument("-p", "--port", type=int, help="GRPC collector port ")
    parser.add_argument("-i", "--ip", type=str, help="GRPC collector ip")
    parser.add_argument("-d", "--duration", type=str, help="Duration of the subscription in seconds")
    parser.add_argument("-k", "--keys", type=str, help="keys to be collected separated by ','")
    parser.add_argument("-s", "--server", type=str, help="GRPC server ip")
    parser.add_argument("-l", "--listen", type=int, help="GRPC server port listen")
    parser.add_argument("-c", "--check", type=int, help="set to 1 to suppress redundant values")
    parser.add_argument("-t", "--time", type=int, help="interval between data in ms")
    parser.add_argument("-o", "--observ", type=str, help="the observation point")
    parser.add_argument("-g", "--group", type=int, help="the template_id")
    parser.add_argument("-r", "--register", type=int, help="subscription id")
    parser.add_argument("-y", "--correlation", type=int, help="correlation enum")
    parser.add_argument("-z", "--positioning", type=int, help="lighthpath positioning")
    args = parser.parse_args()
    if args.check:
        check = int(args.check)
    if args.port:
        collPrt = int(args.port)
    if args.ip:
        collIP = args.ip
    if args.duration:
        if int(args.duration) == 0:
            duration = 0
        else:
            duration = int(args.duration) * 1000
    if args.listen:
        serverPrt = int(args.listen)
    if args.server:
        serverIP = args.server
    if args.time:
        intvl = int(args.time)
    if args.observ:
        obs = args.observ
    if args.group:
        template = int(args.group)
    if args.correlation:
        corr = int(args.correlation)
        '''if int(args.correlation) == 0:
            corr = OCtelemetry_pb2.Correlation.SELF
        elif int(args.correlation) == 1:
            corr = OCtelemetry_pb2.Correlation.NEIGHBOUR
        elif int(args.correlation) == 2:
            corr = OCtelemetry_pb2.Correlation.AGGREGATE
        elif int(args.correlation) == 3:
            corr = OCtelemetry_pb2.Correlation.OTHER'''
    else:
        # corr = OCtelemetry_pb2.Correlation.SELF
        corr = 0
    if args.positioning:
        pos = int(args.positioning)
    else:
        pos = 10
    if args.register:
        subscription = int(args.register)
    listOfkeys = []
    if args.keys:
        listOfkeys = args.keys.split(',')
    else:
        listOfkeys = ["delaySW1", "delaySW2"]
    print("Duration=" + str(duration))
    print("Requested keys=" + str(listOfkeys))
    colls.append(OCtelemetry_pb2.Collector(ip_address=collIP, port=int(collPrt)))
    for elem in listOfkeys:
        keys.append(OCtelemetry_pb2.Resource(path=OCtelemetry_pb2.Path(path=elem)))
    channel = grpc.insecure_channel(serverIP + ":" + str(serverPrt))
    stub = OCtelemetry_pb2_grpc.OCTelemetryStub(channel)
    response = stub.telemetrySubscribe(
        OCtelemetry_pb2.SubscriptionRequest(subscription_id=subscription, duration=duration,
                                            suppression=check, interval=intvl, collectors=colls, resources=keys,
                                            observation_point=obs, template_id=template,
                                            positioning=pos, correlation=corr))
    global subscription_id
    subscription_id = response.id.id
    print("Subscription ID=" + str(subscription_id))


if __name__ == '__main__':
    run()
