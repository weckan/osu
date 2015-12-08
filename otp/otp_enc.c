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
    printf("%s", msg);
    exit(EXIT_FAILURE);
}

//Function to open and copy input plaintext file to buffer, checking that all
//chars are appropriate and that the file exists. Returns pointer to buffer.
void get_text(char *fileText, char *filename) {

    //open file, raise error if nonexistent
    FILE *f = fopen(filename, "r");
    if(f == NULL) {
        error("File open error");
    }

    char c;
    int numRead = 0;
    //read in vars
    while((c = fgetc(f)) != EOF) {
        //verify good characters
        if(((c - 'A') >= 0 && (c - 'A') <= 25)){
            fileText[numRead] = c;
        }
        else if((c == ' ') || (c == '\n')){
            fileText[numRead] = c;
        }
        else {
            error("Bad character in file");
        }
        numRead++;
    }
    //null-terminate string
    fileText[numRead] = '!';// indicate that data is being sent from encode
    fileText[numRead + 1] = '\0';

    fclose(f);
}

int main(int argc, char *argv[])
{
    //initialize variables
    int sockfd, portno, n, i;
    struct sockaddr_in serv_addr;
    struct hostent *server;

    //verify and receive command line args
    char buffer[256];
    if (argc < 4) {
       fprintf(stderr,"usage %s <plaintext> <key> <portnum>\n", argv[0]);
       exit(EXIT_FAILURE);
    }
    portno = atoi(argv[3]);

    //receive and verify key
    char *keyText = malloc(sizeof(char*) * 70000);
    get_text(keyText, argv[2]);
    //receive and verify plaintext file
    char *fileText = malloc(sizeof(char*) * 70000);
    get_text(fileText, argv[1]);

    if (strlen(keyText) < strlen(fileText)) {
        error("Key too short");
    }

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

    //connect to socket
    if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) {
        //catch connect errors
        error("ERROR connecting");
    }

    n = write(sockfd,keyText,strlen(keyText));
        if (n < 0)
             error("ERROR writing to socket");
    sleep(1);
    n = write(sockfd,fileText,strlen(fileText));
        if (n < 0)
             error("ERROR writing to socket");
    memset(buffer, '\0', 256);
    while(n = read(sockfd,buffer,255) > 0) {
        if (n < 0)
            error("ERROR reading from socket");
        buffer[256] = '\0';
        printf("%s", buffer);
        bzero(buffer,256);
    }
    printf("\n");
    close(sockfd);
    return 0;
}
