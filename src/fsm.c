#include <string.h>
#include "fsm.h"

int match(enum match_type mt, char *haystack, char *needle)
{

  if(needle == NULL) {
    return 1;
  }

  if(mt == EXACT) {
    if(strncmp(haystack, needle, strlen(needle)) == 0) {
      return strlen(needle);
      /*return 1;*/
    } else {      
      return 0;
    }
  } else {
       
    int i;
    for(i = 0; i < strlen(needle); i++) {
      if(haystack[0] == needle[i]) {
	return 1;
      }
    }
    return 0;
  }
  return 0;
}

int run_fsm(transition action_table[], char **data, void *context)
{
  int current_state = 0;
  int nbytes_processed = 0;
  
  /* all possible states are numbered positively */
  while(current_state >= 0) {
    transition *current_trans;
    int nbytes_used_transing;
    int successful_trans = 0;

    /* walk the action table, looking for the first transition where
       both the current_state matches, and the character / string
       matching or the FSM execution succeeds */
    for(current_trans = action_table; 
	current_trans->current_state != -1;
	current_trans++) {

      /* check to see if the current state is right, and the
	 transition condition succeeds */
      if( (current_state == current_trans->current_state) &&
	  ((nbytes_used_transing = run_transition(current_trans, data, context)) >= 0)) {
	/* successful transition! run the function to be executed on
	   transition (if there is one), then move forward the number
	   of bytes processed in the input stream */
	if(current_trans->transfn != NULL) {
	  current_trans->transfn(data, context);
	}
	
	/* move forward the number of bytes used transitioning */
	nbytes_processed += nbytes_used_transing;

	/* finally, mark this as a successful transition, and break
	   from the transition-hunting for loop */
	successful_trans = 1;
	break;
      }
    }

    /* if we are done with the for loop, and there was NOT a
       successful transition, then move to the error state, -1 */
    if(successful_trans == 0) {
      current_state = -1;
    }

 
  }

}
