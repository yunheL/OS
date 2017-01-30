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

int main(int argc, char* argv[])
{

	//instantiate the udp socket
	int send_socket;
	send_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if(-1 == send_socket)
	{
		error("send socekt create failed");
	}

	//set blaster_sa address
	struct sockaddr_in send_sa;
	socklen_t send_sa_len;

	memset(&send_sa, 0, sizeof send_sa);
	send_sa.sin_family = AF_INET;
	send_sa.sin_addr.s_addr = 
	


	return EXIT_SUCCESS;
}
