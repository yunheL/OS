/* Author: Xuyi Ruan, Yunhe Liu
 * Data: Dec 1, 2015
 * Acknowledgement: Some code in this file are
 * referenced from Linux Programmer's Manual (the Man Page)
 * and stackoverflow, and wikipedia
 */

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <time.h>
#include <sys/ioctl.h>

//function to print out error msg
void error(const char *msg)
{
  perror(msg);
  exit(-1);
}

int main(int argc, char *argv[])
{
  //valid input command
  if (argc != 5)
  {
    error("Blastee command line argument number error\n");
    exit(1);
  }
  printf("port number is: %s + echo is: %s\n", argv[2], argv[4]);

  int blastee_socket;
  char buffer[50*1024 + 4*9];
  ssize_t recsize;
  socklen_t fromlen;

  //summary info
  float total_bytes = 0;
  float total_pkts = 0;
  float duration = 0;
  int first_time = 1;
  time_t initial_sec = 0;
  long initial_usec = 0;
  time_t end_sec = 0;
  long end_usec = 0;
  time_t dur_sec = 0;
  long dur_usec = 0;
  int more_than_one = 0;

  //set blastee sa
  struct sockaddr_in sa;

  /* create an UDP socket*/
  if(-1 == (blastee_socket = socket(AF_INET, SOCK_DGRAM, 0)))
  {
    error("Socket() failed\n");
  }

  /* set blastee socket parameter */
  //Note: blastee has the addr struct sa
  memset(&sa, 0, sizeof sa);
  sa.sin_family = AF_INET;
  sa.sin_addr.s_addr = htonl(INADDR_ANY);
  //printf("binding to address\n: %s", inet_ntoa(sa.sin_addr));
  sa.sin_port = htons(atoi(argv[2]));
  fromlen = sizeof(sa);

  /* set blaster socket parameter*/
  //Note: blaster has the addr struct blaster_sa
  struct sockaddr_in blaster_sa; 

  blaster_sa.sin_family = AF_INET;
  //blaster_sa.sin_addr.s_addr = sa.sin_addr.s_addr;
  //blaster_sa.sin_addr.s_addr = inet_addr(inet_ntoa(sa.sin_addr));
  //printf("TEST: %s", inet_ntoa(sa.sin_addr));
  blaster_sa.sin_port = htons(atoi(argv[2]));

  //printf("From host %s, port %hd, ",inet_ntoa(sa.sin_addr), ntohs(sa.sin_port));

  /* bind socket to address */
  if(-1 == bind(blastee_socket, (struct sockaddr *) &sa, fromlen))
  {
    error("bind() failed\n");
  }

  /*5sec timeout*/
  struct timeval tv;

  tv.tv_sec = 5;  /* 5 Secs Timeout */
  tv.tv_usec = 0;  // Not init'ing this can cause strange errors

  if (-1 == setsockopt(blastee_socket, SOL_SOCKET, SO_RCVTIMEO, (char *)&tv,sizeof(struct timeval)))
  {
    error("ERROR: setsockopt error");
  }

  /* keep receiving */
  for (;;)
  {
    recsize = recvfrom(blastee_socket, (void*)buffer, sizeof buffer, 0, (struct sockaddr*)&sa, &fromlen);

    struct timeval stamp;
    
    ioctl(blastee_socket, SIOCGSTAMP, &stamp);

    //printf("TEST: %lu, %lu,", stamp.tv_sec, stamp.tv_usec);

    //printf("receive success! ");
    //printf("recsize = %d\n", recsize);


/*    
    if(recsize < 0)
    {
      error("resive size < 0");
      
      error(
      "Summary:\ntotal packets received: %d\n,total bytes received: %d\n,average byte/sec: %f\n,average packet/sec: %f\n,duration is: %lu.%03lu sec\n, error: recsive < 0, this can happen after 5 sec timeout\n",total_pkts, total_bytes, (float)(total_bytes)/duration,(float)(total_pkts)/duration, dur_sec, dur_usec/1000);
    
    }
*/

    if(recsize < 0)
    {
      break;
    }
    /* extract data from buffer and decode*/
    char data;
    memcpy(&data, buffer, 1);    

    long sequence;
    memcpy(&sequence, buffer+1, 4);
    sequence = ntohl(sequence);

    uint32_t length;
    memcpy(&length, buffer+5, 4);
    length = ntohl(length);
    //length = atoi(*len);

    char payload[50*1024];
    memcpy(payload, buffer+9, 50*1024);
    
    
    printf("From: %s, on port: %hd, ",inet_ntoa(sa.sin_addr), ntohs(sa.sin_port));
    //printf("received packet: ");
    //printf("data= %c, ", data);
    printf("sequence= %lu, ", sequence);
    printf("size: %d, ", length);
    printf("at time: %lu.%03lu, ", stamp.tv_sec, stamp.tv_usec/1000);
    printf("payload: ");
    int i = 0;
    for(i = 0; i < 4; i++)
    {
      printf("%c", payload[i]);
    }
    printf("\n");

    total_bytes = total_bytes + length;
    total_pkts++;

    if(first_time == 1)
    {
      first_time = 0;
      initial_sec = stamp.tv_sec;
      initial_usec = stamp.tv_usec;
    }
    else
    {
      more_than_one = 1;
      end_sec = stamp.tv_sec;
      end_usec = stamp.tv_usec;
    }

    //calculate duration
    dur_sec = end_sec - initial_sec;
    dur_usec = end_usec - initial_usec;
    duration = (float)dur_sec + (float)(dur_usec)/1000000.0;      



    /* echo mechanism */

    if(atoi(argv[4]) == 1)
    {

/*
      struct timespec time, time2;
      time.tv_sec = 0;
      time.tv_nsec = 500000000L;


      if(nanosleep(&time, &time2) < 0 )
      {
	error("nanosleep() failed");
	time.tv_nsec = 0;
      }
      else
      {
	printf("Nano sleep successfull \n");
      }
*/      
      //printf("this packet should be echoed\n");

      //label packet as echo packet
      char type = 'C';
      memcpy(buffer, &type, 1);
      int bytes_sent = 0;

      //set address to send to
      blaster_sa.sin_addr.s_addr = inet_addr(inet_ntoa(sa.sin_addr));
      //printf("TEST: %s", inet_ntoa(sa.sin_addr));
      //prtinf("TESTPORT: %hd", ntohs(sa.sin_port));

      //send out the echo packet
      bytes_sent = sendto(blastee_socket, buffer, sizeof buffer, 0, (struct sockaddr*)&blaster_sa, sizeof blaster_sa);

      //printf("bytes_sent is %d\n", bytes_sent);

    }
  
    //END mechanism
    if (data == 'E')
    {
      break;
    }
  }//end of for loop of receving packet

  printf("Summary: \n");
  printf("total packets received(include END): %.0f\n", total_pkts);
  printf("total bytes received: %.0f\n", total_bytes);
  if(more_than_one == 1)
  { 
    printf("average byte/sec: %f\n", total_bytes/duration);
    printf("average packet/sec: %f\n", total_pkts/duration);
    printf("duration is: %lu.%03lu sec\n", dur_sec, dur_usec/1000);
  }
  else
  {
    printf("average byte/sec: less than or only 1 packet\n");
    printf("average packet/sec: less than or only 1 packet\n");
    printf("duration is so small for this case\n");
  }

  printf("**reason packet/rate is different from rate in param is because END pakcet is included in calculation. If use packet -1 (excluding END), then it will be the same as rate in param\n");


  //close the socket
  if (-1 == close(blastee_socket))
  {
    error("Oops, close() failed");
  }
  else
  {
    printf("-Your socket has been successfully closed, thanks for using Xuyi-Yunhe socket!\n");
  }

  return 0;
}
