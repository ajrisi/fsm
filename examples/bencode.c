/**
 * @file   bencode.c
 * @author Adam Risi <ajrisi@gmail.com>
 * @date   Wed Jul  7 12:59:15 2010
 * 
 * @brief This is more complicated example using all of the different
 * transition types in order to parse and output bencoded data.
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
int read_string(char **data, void *context);
void read_element(char **data, void *context);
void read_key(char **data, void *context);
void read_value(char **data, void *context);
void finished_int(char **data, void *context);

/* do an extern declare here so there are no ordering issues of the
   different FSMs in the source code */
extern transition integer_fsm[];
extern transition string_fsm[];
extern transition list_fsm[];
extern transition dict_fsm[];

/* a context for reading the bencoded data - the context must be able
   to hold temp information about the different elements being built
   up by the FSM at any given time - this means it has to hold parts
   for the construction of integers, strings, etc */
struct bencode_context {
  
  /* integer reading context - to read an int we need an is_neg and
     the integer itself */
  int int_is_neg;
  int int_value;

};

transition integer_fsm[] =
  {
    {0, EXACT_STRING("i"),               1, -1, NORMAL, NULL , "read i indicating integer"},
    {1, EXACT_STRING("-"),               2, -1, NORMAL, make_negative  },
    {1, EXACT_STRING("0"),               3, -1                         },
    {1, SINGLE_CHARACTER("123456789"),   4, -1, NORMAL, read_digit     , "read digit in integer"},
    {2, EXACT_STRING("0"),               3, -1                         },
    {2, SINGLE_CHARACTER("123456789"),   4, -1, NORMAL, read_digit     , "read digit in integer"},
    {3, EXACT_STRING("e"),              -1, -1, ACCEPT, finished_int   },
    {4, SINGLE_CHARACTER("0123456789"),  4, -1, NORMAL, read_digit     , "read digit in integer"},
    {4, EXACT_STRING("e"),              -1, -1, ACCEPT, finished_int   },
    {-1},
  };

transition string_fsm[] =
  {
    {0, EXACT_STRING("0"),               1, -1                     },
    {0, SINGLE_CHARACTER("123456789"),   2, -1, NORMAL, read_digit },
    {1, EXACT_STRING(":"),               3, -1                     },
    {2, SINGLE_CHARACTER("0123456789"),  2, -1, NORMAL, read_digit },
    {2, EXACT_STRING(":"),               3, -1                     },
    {3, FUNCTION(read_string),          -1, -1, ACCEPT             },
    {-1},
  };

transition list_fsm[] =
  {
    {0, EXACT_STRING("l"),           1, -1                      },
    {1, EXACT_STRING("e"),          -1, -1, ACCEPT              },
    
    /* read an element of the list */
    {1, FSM(integer_fsm),            1, -1, NORMAL, read_element, "read a integer list element"},
    {1, FSM(string_fsm),             1, -1, NORMAL, read_element, "read a string list element"},
    {1, FSM(list_fsm),               1, -1, NORMAL, read_element, "read a list list element"},
    {1, FSM(dict_fsm),               1, -1, NORMAL, read_element, "read a dictionary list element"},
    
    {-1},
  };

transition dict_fsm[] =
  {
    {0, EXACT_STRING("d"),           1, -1                    },
    {1, EXACT_STRING("e"),          -1, -1, ACCEPT            },
    
    /* read a key */
    {1, FSM(string_fsm),             2, -1, NORMAL, read_key  },
    
    /* read a value */
    {2, FSM(integer_fsm),            1, -1, NORMAL, read_value},
    {2, FSM(string_fsm),             1, -1, NORMAL, read_value},
    {2, FSM(list_fsm),               1, -1, NORMAL, read_value},
    {2, FSM(dict_fsm),               1, -1, NORMAL, read_value},
     
    {-1},
  };

  
transition bencode_fsm[] = 
  {
    /* read a single bencoded value */
    {0, FSM(integer_fsm),           -1, -1, ACCEPT, NULL, "read an integer" },
    {0, FSM(string_fsm),            -1, -1, ACCEPT, NULL, "read a string" },
    {0, FSM(list_fsm),              -1, -1, ACCEPT, NULL, "read a list" },
    {0, FSM(dict_fsm),              -1, -1, ACCEPT, NULL, "read a dictionary" },
    
    {-1}
  };

void make_negative(char **data, void *context)
{
  struct bencode_context *bc = (struct bencode_context*)context;
  bc->int_is_neg = 1;
}

void read_digit(char **data, void *context)
{
  struct bencode_context *bc = (struct bencode_context*)context;
  bc->int_value *= 10;
  bc->int_value += **data - '0';
}

int read_string(char **data, void *context)
{
  /* we used read_digit to build up the ints describing the length of
     the string, all we need to do now is go ahead and read that many
     bytes */
  int strlen;
  struct bencode_context *bc = (struct bencode_context*)context;

  strlen = bc->int_value;

  printf("read a string %.*s\n", strlen, *data);
	 
  /* cleanup the used context variables */
  bc->int_is_neg = 0;
  bc->int_value = 0;

  return strlen;
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

void finished_int(char **data, void *context)
{
  struct bencode_context *bc = (struct bencode_context*)context;
  printf("read an integer: %d\n", bc->int_value * (bc->int_is_neg == 1 ? -1 : 1));
  bc->int_is_neg = 0;
  bc->int_value = 0;
}

int main(int argc, char **argv)
{

  char *str;
  int ret;
  struct bencode_context context = {0};

  /* read a string from the user */
  str = calloc(256, 1);
  if(str == NULL) {
    printf("Unable to allocate string storage space.\n");
    return 1;
  }
  printf("Please enter a string containing whitespace:\n");
  fgets(str, 255, stdin);

  printf("Processing %d byte string...\n", strlen(str));
  /* process string through FSM */
  ret = run_fsm(bencode_fsm, &str, (void*)&context);
  if(ret < 0) {
    printf("Unable to execute FSM on string: %s\n", str);
  } else {
    printf("\nFSM Done - processed %d characters.\n", ret);
  }

  return 0;
}
