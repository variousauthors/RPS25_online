#include "OSCommander.h"
#include <stdio.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <sys/un.h>                /* For AFUNIX sockets */
#define DEFAULT_PROTOCOL    0

#include "protocol.h"

/* gets an integer in the given range, from min to max. 
 * @param min the least acceptable number
 * @param max the greates acceptable number 
 * @returns an integer in the given range */
int get_integer_in_range(int min, int max);

/* gets an integer from stdin. 
 * @param number a pointer to populate with the integer 
 * @returns true if and only if an integer was read */
int get_an_integer(int * number);

/* Confirms that a given number is in the given range. 
 * @param number the number in question
 * @param min the least number in the range 
 * @param max the greatest number in the range 
 * @returns true if and only if the given number is in the range */
int integer_is_in_range(int number, int min, int max) {

  return (min <= number && number <= max);
}

void get_string(char * buffer, int length, FILE * stream) {
  get_text_of_given_length(buffer, length, stream);
}

void display_choices() {
  printf("\n");
  
  int i;
  for(i = 0; i < CARDINALITY; i++) {
    printf("  %d. %s\n", i + 1, SYMBOLS[i]);
  }
  
  printf("\n");
  fflush(stdout);
} 

void display_title() {

  system("clear");
  printf("     ***** ***          ***** **          *******    \n");
  printf("  ******  * **       ******  ****       *       ***  \n");
  printf(" **   *  *  **      **   *  *  ***     *         **  \n");
  printf("*    *  *   **     *    *  *    ***    **        *   \n");
  printf("    *  *    *          *  *      **     ***          \n");
  printf("   ** **   *          ** **      **    ** ***        \n");
  printf("   ** **  *           ** **      **     *** ***      \n");
  printf("   ** ****          **** **      *        *** ***    \n");
  printf("   ** **  ***      * *** **     *           *** ***  \n");
  printf("   ** **    **        ** *******              ** *** \n");
  printf("   *  **    **        ** ******                ** ** \n");
  printf("      *     **        ** **                     * *  \n");
  printf("  ****      ***       ** **           ***        *   \n");
  printf(" *  ****    **        ** **          *  *********    \n");
  printf("*    **     *    **   ** **         *     *****      \n");
  printf("*               ***   *  *          *                \n");
  printf(" **              ***    *            **              \n");
  printf("                  ******   .--.      .            \n");
  printf("                    ***   :    :     |   o         \n");
  printf("                          |    |.--. |   .  .--. .-. \n");
  printf("                          :    ;|  | |   |  |  |(.-' \n");
  printf("                           `--' '  `-`--' `-'  `-`--'\n");
  printf("  Welcome to the Paper, Scissors and Rock game.\n");
  fflush(stdout);
}

int establish_client_file_descriptor() {

  int clientFd, serverLen, result;
  struct sockaddr_un serverUNIXAddress;
  struct sockaddr* serverSockAddrPtr;
  serverSockAddrPtr = (struct sockaddr*) &serverUNIXAddress;
  serverLen = sizeof (serverUNIXAddress);

  /* Create a UNIX socket, bidirectional, default protocol */
  clientFd = socket (AF_UNIX, SOCK_STREAM, 0);
  serverUNIXAddress.sun_family = AF_UNIX; /*Server domain */

  strcpy (serverUNIXAddress.sun_path,"recipe");/*Server name*/

  do /* Loop until a connection is made with the server */
  {
    result = connect (clientFd, serverSockAddrPtr, serverLen);
    if (result == -1) sleep (1); /* Wait and then try again */
  }
  while (result == -1);

  return clientFd;
}

int wait_for_signal(int Fd) {
  int signal;
  read(Fd, &signal, sizeof(int));
  return signal;
}

main () {

  int clientFd = establish_client_file_descriptor();
  enum Signal signal;
  char affirm = 'r';
  int sentinel = 1;
  int length;
  int confirm;
  char buffer[256];

  display_title();

  while(sentinel) {
    signal = (enum Signal)wait_for_signal(clientFd);

    switch(signal) {
      // the server says: hang up!
      case HANG_UP : 

        // we write to let server know we got the message
        write(clientFd, &affirm, sizeof(char));
        sentinel = 0;
        break;

      // the server says: output the next string
      case DISPLAY_OUTPUT : 

        read(clientFd, &length, sizeof(int));
        char * message = malloc(length);
        read(clientFd, message, length);
        printf("  %s\n", message);

        // we write to let server know we got the message
        write(clientFd, &affirm, sizeof(char));
        free(message);
        break;

      // the server says: send me input from the user
      case GET_SYMBOL : 
        display_choices(); 

        selection = get_integer_in_range(1,3);
        selection--; // the symbol array is 0 indexed

        write(clientFd, &selection, sizeof(int));
        break;

      case GET_STRING :

        get_string(buffer, 256, stdin);        
        message = malloc(strlen(buffer));
        strcpy(message, buffer);
        
        length = strlen(message) + 1; 
        write(clientFd, &length, sizeof(int));
        write(clientFd, message, length);
        free(message); 
        break;

      case CONFIRM :
        confirm = user_confirms();
        write(clientFd, &confirm, sizeof(int));
        break;

      default :
        break;
    } 
  } 

  // get input from user
  
  // send the input back to the client

  exit (/* EXIT_SUCCESS */ 0); /* Done */
}

readRecipe (int fd) {
  char str[200];
  while (readLine (fd, str)) /* Read lines until end-of-input */
    printf ("%s\n", str); /* Echo line from socket */
    fflush(stdout);
}

writeResponse (int fd) {
  static char * line = "Eggs? Pease? Anything but spam please!";  

  write(fd, line, strlen(line) + 1); 
  
}

/* Read a single NULL-terminated line */
readLine (int fd, char* str) {
  int n;

  /* Read characters until NULL or end-of-input */
  do {
    n = read (fd,str, 1); /* Read one character */

  } while (n > 0 && *str++ != '\0');

  return (n > 0); /* Return false if end-of-input */
}

int get_text_of_given_length (char * buffer, int max_length, FILE * stream) {

  int i = 0;

  while (true) {
  
    int ch = fgetc(stream);
    if (ch == '\n' || ch == EOF) {
      break;
    }

    if (i < max_length - 1) {
      buffer[i++] = ch;
    }
  }
  
  buffer[i] = '\0';
  return 1;
}

int user_confirms() {

  char choice;
    
  while (choice != 'y' && choice != 'n') {
    int ch = fgetc(stdin);
    choice = (char)ch;
  }

  putc(choice, stdin);
  return (choice == 'y');
}

int get_integer_in_range(int min, int max) {

  int number = -1;

  while( true ) {
    if ( get_an_integer(&number) ) {

      if (integer_is_in_range(number, min, max) ) {
        break;
      }

      printf("%2s... %d\n", "Input was not in range", number);
      printf("%2s ", "Enter your selection:");
    }
    
  }

  return number;
}

int get_an_integer(int * number) {

  char buff[13];

  // if we succesfully read from stdin
  if ( fgets(buff, sizeof buff, stdin) ) {

    // return true if sscanf read one integer
    return sscanf(buff, "%d", number) == 1;
  }
  
  return 0;
}

