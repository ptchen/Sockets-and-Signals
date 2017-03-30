#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>
#include <ctype.h>

#define PORT "5000"
#define BACKLOG 10

// find the number of digits in a string
int count_digit(char *s)
{
    int i = 0;
    while (*s != '\0') {
    	if (isdigit(*s))
       		i++;
    s++;
    }

    return i;
}

// clean up used child
static void sigchld_hdl(int sig)
{
	// Wait for all used processes to finish
	while (waitpid(-1, NULL, WNOHANG) > 0){
	}
}

int main(void)
{
	int sock_fd, new_fd, gai, buf_len, sso = 1, infinite_loop = 1;
	struct addrinfo serv_addr, *serv_info, *p;
	struct sockaddr_storage client_addr;
	socklen_t sin_size;
	struct sigaction sa;
	char s[INET6_ADDRSTRLEN], buf[BUFSIZ];
	
    // set server information
	memset(&serv_addr, 0, sizeof serv_addr);
	serv_addr.ai_family = AF_UNSPEC;
	serv_addr.ai_socktype = SOCK_STREAM;
	serv_addr.ai_flags = AI_PASSIVE;

	// get address info and raise error if it fails
	if ((gai = getaddrinfo(NULL, PORT, &serv_addr, &serv_info)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(gai));
		exit(EXIT_FAILURE);
	}

	// loop through serv_info and bind it to first option available
	for (p = serv_info; p != NULL; p = p->ai_next) {
		
		// raise error if socker isn't created
		if ((sock_fd = socket(p->ai_family, p->ai_socktype,
				p->ai_protocol)) < 0) {
			perror("server: socket wasn't created");
			continue;
		}

		// raise error if setsockopt function fails
		if (setsockopt(sock_fd, SOL_SOCKET, SO_REUSEADDR, &sso,
				sizeof(int)) < 0) {
			perror("setsockopt: failed");
			exit(EXIT_FAILURE);
		}

		// raise error if bind function fails
		if (bind(sock_fd, p->ai_addr, p->ai_addrlen) < 0) {
			close(sock_fd);
			perror("server: bind didn't work");
			continue;
		}
        break;
	}

	// raise error if server failed to bind
	if (p == NULL)  {
		fprintf(stderr, "server: failed to bind\n");
		exit(EXIT_FAILURE);
	}

	// serv_info structure won't be used anymore
	freeaddrinfo(serv_info);

	// listen to incoming sockets
	if (listen(sock_fd, BACKLOG) < 0) {
		perror("listen: failed");
		exit(EXIT_FAILURE);
	}

	// dead processes should be reaped
	sa.sa_handler = sigchld_hdl;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_RESTART;

	// raise error if sigaction fails
	if (sigaction(SIGCHLD, &sa, NULL) < 0) {
		perror("server: sigaction failed");
		exit(EXIT_FAILURE);
	}

	printf("server is now ready and waiting for client connection\n");
    int count_line = 0;
	while(infinite_loop) {
        
        // clear the buffer by reset to 0
        memset(&buf[0], 0, sizeof(buf));

        // accept incoming socket
		sin_size = sizeof client_addr;
		new_fd = accept(sock_fd, (struct sockaddr *)&client_addr, &sin_size);
		if (new_fd < 0) {
			perror("accept");
			continue;
		}
        
        // receive buffer, raise error if fail
	    if ((buf_len = recv(new_fd, buf, BUFSIZ-1, 0)) < 0) {
			perror("recv: couldn't receive from client");
			exit(EXIT_FAILURE);
		}
        count_line++;
		
	    printf("server: %shas %d digits", buf, count_digit(buf));
        printf(" and %d lines has been received\n", count_line);
        close(new_fd); 
    }
    return 0;
}