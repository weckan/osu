/* Filename: keygen.c
 * Author: Andrew Weckwerth
 * Purpose: produces a string of random characters of specified length (command
 * lin input), using only uppercase letters and space character.
 */

#include <string.h>
#include <time.h>
#include <stdlib.h>
#include <stdio.h>

int main(int argc, char *argv[]) {

    //ensure appropriate num command line args
    if (argc != 2) {
        printf("Usage: keygen <length>\n");
    }
    else {
        //initialize variables
        int i, len; //i for loop controller, len for user input length
        len = atoi(argv[1]); //convert from char[] to int
        char key[len + 1]; //hold key at least as long as input length
        char values[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ "; //holds possible values

        //seed rand generator
        srand(time(NULL));

        for(i = 0; i < len; i++) {
            key[i] = values[rand() % 26]; //get char at random index of values
        }
        key[len] = '\0'; // null-terminate string
        printf("%s\n", key);
    }
    return 0;
}

