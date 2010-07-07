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

int match(enum match_type mt, char *haystack, char *needle);

int run_dfa(transition action_table[], int accept_state, char **data, void *context);

#endif /* DFA_H */

