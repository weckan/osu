/* Filename: otp_enc_d.c
 * Author: Andrew Weckwerth (built off example code server2.c)
 * Date Modified: 2015-12-05
 *
 * A simple server in the internet domain using TCP
   The port number is passed as an argument
   This version runs forever, forking off a separate
   process for each connection
*/
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "dynamicArray.h"

//generic error function
void error(const char *msg)
{
    perror(msg);
    exit(1);
}

//SIGCHLD handler for dead child processes, taken from TLPI p1243
static void grimReaper(int sig) {
    int savedErrno;

    savedErrno = errno;
    while(waitpid(-1, NULL, WNOHANG) > 0)
        continue;
    errno = savedErrno;
}

/******** DOSTUFF() *********************
 There is a separate instance of this function
 for each connection.  It handles all communication
 once a connnection has been established.
 *****************************************/
void encryptFxn(int sock, char *key) {

    int numRead, numWrite, totalKey = 0, totalText = 0;
    int toKey = 1;
    char inBuffer[256];
    int c;
    //arrays to hold both key and plaintext input
    DynArr *keyHolder = createDynArr(10);
    DynArr *plaintextHolder = createDynArr(10);
    char *ciphertextBuffer;


    //read from socket
    bzero(inBuffer,256);
    while((numRead = read(sock,inBuffer,255)) > 0) {
        if (numRead < 0){
            error("ERROR reading from socket");
        }
        for(i = 0; i < numRead; i++) {
            //check current character, at newline we should be at end of
            //file, switch destination array
            if(strcmp(inBuffer[i], '\n') == 0) {
                toKey = 0;
            }
            else {
                //convert to int by subtracting 'A' to make A = 0
                if(strcmp(inBuffer[i], ' ') == 0) {
                    // space character is hardcoded
                    c = 26;
                }
                else{
                    c = inBuffer[i] - 'A';
                }


                //if still reading Key, add to that array
                if(toKey == 1) {
                    //add to last position in dynamic array
                    addDynArr(keyHolder, c);
                    totalKey++;
                }
                //if key has already been read, add to plaintext array
                else {
                    addDynArr(plaintextHolder, c);
                    totalText++;
                }
            }
        }
    }

    ciphertextBuffer = malloc((sizeof(char *) * totalText) + 2);

    //encrypt
    if(totalKey < totalText) {
        error("ERROR: key must as long as input file");
    }
    for(i = 0; i < totalText; i++) {
        int k = getDynArr(keyHolder, i);
        int p = getDynArr(plaintextHolder, i);

        //add key and plaintext and take mod 26
        k += p;
        k = (k % 26);

        if(k == 26) {
            ciphertextHolder[i] = ' ';
        }
        else {
            char cipher = k + 'A';
            ciphertextHolder[i] = cipher;
        }
    }
    //terminate string with newline and null char
    ciphertextHolder[totalText] = '\n';
    ciphertextHolder[totalText + 1] = '\0';

    while ((numWrite = write(sock, ciphertextHolder,255)) > 0 ) {
        if (numWrite < 0) {
            error("ERROR writing to socket");
        }
    }
}

int main(int argc, char *argv[])
{
    //initialize vars
    int sockfd, newsockfd, portno, pid;
    socklen_t clilen;
    struct sockaddr_in serv_addr, cli_addr;
    struct sigaction sa;

    //signal handler taken from TLPI, see function definition above
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    sa.sa_handler = grimReaper;
    if (sigaction(SIGCHLD, &sa, NULL) == -1) {
        syslog(LOG_ERR, "Error from sigaction(): %s", strerror(errno));
        exit(EXIT_FAILURE);
    }

    //check command line args
    if (argc < 2) {
        fprintf(stderr,"ERROR, no port provided\n");
        exit(1);
    }

    //create and bind socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
       error("ERROR opening socket");
    bzero((char *) &serv_addr, sizeof(serv_addr));
    portno = atoi(argv[1]);
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portno);
    if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
             error("ERROR on binding");
    }

    //listen on socket for new connections
    listen(sockfd,5);
    clilen = sizeof(cli_addr);
    while (1) {

        //accept new connections
        newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
        if (newsockfd < 0)
            error("ERROR on accept");
        pid = fork();

        //handle fork error
        if (pid < 0)
            error("ERROR on fork");
        //child process
        if (pid == 0)  {
            //close parent's socket
            close(sockfd);
            //call function for child process
            dostuff(newsockfd);
            exit(0);
        }
        //parent process doesn't need new socket connection
        else close(newsockfd);
    } /* end of while */

    //close socket and exit
    close(sockfd);
    return 0; /* we never get here */
}


