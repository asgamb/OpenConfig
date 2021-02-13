///////////////////////////////////////////////
//                Copyright
//          ConfD OpenConfig agent  
//                  developed by
//              Andrea Sgambelluri
////////////////////////////////////////////////
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/poll.h>
#include <time.h>
#include <sys/time.h>
#include <inttypes.h>

#include <confd_lib.h>
#include <confd_dp.h>
#include <confd_cdb.h>
#include <confd_maapi.h>

#include <pthread.h>
#include <signal.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <math.h>

#include <sys/inotify.h>

#include "openconfig-terminal-device.h"
#include "openconfig-telemetry.h"

#include "transponder_driver.h"
#include "uthash.h"
#include "utlist.h"
#include "parameters.h"

#define OK(val) (assert((val) == CONFD_OK))

// Returns number of elements in the array x
#define NELEMS(x)  (sizeof(x) / sizeof((x)[0]))

#define TRUE  1
#define FALSE 0


static char *user = "admin";
static const char *groups[] = {"admin"};
static char *context = "maapi";
static char *confd_addr = "127.0.0.1";

static pthread_t spo_thread_id;
pthread_mutex_t running_mutex = PTHREAD_MUTEX_INITIALIZER;

static pthread_t grpc_thread_id;

static pthread_t copy_pthread;


static volatile int keepRunning = 1;
static int threadsKeepRunning = 1;

static int ctlsock, workersock, cdb_sock, sub_sock, maapis;
static struct confd_daemon_ctx *deamon_p;

struct thread_args {
  char path[BUFSIZ];
  int32_t sample_frequency;
};

typedef struct pthread_t_ll { //linked list of thread ids
    pthread_t thread_id;
    struct pthread_t_ll *next, *prev;
} pthread_t_ll;

struct subscriber {
    int sub_id;        /* key */
    pthread_t_ll *thread_ids;
    UT_hash_handle hh;  /* makes this structure hashable */
};

struct key_value {
  char key[BUFSIZ];
  char value[BUFSIZ];
};

struct subscriber *tel_subs_htable = NULL;    /* important! initialize to NULL */

static int copy_thread_td_activated=0;
static int copy_thread_c_activated=0;

struct copy {
  char root[BUFSIZ];
  uint32_t index;
  char name[BUFSIZ];;
};


struct notif {
    struct confd_datetime eventTime;
    confd_tag_value_t *vals;
    int nvals;
};

struct subscription {
  char port[BUFSIZ];
  char sample_interval[BUFSIZ];
  char heartbeat_interval[BUFSIZ];
  char suppress_redundant[BUFSIZ];
  char destination_address[BUFSIZ];
  char protocol[BUFSIZ];
  char encoding[BUFSIZ];
  char path[BUFSIZ];
  uint8_t init;
};

struct subscription subscriptions[200];

/*
//ber values
float bersum=0.0f;
float beravg=0.0f;
float bermax=0.0f;
float bermin=0.0f;
float count_ber=0.0;
//q values
float qsum=0.0f;
float qavg=0.0f;
float qmax=0.0f;
float qmin=0.0f;
float count_q=0.0;
//cd values
float cdsum=0.0f;
float cdavg=0.0f;
float cdmax=0.0f;
float cdmin=0.0f;
float count_cd=0.0;
//q values
float esnrsum=0.0f;
float esnravg=0.0f;
float esnrmax=0.0f;
float esnrmin=0.0f;
float count_esnr=0.0;
*/


//ber values
float bersum[20000] = {0.0f};
float beravg[20000] = {0.0f};
float bermax[20000] = {0.0f};
float bermin[20000] = {0.0f};
float count_ber[20000] = {0.0f};
//q values
float qsum[20000] = {0.0f};
float qavg[20000] = {0.0f};
float qmax[20000] = {0.0f};
float qmin[20000] = {0.0f};
float count_q[20000] = {0.0f};
//cd values
float cdsum[20000] = {0.0f};
float cdavg[20000] = {0.0f};
float cdmax[20000] = {0.0f};
float cdmin[20000] = {0.0f};
float count_cd[20000] = {0.0f};
//q values
float esnrsum[20000] = {0.0f};
float esnravg[20000] = {0.0f};
float esnrmax[20000] = {0.0f};
float esnrmin[20000] = {0.0f};
float count_esnr[20000] = {0.0f};


struct Floats {
    float favg;
    float fmin;
    float fmax;
};


static pthread_t grpc_sub_thread_id;

struct struct_subscription {
  uint64_t id;
};

/* Our replay buffer is kept in memory in this example.  It's a circular
 * buffer of struct notif.
 */
#define MAX_BUFFERED_NOTIFS 4
static struct notif replay_buffer[MAX_BUFFERED_NOTIFS];
static unsigned int first_replay_idx = 0;
static unsigned int next_replay_idx = 0;

static struct confd_datetime replay_creation;

#define MAX_REPLAYS 10
struct replay {
    int active;
    int started;
    unsigned int idx;
    struct confd_notification_ctx *ctx;
    struct confd_datetime start;
    struct confd_datetime stop;
    int has_stop;
};
/* Keep tracks of active replays */
static struct replay replay[MAX_REPLAYS];

void intHandler(int dummy) {
    int status;
    int status1;
    keepRunning = 0;
    status = pthread_kill( spo_thread_id, SIGUSR1);                                     
    if ( status <  0)                                                              
      perror("pthread_kill failed");
    status1 = pthread_kill( grpc_thread_id, SIGUSR1); 
    if ( status1 <  0)                                                              
      perror("pthread_kill failed");
}


static int GetSock(struct addrinfo *addr_p, enum confd_sock_type sock_type) {
    int sock;

    if ((sock =
         socket(addr_p->ai_family, addr_p->ai_socktype, addr_p->ai_protocol)) < 0)
        return -1;
    if (confd_connect(deamon_p, sock, sock_type,
                      addr_p->ai_addr, addr_p->ai_addrlen) != CONFD_OK) {
        close(sock);
        return -1;
    }
    return sock;
}

#define GetCtrlSock(addr_p) GetSock(addr_p, CONTROL_SOCKET)
#define GetWorkerSock(addr_p) GetSock(addr_p, WORKER_SOCKET)


int GetCdbSock(struct sockaddr_in *addr_p, enum cdb_sock_type s_type) {
  int sock;
  if ((sock = socket(PF_INET, SOCK_STREAM, 0)) < 0)
    return -1;

  if (cdb_connect(sock, s_type, (struct sockaddr *)addr_p,
                 sizeof(struct sockaddr_in)) != CONFD_OK) {
    close(sock);
    return -1;
  }
  return sock;
}

#define GetCdbDataSock(addr_p) GetCdbSock(addr_p, CDB_DATA_SOCKET)
#define GetCdbSubSock(addr_p) GetCdbSock(addr_p, CDB_SUBSCRIPTION_SOCKET)

static int GetMaapiSock(struct sockaddr_in *addr_p) {
    int maapi_sock;
    struct confd_ip ip;

    if ((maapi_sock = socket(PF_INET, SOCK_STREAM, 0)) < 0 )
        confd_fatal("Failed to open socket\n");

    if (maapi_connect(maapi_sock, (struct sockaddr*)addr_p,
                      sizeof (struct sockaddr_in)) < 0)
        confd_fatal("Failed to confd_connect() to confd \n");

    ip.af = AF_INET;
    inet_pton(AF_INET, confd_addr, &ip.ip.v4);

    OK(maapi_start_user_session(maapi_sock, user, context, groups, 1,
                                &ip, CONFD_PROTO_TCP));

    return maapi_sock;
}


static void getdatetime(struct confd_datetime *datetime)
{
    struct tm tm;
    struct timeval tv;

    gettimeofday(&tv, NULL);
    gmtime_r(&tv.tv_sec, &tm);

    memset(datetime, 0, sizeof(*datetime));
    datetime->year = 1900 + tm.tm_year;
    datetime->month = tm.tm_mon + 1;
    datetime->day = tm.tm_mday;
    datetime->sec = tm.tm_sec;
    datetime->micro = tv.tv_usec;
    datetime->timezone = 0;
    datetime->timezone_minutes = 0;
    datetime->hour = tm.tm_hour;
    datetime->min = tm.tm_min;
}


///////////////////////////////////////////////
// Split a string based on a delimiter.
// The result is returned in ret_str which
// is an array of strings.
// The function returns the number of strings
// that has been found.
////////////////////////////////////////////////
static int string_split(char ret_str[100][1000], char * input_str, char * delimiter_str) {
    char *token = strtok(input_str, delimiter_str);
    int jj=0;
    while(token) {
        strcpy(ret_str[jj], token);
        jj++;
        token = strtok(NULL, delimiter_str);
    }
    return jj;
}




static int datetime_le(struct confd_datetime *a, struct confd_datetime *b)
{
    unsigned int ax, bx;
    if (a->year < b->year) { return 1; }
    if (a->year>b->year) { return 0;}
    if (a->month<b->month) { return 1; }
    if (a->month>b->month) { return 0;}
    if (a->day<b->day) { return 1;}
    if (a->day>b->day) { return 0;}
    ax = a->hour - a->timezone;
    bx = b->hour - b->timezone;
    if (ax<bx) { return 1;}
    if (ax>bx) { return 0;}
    ax = a->min - a->timezone_minutes;
    bx = b->min - b->timezone_minutes;
    if (ax<bx) { return 1;}
    if (ax>bx) { return 0;}
    if (a->sec<b->sec) { return 1;}
    if (a->sec>b->sec) { return 0;}
    if (a->micro<b->micro) { return 1;}
    if (a->micro>b->micro) { return 0;}
    return 1;
}



static int continue_replay(struct replay *r)
{
    int done = 0;

    if (!r->started) {
        /* search for first notif to send */
        r->idx = first_replay_idx;
        do {
            if (datetime_le(&r->start, &replay_buffer[r->idx].eventTime))
                break;
            r->idx = (r->idx + 1) % MAX_BUFFERED_NOTIFS;
        } while (r->idx != next_replay_idx);
        r->started = 1;
    }

    /* send one until stop time */
    if (!r->has_stop ||
        datetime_le(&replay_buffer[r->idx].eventTime, &r->stop)) {
        printf("sending notif %d\n", r->idx);
        OK(confd_notification_send(r->ctx,
                                   &replay_buffer[r->idx].eventTime,
                                   replay_buffer[r->idx].vals,
                                   replay_buffer[r->idx].nvals));
        r->idx = (r->idx + 1) % MAX_BUFFERED_NOTIFS;
        if (r->idx == next_replay_idx)
            done = 1;
    } else
        done = 1;

    if (done) {
        /* tell ConfD we're done with replay */
        OK(confd_notification_replay_complete(r->ctx));
        r->active = 0;
    }
    return CONFD_OK;
}


/*
static int cdb_conn_get_value(confd_value_t *ret_val_ptr, char const* const path) {
    struct sockaddr_in addr;
    int rsock, st = CONFD_OK;

    addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    addr.sin_family = AF_INET;
    addr.sin_port = htons(CONFD_PORT);

    if ((rsock = socket(PF_INET, SOCK_STREAM, 0)) < 0 )
        return CONFD_ERR;
    if (cdb_connect(rsock, CDB_READ_SOCKET, (struct sockaddr*)&addr,
                    sizeof (struct sockaddr_in)) < 0)
        return CONFD_ERR;
    if (cdb_start_session(rsock, CDB_RUNNING) != CONFD_OK)
        return CONFD_ERR;

    // set to transponder namespace
    cdb_set_namespace(rsock, oc_opt_term__ns);
    
    //st = cdb_get_str(rsock, buf, BUFSIZ, tmppath); 
    st = cdb_get(rsock, ret_val_ptr, path);
    //st = cdb_get_decimal64(rsock, ret_val_ptr, path);

    cdb_end_session(rsock),
    cdb_close(rsock);
    return st;
}
*/

int updateDb(char *path, char *value_str) {
    int transaction_id;

    if ((transaction_id = maapi_start_trans(maapis, CONFD_RUNNING, CONFD_READ_WRITE)) < 0) {
        confd_fatal("failed to start transition \n");
        return -1;
    }

    maapi_set_elem2(maapis, transaction_id, value_str, path);

    if (maapi_apply_trans(maapis, transaction_id, 0) != CONFD_OK) {
        confd_fatal("failed to apply the transition \n");
        return -1;
    }

    maapi_finish_trans(maapis,transaction_id);

    return 0;
}



static void* copythread(void *vargp) {
    struct copy *thr_args = vargp;
    char rootx[BUFSIZ];
    strcpy(rootx, thr_args->root);
    
    //strcpy(value_str, thr_args->value);   
    //confd_value_t ret_val;
    //int i = 0;
    usleep( 2000000 );
    int transaction_id;

    if (strcmp(rootx,"terminal-device")==0 ){
        int index=thr_args->index;
    
        if ((transaction_id = maapi_start_trans(maapis, CONFD_RUNNING, CONFD_READ_WRITE)) < 0) {
                confd_fatal("failed to start transition \n");
                //return -1;
        }
        char from[BUFSIZ], to[BUFSIZ];// pathx[BUFSIZ];
        //confd_value_t ret_val;
        sprintf(from,"/terminal-device/logical-channels/channel{%d}/config/", index);
        sprintf(to,"/terminal-device/logical-channels/channel{%d}/state/", index);
        maapi_copy_tree(maapis, transaction_id, from, to);


        if (maapi_apply_trans(maapis, transaction_id, 0) != CONFD_OK) {
            confd_fatal("failed to apply the transition \n");
            //return -1;
        }
        maapi_finish_trans(maapis,transaction_id);
        if ((transaction_id = maapi_start_trans(maapis, CONFD_RUNNING, CONFD_READ_WRITE)) < 0) {
                confd_fatal("failed to start transition \n");
                //return -1;
        }
        copy_thread_td_activated=0;
    }else
    if (strcmp(rootx,"components")==0 ){
        usleep( 500000 );
        char namex[BUFSIZ];
        strcpy(namex, thr_args->name);
    
        if ((transaction_id = maapi_start_trans(maapis, CONFD_RUNNING, CONFD_READ_WRITE)) < 0) {
                confd_fatal("failed to start transition \n");
                //return -1;
        }
        char from[BUFSIZ], to[BUFSIZ];// pathx[BUFSIZ];
        //confd_value_t ret_val;
        sprintf(from,"/components/component{%s}/optical-channel/config/", namex);
        sprintf(to,"/components/component{%s}/optical-channel/state/", namex);
        maapi_copy_tree(maapis, transaction_id, from, to);


        if (maapi_apply_trans(maapis, transaction_id, 0) != CONFD_OK) {
            confd_fatal("failed to apply the transition \n");
            //return -1;
        }
        maapi_finish_trans(maapis,transaction_id);
        if ((transaction_id = maapi_start_trans(maapis, CONFD_RUNNING, CONFD_READ_WRITE)) < 0) {
                confd_fatal("failed to start transition \n");
                //return -1;
        }
        copy_thread_c_activated=0;
    }

        /*
        //get direction
        sprintf(pathx,"/xponder-ne/transponder{%d}/config/direction/", tp_id);
        if (cdb_conn_get_value(&ret_val,pathx) != CONFD_OK) {
        //if (cdb_conn_get_value(&ret_val,"/finite-state-machine/states/state[0]/events/event[0]/reaction/operation[0]/simple/local-address") != CONFD_OK) {
            perror("error getting direction");
        }
        char direction[BUFSIZ];
        confd_pp_value(direction, BUFSIZ, &ret_val);
        fprintf(stderr, "Value of direction: %s \n", direction);
        if ((strcmp(direction,"enum<1>")==0) || (strcmp(direction,"enum<2>")==0)){
            char path[BUFSIZ];
            //osnr
            sprintf(path,"/xponder-ne/transponder{%d}/state/receiver/osnr/", tp_id);
            maapi_create(maapis, transaction_id, path);
            maapi_set_elem2(maapis, transaction_id, "0.0", path);
            //pmd
            sprintf(path,"/xponder-ne/transponder{%d}/state/receiver/pmd/", tp_id);
            maapi_create(maapis, transaction_id, path);
            maapi_set_elem2(maapis, transaction_id, "0.0", path);
            //cd
            sprintf(path,"/xponder-ne/transponder{%d}/state/receiver/cd/", tp_id);
            maapi_create(maapis, transaction_id, path);
            maapi_set_elem2(maapis, transaction_id, "0.0", path);
            //snr
            sprintf(path,"/xponder-ne/transponder{%d}/state/receiver/snr/", tp_id);
            maapi_create(maapis, transaction_id, path);
            maapi_set_elem2(maapis, transaction_id, "0.0", path);
            //q-factor
            sprintf(path,"/xponder-ne/transponder{%d}/state/receiver/q-factor/", tp_id);
            maapi_create(maapis, transaction_id, path);
            maapi_set_elem2(maapis, transaction_id, "0.0", path);
            //input-power
            //sprintf(path,"/xponder-ne/transponder{%d}/state/receiver/input-power/", tp_id);
            //maapi_create(maapis, transaction_id, path);
            //maapi_set_elem2(maapis, transaction_id, "0.0",path);
            //pre-fec-ber
            sprintf(path,"/xponder-ne/transponder{%d}/state/receiver/pre-fec-ber/", tp_id);
            maapi_create(maapis, transaction_id, path);
            maapi_set_elem2(maapis, transaction_id, "0.0", path);
        }
        
        if (maapi_apply_trans(maapis, transaction_id, 0) != CONFD_OK) {
            confd_fatal("failed to apply the transition \n");
            //return -1;
        }

        maapi_finish_trans(maapis,transaction_id);
        */
        
        
        //return 0;
   
    return NULL;
    //else
    //    return -1;
}




static void callbacks_registration(struct confd_daemon_ctx *dctx_ptr) {
    // -- REGISTER NOTIFICATION Callback -----
    //register_transponder_stream_notification(dctx_ptr);

    //----- REGISTER RPC Callback -----
    //register_outage_record_rpc(dctx_ptr);
    //register_telemetry_subscribe_rpc(dctx_ptr);

    // finish to register callbacks
    if (confd_register_done(dctx_ptr) != CONFD_OK)  {
        confd_fatal("Failed to complete callback registration \n");
    }
}


//GRPC
/*
struct struct_subscription {
  uint64_t id;
};
*/
void *grpc_server(void *vargp) {

    char command[BUFSIZ];
    sprintf(command,"pkill -9 python");
    system(command);
    sprintf(command,"python2.7 %sserver_telemetry3.0.py %s %s", GRPCPATH, GRPC_SERVER_PORT, GRPC_SERVER_IP);
    //sprintf(commandtx,"python %snetconf-console --proto=tcp --port=2023 --host=%s --user=admin --password=admin --edit-conf  '%s'", local_path, r_addr,"/conf.xml");
    system(command);
    return NULL;
}
/*
struct subscription {
  char port[BUFSIZ];
  char sample_interval[BUFSIZ];
  char heartbeat_interval[BUFSIZ];
  char suppress_redundant[BUFSIZ];
  char destination_address[BUFSIZ];
  char path[BUFSIZ];
  uint8_t init;
};
subscribe inputs
  s           list of ip:port splitted by # 10.30.2.24:50082#10.30.2.24:50083
  k           list of paths splitted by # /terminal-device/logical-
              channels/channel[index=11811]/otn/state/pre-fec-ber/instant
  l           Telemetry server IP:port 10.30.2.37:50051
  c           1=avoid duplicated data
  i           interval in seconds

*/

void *suscribe_grpc(void *vargp) {

    struct struct_subscription *thr_args = vargp;
    uint64_t idy=thr_args->id;
    char command[BUFSIZ];
    usleep( 1000000 );

    if ((strcmp(subscriptions[idy].protocol,"STREAM_GRPC") == 0)&&(strcmp(subscriptions[idy].encoding,"ENC_PROTO3") == 0)){
        if(strcmp(subscriptions[idy].suppress_redundant,"true") == 0){
            sprintf(command,"python2.7 %ssubscribeTelemetry3.0.py %s:%s %s %s:%s 1 %s %s %"PRIu64" ", GRPCPATH, subscriptions[idy].destination_address, subscriptions[idy].port, subscriptions[idy].path, GRPC_SERVER_IP, GRPC_SERVER_PORT, subscriptions[idy].heartbeat_interval, subscriptions[idy].sample_interval, idy);
        }
        else{
            sprintf(command,"python2.7 %ssubscribeTelemetry3.0.py %s:%s %s %s:%s 0 %s %s %"PRIu64" ", GRPCPATH, subscriptions[idy].destination_address, subscriptions[idy].port, subscriptions[idy].path, GRPC_SERVER_IP, GRPC_SERVER_PORT, subscriptions[idy].heartbeat_interval, subscriptions[idy].sample_interval, idy);
        }
        //sprintf(commandtx,"python %snetconf-console --proto=tcp --port=2023 --host=%s --user=admin --password=admin --edit-conf  '%s'", local_path, r_addr,"/conf.xml");
        //fprintf(stderr, "Command: %s\n", command);
        system(command);

    }
    else{
        fprintf(stderr, "Not GRPC or Proto buf v3\n");
    }
    return NULL;
}




// Function called each time a database change occurs
static enum cdb_iter_ret Iter(confd_hkeypath_t *kp,
                              enum cdb_iter_op operation,
                              confd_value_t *oldv,
                              confd_value_t *newv,
                              void *state) {
    char buf[BUFSIZ];
    //int cdbsock = *((int *)state);
    confd_pp_kpath(buf, BUFSIZ, kp);
    // keypath is
    // /terminal-device/logical-channels/channel{$key}/config/....
    //        len-1         -2            -3      -4    -5
    // /components/component{$key}/optical-channel/config/...
    //    len-1       -2      -3       -4            -5
    
    switch (operation) {
                case MOP_CREATED:
                    fprintf(stderr, "Create: %s\n", buf);
                break;
                case MOP_MODIFIED: {
                        fprintf(stderr, "Modified %s\n", buf);
                }
                break;
                case MOP_VALUE_SET: {
                    confd_value_t *root = &kp->v[kp->len-1][0];
                    //conversion to string of new value
                    char newval[BUFSIZ];
                    confd_pp_value(newval, BUFSIZ, newv);
                    fprintf(stderr, "Value Set: %s --> (%s)\n", buf, newval);
                    //get root value
                    char root_char[BUFSIZ];
                    confd_pp_value(root_char, BUFSIZ, root);
                    //case root value is terminal-device
                    if (strcmp(root_char,"terminal-device") == 0){
                        uint32_t lchannel = CONFD_GET_UINT32(&kp->v[kp->len-4][0]);
                        //fprintf(stderr, "Key is t: %d\n", lchannel);
                        //we match the tag of the value that has been set
                        confd_value_t *ctag = &kp->v[kp->len-6][0];
                        switch (CONFD_GET_XMLTAG(ctag)) {
                            //admin-state
                            case oc_opt_term_admin_state:  {
                                //if TX
                                if (strcmp(newval,"enum<0>") == 0){
                                    if(CONF_TRANSPONDER==1)
                                        lch_set_admin_state(lchannel,"ENABLE");
                                }
                                else if (strcmp(newval,"enum<1>") == 0){
                                    if(CONF_TRANSPONDER==1)
                                        lch_set_admin_state(lchannel,"DISABLE");
                                }
                                struct copy *thr_args=  malloc(sizeof(struct copy)); 
                                strcpy(thr_args->root, root_char);
                                thr_args->index=lchannel;
                                if (copy_thread_td_activated==0){
                                    copy_thread_td_activated=1;
                                    pthread_create(&copy_pthread, NULL, copythread, thr_args);
                                }
                            }
                            break;
                        }
                    }else
                    //case root value is components
                    if(strcmp(root_char,"components") == 0){
                        char name[BUFSIZ];
                        strcpy(name, (char*)CONFD_GET_BUFPTR(&kp->v[kp->len-3][0]));
                        //fprintf(stderr, "Key is t: %s\n", name);
                        confd_value_t *ctag = &kp->v[kp->len-6][0];
                        switch (CONFD_GET_XMLTAG(ctag)) {
                            //target-output-power
			    /*
                            case oc_opt_term_target_output_power:  {
                                if(CONF_TRANSPONDER==1)
                                    component_set_target_power(name,newval);
                                struct copy *thr_args=  malloc(sizeof(struct copy)); 
                                strcpy(thr_args->root, root_char);
                                strcpy(thr_args->name, name);
                                if (copy_thread_c_activated==0){
                                    copy_thread_c_activated=1;
                                    pthread_create(&copy_pthread, NULL, copythread, thr_args);
                                }
                            }
                            break;
                            */
                            //frequency
			                case oc_opt_term_frequency:  {
                                if(CONF_TRANSPONDER==1)
                                    component_set_frequency(name,newval);
                                struct copy *thr_args=  malloc(sizeof(struct copy)); 
                                strcpy(thr_args->root, root_char);
                                strcpy(thr_args->name, name);
                                if (copy_thread_c_activated==0){
                                    copy_thread_c_activated=1;
                                    pthread_create(&copy_pthread, NULL, copythread, thr_args);
                                }
                            }
                            break;
                            //operational-mode
                            case oc_opt_term_operational_mode:  {
                                if(CONF_TRANSPONDER==1)
                                    component_set_operational_mode(name,newval);
                            }
                            break;
                        }
                    }
                    /*
    telemetry-system/subscriptions/dynamic-subscriptions/dynamic-subscription[id]/state/destination
    //    len-1          -2               -3                     -4           -5    -6    -7
    telemetry-system/subscriptions/dynamic-subscriptions/dynamic-subscription[id]/sensor-paths/sensor-path[path]/state/path
    //    len-1          -2               -3                     -4           -5    -6             -7       -8    -9    -10

    /telemetry-system/subscriptions/dynamic-subscriptions/dynamic-subscription{1}/state/heartbeat-interval --> (3)
    /telemetry-system/subscriptions/dynamic-subscriptions/dynamic-subscription{1}/state/suppress-redundant --> (true)
    /telemetry-system/subscriptions/dynamic-subscriptions/dynamic-subscription{1}/state/destination-port --> (50082)
    /telemetry-system/subscriptions/dynamic-subscriptions/dynamic-subscription{1}/state/sample-interval --> (300)
    /telemetry-system/subscriptions/dynamic-subscriptions/dynamic-subscription{1}/state/destination-address --> (10.30.2.24)



                    */
                    if(strcmp(root_char,"telemetry-system") == 0){
                        uint64_t id = CONFD_GET_UINT64(&kp->v[kp->len-5][0]);
                        //fprintf(stderr, "ID is: %d\n", id);
                        
                        confd_value_t *ctag = &kp->v[kp->len-6][0];
                        switch (CONFD_GET_XMLTAG(ctag)) {
                            
                            case oc_telemetry_state:  {
                                confd_value_t *ctag2 = &kp->v[kp->len-7][0];
                                switch (CONFD_GET_XMLTAG(ctag2)) {
                                    case oc_telemetry_heartbeat_interval:  {
                                        strcpy(subscriptions[id].heartbeat_interval, newval);
                                    }
                                    break;
                                    case oc_telemetry_suppress_redundant:  {
                                        strcpy(subscriptions[id].suppress_redundant, newval);
                                    }
                                    break;
                                    case oc_telemetry_destination_port:  {
                                        strcpy(subscriptions[id].port, newval);
                                    }
                                    break;
                                    case oc_telemetry_sample_interval:  {
                                        strcpy(subscriptions[id].sample_interval, newval);
                                    }
                                    break;
                                    case oc_telemetry_destination_address:  {
                                        strcpy(subscriptions[id].destination_address, newval);
                                        struct struct_subscription *thr_args1=  malloc(sizeof(struct struct_subscription)); 
                                        thr_args1->id=id;
                                        //fprintf(stderr, "thread enabling subscription %d\n", id);
                                        pthread_create(&grpc_sub_thread_id, NULL, suscribe_grpc, thr_args1);
                                        //fprintf(stderr, "thread enabled\n");
                                    }
                                    break;
                                    case oc_telemetry_encoding:  {
                                        strcpy(subscriptions[id].encoding, newval);
                                    }
                                    break;
                                    case oc_telemetry_protocol:  {
                                        strcpy(subscriptions[id].protocol, newval);
                                    }
                                    break;
                                }

                            }
                            break;
                            case oc_telemetry_sensor_paths:  {
                                confd_value_t *ctag2 = &kp->v[kp->len-9][0];

                                switch (CONFD_GET_XMLTAG(ctag2)) {
                                    case oc_telemetry_state:  {
                                        char path[BUFSIZ];
                                        if (subscriptions[id].init!=1){
                                            //if (strcmp(subscriptions[id].path,"") == 0 ) {
                                            strcpy(path, (char*)CONFD_GET_BUFPTR(&kp->v[kp->len-8][0]));
                                            strcpy(subscriptions[id].path, path);
                                            subscriptions[id].init=1;
                                        }
                                        else{
                                            sprintf(path,"%s#%s", subscriptions[id].path,(char*)CONFD_GET_BUFPTR(&kp->v[kp->len-8][0]));                                    
                                            strcpy(subscriptions[id].path, path);
                                        }
                                        //fprintf(stderr, "The paths are: %s\n", subscriptions[id].path);
                                    }
                                    break;
                                }
                            }
                            break;
                        }
                        
                    }
                }
                break;
                case MOP_DELETED:
                    fprintf(stderr, "Delete: %s\n", buf);
                break;
                default:
                    /* We should never get MOP_MOVED_AFTER or MOP_ATTR_SET */
                    fprintf(stderr, "Unexpected operation %d for %s\n", operation, buf);
                      break;
    }
    return ITER_RECURSE;
}

static int strConcat(char *strFinal, char *str1, char *str2, int position) {
    if (position > strlen(str1) || position < 0) return -1;
    strncpy(strFinal,str1, position);
    strFinal[position] = '\0';
    strcat(strFinal,str2);
    strcat(strFinal,str1+position);
    return 0;
}



static int confd_decimal64_to_string(char *strFinal, 
                                    struct confd_decimal64 const * const d64_ptr) {
    char str1[25], str2[25];
    int pos_to_insert;
    //Create the string with the value
    sprintf(str1, "%ld", d64_ptr->value);
    pos_to_insert = strlen(str1) - d64_ptr->fraction_digits;
    if (pos_to_insert<0) {
        strcpy(str2, "0.");
        for (;pos_to_insert<0;pos_to_insert++) {
            strcat(str2, "0");
        }
    } else {
        strcpy(str2, ".");
    }
    strConcat(strFinal,str1,str2,pos_to_insert);
    if (d64_ptr->fraction_digits == 0) {
        strcat(strFinal, "0");
    }

    if (strFinal[0]=='.') {
        strcpy(str2, strFinal);
        strConcat(strFinal,str2,"0",0);
    }

    if(strlen(strFinal)>18)
        strFinal[18] = '\0';

    return -1;
}


static struct confd_decimal64 string_to_confd_decimal64(char *input_str) {
    char parts[100][1000];
    char parts1[100][1000];
    char *float_part;
    char *endptr;
    struct confd_decimal64 d64;
    int n,n2;
    
    n = string_split(parts,input_str,"e");
    if (1 == n) {
        n = string_split(parts,input_str,"E");
    }

    float_part = parts[0];
    //printf("float_part %s\n",float_part);
    n2 = string_split(parts1, float_part, ".");
    if (n2>1) {
        char temp_str[80];
        strcpy(temp_str, parts1[0]);
        strcat(temp_str, parts1[1]);
        //printf("temp_str %s\n",temp_str);
        d64.value = strtoimax(temp_str,&endptr,10);
        d64.fraction_digits = strlen(parts1[1]);
    } else {
        d64.value = strtoimax(float_part,&endptr,10);
        d64.fraction_digits = 0;
    }
    
    // deal with the exponential part (e.g. E-3)
    if (n>=2) {
        char *exp_part = parts[1];
        int exponent = strtoimax(exp_part,&endptr,10);
        //printf("exp_part %s\n",exp_part);
        if (exponent<=0) {
            d64.fraction_digits += exponent*(-1);
        } else 
        if (exponent>=d64.fraction_digits) {
            int64_t multiplier;
            exponent -= d64.fraction_digits;
            d64.fraction_digits = 0;
            multiplier = pow(10, exponent); //10^exponent
            d64.value = d64.value*multiplier;
        } else {
            d64.fraction_digits -= exponent;
        }
    }

    //printf("d64 int part %ld\n",d64.value);
    //printf("d64 digits part %d\n",d64.fraction_digits);
    return d64;
}


struct Floats parseValuesFloat(float val, char const* const type, int channel){
    struct Floats result;
    if (strcmp(type,"BER") == 0 ) {
        count_ber[channel]=count_ber[channel]+1.0f;
        bersum[channel]=bersum[channel]+val;
        beravg[channel]=bersum[channel]/count_ber[channel];
        if(bermin[channel]==0.0f){
            bermin[channel]=val;            
        }
        else{
            if(val<bermin[channel])
               bermin[channel]=val;  
        }
        if(bermax[channel]==0.0f){
            bermax[channel]=val;
        }
        else{
            if(val>bermax[channel])
               bermax[channel]=val; 
        }
        result.favg=beravg[channel];
        result.fmin=bermin[channel];
        result.fmax=bermax[channel];
    }else
    if (strcmp(type,"QVAL") == 0 ) {
        count_q[channel]=count_q[channel]+1.0f;
        qsum[channel]=qsum[channel]+val;
        qavg[channel]=qsum[channel]/count_q[channel];
        if(qmin[channel]==0.0f){
            qmin[channel]=val;            
        }
        else{
            if(val<qmin[channel])
               qmin[channel]=val;  
        }
        if(qmax[channel]==0.0f){
            qmax[channel]=val;
        }
        else{
            if(val>qmax[channel])
               qmax[channel]=val; 
        }
        result.favg=qavg[channel];
        result.fmin=qmin[channel];
        result.fmax=qmax[channel];
    }else
    if (strcmp(type,"ESNR") == 0 ) {
        count_esnr[channel]=count_esnr[channel]+1.0f;
        esnrsum[channel]=esnrsum[channel]+val;
        esnravg[channel]=esnrsum[channel]/count_esnr[channel];
        if(esnrmin[channel]==0.0f){
            esnrmin[channel]=val;            
        }
        else{
            if(val<esnrmin[channel])
               esnrmin[channel]=val;  
        }
        if(esnrmax[channel]==0.0f){
            esnrmax[channel]=val;
        }
        else{
            if(val>esnrmax[channel])
               esnrmax[channel]=val; 
        }
        result.favg=esnravg[channel];
        result.fmin=esnrmin[channel];
        result.fmax=esnrmax[channel];
    }else
    if (strcmp(type,"CD") == 0 ) {
        count_cd[channel]=count_cd[channel]+1.0f;
        cdsum[channel]=cdsum[channel]+val;
        cdavg[channel]=cdsum[channel]/count_cd[channel];
        if(cdmin[channel]==0.0f){
            cdmin[channel]=val;            
        }
        else{
            if(val<cdmin[channel])
               cdmin[channel]=val;  
        }
        if(cdmax[channel]==0.0f){
            cdmax[channel]=val;
        }
        else{
            if(val>cdmax[channel])
               cdmax[channel]=val; 
        }
        result.favg=cdavg[channel];
        result.fmin=cdmin[channel];
        result.fmax=cdmax[channel];
    }
    return result;
}



void *monitoring(void *vargp) {
    int    run=1;
    int    len, rc, on = 1;
    int    listen_sd = -1, new_sd = -1;
    int    compress_array = FALSE;
    int    close_conn;
    char   buffer[1000];
    struct sockaddr_in   addr;
    int    timeout;
    struct pollfd fds[200];
    int    nfds = 1, current_size = 0, i, j;
    //#define SERVER_PORT  12346

    printf("Monitoring thread started\n");

    ///////////////////////////////////////////////////////////////
    // Create an AF_INET stream socket to receive incoming       //
    // connections on                                            //
    ///////////////////////////////////////////////////////////////
    listen_sd = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_sd < 0) {
        perror("socket() failed");
        return NULL;
    }

    ///////////////////////////////////////////////////////////////
    // Allow socket descriptor to be reuseable                   //
    ///////////////////////////////////////////////////////////////
    rc = setsockopt(listen_sd, SOL_SOCKET,  SO_REUSEADDR,
                    (char *)&on, sizeof(on));
    if (rc < 0) {
        perror("setsockopt() failed");
        close(listen_sd);
        return NULL;
    }

    ///////////////////////////////////////////////////////////////
    // Set socket to be nonblocking. All of the sockets for      //
    // the incoming connections will also be nonblocking since   //
    // they will inherit that state from the listening socket.   //
    ///////////////////////////////////////////////////////////////
    rc = ioctl(listen_sd, FIONBIO, (char *)&on);
    if (rc < 0) {
        perror("ioctl() failed");
        close(listen_sd);
        return NULL;
    }

    ///////////////////////////////////////////////////////////////
    // Bind the socket                                           //
    ///////////////////////////////////////////////////////////////
    memset(&addr, 0, sizeof(addr));
    addr.sin_family      = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port        = htons(SERVER_PORT);
    rc = bind(listen_sd,
                (struct sockaddr *)&addr, sizeof(addr));
    if (rc < 0) {
        perror("bind() failed");
        close(listen_sd);
        return NULL;
    }

    ///////////////////////////////////////////////////////////////
    // Set the listen back log                                   //
    ///////////////////////////////////////////////////////////////
    rc = listen(listen_sd, 32);
    if (rc < 0) {
        perror("listen() failed");
        close(listen_sd);
        return NULL;
    }

    
    ///////////////////////////////////////////////////////////////
    // Initialize the pollfd structure                           //
    ///////////////////////////////////////////////////////////////
    memset(fds, 0 , sizeof(fds));
    ///////////////////////////////////////////////////////////////
    // Set up the initial listening socket                       //
    ///////////////////////////////////////////////////////////////
    fds[0].fd = listen_sd;
    fds[0].events = POLLIN;

    ///////////////////////////////////////////////////////////////
    // Loop waiting for incoming connects or for incoming data   //
    // on any of the connected sockets.                          //
    ///////////////////////////////////////////////////////////////
    timeout = (10 * 1000); //10 seconds
    do {
        switch (poll(fds, nfds, timeout)) {
            case -1:
                perror("  poll() failed");
                break;

            default:
                /////////////////////////////////////////////////////////////
                // One or more descriptors are readable.  Need to          //
                // determine which ones they are.                          //
                /////////////////////////////////////////////////////////////
                current_size = nfds;
                for (i = 0; i < current_size; i++) {
                    
                    if (fds[i].revents & POLLIN) {
                        if (fds[i].fd == listen_sd) {
                            /////////////////////////////////////////////////////////
                            // Listening descriptor is readable.                   //
                            /////////////////////////////////////////////////////////
                            printf("  Listening socket is readable\n");

                            /////////////////////////////////////////////////////////
                            // Accept all incoming connections that are            //
                            // queued up on the listening socket before we         //
                            // loop back and call poll again.                      //
                            /////////////////////////////////////////////////////////
                            do {
                                /////////////////////////////////////////////////////
                                // Accept each incoming connection. If             //
                                // accept fails with EWOULDBLOCK, then we          //
                                // have accepted all of them. Any other            //
                                // failure on accept will cause us to end the      //
                                // server.                                         //
                                /////////////////////////////////////////////////////
                                new_sd = accept(listen_sd, NULL, NULL);
                                if (new_sd < 0) {
                                    if (errno != EWOULDBLOCK) {
                                        perror("  accept() failed\n");
                                    }
                                    break;
                                }

                                /////////////////////////////////////////////////////
                                // Add the new incoming connection to the          //
                                // pollfd structure                                //
                                /////////////////////////////////////////////////////
                                printf("  New device connected - %d\n", new_sd);
                                fds[nfds].fd = new_sd;
                                fds[nfds].events = POLLIN;
                                nfds++;

                                /////////////////////////////////////////////////////
                                // Loop back up and accept another incoming        //
                                // connection                                      //
                                /////////////////////////////////////////////////////
                            } while (new_sd != -1);
                        }

                        else {
                            /////////////////////////////////////////////////////////
                            // This is not the listening socket, therefore an      //
                            // existing connection must be readable                //
                            /////////////////////////////////////////////////////////
                            do {
                                int num_iterations,iter_i;
                                char buffer2[10][1000];

                                printf("  Descriptor %d is readable\n", fds[i].fd);
                                close_conn = FALSE;
                                /////////////////////////////////////////////////////
                                // Receive all incoming data on this socket        //
                                // before we loop back and call poll again.        //
                                /////////////////////////////////////////////////////

                                /////////////////////////////////////////////////////
                                // Receive data on this connection until the       //
                                // recv fails with EWOULDBLOCK. If any other       //
                                // failure occurs, we will close the               //
                                // connection.                                     //
                                /////////////////////////////////////////////////////
                                memset(buffer, 0, sizeof(buffer));
                                rc = recv(fds[i].fd, buffer, sizeof(buffer), 0);
                                if (rc < 0) {
                                    if (errno != EWOULDBLOCK) {
                                      perror("  recv() failed");
                                      close_conn = TRUE;
                                    }
                                    break;
                                }

                                /////////////////////////////////////////////////////
                                // Check to see if the connection has been         //
                                // closed by the client                            //
                                /////////////////////////////////////////////////////
                                if (rc == 0) {
                                    printf("  Device disconnected closed\n");
                                    close_conn = TRUE;
                                    break;
                                }

                                /////////////////////////////////////////////////////
                                // Data was received                               //
                                /////////////////////////////////////////////////////
                                len = rc;
                                printf("  %d bytes received\n", len);
                                printf("  Received: %s\n", buffer);

                                num_iterations = string_split(buffer2,buffer,"&&");


                                for (iter_i=0; iter_i<num_iterations; iter_i++) {
                                    /////////////////////////////////////////////////////
                                    // Split data by the delimiter                     //
                                    /////////////////////////////////////////////////////
                                    char tokens[10][1000];
                                    // parse the received message
                                    int num_tokens = string_split(tokens,buffer2[iter_i],"###");
                                    if (num_tokens >= 2) {
                                    /////////////////////////////////////////////
                                    // UPDATE THE DATABASE                     //
                                    ///////////////////////////////////////////// 
                                    int logchan = atoi(tokens[0]);
                                    //logical-channels monitoring parameters 
                                    //parsing of PRE-FEC-BER values
                                    if (strcmp(tokens[1],"PRE-FEC-BER") == 0 ) {
                                        if (num_tokens >=6) {
                                            
                                            char value_str[25], path[100];
                                            struct confd_decimal64 ber_d64;
                                            printf("Updating PREFECBER of logical channel:%d \n",logchan);
                                            //instant pre-fec-ber
                                            ber_d64 = string_to_confd_decimal64(tokens[2]);
                                            confd_decimal64_to_string(value_str,&ber_d64);
                                            sprintf(path,"/terminal-device/logical-channels/channel{%d}/otn/state/pre-fec-ber/%s/", logchan, "instant");
                                            updateDb(path,value_str);
                                            //mod Andrea
                                            
                                            //avg pre-fec-ber
                                            ber_d64 = string_to_confd_decimal64(tokens[3]);
                                            confd_decimal64_to_string(value_str,&ber_d64);
                                            sprintf(path,"/terminal-device/logical-channels/channel{%d}/otn/state/pre-fec-ber/%s/", logchan, "avg");
                                            updateDb(path,value_str);
                                            //min pre-fec-ber
                                            ber_d64 = string_to_confd_decimal64(tokens[4]);
                                            confd_decimal64_to_string(value_str,&ber_d64);
                                            sprintf(path,"/terminal-device/logical-channels/channel{%d}/otn/state/pre-fec-ber/%s/", logchan, "min");
                                            updateDb(path,value_str);
                                            //max pre-fec-ber
                                            ber_d64 = string_to_confd_decimal64(tokens[5]);
                                            confd_decimal64_to_string(value_str,&ber_d64);
                                            sprintf(path,"/terminal-device/logical-channels/channel{%d}/otn/state/pre-fec-ber/%s/", logchan, "max");
                                            updateDb(path,value_str);
                                        } else
                                        if (num_tokens ==3) {
                                            float binstant=0.0f;
                                            
                                            char value_str[25], path[100];
                                            struct confd_decimal64 ber_d64;
                                            printf("Updating PREFECBER of logical channel:%d \n",logchan);
                                            //instant pre-fec-ber
                                            ber_d64 = string_to_confd_decimal64(tokens[2]);
                                            confd_decimal64_to_string(value_str,&ber_d64);
                                            sprintf(path,"/terminal-device/logical-channels/channel{%d}/otn/state/pre-fec-ber/%s/", logchan, "instant");
                                            updateDb(path,value_str);
                                            
                                            //mod Andrea
                                            binstant=atof(value_str);
                                            
                                            struct Floats ff = parseValuesFloat(binstant, "BER", logchan);
                                            printf("after %f \n", ff.favg);
                                            //avg pre-fec-ber
                                            
                                            sprintf(value_str, "%f", ff.favg);
                                            //snprintf(value_str, sizeof value_str, '%f', ff.favg);
                                            sprintf(path,"/terminal-device/logical-channels/channel{%d}/otn/state/pre-fec-ber/%s/", logchan, "avg");
                                            
                                            updateDb(path,value_str);
                                            printf("before\n");
                                            //min pre-fec-ber
                                            sprintf(value_str, "%f", ff.fmin);
                                            //snprintf(value_str, sizeof value_str, '%f', ff.favg);
                                            sprintf(path,"/terminal-device/logical-channels/channel{%d}/otn/state/pre-fec-ber/%s/", logchan, "min");
                                            updateDb(path,value_str);
                                            
                                            //max pre-fec-ber
                                            sprintf(value_str, "%f", ff.fmax);
                                            //snprintf(value_str, sizeof value_str,  '%f', ff.favg);
                                            sprintf(path,"/terminal-device/logical-channels/channel{%d}/otn/state/pre-fec-ber/%s/", logchan, "max");
                                            updateDb(path,value_str);
                                            //printf("Summary BER sum: %f, count:%f, min:%f max%f \n", bersum, count_ber, bermin, bermax);
                                        } else
                                            printf("WARNING, pre-fec-ber update with %d elements\n",num_tokens);
                                    } else
                                    //parsing of Q-VALUE values
                                    if (strcmp(tokens[1],"Q-VALUE") == 0) {
                                        if (num_tokens >=6) {
                                            char value_str[25], path[100] ;
                                            struct confd_decimal64 q64;
                                            printf("Updating Q-FACTOR of logical channel:%d\n",logchan);
                                            //instant q-factor
                                            q64 = string_to_confd_decimal64(tokens[2]);
                                            confd_decimal64_to_string(value_str,&q64);
                                            sprintf(path,"/terminal-device/logical-channels/channel{%d}/otn/state/q-value/%s/", logchan, "instant");
                                            updateDb(path,value_str);
                                            //avg q-factor
                                            q64 = string_to_confd_decimal64(tokens[3]);
                                            confd_decimal64_to_string(value_str,&q64);
                                            sprintf(path,"/terminal-device/logical-channels/channel{%d}/otn/state/q-value/%s/", logchan, "avg");
                                            updateDb(path,value_str);
                                            //min q-factor
                                            q64 = string_to_confd_decimal64(tokens[4]);
                                            confd_decimal64_to_string(value_str,&q64);
                                            sprintf(path,"/terminal-device/logical-channels/channel{%d}/otn/state/q-value/%s/", logchan, "min");
                                            updateDb(path,value_str);
                                            //max q-factor
                                            q64 = string_to_confd_decimal64(tokens[5]);
                                            confd_decimal64_to_string(value_str,&q64);
                                            sprintf(path,"/terminal-device/logical-channels/channel{%d}/otn/state/q-value/%s/", logchan, "max");
                                            updateDb(path,value_str);
                                        } else
                                        if (num_tokens ==3) {
                                            float qinstant=0.0f;
                                            
                                            char value_str[25], path[100];
                                            struct confd_decimal64 ber_d64;
                                            printf("Updating Q-FACTOR of logical channel:%d \n",logchan);
                                            //instant pre-fec-ber
                                            ber_d64 = string_to_confd_decimal64(tokens[2]);
                                            confd_decimal64_to_string(value_str,&ber_d64);
                                            sprintf(path,"/terminal-device/logical-channels/channel{%d}/otn/state/q-value/%s/", logchan, "instant");
                                            updateDb(path,value_str);
                                            
                                            //mod Andrea
                                            qinstant=atof(value_str);
                                            struct Floats ff1 = parseValuesFloat(qinstant, "QVAL", logchan);
                                            //avg pre-fec-ber
                                            sprintf(value_str, "%.2f", ff1.favg);
                                            sprintf(path,"/terminal-device/logical-channels/channel{%d}/otn/state/q-value/%s/", logchan, "avg");
                                            updateDb(path,value_str);
                                            
                                            //min pre-fec-ber
                                            sprintf(value_str, "%.2f", ff1.fmin);
                                            sprintf(path,"/terminal-device/logical-channels/channel{%d}/otn/state/q-value/%s/", logchan, "min");
                                            updateDb(path,value_str);
                                            
                                            //max pre-fec-ber
                                            sprintf(value_str, "%.2f", ff1.fmax);
                                            sprintf(path,"/terminal-device/logical-channels/channel{%d}/otn/state/q-value/%s/", logchan, "max");
                                            updateDb(path,value_str);
                                        } else
                                            printf("WARNING, Q-FACTOR update with %d elements\n",num_tokens);
                                    } else
                                    //parsing of ESNR values
                                    if (strcmp(tokens[1],"ESNR") == 0) {
                                        if (num_tokens >=6) {
                                            char value_str[25], path[100];
                                            struct confd_decimal64 d64;
                                            printf("Updating ESNR of logical channel:%d \n",logchan);
                                            //instant pre-fec-ber
                                            d64 = string_to_confd_decimal64(tokens[2]);
                                            confd_decimal64_to_string(value_str,&d64);
                                            sprintf(path,"/terminal-device/logical-channels/channel{%d}/otn/state/esnr/%s/", logchan, "instant");
                                            updateDb(path,value_str);
                                            //avg pre-fec-ber
                                            d64 = string_to_confd_decimal64(tokens[3]);
                                            confd_decimal64_to_string(value_str,&d64);
                                            sprintf(path,"/terminal-device/logical-channels/channel{%d}/otn/state/esnr/%s/", logchan, "avg");
                                            updateDb(path,value_str);
                                            //min pre-fec-ber
                                            d64 = string_to_confd_decimal64(tokens[4]);
                                            confd_decimal64_to_string(value_str,&d64);
                                            sprintf(path,"/terminal-device/logical-channels/channel{%d}/otn/state/esnr/%s/", logchan, "min");
                                            updateDb(path,value_str);
                                            //max pre-fec-ber
                                            d64 = string_to_confd_decimal64(tokens[5]);
                                            confd_decimal64_to_string(value_str,&d64);
                                            sprintf(path,"/terminal-device/logical-channels/channel{%d}/otn/state/esnr/%s/", logchan, "max");
                                            updateDb(path,value_str);
                                        } else
                                        if (num_tokens ==3) {
                                            float einstant=0.0f;
                                            
                                            char value_str[25], path[100];
                                            struct confd_decimal64 ber_d64;
                                            printf("Updating ESNR of logical channel:%d \n",logchan);
                                            //instant pre-fec-ber
                                            ber_d64 = string_to_confd_decimal64(tokens[2]);
                                            confd_decimal64_to_string(value_str,&ber_d64);
                                            sprintf(path,"/terminal-device/logical-channels/channel{%d}/otn/state/esnr/%s/", logchan, "instant");
                                            updateDb(path,value_str);
                                            
                                            //mod Andrea
                                            einstant=atof(value_str);
                                            struct Floats ff2 = parseValuesFloat(einstant, "ESNR", logchan);
                                            //avg pre-fec-ber
                                            sprintf(value_str, "%.2f", ff2.favg);
                                            sprintf(path,"/terminal-device/logical-channels/channel{%d}/otn/state/esnr/%s/", logchan, "avg");
                                            updateDb(path,value_str);
                                            
                                            //min pre-fec-ber
                                            sprintf(value_str, "%.2f", ff2.fmin);
                                            sprintf(path,"/terminal-device/logical-channels/channel{%d}/otn/state/esnr/%s/", logchan, "min");
                                            updateDb(path,value_str);
                                            
                                            //max pre-fec-ber
                                            sprintf(value_str, "%.2f", ff2.fmax);
                                            sprintf(path,"/terminal-device/logical-channels/channel{%d}/otn/state/esnr/%s/", logchan, "max");
                                            updateDb(path,value_str);
                                        } else
                                            printf("WARNING, ESNR update with %d elements\n",num_tokens);
                                    } else
                                    //components monitoring parameters 
                                    
                                    //parsing of CHROMATIC-DISPERSION value
                                    if (strcmp(tokens[1],"CHROMATIC-DISPERSION") == 0) {
                                        if (num_tokens >=6) {
                                            char value_str[25], path[100];
                                            struct confd_decimal64 d64;
                                            printf("Updating Chromatic Dispersion of of component %s\n",tokens[0]);
                                            //instant pre-fec-bers
                                            d64 = string_to_confd_decimal64(tokens[2]);
                                            confd_decimal64_to_string(value_str,&d64);
                                            sprintf(path,"/components/component{channel-%d}/optical-channel/state/%s/", logchan, "chromatic-dispersion/instant");
                                            updateDb(path,value_str);
                                            //avg pre-fec-ber
                                            d64 = string_to_confd_decimal64(tokens[3]);
                                            confd_decimal64_to_string(value_str,&d64);
                                            sprintf(path,"/components/component{channel-%d}/optical-channel/state/%s/", logchan, "chromatic-dispersion/avg");
                                            updateDb(path,value_str);
                                            //min pre-fec-ber
                                            d64 = string_to_confd_decimal64(tokens[4]);
                                            confd_decimal64_to_string(value_str,&d64);
                                            sprintf(path,"/components/component{channel-%d}/optical-channel/state/%s/", logchan, "chromatic-dispersion/min");
                                            updateDb(path,value_str);
                                            //max pre-fec-ber
                                            d64 = string_to_confd_decimal64(tokens[5]);
                                            confd_decimal64_to_string(value_str,&d64);
                                            sprintf(path,"/components/component{channel-%d}/optical-channel/state/%s/", logchan, "chromatic-dispersion/max");
                                            updateDb(path,value_str);
                                        } else
                                        if (num_tokens ==3) {
                                            float cinstant=0.0f;
                                            
                                            char value_str[25], path[100];
                                            struct confd_decimal64 ber_d64;
                                            printf("Updating Chromatic Dispersion of logical channel:%d \n",logchan);
                                            //instant pre-fec-ber
                                            ber_d64 = string_to_confd_decimal64(tokens[2]);
                                            confd_decimal64_to_string(value_str,&ber_d64);
                                            sprintf(path,"/components/component{channel-%d}/optical-channel/state/%s/", logchan, "chromatic-dispersion/instant");
                                            updateDb(path,value_str);
                                            
                                            //mod Andrea
                                            cinstant=atof(value_str);
                                            struct Floats ff3 = parseValuesFloat(cinstant, "CD", logchan);
                                            //avg pre-fec-ber
                                            sprintf(value_str, "%.2f", ff3.favg);
                                            sprintf(path,"/components/component{channel-%d}/optical-channel/state/%s/", logchan, "chromatic-dispersion/avg");
                                            updateDb(path,value_str);
                                            
                                            //min pre-fec-ber
                                            sprintf(value_str, "%.2f", ff3.fmin);
                                            sprintf(path,"/components/component{channel-%d}/optical-channel/state/%s/", logchan, "chromatic-dispersion/min");
                                            updateDb(path,value_str);
                                            
                                            //max pre-fec-ber
                                            sprintf(value_str, "%.2f", ff3.fmax);
                                            sprintf(path,"/components/component{channel-%d}/optical-channel/state/%s/", logchan, "chromatic-dispersion/max");
                                            updateDb(path,value_str);
                                        } else
                                            printf("WARNING, Chromatic-dispersion update with %d elements\n",num_tokens);
                                    } else
                                    //parsing of FREQUENCY value
                                    if (strcmp(tokens[1],"FREQUENCY") == 0) {
                                        char value_str[8], path[100];
                                        if (strlen(tokens[2])>=9){
                                           strncpy(value_str, tokens[2], 9);
                                        }
                                        else{
                                           int fill=9-(int)strlen(tokens[2]);
                                           strcpy(value_str, tokens[2]);
                                           char filler[fill];
                                           sprintf(filler, "%0*d", fill, 0);
                                           strncat(value_str, filler, fill);
                                           printf("Final string %s\n", value_str);
                                        }

                                        printf("Updating Central Frequency of component %s\n",tokens[0]);
                                        sprintf(path,"/components/component{channel-%s}/optical-channel/state/%s/", tokens[0], "frequency");
                                        updateDb(path,value_str);
                                    } else
                                    if (strcmp(tokens[1],"DOWN") == 0) {
                                        printf("Subcarrier:%d DOWN \n", logchan);
                                    }
                                    else
                                        printf("WARNING:Update unknown for logical channel %d \n", logchan);
                                    }                                
                                }                         

                            } while(TRUE);

                            /////////////////////////////////////////////////////////
                            // If the close_conn flag was turned on, we need       //
                            // to clean up this active connection. This            //
                            // clean up process includes removing the              //
                            // descriptor.                                         //
                            /////////////////////////////////////////////////////////
                            if (close_conn) {
                                close(fds[i].fd);
                                fds[i].fd = -1;
                                compress_array = TRUE;
                            }

                        }
                    }
                }

                /////////////////////////////////////////////////////////////
                // If the compress_array flag was turned on, we need       //
                // to squeeze together the array and decrement the number  //
                // of file descriptors. We do not need to move back the    //
                // events and revents fields because the events will always//
                // be POLLIN in this case, and revents is output.          //
                /////////////////////////////////////////////////////////////
                if (compress_array) {
                    compress_array = FALSE;
                    for (i = 0; i < nfds; i++) {
                        if (fds[i].fd == -1) {
                            for(j = i; j < nfds; j++) {
                                fds[j].fd = fds[j+1].fd;
                            }
                            nfds--;
                        }
                    }
                }

                break;
        } // switch
        
        
        pthread_mutex_lock( &running_mutex );
        run = threadsKeepRunning;
        pthread_mutex_unlock( &running_mutex );

    } while (run);

    /////////////////////////////////////////////////////////////
    // Clean up all of the sockets that are open               //
    /////////////////////////////////////////////////////////////
    for (i = 0; i < nfds; i++) {
        if(fds[i].fd >= 0)
            close(fds[i].fd);
    }
    close(listen_sd);

    printf("Mo0nitoring thread terminated\n");
    return NULL;
}



void *files_monitoring(void *vargp) {

  #define EVENT_SIZE  ( sizeof (struct inotify_event) )
  #define EVENT_BUF_LEN     ( 1024 * ( EVENT_SIZE + 16 ) )
  #define BER_FILENAME "monitoredFiles/ber.dat"
  #define VALUES_SEPARATOR " "

  int length, i = 0, run=1;
  int fd;
  int wd;
  char buffer[EVENT_BUF_LEN];

  /*creating the INOTIFY instance*/
  fd = inotify_init();
  /*checking for error*/
  if ( fd < 0 ) {
    perror( "inotify_init error" );
  }

  
  if( access( BER_FILENAME, F_OK ) == -1 ) {
      // file doesn't exist
      printf( "File %s does not extist\n", BER_FILENAME);
      return NULL;
  } else if( access( BER_FILENAME, R_OK ) == -1 ) {
      // read not allowed on the file
      printf( "File %s cannot be read (NO read permission)\n", BER_FILENAME);
      return NULL;
  }

  wd = inotify_add_watch( fd, BER_FILENAME, IN_MODIFY);//IN_CREATE | IN_DELETE | IN_ACCESS | IN_MODIFY | IN_OPEN );
  
  do {
      // read to determine the event change happens on the files/directory on the watch list. 
      // Actually this read blocks until the change event occurs 
      length = read( fd, buffer, EVENT_BUF_LEN ); 
      /* checking for error */
      if ( length < 0 ) {
        perror( "read" );
        return NULL;
      }  

      
      i=0;
      //  Actually read return the list of change events happens. 
      //  Here, read the change event one by one and process it accordingly.
      while ( i < length ) {
        struct inotify_event *event = ( struct inotify_event * ) &buffer[ i ];
        char *name;
        char fixName[] = BER_FILENAME;

        if( event->len == 0) {
          // For a single file watching, the event->name is empty, and event->len = 0
          name = fixName;
        } else {
          name = event->name; 
        }

        if( event->mask & IN_MODIFY ) {
            if ( event->mask & IN_ISDIR ) {
              printf( "Directory %s modified.\n", name );
            } else {
              printf(" File %s modified. \n", name );

              {
                char *line = NULL;
                size_t len = 0;
                ssize_t read;
                FILE *fp = fopen(BER_FILENAME, "r" );

                while ((read = getline(&line, &len, fp)) != -1) {
                    char tokens[10][1000];
                    printf("Retrieved line of length %zu :\n", read);
                    printf("%s", line);
                    int num_tokens = string_split(tokens,line,VALUES_SEPARATOR);
                    if (num_tokens>1) {
                      char path[100];
                      char *value_str = tokens[1];
                      printf("Update pre-FEC-BER:%s of subcarrier:%d to DATABASE \n",value_str,1);
                      // create database path string 
                      sprintf(path,"/transponder/subcarrier-module{%d}/state/receiver/%s/", 1, "pre-fec-ber");
                      updateDb(path,value_str);
                    }
                    break;
                }

                free(line);
              }

            }
        } else if ( event->mask & IN_CREATE ) {
            if ( event->mask & IN_ISDIR ) {
              printf( "New directory %s created.\n", name );
            } else {
              printf( "New file %s created.\n", name );
            }
        } else if ( event->mask & IN_DELETE ) {
            if ( event->mask & IN_ISDIR ) {
              printf( "Directory %s deleted.\n", name );
            } else {
              printf( "File %s deleted.\n", name );
            }
        } else if( event->mask & IN_ACCESS ) {
            if ( event->mask & IN_ISDIR ) {
              printf( "Directory %s accessed.\n", name );
            } else {
              printf(" File %s accessed. \n", name );
            }
        } else if( event->mask & IN_OPEN ) {
            if ( event->mask & IN_ISDIR ) {
              printf( "Directory %s opened.\n", name );
            } else {
              printf(" File %s opened. \n", name );
            }
        } else {
            printf( "Directory or File is accessed by other mode\n");
        }
        
        i += EVENT_SIZE + event->len;
      }

      pthread_mutex_lock( &running_mutex );
      run = threadsKeepRunning;
      pthread_mutex_unlock( &running_mutex );

  } while(run);

  /* removing the “/tmp/test_inotify” directory from the watch list. */
  inotify_rm_watch( fd, wd );

  /* closing the INOTIFY instance */
  close( fd );
  
  return NULL;
}




//------------------------------MAIN-----------------------------//

int main(int argc, char **argv) {
    char confd_port[16];
    struct addrinfo hints;
    struct addrinfo *addr = NULL;
    int debuglevel = CONFD_DEBUG; //CONFD_SILENT
    struct sockaddr_in s_addr;
    int i;
    int c;
    char *p, *dname;
    int ret;
    int timeout;
    int rnum;
    int spoint; 

    snprintf(confd_port, sizeof(confd_port), "%d", CONFD_PORT);
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = PF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    while ((c = getopt(argc, argv, "dtprc:")) != -1) {
        switch (c) {
        case 'd':
            debuglevel = CONFD_DEBUG;
            break;
        case 't':
            debuglevel = CONFD_TRACE;
            break;
        case 'p':
            debuglevel = CONFD_PROTO_TRACE;
            break;
        case 'c':
            if ((p = strchr(optarg, '/')) != NULL)
                *p++ = '\0';
            else
                p = confd_port;
            if (getaddrinfo(optarg, p, &hints, &addr) != 0) {
                if (p != confd_port) {
                    *--p = '/';
                    p = "/port";
                } else {
                    p = "";
                }
                fprintf(stderr, "%s: Invalid address%s: %s\n",
                        argv[0], p, optarg);
                exit(1);
            }
            break;
        default:
            fprintf(stderr,
                    "Usage: %s [-dtpr] [-c address[/port]]\n",
                    argv[0]);
            exit(1);
        }
    }
    //Connection to the ConfD database
    if (addr == NULL &&
        ((i = getaddrinfo("127.0.0.1", confd_port, &hints, &addr)) != 0))
        /* "Can't happen" */
        confd_fatal("%s: Failed to get address for ConfD: %s\n",
                    argv[0], gai_strerror(i));
    if ((dname = strrchr(argv[0], '/')) != NULL)
        dname++;
    else
        dname = argv[0];
    /* Init library */
    confd_init(dname, stderr, debuglevel);

      if ((deamon_p = confd_init_daemon(dname)) == NULL)
      confd_fatal("Failed to initialize ConfD\n");
      if ((ctlsock = GetCtrlSock(addr)) < 0)
          confd_fatal("Failed to connect to ConfD\n");
      if ((workersock = GetWorkerSock(addr)) < 0)
          confd_fatal("Failed to connect to ConfD\n");


    memset(replay_buffer, 0, sizeof(replay_buffer));
    memset(replay, 0, sizeof(replay));
    getdatetime(&replay_creation);

       
    // SUBSCRIBE TO DATABASE CHANGE NOTIFICATIONS
    s_addr.sin_addr.s_addr = inet_addr(confd_addr);
    s_addr.sin_family = AF_INET;
    s_addr.sin_port = htons(CONFD_PORT);

    if ((cdb_sock = GetCdbDataSock(&s_addr)) < 0)
      confd_fatal("Failed to connect to ConfD\n");

    if (confd_load_schemas((struct sockaddr*)&s_addr,
                         sizeof (struct sockaddr_in)) != CONFD_OK)
      confd_fatal("%s: Failed to load schemas from confd\n", argv[0]);

    if ((sub_sock = GetCdbSubSock(&s_addr)) < 0)
      confd_fatal("Failed to connect to ConfD\n");

    // terminal-devices config subscription
    if ((cdb_subscribe(sub_sock, 1, oc_opt_term__ns, &spoint,
                        "/terminal-device/logical-channels/channel/config/"))!= CONFD_OK) {
      confd_fatal("Terminate: subscribe \n");
    }
    // components config subscription
    if ((cdb_subscribe(sub_sock, 2, oc_opt_term__ns, &spoint,
                        "/components/component/optical-channel/config/"))!= CONFD_OK) {
      confd_fatal("Terminate: subscribe \n");
    }
    // telemetry subscription
    if ((cdb_subscribe(sub_sock, 2, oc_telemetry__ns, &spoint,
                        "/telemetry-system/subscriptions/dynamic-subscriptions/"))!= CONFD_OK) {
      confd_fatal("Terminate: subscribe \n");
    }

    // Tell to ConfD when the Application is ready to receive notifications
    // of CDB updates from ConfD
    if (cdb_subscribe_done(sub_sock) != CONFD_OK)
      confd_fatal("cdb_subscribe_done() failed");


    //----- REGISTER Callbacks -----
    callbacks_registration(deamon_p);

    // GET MAAPI SOCKET used to write into the CDB
    maapis = GetMaapiSock(&s_addr);


    
    // start monitoring thread
    pthread_create(&spo_thread_id, NULL, monitoring, NULL);

    if (ENABLE_GRPC)
        pthread_create(&grpc_thread_id, NULL, grpc_server, NULL);
    // start thread for configuration of transponder
    fprintf(stderr,
"\n\n*** ConfD OpenConfig NETCONF agent ***\n\
***          developed by          ***\n\
***       Andrea Sgambelluri       ***\n\
***           CNIT-SSSA            ***\n\n\n");
    if(CONF_TRANSPONDER==1) intHardwareCommunication();
    
    // register handler for CRTL+C command
    signal(SIGINT, intHandler); 

    while (keepRunning) {
        struct pollfd set[4];
        set[0].fd = STDIN_FILENO; // fd of standard input (0)
        set[0].events = POLLIN;
        set[0].revents = 0;

        set[1].fd = ctlsock;
        set[1].events = POLLIN;
        set[1].revents = 0;

        set[2].fd = workersock;
        set[2].events = POLLIN;
        set[2].revents = 0;

        set[3].fd = sub_sock;
        set[3].events = POLLIN;
        set[3].revents = 0;

        /* if we're doning a replay, don't use a timeout */
        timeout = -1;
        for (rnum = 0; rnum < MAX_REPLAYS; rnum++) {
            if (replay[rnum].active) {
                timeout = 0;
                break;
            }
        }

        switch (poll(set, NELEMS(set), timeout)) {
        case -1:
            break;

        default:
            
            if (set[1].revents & POLLIN) { /* ctlsock */
                if ((ret = confd_fd_ready(deamon_p, ctlsock)) == CONFD_EOF) {
                    confd_fatal("Control socket closed\n");
                } else if (ret == CONFD_ERR && confd_errno != CONFD_ERR_EXTERNAL) {
                    confd_fatal("Error on control socket request: %s (%d): %s\n",
                                    confd_strerror(confd_errno),
                                    confd_errno,
                                    confd_lasterr());
                }
            }

            if (set[2].revents & POLLIN) { /* workersock */
                if ((ret = confd_fd_ready(deamon_p, workersock)) == CONFD_EOF) {
                    confd_fatal("Worker socket closed\n");
                } else if (ret == CONFD_ERR && confd_errno != CONFD_ERR_EXTERNAL) {
                    confd_fatal("Error on worker socket request: %s (%d): %s\n",
                                    confd_strerror(confd_errno),
                                    confd_errno,
                                    confd_lasterr());
                }
            }

            if (set[3].revents & POLLIN) {
                int sub_points[1];
                int reslen;
                int status;

                if ((status = cdb_read_subscription_socket(sub_sock,
                                                       &sub_points[0],
                                                       &reslen)) != CONFD_OK) {
                    confd_fatal("terminate sub_read: %d\n", status);
                }
                if (reslen > 0) {
                    fprintf(stderr, "*** Config updated \n");

                    if ((status = cdb_start_session(cdb_sock,CDB_RUNNING)) != CONFD_OK)
                        confd_fatal("Cannot start session\n");
                    if ((status = cdb_set_namespace(cdb_sock, oc_opt_term__ns)) != CONFD_OK)
                        confd_fatal("Cannot set namespace\n");

                    cdb_diff_iterate(sub_sock, sub_points[0], Iter,
                                     ITER_WANT_PREV, (void*)&cdb_sock);
                    cdb_end_session(cdb_sock);
                }

                if ((status = cdb_sync_subscription_socket(sub_sock,
                                                           CDB_DONE_PRIORITY))
                    != CONFD_OK) {
                    confd_fatal("failed to sync subscription: %d\n", status);
                }
            }
            if (timeout == 0) {
                /* continue replay */
                for (rnum = 0; rnum < MAX_REPLAYS; rnum++) {
                    if (replay[rnum].active) {
                        continue_replay(&replay[rnum]);
                    }
                }
            }
        }
    } // while(keepRunning)

    close(ctlsock);
    close(workersock);
    close(sub_sock);
    close(cdb_sock);
    close(maapis);

    pthread_mutex_lock( &running_mutex );
    threadsKeepRunning = 0;
    pthread_mutex_unlock( &running_mutex );
    pthread_join(spo_thread_id, NULL);
    if (ENABLE_GRPC)
        pthread_join(grpc_thread_id, NULL);
    stopHardwareCommunication();
    printf("\nMAIN Terminated\n");

    return 0;
}
