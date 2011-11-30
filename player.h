#include "OSCommander.h"
#include <stdio.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <sys/un.h>                /* For AFUNIX sockets */
#define DEFAULT_PROTOCOL    0

#include "protocol.h"

/* gets an integer in the given positive range, from min to max. 
 * @param min the least acceptable number 0 < min < max
 * @param max the greates acceptable number 0 < min < max
 * @returns an integer in the given range */
int get_integer_in_range(int min, int max);

/* gets an integer from stdin. 
 * @param number a pointer to populate with the integer 
 * @returns true if and only if an integer was read */
int get_an_integer(int * number);

/* Confirms that a given number is in the given positive range. 
 * @param number the number in question
 * @param min the least number in the range , 0 < min < max
 * @param max the greatest number in the range, 0 < min < max 
 * @returns true if and only if the given number is in the range */
int integer_is_in_range(int number, int min, int max);

/* get a string from the given stream of the given length
 * actually this is just a cover method for get_text_of_given_length
 * which has quite a long name. 
 * @see get_text_of_given_length */
void get_string(char * buffer, int length, FILE * stream);

/* display the rock paper scissors symbols in an ordered list */
void display_choices();

/* display the "Splash Page" for a user that just logged in.
 * this is the ballinest method around yo. */
void display_title();

/* sets up a connection to the server and returns the file desriptor
 * @returns a file descriptor pointing to the server */
int establish_client_file_descriptor();

/* waits for a signal from the server. Each signal is interpreted
 * by the client as an instruction to wait on a particular kind of
 * input, or perform a particular kind of action. The signal is returned.
 * @returns an integer signal from the server */
int wait_for_signal(int Fd);

