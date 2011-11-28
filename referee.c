#include <stdio.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <sys/un.h>		/* For AFUNIX sockets */

#include "protocol.h"

// each row i is the symbol player 1 chose
// each col j is the symbol player 2 chose
// each entry ij is how player 1 fared
int RESULT_MATRIX[CARDINALITY][CARDINALITY] = {
  /*       R   P   S */ 

/* R */  { 0,  2,  1},
/* P */  { 1,  0,  2},
/* S */  { 2,  1,  0}
};
/* legend:
    0: tie
    1: player 1 wins
    2: player 2 wins */

int display_output(char * message, int Fd);
char * get_string(int Fd);
int get_symbol_index(int Fd);
int perform_one_contest(int player_1, int player_2);
void adjudicate_game(int result, int player_1, int player_2);
int disconnect_client(int Fd); 
int user_confirms(char * confirmation_message, int Fd);

int make_server_file_descriptor() {

  int serverFd, serverLen, clientFd;
  struct sockaddr_un serverUNIXAddress;/* Server address */
  struct sockaddr* serverSockAddrPtr; /* Ptr to server address */

  serverSockAddrPtr = (struct sockaddr*) &serverUNIXAddress;
  serverLen = sizeof (serverUNIXAddress);

  /* Create a UNIX socket, bidirectional, default protocol */
  serverFd = socket (AF_UNIX, SOCK_STREAM, 0);
  serverUNIXAddress.sun_family = AF_UNIX; /* domain type */

  strcpy (serverUNIXAddress.sun_path, "recipe"); /* Set name */
  unlink ("recipe"); /* Remove file if it already exists */

  bind (serverFd, serverSockAddrPtr, serverLen); /* Create file */

  return serverFd;
}

main () {

  int clientFd, clientFd_2;
  int serverFd = make_server_file_descriptor();

  printf("%s\n", "Rock Paper Scissors client is up.");
  fflush(stdout);

  listen (serverFd, 5); /* Maximum pending connection length */

  while (1) {

    // wait for client connection 
    printf("%s\n", "Waiting for players...");
    fflush(stdout);
   
    /* Accept a client connection */
    clientFd = accept (serverFd, 0, 0);
    printf("%s\n", "Player One: ready");
    fflush(stdout);

    clientFd_2 = accept (serverFd, 0, 0);
    printf("%s\n", "Player Two: ready");
    fflush(stdout);

    printf("%s\n", "GO!");
    fflush(stdout);

    int rc;
    int length;
    char ch;
    enum Signal signal;
    pid_t pid;

    /* Create child to run the event */ 
    if ((pid = fork ()) == 0) {

      int keep_going; // ha ha ha we can't use continue because it is reserved
   
      do {
        printf("Game Starts\n");
        int result = perform_one_contest(clientFd, clientFd_2);

        printf("Announcing results\n");
        adjudicate_game(result, clientFd, clientFd_2);
  
        display_output("Waiting for player 1...", clientFd_2);

        int player_1_keep_going = user_confirms("Continue? (y/n)", clientFd); 
        display_output("Waiting for player 2...", clientFd);
        int player_2_keep_going = user_confirms("Continue? (y/n)", clientFd_2); 

        if (player_1_keep_going && player_2_keep_going) {

          keep_going = 1;
          display_output("Both players have agreed to CONTINUE", clientFd);
          display_output("Both players have agreed to CONTINUE", clientFd_2);
          display_output("Waiting for player 1...", clientFd_2);

        } else if (player_1_keep_going) {

          keep_going = 0;
          display_output("Player 2 is backing down...", clientFd);
          display_output("Player 2 is backing down...", clientFd_2);

        } else if (player_2_keep_going) {

          keep_going = 0;
          display_output("Player 1 is backing down...", clientFd);
          display_output("Player 1 is backing down...", clientFd_2);

        } else {
          keep_going = 0;
          display_output("You both agreed to stop playing.", clientFd);
          display_output("You both agreed to stop playing.", clientFd_2);
        }

      } while (keep_going);

      printf("%s\n", "Tell both players to hang up\n");

      disconnect_client(clientFd);
      disconnect_client(clientFd_2);

      printf("%s\n", "Close the connections\n");
      

      exit(0);
    } else {

      waitpid(pid, NULL);
      printf("%s\n", "Back from child.");
      printf("%s\n", "Game over");
      fflush(stdin);
    }

  }

  printf("server exiting\n");

}

writeRecipe (int fd)
{
  static char* line1 = "spam, spam, spam, spam,";
  static char* line2 = "spam, and spam.";
  write (fd, line1, strlen (line1) + 1); /* Write first line */
  write (fd, line2, strlen (line2) + 1); /* Write second line */
}

readResponse(int fd) {
  char str[200];
  while(readline(fd, str))
    printf("%s\n", str);
}

readline (int fd, char * str) {

  int n;

  do {
    n = read(fd, str, 1);

  } while (n > 0 && *str++ != '\0');

  return (n > 0); 

}

int display_output(char * message, int Fd) {

  enum Signal signal = DISPLAY_OUTPUT;
  char response;

  int length = strlen(message) + 1;

  // send the signal, incoming length, then message
  write(Fd, &signal, sizeof(int));
  write(Fd, &length, sizeof(int));
  write(Fd, message, length);

  // read a response before moving on
  read(Fd, &response, sizeof(char));

  return 1; // for boolean
}

char * get_string(int Fd) {

  enum Signal signal = GET_STRING;
  int length;

  write(Fd, &signal, sizeof(int));
  read(Fd, &length, sizeof(int));

  char * message = malloc(length);
  read(Fd, message, length);

  return message;
}

int get_symbol_index(int Fd) {

  enum Signal signal = GET_SYMBOL;
  int index;

  write(Fd, &signal, sizeof(int));
  read(Fd, &index, sizeof(int));


  return index;
}

int perform_one_contest(int player_1, int player_2) {

  // get player one's choice
  int index_1 = get_symbol_index(player_1);

  display_output("You chose...", player_1);
  display_output(SYMBOLS[index_1], player_1);
  display_output("Waiting for player 2...", player_1);

  // get player two's choice
  int index_2 = get_symbol_index(player_2);

  display_output(SYMBOLS[index_2], player_2);

  printf("%s\n", "display results\n");
  printf("result: %d\n", RESULT_MATRIX[index_1][index_2]);

  return RESULT_MATRIX[index_1][index_2];
}

void adjudicate_game(int result, int player_1, int player_2) {

  switch (result) {
    case  0 :
      printf("tie game\n");

      display_output("Tie Game!", player_1);
      display_output("Tie Game!", player_2);
      break;

    case  1 :
      printf("player 1 wins!\n");

      display_output("You Win!", player_1);
      display_output("Player 1 wins...", player_2);
      break;

    case  2 :
      printf("player 1 loses...\n");

      display_output("Player 2 wins...", player_1);
      display_output("You Win!", player_2);
      break;

    default :
      printf("YOU ALL SUCK\n");
      break;
  }
}

int disconnect_client(int Fd) {

  enum Signal signal = HANG_UP;
  char response;

  write(Fd, &signal, sizeof(int));
  read(Fd, &response, sizeof(char));

  close(Fd);

  return 1; // for boolean 
}

int user_confirms(char * confirmation_message, int Fd) {

  int confirms;
    
  display_output(confirmation_message, Fd);

  enum Signal signal = CONFIRM;

printf("before write");
  write(Fd, &signal, sizeof(int));
  read(Fd, &confirms, sizeof(int));
printf("after read");

  return confirms;
}
