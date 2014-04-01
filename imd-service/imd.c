#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <fcntl.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>
#include <sys/stat.h>

#define BUFSIZE 1024
#define MAXERRS 16

/*
 * error - wrapper for perror used for bad syscalls
 */
void error(char *msg) {
  perror(msg);
  exit(1);
}

void rand_str(char *dest, size_t length) {
	srand(time(NULL));
    char charset[] = "0123456789"
                     "abcdefghijklmnopqrstuvwxyz"
                     "ABCDEFGHIJKLMNOPQRSTUVWXYZ";

    while (length-- > 0) {
        size_t index = (double) rand() / RAND_MAX * (sizeof charset - 1);
        *dest++ = charset[index];
    }
    *dest = '\0';
}

int main(int argc, char **argv) {

	/* variables for connection management */
	int parentfd;          /* parent socket */
	int childfd;           /* child socket */
	int portno;            /* port to listen on */
	int childPID;           /* child socket */
	int clientlen;         /* byte size of client's address */
	struct hostent *hostp; /* client host info */
	char *hostaddrp;       /* dotted decimal host addr string */
	int optval;            /* flag value for setsockopt */
	struct sockaddr_in serveraddr; /* server's addr */
	struct sockaddr_in clientaddr; /* client addr */


	FILE *stream;          	/* stream version of childfd */
	FILE *message;
	char buf[BUFSIZE];     	/* message buffer */
  	char to[50];     	
	char filename[255]; 
	char directory[255]; 
	char msgname[45]; 

	struct stat st = {0};

	/* check command line args */
	portno = 5865;
	
	/* open socket descriptor */
	parentfd = socket(AF_INET, SOCK_STREAM, 0);
	if (parentfd < 0) 
		error("ERROR opening socket");

	/* allows us to restart server immediately */
	optval = 1;
	setsockopt(parentfd, SOL_SOCKET, SO_REUSEADDR, 
		(const void *)&optval , sizeof(int));

	/* bind port to socket */
	bzero((char *) &serveraddr, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
	serveraddr.sin_port = htons((unsigned short)portno);
	if (bind(parentfd, (struct sockaddr *) &serveraddr, 
		sizeof(serveraddr)) < 0) 
	error("ERROR on binding");

	/* get us ready to accept connection requests */
	if (listen(parentfd, 5) < 0) /* allow 5 requests to queue up */ 
	error("ERROR on listen");

	/* 
	* main loop: wait for a connection request, parse HTTP,
	* serve requested content, close connection.
	*/
	clientlen = sizeof(clientaddr);
	while (1) {

		/* wait for a connection request */
		childfd = accept(parentfd, (struct sockaddr *) &clientaddr, &clientlen);
		if (childfd < 0) 
			error("ERROR on accept");

		childPID = fork();

		if(childPID >= 0) // fork was successful
		{
		    if(childPID == 0) // child process
		    {
		        /* determine who sent the message */
				hostp = gethostbyaddr((const char *)&clientaddr.sin_addr.s_addr, 
					sizeof(clientaddr.sin_addr.s_addr), AF_INET);

				if (hostp == NULL)
					error("ERROR on gethostbyaddr");

				hostaddrp = inet_ntoa(clientaddr.sin_addr);

				if (hostaddrp == NULL)
					error("ERROR on inet_ntoa\n");

				/* open the child socket descriptor as a stream */
				if ((stream = fdopen(childfd, "r+")) == NULL)
					error("ERROR on fdopen");
		
				fgets(buf, BUFSIZE, stream);
				sscanf(buf, "TO:%s\n", to);
				printf("MSG To: %s\n",  to);
		
				rand_str(msgname,40);
				strcpy(directory, "");;
				strcat(directory, "../messages/");
				strcat(directory, to);

				if (stat(directory, &st) == -1) {
					error("Error unknown user");
				}			

				strcpy(filename, directory);
				strcat(filename, "/");
				strcat(filename, msgname);
				strcat(filename, ".xml");
				printf("MSG File: %s\n",  filename);
				message = fopen(filename,"w");
		
				fgets(buf, BUFSIZE, stream);
				while(strcmp(buf, "MESSAGE_END\n")) {			
			  		fputs(buf, message);
					printf("BUF: %s",  buf);
					fgets(buf, BUFSIZE, stream);
				}
				printf("MSGEND");
				fclose(message);
				return 0;
			}
		}
		else // fork failed
		{
		    printf("\n Fork failed, quitting!!!!!!\n");
		    return 1;
		}
	}
}
