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

/* Private functions */
void make_negative(char **data, void *context);
void read_digit(char **data, void *context);
void read_string(char **data, void *context);
void read_element(char **data, void *context);
void read_key(char **data, void *context);
void read_value(char **data, void *context);


/* do an extern declare here so there are no ordering issues of the
   different FSMs in the source code */
extern transition integer_fsm[];
extern transition string_fsm[];
extern transition list_fsm[];
extern transition dict_fsm[];


transition integer_fsm[] =
  {
    {0, EXACT_STRING("i"),              1, -1                       },
    {1, EXACT_STRING("-"),              2, -1, NORMAL, make_negative},
    {1, EXACT_STRING("0"),              3, -1                       },
    {1, SINGLE_CHARACTER("123456789"),  4, -1, NORMAL,    read_digit},
    {2, EXACT_STRING("0"),              3, -1                       },
    {2, SINGLE_CHARACTER("123456789"),  4, -1, NORMAL,    read_digit},
    {3, EXACT_STRING("e"),              5, -1,                      },
    {4, SINGLE_CHARACTER("0123456789"), 4, -1, NORMAL,    read_digit},
    {4, EXACT_STRING("e"),              5, -1                       },
    {-1},
  };

transition string_fsm[] =
  {
    {0, EXACT_STRING("0"),              1, -1                     },
    {0, SINGLE_CHARACTER("123456789"),  2, -1, NORMAL, read_digit },
    {1, EXACT_STRING(":"),              3, -1                     },
    {2, SINGLE_CHARACTER("0123456789"), 2, -1, NORMAL, read_digit },
    {2, EXACT_STRING(":"),              3, -1, NORMAL, read_string},
    {-1},
  };

transition list_fsm[] =
  {
    {0, EXACT_STRING("l"),          1, -1                      },
    {1, EXACT_STRING("e"),          2, -1                      },
    
    /* read an element of the list */
    {1, FSM(integer_fsm),           1, -1, NORMAL, read_element},
    {1, FSM(string_fsm),            1, -1, NORMAL, read_element},
    {1, FSM(list_fsm),              1, -1, NORMAL, read_element},
    {1, FSM(dict_fsm),              1, -1, NORMAL, read_element},
    
    {-1},
  };

transition dict_fsm[] =
  {
    {0, EXACT_STRING("d"),          1, -1                    },
    {1, EXACT_STRING("e"),          3, -1                    },
    
    /* read a key */
    {1, FSM(string_fsm),            2, -1, NORMAL, read_key  },
    
    /* read a value */
    {2, FSM(integer_fsm),           1, -1, NORMAL, read_value},
    {2, FSM(string_fsm),            1, -1, NORMAL, read_value},
    {2, FSM(list_fsm),              1, -1, NORMAL, read_value},
    {2, FSM(dict_fsm),              1, -1, NORMAL, read_value},
    
    {-1},
  };

  
transition bencode_fsm[] = 
  {
    /* read as many various bencoded values there are, until there are
       no more */
    {0, FSM(integer_fsm),           0, -1, ACCEPT },
    {0, FSM(string_fsm),            0, -1, ACCEPT },
    {0, FSM(list_fsm),              0, -1, ACCEPT },
    {0, FSM(dict_fsm),              0, -1, ACCEPT },
    
    {-1}
  };

void make_negative(char **data, void *context)
{
}

void read_digit(char **data, void *context)
{
}

void read_string(char **data, void *context)
{
}

void read_element(char **data, void *context)
{
}

void read_key(char **data, void *context)
{
}

void read_value(char **data, void *context)
{
}

int main(int argc, char **argv)
{

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
  ret = run_fsm(bencode_fsm, &str, NULL);
  if(ret < 0) {
    printf("Unable to execute FSM on string: %s\n", str);
  } else {
    printf("\nFSM Done - processed %d characters.\n", ret);
  }

  return 0;
}
