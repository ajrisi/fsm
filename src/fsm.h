/**
 * @file   fsm.h
 * @author Adam Risi <ajrisi@gmail.com>
 * @date   Tue Jul  6 20:32:20 2010
 * 
 * @brief This is the header file for the Finite State Machine
 * code. It provides the data structures for the FSM action tables, as
 * well as the public API declaration for the run_fsm function that
 * executes an FSM on programmer provided input.
 * 
 * 
 */


#ifndef DFA_H
#define DFA_H

enum match_type {
  EXACT_STR,
  SINGLE_CHR,
  FSM,
  FUNC,
};

enum state_type {
  NORMAL = 0,
  ACCEPT,
  REJECT,
};

typedef struct transition_s transition;
struct transition_s {
  int current_state;

  /* the next three areguments are the type of match required for this
     transition, and the data needed to make that match. A macro is
     used so there arent ugly NULLs too much in the table (also makes
     it easier to read) */
#define EXACT_STRING(x)     EXACT_STR,  x,    NULL, NULL
#define SINGLE_CHARACTER(x) SINGLE_CHR, x,    NULL, NULL
#define FSM(x)              FSM,        NULL, x,    NULL
#define FUNCTION(x)         FUNC,       NULL, NULL, x

  /* an internal variable, used for storing this transitions match
     type */
  enum match_type match_type;

  /* when matching against a string, the string is stored in
     command */
  char *command;

  /* set to value other than NULL if this state will transition to
     another DFA (like DFA macros) */
  transition *transition_table;

  /* a function to execute to check if the transition is valid - must
     return the number of bytes used to transition */
  int (*action)(char **data, void *context);

  int state_pass;
  int state_fail;

  /* optional arguments */
  
  /* by default, all states are normal, but can be made an accept or
     reject state if desired */
  enum state_type type;

  /* a function can be supplied that is executed if this transition is
     made */
  void (*transfn)(char **data, void *context);

};

/** 
 * Run a finite state machine on some data
 * 
 * @param action_table the actual finite state machine main table
 * @param data the data to use while running the FSM
 * @param context a context - this can be anything the user provides,
 *                and will be accessible in transition functions, etc.
 * 
 * @return 
 */
int run_fsm(transition action_table[], char **data, void *context);

#endif /* DFA_H */

