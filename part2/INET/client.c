#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int main(int argc, char* argv[])
{
	//msg buffer
	int buf_size = 1024;
	char buf[buf_size];
	memset(&buf, 1, sizeof buf_size);

	//TCP socket
	int SocketFD;
	SocketFD = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (SocketFD == -1) {
		perror("cannot create socket");
		exit(EXIT_FAILURE);
	}

	//local sa
	struct sockaddr_in sa;
//	socklen_t sa_len;

	memset(&sa, 0, sizeof sa);
	sa.sin_family = AF_INET;
	sa.sin_port = htons(atoi(argv[4]));
	sa.sin_addr.s_addr = htonl(INADDR_ANY);
//	sa_len = sizeof(sa);

	//remote sa
	struct sockaddr_in remote_sa;
//	socklen_t remote_sa_len;

	memset(&remote_sa, 0, sizeof remote_sa);
	remote_sa.sin_family = AF_INET;
	remote_sa.sin_addr.s_addr = inet_addr(argv[2]);
	remote_sa.sin_port = htons(atoi(argv[4]));
//	remote_sa_len = sizeof(remote_sa);

	//connect
	if (connect(SocketFD, (struct sockaddr *)&remote_sa, sizeof remote_sa) == -1) {
		perror("connect failed");
		close(SocketFD);
		exit(EXIT_FAILURE);
	}

	/* perform read write operations ... */
	int byte_sent = sendto(SocketFD, buf, sizeof buf, 0, (struct sockaddr*)&remote_sa, sizeof remote_sa);	
	if(byte_sent<0){
		perror("byte_sent < 0\n");
		exit(EXIT_FAILURE);
	}

	shutdown(SocketFD, SHUT_RDWR);

	close(SocketFD);
	return EXIT_SUCCESS;
}
