#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/msg.h>
#include <netdb.h>


#include <pthread.h>
#include <signal.h>

#include "filter_driver.h"
#include "parameters.h"


static pthread_t config_thread_id1;

static int msg_queue_id1;


struct message_s {
  long int mtype;
  char     mtext[256];
};

static const size_t msg_size = sizeof(struct message_s) - sizeof(long int);

#define MESSAGE_TYPE 1
#define IPC_WAIT 0
#define MESSAGE_QUEUE_KEY 1234


// socket info
//#define FILTER_ADDR "10.30.2.51"//"10.30.2.24"
//#define FILTER_PORT 16002

//struct subscriber *tel_subs_htable = NULL;    // important! initialize to NULL //

static void *config_communication_job(void *vargp) {
    struct hostent *hostnm;    /* server host name information        */
    struct sockaddr_in server; /* server address                      */
    int sock_id;                     /* client socket                       */
    int rc;
    struct message_s mymsg;


    //
    // Get the server address.
    //
    hostnm = gethostbyname(FILTER_ADDR);
    if (hostnm == (struct hostent *) 0) {
        fprintf(stderr, "Gethostbyname failed\n");
        pthread_exit(NULL);
    }

    //
    // Put the server information into the server structure.
    // The port must be put into network byte order.
    //
    server.sin_family      = AF_INET;
    server.sin_port        = htons(FILTER_PORT);
    server.sin_addr.s_addr = *((unsigned long *)hostnm->h_addr);

    //
    // Get a stream socket.
    //
    if ((sock_id = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Socket()");
    }

    //
    // Connect to the server.
    //
    while (connect(sock_id, (struct sockaddr *)&server, sizeof(server)) < 0) {
        sleep(5);//sleep 5 seconds and retry
        printf("Filter Driver - Trying to connect...\n");
    }

    printf("Filter Driver - Connected\n");

    while ( (rc = msgrcv(msg_queue_id1, &mymsg, msg_size, MESSAGE_TYPE, IPC_WAIT)) >= 0) {

        // send the message to the device
        if (send(sock_id, mymsg.mtext, strlen(mymsg.mtext), 0) < 0) {
            perror("Send()");
        }
        printf("Filter Driver - Sent command: %s\n", mymsg.mtext);
    }


    /*
    if (recv(sock_id, buf, sizeof(buf), 0) < 0) {
        tcperror("Recv()");
        exit(6);
    }*/

    // Close the socket.
    close(sock_id);

    printf("Filter Driver - Disconnected\n");

    return NULL;
}


void intFilterCommunication() {
    // Set up the message queue
    msg_queue_id1 = msgget((key_t)MESSAGE_QUEUE_KEY, 0666 | IPC_CREAT);

    if (msg_queue_id1 == -1) {
      perror("msgget failed with error");
      exit(1);
    }

    // launch thread for configuring the device
    pthread_create(&config_thread_id1, NULL, config_communication_job, NULL);

    printf("Filter Driver Started\n");
}

void stopFilterCommunication() {
    int status;
    status = pthread_kill( config_thread_id1, SIGUSR1);
    if ( status <  0)                                                              
      perror("pthread_kill failed");
}


void wss_m_set(uint32_t connection_id, char const* const  m_value) {
    struct message_s mymsg;

    mymsg.mtype = MESSAGE_TYPE;
    sprintf(mymsg.mtext,"CONFIGWSS###%d###M_VALUE###%s$$$", connection_id, m_value);

    if (msgsnd(msg_queue_id1, &mymsg, msg_size, IPC_NOWAIT) != 0) {
        perror("failed m_set"); 
        //maybe queue is full, check msgctl()
    }
}

void wss_n_set(uint32_t connection_id, char const* const  n_value) {
    struct message_s mymsg;

    mymsg.mtype = MESSAGE_TYPE;
    sprintf(mymsg.mtext,"CONFIGWSS###%d###N_VALUE###%s$$$", connection_id, n_value);

    if (msgsnd(msg_queue_id1, &mymsg, msg_size, IPC_NOWAIT) != 0) {
        perror("failed n_set"); 
        //maybe queue is full, check msgctl()
    }
}

void wss_inport_set(uint32_t connection_id, char const* const inport) {
    struct message_s mymsg;

    mymsg.mtype = MESSAGE_TYPE;
    sprintf(mymsg.mtext,"CONFIGWSS###%d###INPORT###%s$$$", connection_id, inport);

    if (msgsnd(msg_queue_id1, &mymsg, msg_size, IPC_NOWAIT) != 0) {
        perror("failed inport_set"); 
        //maybe queue is full, check msgctl()
    }
}

void wss_outport_set(uint32_t connection_id, char const* const  outport) {
     struct message_s mymsg;

    mymsg.mtype = MESSAGE_TYPE;
    sprintf(mymsg.mtext,"CONFIGWSS###%d###OUTPORT###%s$$$", connection_id, outport);

    if (msgsnd(msg_queue_id1, &mymsg, msg_size, IPC_NOWAIT) != 0) {
        perror("failed output_set"); 
        //maybe queue is full, check msgctl()
    }
}
