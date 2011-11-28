#include "unistd.h"
#include "time.h"
#include "stdbool.h"
#include "assert.h"
#include "stdio.h"
#include "string.h"
#include "stdlib.h"
#include "sys/stat.h"

// defines for constants
#define MENU_LENGTH 6
#define FILE_NAME_SIZE 64 + 1 // plus one for the terminating null

// menu is an enum and an array of strings
enum MENU_ITEM {

  rename_a_file = 1,
  delete_a_file,
  restore_deleted_file,
  output_process_information,
  execute_mode,
  exit_OSCommander

} selection;

const char * menu[MENU_LENGTH] = {
  "rename a file",
  "delete a file or directory",
  "restore a file you deleted",
  "output information about this process",
  "execute \"mode 644 ls -l\"",
  "exit OSCommander"
};

/* Garbage is files that have been "deleted" tentatively
 * by OSCommander. The original name of the garbage file
 * is stored so that it can be deleted later when OSCommander exits
 * if the number of such files exceeds MAX, OSCommander will clean up
 *
 * @see silently_delete_hidden_OSC_files() */
int GARBAGE_MAX = 100;
int GARBAGE_CURRENT = 0;
char * GARBAGE_FILES[100];

// helper methods
/* get a y or n from the user, and return 1 or 0 for boolean */
int user_confirms();

/* get an integer from min to max inclusive and return it */
int get_integer_in_range(int minimum, int maximum);

/* get an integer and populate the given reference */
int get_an_integer(int * number);

/* varifies that the given number is in the given range and returns 1 or 0 */
int integer_is_in_range(int number, int minimum, int maximum);

/* gets a text of at most the given length and puts it in the buffer */
int get_text_of_given_length(char * buffer, int max_length, FILE * stream);

/* reads a string into the given variable */
void get_file_name(char * file_name);

/* iterates over the modal menu (defined above) and prints it */
void display_modal_menu();

/* the user's input was not in the expected range */
void chide_user_for_invalid_input();

// command methods used in the switch below

/* rename a file by typing the old name and then new name */
void user_renames_a_file();

/* delete a file by typing its name. Files deleted in this way
 * can be restored by the method below, but only until OSCommander
 * cleans up. OSCommander cleans up when it shuts down or 
 * runs out of space. */
void user_deletes_a_file_or_directory();

/* restore a deleted file by typing its name. Files can only be
 * restored before exiting: a file is truely lost once OSCommander
 * shuts down. */
void user_restored_a_file_she_deleted();

/* Shows the process information for this process. Includes PID and PPID */
void display_process_information_for_user(); 

/* Executes the famous "mode" command with arguments as follows: 
 * 
 *  ./mode -c ls -m 644 -- -l 
 */
void execute_mode_for_user();

/* Shuts down OSCommander and cleans up garbage files (those deleted by the
 * user) */
void user_exits_OSCommander();

// Slightly deeper helper methods: these methods
// carry out the tasks defined by the above methods

/* renames a file if it exists */
int safely_rename_file(const char * old_name, const char * new_name);

/* Deletes a file by temporarily renaming and hiding it. 
 * The file can be restored during the session in which it
 * was deleted, but will be permanently lost when OSCommander 
 * shuts down. */
int safely_delete_file_or_directory(char * file_to_be_deleted);

/* Deletes the hidden "garbage files" files permanently. */
void silently_delete_hidden_OSC_files();

/* Hide a file of the given name by renaming it to .filename.OSC
 * This allows the user to delete and restor files during a 
 * session. */
char * hide(const char * file_name);

/* Test whether the file is a directory or a normal file */
int file_is_a_directory(const char * file_to_be_deleted);

/* Test whether the file exists */
int file_exists(const char * file_name);
