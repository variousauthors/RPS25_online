#include "player.h"

main () {

  // get a file descriptor that identifies the server
  int clientFd = establish_client_file_descriptor();

  // set up some local variables
  enum Signal signal;
  char affirm = 'r'; // r = response. 
  int sentinel = 1;
  int length;
  int confirm;
  char buffer[256];

  // show the splash page to new users
  display_title();

  // the run loop: this will continue until the server sends HANG_UP
  while(sentinel) {

    // wait for instructions from the server
    signal = (enum Signal)wait_for_signal(clientFd);

    switch(signal) {

      // the server says: hang up!
      case HANG_UP : 

        // we write to let server know we got the message
        write(clientFd, &affirm, sizeof(char));
        sentinel = 0; // takes us out of the loop
        break;

      // the server says: output the next string
      case DISPLAY_OUTPUT : 

        // get the length of incoming input and set up a string
        read(clientFd, &length, sizeof(int));
        char * message = malloc(length);

        read(clientFd, message, length);
        printf("  %s\n", message); // display the message

        // we write to let server know we got the message
        write(clientFd, &affirm, sizeof(char));
        free(message);
        break;

      // the server says: send me input from the user
      case GET_SYMBOL : 
        display_choices(); // shwo the menu

        selection = get_integer_in_range(1,3);
        selection--; // the symbol array is 0 indexed

        write(clientFd, &selection, sizeof(int));
        break;

      case GET_STRING :

        // get a string from the user and malloc enough memory to keep it
        get_string(buffer, 256, stdin); 
        message = malloc(strlen(buffer));
        strcpy(message, buffer);
        
        // first send the length, then the message
        length = strlen(message) + 1; 
        write(clientFd, &length, sizeof(int));
        write(clientFd, message, length);

        free(message); 
        break;

      case CONFIRM :
        // get confirmation of something from the user
        confirm = user_confirms();

        write(clientFd, &confirm, sizeof(int));
        break;

      default :
        break;
    } 
  } 

  // we are finished now
  return 0;
}

int get_text_of_given_length (char * buffer, int max_length, FILE * stream) {

  int i = 0;

  // read one character at a time until they are ALL GONE
  while (true) {
  
    // stop prematurely if we get less
    int ch = fgetc(stream);
    if (ch == '\n' || ch == EOF) {
      break;
    }

    // if there are more character on the stream we ignore them
    if (i < max_length - 1) {
      buffer[i++] = ch;
    }
  }
  
  // get that NULL in there
  buffer[i] = '\0';

  return 1; // return true for boolean 
}

int user_confirms() {

  char choice;
    
  // just keep getting char until the user says y or n
  while (choice != 'y' && choice != 'n') {
    int ch = fgetc(stdin);
    choice = (char)ch;
  }

  putc(choice, stdin);

  return (choice == 'y'); // returns boolean
}

int get_integer_in_range(int min, int max) {

  int number = -1; // maybe this should be int min

  // keep trying to get an int in range until we have it
  while( true ) {

    if ( get_an_integer(&number) ) {

      if (integer_is_in_range(number, min, max) ) {
        break;
      }

      // chide the user on fail
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

// positive integers please
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

  // OH BABY! (^0^)//

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
  read(Fd, &signal, sizeof(int)); // client will wait until something comes
  return signal;
}

