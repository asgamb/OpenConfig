//Local port for monitoring socket
#define SERVER_PORT  12346
//parameters of remote socket for the configuration of the transponder
#define TRANSPONDER_ADDR "10.10.255.253"//"10.30.2.24"
#define TRANSPONDER_PORT 16001
//enable the configuration via driver to the transponder
#define CONF_TRANSPONDER 0

//confd lib for docker agents
#define LOCALPATH  ((const unsigned char *)"/confd/bin/")

//confd lib for development agents
//#define LOCALPATH  ((const unsigned char *)"/home/mininet/docker_images/confd/bin/")
#define ENABLE_GRPC 1
#define GRPCPATH  ((const unsigned char *)"/home/mininet/grpc/python/OCtelemetry/")
#define GRPC_SERVER_IP  ((const unsigned char *)"10.30.2.190")
#define GRPC_SERVER_PORT  ((const unsigned char *)"50051")
