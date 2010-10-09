/**
 * @file   whitespace.c
 * @author Adam Risi <ajrisi@gmail.com>
 * @date   Tue Jul  6 20:20:29 2010
 * 
 * @brief An example of how to use the FSM code. This demonstration
 * takes input from the user and replaces all instances of whitespace
 * characters - spaces, tabs, newlines, and carriage returns, with the
 * word "WHITESPACE".
 * 
 * 
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fsm.h>

void print_whitespace(char **data, int data_used, void *notused, void *notused2)
{
  printf(" WHITESPACE ");
}

int print_char(char **data, void *notused, void *notused2)
{
  if(**data == '\0') {
    /* we hit the null character, we are done */
    return -1;
  }

  printf("%c", **data);
  return 1;
}

int main(int argc, char **argv)
{
  transition whitespace_fsm[] = {
    {0, SINGLE_CHARACTER("\n\r \t"),      0, -1, ACCEPT, print_whitespace},
    {0, FUNCTION(print_char),             0, -1, ACCEPT                  },
    {-1}
  };
  char *str;
  int ret;

  /* read a string from the user */
  str = calloc(256, 1);
  if(str == NULL) {
    printf("Unable to allocate string storage space.\n");
    return 1;
  }
  printf("Please enter a string containing whitespace:\n");
  fgets(str, 255, stdin);

  /* process string through FSM */
  ret = run_fsm(whitespace_fsm, &str, NULL, NULL, NULL);
  if(ret < 0) {
    printf("Unable to execute FSM on string: %s\n", str);
  } else {
    printf("\nFSM Done - processed %d characters.\n", ret);
  }

  return 0;
}
