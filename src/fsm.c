/**
 * @file   fsm.c
 * @author Adam Risi <ajrisi@gmail.com>
 * @date   Tue Jul  6 20:33:30 2010
 * 
 * @brief  This is the Finite State Machine source code.
 * 
 * 
 */


#include <string.h>

#ifdef FSM_DEBUG
#include <stdio.h>
#endif

#include "fsm.h"

int depth = 0;


/* Private Functions */
static int run_transition(transition *trans, char **data, void **context, dup_fn dup_context, free_fn free_context);

static int run_transition(transition *trans, char **data, void **context, dup_fn dup_context, free_fn free_context)
{
  /* printf("run_transition\n"); */

  if((trans == NULL) ||
     (data == NULL)) {
    /* we need to be provided a valid transition and data in order to
       run a transition */
    /* printf("run_transition error\n"); */
    return -1;
  }

#ifdef FSM_DEBUG
  depth++;
  if(trans->transition_name != NULL) {
    int i; for(i = 0; i < depth; i++) printf(" ");
    printf("attempting transition %s\n", trans->transition_name);
  }
#endif

  /* switch on the transition type - are we trying to do a string
     match, a single character match, a function execution, or a whole
     seperate FSM? */

  switch(trans->match_type) {

  case EXACT_STR: {
    /* check to see if the string stored in the transition matches the
       string at the beginning of the data - if so, return the length
       of the matched string, if not, then return -1 */
    /* printf("run_transition on an exact string\n"); */
    
    if(trans->str == NULL) {
      /* if there is no string to match, it is an error */
      return -1;
    }
    if(memcmp(*data, trans->str, strlen(trans->str)) == 0) {
      /* the string matched, return the length of the matched
	 string */
#ifdef FSM_DEBUG
      if(trans->transition_name != NULL) {
	int i; for(i = 0; i < depth; i++) printf(" ");
	printf("made transition %s with string %s\n", trans->transition_name, trans->str);
      }
      depth--;
#endif
      return strlen(trans->str);
    } else {
      /* no matching string, return -1 for no transition made */
#ifdef FSM_DEBUG
      depth--;
#endif
      return -1;
    }
  } break;

  case SINGLE_CHR: {
    /* check to see if any of the single characters in trans->str
       match the first character of *data - if so, return the number 1
       for one character matched, else, -1 for no transition made */
    int i;
    /* printf("run_transition trans on single char\n"); */
    if(trans->str == NULL) {
      /* unable to transition on NULL! */
      return -1;
    }

    for(i = 0; i < strlen(trans->str); i++) {
      if(*data[0] == trans->str[i]) {
#ifdef FSM_DEBUG
	if(trans->transition_name != NULL) {
	  int j; for(j = 0; j < depth; j++) printf(" ");
	  printf("made transition %s with character %c\n", trans->transition_name, trans->str[i]);
	}
	depth--;
#endif
	return 1;
      }
    }

    /* no single character match made, return -1 */
#ifdef FSM_DEBUG
    depth--;
#endif
    return -1;
  } break;

  case SUBFSM: {
    /* here, we want to run a FSM, and use its output to determine if
       a transition could be made - in interesting question arises -
       how do we preserve the context before traveling down a FSM that
       could alter it so that it can be restored after the FSM if it
       could not be completed? This might need to be a version 0.3
       problem */
    /* printf("transitioning to another FSM\n"); */
    void *context_copy;
    int ret;

    if(trans->transition_table == NULL) {
      /* unable to transition on an empty transition table */
      return -1;
    }

    /* make a copy of the context so that if the sub-FSM succeeds,
       then we keep the new copy, and if it fails, we keep the old
       one */
    if(dup_context != NULL) {
      context_copy = dup_context(*context);
      if(context_copy == NULL) {
	/* there was a problem with making a copy of the context - abort! */
	return -1;
      }
    } else {
      /* there was no context-copy function, so just set the copy to the original */
      context_copy = *context;
    }

    /* run the sub FSM on the copy of the context */
    ret = run_fsm(trans->transition_table, data, &context_copy, dup_context, free_context);    

    if(ret >= 0) {
      /* successful sub FSM  - keep the new context and free the old one */
      if(free_context != NULL) {
	free_context(*context);
      }
      *context = context_copy;

#ifdef FSM_DEBUG
      if(trans->transition_name != NULL) {
	int i; for(i = 0; i < depth; i++) printf(" ");
	printf("made transition %s with FSM\n", trans->transition_name);
      }
      depth--;
#endif
    } else {
      /* sub FSM failed, free the duplicated context */
      if(free_context != NULL) {
	free_context(context_copy);
      }
    }

    return ret;
    
  } break;
    
  case FUNC: {
    /* here, we run a function, and use its output to determine if we
       are going to transition or not. The function should act just
       like run_transition - it should return -1 on no transition, and
       0 or more on transition */
    int ret;
    void *context_copy;

    if(trans->action == NULL) {
      return -1;
    }

    if(dup_context != NULL) {
      context_copy = dup_context(*context);
      if(context_copy == NULL) {
	/* there was a problem with making a copy of the context - abort! */
	return -1;
      }
    } else {
      /* there was no context-copy function, so just set the copy to the original */
      context_copy = *context;
    }
    
    ret = trans->action(data, context_copy, trans->local_context);
    if(ret >= 0) {
      /* good transition, keep the new context, free the old one */
      if(free_context != NULL) {
	free_context(*context);
      }
      *context = context_copy;      
    } else {
      /* transition failed, free the new context */
      if(free_context != NULL) {
	free_context(context_copy);
      }
    }
    

    return ret;
  } break; 

  case INVALID: {
    /* this should never really happen in code, its absolutely an
       error on the programmers part */
    return -1;
  } break;

  default:
    /* invalid transition type, return -1 */
    /* printf("run_transition error\n"); */
    return -1;
  }
  

  return -1;
}

int run_fsm(transition action_table[], char **data, void **context, dup_fn dup_context, free_fn free_context)
{
  int current_state = 0;
  int nbytes_processed = 0;
  int in_accept = 0;
  
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

      /* printf("attempting to transition from %s at state %d\n", *data, current_state); */

      /* check to see if the current state is right, and the
	 transition condition succeeds */
      if(current_state == current_trans->current_state) {
	/* we need a copy of the data pointer because, if a failing
	   transition happens where some of the data is processed in
	   another FSM, we can not have that sub-FSM moving our data
	   pointer, so we give it a copy, and only let ourselves move it
	   based on the returned amount of processed bytes */
	char *data_copy = *data;
	

	/* if we are in a transition moving from our current state.. */
	if((nbytes_used_transing = run_transition(current_trans, &data_copy, context, dup_context, free_context)) >= 0) {
	  /* successful transition! run the function to be executed on
	     transition (if there is one), then move forward the number
	     of bytes processed in the input stream */
	  /* printf("run_transition success\n"); */
	  if(current_trans->transfn != NULL) {
	    current_trans->transfn(data, nbytes_used_transing, *context, current_trans->local_context);
	  }

	  /* move forward the number of bytes used transitioning */
	  nbytes_processed += nbytes_used_transing;
	  *data += nbytes_used_transing;
	  
	  /* change the state to the success state */
	  current_state = current_trans->state_pass;
	  
	  /* if the target of this transition was an accept state,
	     mark that, otherwise, clear the in_accept variable */
	  in_accept = 0;
	  if(current_trans->type == ACCEPT) {
	    in_accept = 1;
	  } else if (current_trans->type == REJECT) {
	    /* if we are in a reject state, then we immediately abort */
	    return -1;
	  } else {
	    /* our target was a normal state, nothing special to do */
	  }


	  /* printf("transition done, data aligned at %s\n", *data); */

	  /* finally, mark this as a successful transition, and break
	     from the transition-hunting for loop */
	  successful_trans = 1;
	  break;
	} else {
	  /* the transition failed. check to see if the state_fail is
	     positive indicating that there is a state to move to if
	     this transition fails. if the state_fail is negative, it
	     just gets ignored */
	  if(current_trans->state_fail >= 0) {
	    current_state = current_trans->state_fail;
	  }
	}
      }
    }

    /* if we are done with the for loop, and there was NOT a
       successful transition, then move to the error state, -1 */
    if(successful_trans == 0) {
      current_state = -1;
    }

 
  }

  /* return 0 or more if we landed in an ACCEPT state, and -1 if we
     were unable to use the FSM to parse the input */
  return (in_accept == 1) ? nbytes_processed : -1;
}
