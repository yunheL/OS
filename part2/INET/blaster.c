#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <sys/un.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <errno.h>
#include <sys/types.h>
#include <time.h>
#include <pthread.h>

#define EVER ;;
//function that generates error message
void error(const char *msg)
{
  perror(msg);
  exit(-1);
}

void *connection_handler(void *);


struct my_arg
{
  int blaster_socket;
  int counter;
  struct sockaddr *blaster_sa;
  socklen_t *blaster_sa_len;

};

void *handleEcho(void *arg)
{

  while (1){
    struct my_arg *echo_arg = (struct my_arg*)arg;
    /**
      pkt_count++;

      if(pkt_count == atoi(argv[8])+1)
      {
      printf("Thanks for using Xuyi & Yunhe Socket!\n");
      break;
      }
      */

    //printf("here1\n");
    ssize_t recsize = 0;
    int buf_size = 50*1024 + 36;
    char buffer_recv[buf_size];

    recsize = recvfrom(echo_arg->blaster_socket, (void*)buffer_recv, sizeof buffer_recv, 0, echo_arg->blaster_sa, echo_arg->blaster_sa_len);
    //printf("here2\n");
    //printf("receive success! ");
    //printf("recsize = %d\n", recsize);

    char echo_type;
    memcpy(&echo_type, buffer_recv, 1);

    long echo_sequence;
    memcpy(&echo_sequence, buffer_recv+1, 4);
    echo_sequence = ntohl(echo_sequence);

    uint32_t echo_length;
    memcpy(&echo_length, buffer_recv+5, 4);
    echo_length = ntohl(echo_length);

    char echo_payload[50*1024];
    memcpy(echo_payload, buffer_recv+9, 50*1024);

    //printf("received packet: [echo] ");
    printf("[echo]");
    printf("type= %c, ", echo_type);
    printf("sequence= %lu, ", echo_sequence);
    printf("length: %d, ", echo_length);

    printf("payload: ");
    int j = 0;
    for(j = 0; j < 4; j++)
    {
      printf("%c", echo_payload[j]);
    }
    printf("\n");
  }
}


int main(int argc, char *argv[])
{
  uint32_t pkt_len = atoi(argv[12]);
  pthread_t tid =  0;

  //this struct construct the packet to be sent
  struct packet
  {
    char 	type;			//D = DATA, E = END, C = ECHO
    long 	sequence;	//increase in the number of payload octets
    uint32_t	length;			//number of octets in the payload
    //char 	payload[12];
    char	payload[pkt_len];	//vaiable size
    //char 	payload[50*1024 + 36];
  };

  //valid input command
  if (argc != 15)
  {
    error("Blaster command line argument number error\n");
    exit(1);
  }
  printf("host to send to: %s, port on receiver: %s\n", argv[2], argv[4]);

  //instantiate the udp socket
  int blaster_socket;
  blaster_socket = socket(AF_INET, SOCK_DGRAM, 0);
  if(-1 == blaster_socket)
  {
    error("socekt create failed");
  }

  //set blaster_sa address
  struct sockaddr_in blaster_sa;
  socklen_t blaster_sa_len;

  memset(&blaster_sa, 0, sizeof blaster_sa);
  blaster_sa.sin_family = AF_INET;
  blaster_sa.sin_addr.s_addr = htonl(INADDR_ANY);
  blaster_sa.sin_port = htons(atoi(argv[4]));
  blaster_sa_len = sizeof(blaster_sa);


  //set blastee_sa address
  struct sockaddr_in blastee_sa;
  socklen_t blastee_sa_len;

  memset(&blastee_sa, 0, sizeof blastee_sa);
  blastee_sa.sin_family = AF_INET;
  blastee_sa.sin_addr.s_addr = inet_addr(argv[2]);
  blastee_sa.sin_port = htons(atoi(argv[4]));
  blastee_sa_len = sizeof(blastee_sa);

  //bind instantiated socket to local
  if(-1 == bind(blaster_socket, (struct sockaddr *)&blaster_sa, blaster_sa_len))
  {
    close(blaster_socket);
    error("bind failed");
  }


  //TODO instantiate buffer size according to input
  //int buf_size = atoi(argv[12])+36; //as length < 50KB
  int buf_size = 50*1024 + 36;
  char buffer[buf_size];
  //char buffer_recv[buf_size];
  //int msg_size = atoi(argv[12]);
  int bytes_sent;
  int packet_number = atoi(argv[8]);
  //int pkt_count = 0;

  //construct packet
  struct packet pkt0;

  //the sequence number mechanism is done here
  char *ptr;
  long unsign_seq;
  long prev_byte_sent;

  unsign_seq = strtoul(argv[10], &ptr, 10);
  printf("base seq is: %lu\n", unsign_seq);
  pkt0.sequence = unsign_seq;
  prev_byte_sent = 0;


  //listening when expecting echo
  // thread_create
  if(atoi(argv[14]) == 1)
  {
    struct my_arg arg;
    arg.blaster_socket = blaster_socket;
    arg.counter = 0;
    arg.blaster_sa = (struct sockaddr*) &blaster_sa;
    arg.blaster_sa_len = &blaster_sa_len;

    pthread_create(&tid, NULL, handleEcho, (void *) &arg);

  }



  //for loop to send packet
  int i = 0;
  for(i = 0; i<packet_number + 1;i++)
  {
    //if this is already packet_num packet sent, this
    //should be the END packet
    if(i == packet_number)
    {
      pkt0.type = 'E';
    }
    //otherwise it should be either D or C based on
    //the passed in echo value
    else if(i < packet_number)
    {
      pkt0.type = 'D';
      /*
      //if echo == 1
      if(atoi(argv[14]) == 1)
      {
      pkt0.type = 'C';
      }
      //if echo == 0
      else
      {
      pkt0.type = 'D';
      }
      */
    }
    //if i gets larger than packet_number then we
    //have a problem
    else
    {
      error("error in determining pakcet type");
    }

    //stuff pkt0
    pkt0.sequence = pkt0.sequence + pkt_len;
    memset(pkt0.payload, 0, sizeof pkt0.payload);

    sprintf(pkt0.payload, "That is pakcet%d", i);
    //pkt0.length = strlen(pkt0.payload);
    pkt0.length = pkt_len;

    //record prev_byte_sent to update sequence number
    prev_byte_sent = pkt0.length;

    //copy pkt0 into buffer
    memcpy(buffer, &pkt0.type, 1);
    long seq;
    seq = htonl(pkt0.sequence);
    memcpy(buffer+1, &seq, 4);
    uint32_t len;
    len = htonl(pkt0.length);
    memcpy(buffer+5, &len, 4);
    memcpy(buffer+9, &pkt0.payload, pkt_len);

    //before send sleep for certain amound of time
    time_t sec_wait = 0;
    long nano_wait = 0;
    long raw_time = 0;
    double raw_rate = atof(argv[6]);

    if(raw_rate > 1.0)
    {
      nano_wait = 1000000000/raw_rate;
    }
    else if (raw_rate == 1.0)
    {
      sec_wait = 1;
      nano_wait = 0;
    }
    else if(raw_rate < 1.0 && raw_rate > 0)
    {
      raw_time = 1000000000/raw_rate;

      while(raw_time > 1000000000 || raw_time == 1000000000)
      {
	raw_time = raw_time - 1000000000;
	sec_wait = sec_wait + 1;
      }
      nano_wait = (long)raw_time;
    }
    else
    {
      error("ERROR: send rate cannot be 0 or negative");
    }

    struct timespec time, time2;
    time.tv_sec = sec_wait;
    time.tv_nsec = nano_wait;


    if(nanosleep(&time, &time2) < 0 )
    {
      error("nanosleep() failed");
      //time.tv_nsec = 0;
    }
    /*
       else
       {
       printf("Nano sleep successfull \n");
       }
       */

    //this is where the packet get sent
    bytes_sent = sendto(blaster_socket, buffer, sizeof buffer, 0, (struct sockaddr*)&blastee_sa, sizeof blastee_sa);

    if(bytes_sent < 0)
    {
      error("bytes_sent < 0\n");
    }


    /*print statments to display information */
    //printf("sent to host %s, port %hd, ",inet_ntoa(blastee_sa.sin_addr), ntohs(blastee_sa.sin_port));
    //printf("bytes_sent is %d\n", bytes_sent);


    //    printf("sent packet: ");
    printf("type = %c, ", pkt0.type);
    /*
    //this is the encrypted set for testing
    printf("sequence is %d, ", seq);
    printf("length is %d, ", len);
    printf("payload is %s\n", pkt0.payload);
    */
    printf("sequence= %lu, ", pkt0.sequence);
    //printf("length= %d, ", pkt0.length);
    //printf("payload= %.*s", 4,pkt0.payload);
    printf("payload: ");

    int plen = 4;
    int k = 0;
    if(pkt_len <  4)
    {
      plen = pkt_len;
    }


    for(k = 0; k < plen; k++)
    {
      printf("%c", pkt0.payload[k]);
    }
    printf("\n");
    //printf("\n");



  }//end of for for sending pakcets




  close(blaster_socket);
  //pthread_join(tid, NULL);
  return 0;
}


/*
   void *connection_handler(void *blaster_socket)
   {
   int sock = *(int*) blaster_socket;
//char *buff;
char rec_buff[36+50*1024];

struct sockaddr_in blastee_sa;
socklen_t blastee_sa_len;

memset(&blastee_sa, 0, sizeof blastee_sa);
blastee_sa.sin_family = AF_INET;
blastee_sa.sin_addr.s_addr = inet_addr("128.105.37.165");
blastee_sa.sin_port = htons(7001);
blastee_sa_len = sizeof(blastee_sa);


//  if(atoi(argv[14]) == 1)
//  {
ssize_t recsize = 0;
recsize = recvfrom(sock, (void*)rec_buff, sizeof rec_buff, 0, (struct sockaddr*)&blastee_sa, &blastee_sa_len);

printf("receive success! ");
printf("recsize = %d\n", recsize);
//  }

free(blaster_socket);
return 0;

}
*/


