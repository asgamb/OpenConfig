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

"""The Python implementation of the GRPC helloworld.Greeter client."""

from __future__ import print_function

from concurrent import futures
import grpc
import time
import OCtelemetry_pb2
import OCtelemetry_pb2_grpc

_ONE_DAY_IN_SECONDS = 60 * 60 * 24

localPort = 50083


class ClientTelemetry(OCtelemetry_pb2_grpc.OCReplyServicer):

    def StreamData(self, request, context):
        print("Subscription request received")
        for data in request:
            for kv in data.kv:
                print(str(data.timestamp) + " " + str(data.observation_point) + " " + str(data.template_id) + " " + str(
                    data.subscription_id) + " " + str(data.last_sample) + " " + str(
                    data.sequence_number) + " " + kv.key + "=" + kv.str_value + " " + str(data.correlation) + " " + str(
                    data.positioning))
        return OCtelemetry_pb2.NoMessage()


def run():
    server = grpc.server(futures.ThreadPoolExecutor(max_workers=10))
    OCtelemetry_pb2_grpc.add_OCReplyServicer_to_server(ClientTelemetry(), server)
    server.add_insecure_port('[::]:' + str(localPort))
    print("Client listening on port " + str(localPort))
    server.start()
    try:
        while True:
            time.sleep(_ONE_DAY_IN_SECONDS)
    except KeyboardInterrupt:
        server.stop(0)


if __name__ == '__main__':
    run()
