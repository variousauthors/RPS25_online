#include <stdio.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <sys/un.h>		/* For AFUNIX sockets */

#include "protocol.h"

// this variable should only be incremented in forks
int GAME_NUMBER = 0;

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

/* Send the given message to the client with the given file
 * descriptor. The client will then display that message
 * in an independant manner (maybe in XML on the web, or 
 * on the commandline, or graphically). 
 * @param message the string to be interpreted and displayed 
 * @param Fd the file descriptor of the client to receive the message 
 * returns true for boolean expressions */
int display_output(char * message, int Fd);

/* Just calls display output twice. This is a useful refactor
 * as it is a common activity. I would optimally have this pass
 * an array of strings, but for now this will do. When I implement
 * a chatroom I'll make this take an array. 
 * @see display_output */
int echo_output(char* message, int Fd_1, int Fd_2);

/* Have the client prompt the user in one way or another,
 * and collect a string. Get_String returns the string gotten
 * @note this function is not used anywhere in the program
 *       but it was very useful in initial explorations
 *       I have decided to include it, since I had originally
 *       intended to accept connections from an arbitrary number
 *       of players and have a chat room for them to hang out
 *       in. This method would in that case encode the functionality
 *       of receiving chat from players.
 * @param Fd the client's file descriptor 
 * @returns a string presumably input by the user */
char * get_string(int Fd);

/* have the client prompt the user for a symbol (1: rock, 2:paper...)
 * returns the index (0, 1, 2...) of the symbol in SYMBOLS 
 * @param Fd the file descriptor of the client */
int get_symbol_index(int Fd);

/* given two player file descriptor, this method instructs the 
 * client programs of each player to collect a move from them
 * and then determines the result of the moves. 
 * @param player_1 the file descriptor for P1 
 * @param player_2 the file descriptor for P1 */
int perform_one_contest(int player_1, int player_2);

/* Receives the results, and instructs the client program to
 * inform both players of those results. 
 * @param result the result of one game of RPS
 * @param player_1 the file descriptor for P1 
 * @param player_2 the file descriptor for P1 */
void adjudicate_game(int result, int player_1, int player_2);

/* requests that both clients prepare to hang up, whatever
 * that may intail on a particular implementation of the
 * RPS client (this is open source, so you know, anyone can
 * make their own client). 
 * @param Fd the file descriptor of the client being told to hang up */
int disconnect_client(int Fd); 

/* has the client request confirmation from the user and 
 * returns 1 or 0 for boolean. 
 * @param confirmation_message the message to be sent to the player
                               prompting them to confirm. 
 * @param Fd the file descriptor of the client making the choice */
int user_confirms(char * confirmation_message, int Fd);

/* Initializes the run loop for one game. This method is meant
 * to be called in a fork with two distinct client file descriptors
 * @param player_1 the file descriptor for P1 
 * @param player_2 the file descriptor for P1 */
int rock_paper_scissors(int player_1, int player_2);

/* After a game, the players will be polled as to whether they
 * are up for a rematch. This method is also responsible for
 * communicating the choices to the players. 
 * @param player_1 the file descriptor for P1 
 * @param player_2 the file descriptor for P1 */
int ask_the_players_to_continue(int player_1, int player_2);    

/* sets up a file descriptor for the server so that other 
 * programs can connect to this one. 
 * @returns a file descriptor pointing to the server */
int make_server_file_descriptor();

