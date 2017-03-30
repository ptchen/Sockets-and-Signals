#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#define PORT "5000"

// get local IP address
void get_ip_addr(char *ip_addr)
{
	char buffer[200];
	struct hostent* host;

	gethostname(buffer, 200);
	host = (struct hostent *) gethostbyname(buffer);

	strcpy(ip_addr,inet_ntoa(*((struct in_addr *)host->h_addr)));
}

// get socket address
void *get_in_addr(struct sockaddr *sa)
{
	if (sa->sa_family == AF_INET){
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}

	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int main(void){

	int sockfd, gai, infinite_loop = 1;
	struct addrinfo serv_addr, *serv_info, *p;
	char ip_addr[200]; 
	char *user_input = malloc(sizeof(char)*BUFSIZ);

	while(infinite_loop){
 
        // configure server address
		memset(&serv_addr, 0, sizeof serv_addr);
		serv_addr.ai_family = AF_UNSPEC;
		serv_addr.ai_socktype = SOCK_STREAM;

		// get local IP address
		get_ip_addr(ip_addr);

		// setup address info
		if ((gai = getaddrinfo(ip_addr, PORT, &serv_addr, &serv_info)) != 0){
			fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(gai));
			exit(EXIT_FAILURE);
		}
		
        // prompt user input
        printf("Enter an alpha numeric string: ");
		fgets(user_input, BUFSIZ, stdin);
        
        if(strstr(user_input, "COOL") != NULL){
			
			// loop through serv_info and bind it to first option available
			for(p = serv_info; p != NULL; p = p->ai_next){
				
				// if socker isn't created
				if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) < 0){
					perror("client: socket wasn't created");
					continue;
				}
                // if connection function fails
				if (connect(sockfd, p->ai_addr, p->ai_addrlen) < 0){
					close(sockfd);
					perror("client: connect didn't work");
					continue;
				}
				break;
			}

			// raise error if client failed to connect
			if (p == NULL) {
				fprintf(stderr, "client: failed to connect\n");
				exit(EXIT_FAILURE);
			}

			printf("client is now connecting to server\n");

			// serv_info structure won't be used anymore
			freeaddrinfo(serv_info);

            // send data to server
			send(sockfd, user_input, strlen(user_input), 0);
	        close(sockfd);
	    }
	   
	   else{
	   	    continue;
	   }    
	}
    return 0;
}