#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

void error(const char *msg)
{
  perror(msg);
  exit(EXIT_FAILURE);
}

int main(int argc, char* argv[])
{
	//msg buffer
	int buf_size = 1024;
	char buf[buf_size];
	memset(&buf, 0, sizeof buf_size);

	//TCP socket
	int SocketFD = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (SocketFD == -1) {
		error("cannot create socket");
	}

	//local sa
	struct sockaddr_in sa;
//	socklen_t sa_len;

	memset(&sa, 0, sizeof sa);
	sa.sin_family = AF_INET;
	sa.sin_port = htons(atoi(argv[2]));
	sa.sin_addr.s_addr = htonl(INADDR_ANY);
//	sa_len = sizeof(sa);

	//remote sa
	struct sockaddr_in remote_sa;
	socklen_t remote_sa_len;

	memset(&remote_sa, 0, sizeof remote_sa);
	remote_sa.sin_family = AF_INET;
	remote_sa.sin_port = htons(atoi(argv[2]));
	//TODO figure out incoming packet addr
	remote_sa.sin_addr.s_addr = htonl(INADDR_ANY);
	remote_sa_len = sizeof(remote_sa);

	//bind
	if (bind(SocketFD,(struct sockaddr *)&sa, sizeof sa) == -1) {
		close(SocketFD);
		error("bind failed");
	}

	//listen()
	if (listen(SocketFD, 200) == -1) {
		close(SocketFD);
		error("listen failed");
	}

	printf("server starts to wait\n");
	for (;;) {
		int ConnectFD = accept(SocketFD, (struct sockaddr *)&remote_sa, &remote_sa_len);

		if (0 > ConnectFD) {
			perror("accept failed");
			close(SocketFD);
			exit(EXIT_FAILURE);
		}

		/* perform read write operations ... 
		   read(ConnectFD, buff, size)
		 */
		int recsize = recvfrom(SocketFD, (void*) buf, sizeof buf, 0, (struct sockaddr*)&remote_sa, &remote_sa_len);
		if(recsize < 0)
		{
			printf("Oops, recsive < 0\n");
		}

		printf("rec: %.*s", 4, buf);

		if (shutdown(ConnectFD, SHUT_RDWR) == -1) {
			perror("shutdown failed");
			close(ConnectFD);
			close(SocketFD);
			exit(EXIT_FAILURE);
		}
		close(ConnectFD);
	}

	close(SocketFD);
	return EXIT_SUCCESS;  
}
