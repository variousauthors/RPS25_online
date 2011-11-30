#include "referee.h"

main () {

  int clientFd, clientFd_2;
  int serverFd = make_server_file_descriptor();

  printf("%s\n", "LOG: Rock Paper Scissors client is up.");
  fflush(stdout);

  listen (serverFd, 5); /* Maximum pending connection length */

  // server outer run loop: this is the loop that waits for clients
  while (1) {

    // wait for client connection 
    printf("%s\n", "LOG: Waiting for players...");
    fflush(stdout);
   
    /* Accept a client connection */
    clientFd = accept (serverFd, 0, 0);
    printf("%s\n", "LOG: Player One: ready");
    fflush(stdout);

    /* Accept a second client connection */
    clientFd_2 = accept (serverFd, 0, 0);
    printf("%s\n", "LOG: Player Two: ready");
    fflush(stdout);

    printf("%s\n", "LOG: Two players have connected");
    fflush(stdout);

    // get ready to FORK
    pid_t pid;

    /* Create child to run the event */ 
    if ((pid = fork ()) == 0) {

      int keep_going; // ha ha ha we can't use continue because it is reserved
   
      // fork's run loop: this is the rock paper scissors game 
      do {
        rock_paper_scissors(clientFd, clientFd_2);
        keep_going = ask_the_players_to_continue(clientFd, clientFd_2);    
  
      } while (keep_going);

      printf("%s\n", "LOG: Tell both players to hang up\n");

      disconnect_client(clientFd);
      disconnect_client(clientFd_2);

      printf("%s\n", "LOG: Close the connections\n");
      
      exit(0);
    } else {

      waitpid(pid, NULL);
      printf("%s\n", "LOG: Back from child.");
      printf("%s\n", "LOG: Game over");
      fflush(stdin);
    }

  }

  printf("LOG: server exiting\n");
  return 0;
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

  // read length and allocate enough space
  read(Fd, &length, sizeof(int));
  char * message = malloc(length);
  read(Fd, message, length);

  return message;
}

int get_symbol_index(int Fd) {

  enum Signal signal = GET_SYMBOL;
  int index;

  write(Fd, &signal, sizeof(int));
  read(Fd, &index, sizeof(int)); // get the 0-index into the SYMBOLS array

  return index;
}

int perform_one_contest(int player_1, int player_2) {

  echo_output("Choose Your WEAPON:", player_1, player_2);
  
  // get player one's choice
  display_output("  wait while player 1 chooses...", player_2);
  int index_1 = get_symbol_index(player_1);

  display_output("You chose...", player_1);
  display_output(SYMBOLS[index_1], player_1);
  display_output("  wait while player 2 chooses...", player_1);

  // get player two's choice
  int index_2 = get_symbol_index(player_2);

  display_output("You chose...", player_2);
  display_output(SYMBOLS[index_2], player_2);

  printf("%s\n", "LOG: display results\n");
  printf("LOG: result: %d\n", RESULT_MATRIX[index_1][index_2]);

  // build a string to display "symbol vs symbol"
  char buffer[256];
  snprintf(buffer, sizeof buffer, "%s %s %s\n", 
           SYMBOLS[index_1], "vs", SYMBOLS[index_2]);

  char * message = malloc(strlen(buffer));
  strcpy(message, buffer);
  
  echo_output(message, player_1, player_2);

  free(message);

  return RESULT_MATRIX[index_1][index_2];
}

void adjudicate_game(int result, int player_1, int player_2) {

  // 1 is a player 1 victory, 2 is a player 2 victory
  switch (result) {
    case  0 :
      printf("LOG: tie game\n");

      echo_output("Tie Game!", player_1, player_2);

      break;

    case  1 :
      printf("LOG: player 1 wins!\n");

      display_output("You Win!", player_1);
      display_output("Player 1 wins...", player_2);
      break;

    case  2 :
      printf("LOG: player 1 loses...\n");

      display_output("Player 2 wins...", player_1);
      display_output("You Win!", player_2);
      break;

    default :
      // This should not occur.
      printf("LOG: HOW DID YOU BREAK THIS?\n");
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

  display_output(confirmation_message, Fd);

  enum Signal signal = CONFIRM; // setting the signal is important

  int confirms;
  write(Fd, &signal, sizeof(int));
  read(Fd, &confirms, sizeof(int));

  return confirms;
}

int rock_paper_scissors(int player_1, int player_2) {

  printf("LOG: Game %d Starts\n", ++GAME_NUMBER);
  int result = perform_one_contest(player_1, player_2);

  printf("LOG: Announcing results\n");
  adjudicate_game(result, player_1, player_2);

}

int ask_the_players_to_continue(int player_1, int player_2) {
  int keep_going;

  display_output("Waiting for player 1...", player_2);

  // get confirmation from both players
  int player_1_keep_going = user_confirms("Continue? (y/n)", player_1); 
  display_output("Waiting for player 2...", player_1);
  int player_2_keep_going = user_confirms("Continue? (y/n)", player_2); 

  if (player_1_keep_going && player_2_keep_going) {

    keep_going = 1;
    echo_output("Both players have agreed to CONTINUE", player_1, player_2);


  } else if (player_1_keep_going) {

    keep_going = 0;
    echo_output("Player 2 is backing down...", player_1, player_2);


  } else if (player_2_keep_going) {

    keep_going = 0;
    echo_output("Player 1 is backing down...", player_1, player_2);


  } else {
    keep_going = 0;
    echo_output("You both agreed to stop playing.", player_1, player_2);

  }

  return keep_going;
}

int make_server_file_descriptor() {
  // boiler plate

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

int echo_output(char* message, int Fd_1, int Fd_2) {

  display_output(message, Fd_1);
  display_output(message, Fd_2);

  return 1; // for boolean 
}
