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


#ifndef FSM_H
#define FSM_H

#define FSM_VERSION "0.3"

enum match_type {
  INVALID,
  EXACT_STR,
  SINGLE_CHR,
  SUBFSM,
  FUNC
};

enum state_type {
  NORMAL = 0,
  ACCEPT,
  REJECT
};

typedef void*(*dup_fn)(void*);
typedef void(*free_fn)(void*);

typedef struct transition_s transition;
struct transition_s {
  int current_state;

  /* the next three arguments are the type of match required for this
     transition, and the data needed to make that match. A macro is
     used so there arent ugly NULLs too much in the table (also makes
     it easier to read) */
#define EXACT_STRING(x)     EXACT_STR,     x,    NULL, NULL
#define SINGLE_CHARACTER(x) SINGLE_CHR,    x,    NULL, NULL
#define FSM(x)              SUBFSM,     NULL,       x, NULL
#define FUNCTION(x)         FUNC,       NULL,    NULL, x
#define NOTHING             EXACT_STR,    "",    NULL, NULL

  /* an internal variable, used for storing this transitions match
     type */
  enum match_type match_type;

  /* when matching against a string, the string is stored in
     command */
  char *str;

  /* set to value other than NULL if this state will transition to
     another DFA (like DFA macros) */
  transition *transition_table;

  /* a function to execute to check if the transition is valid - must
     return the number of bytes used to transition */
  int (*action)(char **data, void *global_context, void *local_context);

  int state_pass;
  int state_fail;

  /* optional arguments */
  
  /* by default, all states are normal, but can be made an accept or
     reject state if desired */
  enum state_type type;

  /* a function can be supplied that is executed if this transition is
     made */
  void (*transfn)(char **data, int nbytes_used_transing, void *global_context, void *local_context);

  void *local_context;

  char *transition_name;

};

/** 
 * Run a finite state machine on some data
 * 
 * @param action_table the actual finite state machine main table
 * @param data the data to use while running the FSM
 * @param context a context - this can be anything the user provides,
 *                and will be accessible in transition functions, etc.
 * @param dup_context a function which will duplicate the context
 * @param free_context a function which will free the memory
 *                     associated with a context
 * 
 * @return 
 */
int run_fsm(transition action_table[], char **data, void **context, dup_fn dup_context, free_fn free_context);

#endif /* FSM_H */

