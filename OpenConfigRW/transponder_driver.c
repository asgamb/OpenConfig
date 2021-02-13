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

#include "transponder_driver.h"
#include "parameters.h"


static pthread_t config_thread_id;

static int msg_queue_id;


struct message_s {
  long int mtype;
  char     mtext[256];
};

static const size_t msg_size = sizeof(struct message_s) - sizeof(long int);

#define MESSAGE_TYPE 1
#define IPC_WAIT 0
#define MESSAGE_QUEUE_KEY 1235


// socket info
//#define TRANSPONDER_ADDR "10.30.2.24"//"10.30.2.24"
//#define TRANSPONDER_PORT 16001

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
    hostnm = gethostbyname(TRANSPONDER_ADDR);
    if (hostnm == (struct hostent *) 0) {
        fprintf(stderr, "Gethostbyname failed\n");
        pthread_exit(NULL);
    }

    //
    // Put the server information into the server structure.
    // The port must be put into network byte order.
    //
    server.sin_family      = AF_INET;
    server.sin_port        = htons(TRANSPONDER_PORT);
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
        printf("Transponder Driver - Trying to connect...\n");
    }

    printf("Transponder Driver - Connected\n");

    while ( (rc = msgrcv(msg_queue_id, &mymsg, msg_size, MESSAGE_TYPE, IPC_WAIT)) >= 0) {

        // send the message to the device
        if (send(sock_id, mymsg.mtext, strlen(mymsg.mtext), 0) < 0) {
            perror("Send()");
        }
        printf("Transponder Driver - Sent command: %s\n", mymsg.mtext);
    }


    /*
    if (recv(sock_id, buf, sizeof(buf), 0) < 0) {
        tcperror("Recv()");
        exit(6);
    }*/

    // Close the socket.
    close(sock_id);

    printf("Transponder Driver - Disconnected\n");

    return NULL;
}


void intHardwareCommunication() {
    // Set up the message queue
    msg_queue_id = msgget((key_t)MESSAGE_QUEUE_KEY, 0666 | IPC_CREAT);

    if (msg_queue_id == -1) {
      perror("msgget failed with error");
      exit(1);
    }

    // launch thread for configuring the device
    pthread_create(&config_thread_id, NULL, config_communication_job, NULL);

    printf("Transponder Driver Started\n");
}

void stopHardwareCommunication() {
    int status;
    status = pthread_kill( config_thread_id, SIGUSR1);
    if ( status <  0)                                                              
      perror("pthread_kill failed");
}

void lch_set_admin_state(uint32_t lch, char const* const val) {
    struct message_s mymsg;

    mymsg.mtype = MESSAGE_TYPE;
    sprintf(mymsg.mtext,"CONFIGOC###%d###ADMINSTATE###%s$$$", lch, val);

    if (msgsnd(msg_queue_id, &mymsg, msg_size, IPC_NOWAIT) != 0) {
        perror("failed admn_state_set"); 
        //maybe queue is full, check msgctl()
    }
}

void component_set_target_power(char const* const name, char const* const val) {
    struct message_s mymsg;

    mymsg.mtype = MESSAGE_TYPE;
    sprintf(mymsg.mtext,"CONFIGOC###%s###TARGETPOWER###%s$$$", name, val);

    if (msgsnd(msg_queue_id, &mymsg, msg_size, IPC_NOWAIT) != 0) {
        perror("failed target_power_set"); 
        //maybe queue is full, check msgctl()
    }
}

void component_set_frequency(char const* const name, char const* const val) {
    struct message_s mymsg;

    mymsg.mtype = MESSAGE_TYPE;
    sprintf(mymsg.mtext,"CONFIGOC###%s###FREQUENCY###%s$$$", name, val);

    if (msgsnd(msg_queue_id, &mymsg, msg_size, IPC_NOWAIT) != 0) {
        perror("failed frequency_set"); 
        //maybe queue is full, check msgctl()
    }
}

void component_set_operational_mode(char const* const name, char const* const val) {
    struct message_s mymsg;

    mymsg.mtype = MESSAGE_TYPE;
    sprintf(mymsg.mtext,"CONFIGOC###%s###OPERATIONALMODE###%s$$$", name, val);

    if (msgsnd(msg_queue_id, &mymsg, msg_size, IPC_NOWAIT) != 0) {
        perror("failed operational_mode_set"); 
        //maybe queue is full, check msgctl()
    }
}
/*
void bit_rate_set(int subcarrier_id, char const* const bit_rate) {
    struct message_s mymsg;

    mymsg.mtype = MESSAGE_TYPE;
    sprintf(mymsg.mtext,"CONFIG###%d###BITRATE###%s$$$", subcarrier_id, bit_rate);

    if (msgsnd(msg_queue_id, &mymsg, msg_size, IPC_NOWAIT) != 0) {
        perror("failed bit_rate_set"); 
        //maybe queue is full, check msgctl()
    }
}

void modulation_set(int subcarrier_id, char const* const modulation) {
    struct message_s mymsg;

    mymsg.mtype = MESSAGE_TYPE;
    sprintf(mymsg.mtext,"CONFIG###%d###MODULATION###%s$$$", subcarrier_id, modulation);

    if (msgsnd(msg_queue_id, &mymsg, msg_size, IPC_NOWAIT) != 0) {
        perror("failed bit_rate_set"); 
        //maybe queue is full, check msgctl()
    }
}

void baud_rate_set(int subcarrier_id, char const* const baud_rate) {
    struct message_s mymsg;

    mymsg.mtype = MESSAGE_TYPE;
    sprintf(mymsg.mtext,"CONFIG###%d###BAUDRATE###%s$$$", subcarrier_id, baud_rate);

    if (msgsnd(msg_queue_id, &mymsg, msg_size, IPC_NOWAIT) != 0) {
        perror("failed baud_rate_set"); 
        //maybe queue is full, check msgctl()
    }
}


void pathLength_set(int subcarrier_id, char const* const path_length) {
    struct message_s mymsg;

    mymsg.mtype = MESSAGE_TYPE;
    sprintf(mymsg.mtext,"CONFIG###%d###PLEN###%s$$$", subcarrier_id, path_length);

    if (msgsnd(msg_queue_id, &mymsg, msg_size, IPC_NOWAIT) != 0) {
        perror("failed pathLength_set"); 
        //maybe queue is full, check msgctl()
    }
}


void outputPower_set(int subcarrier_id, char const* const outPower) {
    struct message_s mymsg;

    mymsg.mtype = MESSAGE_TYPE;
    sprintf(mymsg.mtext,"CONFIG###%d###OUTPOW###%s$$$", subcarrier_id, outPower);

    if (msgsnd(msg_queue_id, &mymsg, msg_size, IPC_NOWAIT) != 0) {
        perror("failed outputPower_set"); 
        //maybe queue is full, check msgctl()
    }
}


void direction_set(int subcarrier_id, char const* const direction) {
    struct message_s mymsg;

    mymsg.mtype = MESSAGE_TYPE;
    sprintf(mymsg.mtext,"CONFIG###%d###DIRECTION###%s$$$", subcarrier_id, direction);

    if (msgsnd(msg_queue_id, &mymsg, msg_size, IPC_NOWAIT) != 0) {
        perror("failed direction_set"); 
        //maybe queue is full, check msgctl()
    }
}



void fec_set(int subcarrier_id, int16_t message_length, int16_t block_length) {
    //message_length: number of message bits
    //block_length: total number of bits (message+redundancy)
    struct message_s mymsg;

    mymsg.mtype = MESSAGE_TYPE;

    if (message_length==5) {
        sprintf(mymsg.mtext,"CONFIG###%d###CHANGEFEC###1$$$", subcarrier_id);
    }
    else {
        sprintf(mymsg.mtext,"CONFIG###%d###CHANGEFEC###2$$$", subcarrier_id);
    }
    
    if (msgsnd(msg_queue_id, &mymsg, msg_size, IPC_NOWAIT) != 0) {
        perror("failed fec_set"); 
        //maybe queue is full, check msgctl()
    }
}


void central_frequency_reconfig(int subcarrier_id, char const* const oldfreq, char const* const newfreq) {
    struct message_s mymsg;

    mymsg.mtype = MESSAGE_TYPE;

    sprintf(mymsg.mtext,"CONFIG###%d###CHANGEWAVE###%s###%s$$$", subcarrier_id, oldfreq, newfreq);

    if (msgsnd(msg_queue_id, &mymsg, msg_size, IPC_NOWAIT) != 0) {
        perror("failed central_frequency_set"); 
        //maybe queue is full, check msgctl()
    }
}


void central_frequency_set(int subcarrier_id, char const* const newfreq) {
    struct message_s mymsg;

    mymsg.mtype = MESSAGE_TYPE;

    sprintf(mymsg.mtext,"CONFIG###%d###NEWWAVE###%s$$$", subcarrier_id, newfreq);

    if (msgsnd(msg_queue_id, &mymsg, msg_size, IPC_NOWAIT) != 0) {
        perror("failed central_frequency_set"); 
        //maybe queue is full, check msgctl()
    }
}


void band_set(int subcarrier_id, char const* const band) {
    struct message_s mymsg;

    mymsg.mtype = MESSAGE_TYPE;

    sprintf(mymsg.mtext,"CONFIG###%d###BAND###%s$$$", subcarrier_id, band);

    if (msgsnd(msg_queue_id, &mymsg, msg_size, IPC_NOWAIT) != 0) {
        perror("failed band_set"); 
        //maybe queue is full, check msgctl()
    }
}

void state_central_frequency(int subcarrier_id, char const* const cf) {
    struct message_s mymsg;

    mymsg.mtype = MESSAGE_TYPE;
    sprintf(mymsg.mtext,"UPDATE###%d###CENTRFREQ###%s$$$", subcarrier_id, cf);

    if (msgsnd(msg_queue_id, &mymsg, msg_size, IPC_NOWAIT) != 0) {
        perror("failed state_central_frequency"); 
        //maybe queue is full, check msgctl()
    }
}*/
