/* Filename: otp_enc.c
 * Author: Andrew Weckwerth (built off base code client.c, provided on class
 * page)
 * Date Modified: 2015-12-05
 * Purpose: Program connects to encoding "daemon" and requests a one-time-pad
 * style encryption.
 * -should raise error if key or plaintext file has bad characters
 * -should raise error if key is too short
 * -any error should set exit value to 1
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

void error(const char *msg)
{
    perror(msg);
    exit(EXIT_FAILURE);
}

int main(int argc, char *argv[])
{
    //initialize variables
    int sockfd, portno, n;
    struct sockaddr_in serv_addr;
    struct hostent *server;

    //verify and receive command line args
    char buffer[256];
    if (argc < 4) {
       fprintf(stderr,"usage %s <plaintext> <key> <portnum>\n", argv[0]);
       exit(EXIT_FAILURE);
    }
    portno = atoi(argv[3]);

    //receive and verify plaintext file


    //set up socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        //handle socket open failure
        error("ERROR opening socket");
    }

    //set up address to connect socket
    server = gethostbyname("localhost");
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr,
            server->h_length);

    serv_addr.sin_port = htons(portno);
    if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) {
        error("ERROR connecting");
    }
    printf("Please enter the message: ");
    bzero(buffer,256);
    fgets(buffer,255,stdin);
    n = write(sockfd,buffer,strlen(buffer));
    if (n < 0)
         error("ERROR writing to socket");
    bzero(buffer,256);
    n = read(sockfd,buffer,255);
    if (n < 0)
         error("ERROR reading from socket");
    printf("%s\n",buffer);
    close(sockfd);
    return 0;
}
