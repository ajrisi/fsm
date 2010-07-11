/**
 * @file   date.c
 * @author Adam Risi <ajrisi@gmail.com>
 * @date   Fri Jul  9 12:20:02 2010
 * 
 * @brief This is an example program that parses 3 different kinds of
 * dates - rfc1123, rfc850, and asctime. These are the three dates
 * used by the HTTP protocol
 * 
 * 
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <fsm.h>

#define MAX_INPUT 2048

/* Private functions */

/*
     HTTP-date    = rfc1123-date | rfc850-date | asctime-date
       rfc1123-date = wkday "," SP date1 SP time SP "GMT"
       rfc850-date  = weekday "," SP date2 SP time SP "GMT"
       asctime-date = wkday SP date3 SP time SP 4DIGIT
       date1        = 2DIGIT SP month SP 4DIGIT
       ; day month year (e.g., 02 Jun 1982)
       date2        = 2DIGIT "-" month "-" 2DIGIT
	 ; day-month-year (e.g., 02-Jun-82)
	 date3        = month SP ( 2DIGIT | ( SP 1DIGIT ))
	     ; month day (e.g., Jun  2)
       time         = 2DIGIT ":" 2DIGIT ":" 2DIGIT
	       ; 00:00:00 - 23:59:59
       wkday        = "Mon" | "Tue" | "Wed"
                    | "Thu" | "Fri" | "Sat" | "Sun"
       weekday      = "Monday" | "Tuesday" | "Wednesday"
                    | "Thursday" | "Friday" | "Saturday" | "Sunday"
       month        = "Jan" | "Feb" | "Mar" | "Apr"
                    | "May" | "Jun" | "Jul" | "Aug"
                    | "Sep" | "Oct" | "Nov" | "Dec"
*/

struct setmonth_args {
  int month_num;
};

void set_month(char **data, void *global_context, void *local_context) 
{
  struct setmonth_args *sma = (struct setmonth_args*)local_context;
  struct tm *tmp = (struct tm*)global_context;
  tmp->tm_mon = sma->month_num;
}

void set_hours(char **data, void *global_context, void *local_context)
{
  struct tm *tmp = (struct tm*)global_context;
}

void set_minutes(char **date, void *global_context, void *local_context)
{
  struct tm *tmp = (struct tm*)global_context;
}

void set_seconds(char **date, void *global_context, void *local_context)
{
  struct tm *tmp = (struct tm*)global_context;
}

void set_year(char **date, void *global_context, void *local_context)
{
  struct tm *tmp = (struct tm*)global_context;
}

struct setweekday_args
{
  int weekday;
};

void set_weekday(char **date, void *global_context, void *local_context)
{
  struct setweekday_args *swa = (struct setweekday_args*)local_context;
  struct tm *tmp = (struct tm*)global_context;
  tmp->tm_wday = swa->weekday;
}

struct settime_args
{
  enum {
    HOURS_HIGH,
    HOURS_LOW,
    MINUTES_HIGH,
    MINUTES_LOW,
    SECONDS_HIGH,
    SECONDS_LOW
  } time_component;
};

void set_time(char **date, void *global_context, void *local_context)
{
  struct settime_args *sta = (struct settime_args*)local_context;
  struct tm *tmp = (struct tm*)global_context;
  int new_digit = (int)(*date[0] - '0');

  switch(sta->time_component) {
  case HOURS_HIGH: {
    tmp->tm_hour = new_digit * 10;
  } break;

  case HOURS_LOW: {
    tmp->tm_hour += new_digit;
  } break;

  case MINUTES_HIGH: {
    tmp->tm_min = new_digit * 10;
  } break;

  case MINUTES_LOW: {
    tmp->tm_min += new_digit;
  } break;

  case SECONDS_HIGH: {
    tmp->tm_sec = new_digit * 10;
  } break;

  case SECONDS_LOW: {
    tmp->tm_sec += new_digit;
  } break;

  };

}

transition time_fsm[] =
  {
    {0, SINGLE_CHARACTER("0123456789"),  1, -1, NORMAL, set_time, (void*)&(struct settime_args){HOURS_HIGH}   },
    {1, SINGLE_CHARACTER("0123456789"),  2, -1, NORMAL, set_time, (void*)&(struct settime_args){HOURS_LOW}    },
    {2, EXACT_STRING(":"),               3, -1, NORMAL                                                        },
    {3, SINGLE_CHARACTER("0123456789"),  4, -1, NORMAL, set_time, (void*)&(struct settime_args){MINUTES_HIGH} },
    {4, SINGLE_CHARACTER("0123456789"),  5, -1, NORMAL, set_time, (void*)&(struct settime_args){MINUTES_LOW}  },
    {5, EXACT_STRING(":"),               6, -1, NORMAL                                                        },
    {6, SINGLE_CHARACTER("0123456789"),  7, -1, NORMAL, set_time, (void*)&(struct settime_args){SECONDS_HIGH} },
    {7, SINGLE_CHARACTER("0123456789"), -1, -1, ACCEPT, set_time, (void*)&(struct settime_args){SECONDS_LOW}  },
    {-1}
  };

transition wkday_fsm[] = 
  {
    /* wkday */
    {0, EXACT_STRING("Mon"), -1, -1, ACCEPT, set_weekday, (void*)&(struct setweekday_args){1} },
    {0, EXACT_STRING("Tue"), -1, -1, ACCEPT, set_weekday, (void*)&(struct setweekday_args){2} },
    {0, EXACT_STRING("Wed"), -1, -1, ACCEPT, set_weekday, (void*)&(struct setweekday_args){3} },
    {0, EXACT_STRING("Thu"), -1, -1, ACCEPT, set_weekday, (void*)&(struct setweekday_args){4} },
    {0, EXACT_STRING("Fri"), -1, -1, ACCEPT, set_weekday, (void*)&(struct setweekday_args){5} },
    {0, EXACT_STRING("Sat"), -1, -1, ACCEPT, set_weekday, (void*)&(struct setweekday_args){6} },
    {0, EXACT_STRING("Sun"), -1, -1, ACCEPT, set_weekday, (void*)&(struct setweekday_args){0} },
    {-1}
  };

transition month_fsm[] =
  {
    {0, EXACT_STRING("Jan"), -1, -1, ACCEPT, set_month, (void*)&(struct setmonth_args){0 } },
    {0, EXACT_STRING("Feb"), -1, -1, ACCEPT, set_month, (void*)&(struct setmonth_args){1 } },
    {0, EXACT_STRING("Mar"), -1, -1, ACCEPT, set_month, (void*)&(struct setmonth_args){2 } },
    {0, EXACT_STRING("Apr"), -1, -1, ACCEPT, set_month, (void*)&(struct setmonth_args){3 } },
    {0, EXACT_STRING("May"), -1, -1, ACCEPT, set_month, (void*)&(struct setmonth_args){4 } },
    {0, EXACT_STRING("Jun"), -1, -1, ACCEPT, set_month, (void*)&(struct setmonth_args){5 } },
    {0, EXACT_STRING("Jul"), -1, -1, ACCEPT, set_month, (void*)&(struct setmonth_args){6 } },
    {0, EXACT_STRING("Aug"), -1, -1, ACCEPT, set_month, (void*)&(struct setmonth_args){7 } },
    {0, EXACT_STRING("Sep"), -1, -1, ACCEPT, set_month, (void*)&(struct setmonth_args){8 } },
    {0, EXACT_STRING("Oct"), -1, -1, ACCEPT, set_month, (void*)&(struct setmonth_args){9 } },
    {0, EXACT_STRING("Nov"), -1, -1, ACCEPT, set_month, (void*)&(struct setmonth_args){10} },
    {0, EXACT_STRING("Dec"), -1, -1, ACCEPT, set_month, (void*)&(struct setmonth_args){11} },
    {-1}
  };

/* example: Mon Jan  1 12:34:45 2010 */
transition asctime_date_fsm[] =
  {
    {0,  FSM(wkday_fsm),                  1, -1, NORMAL, NULL, NULL, "weekday in asctime"},
    
    {1,  EXACT_STRING(" "),               2, -1                                          },

    /* date 3 */
    {2,  FSM(month_fsm),                  3, -1                                          },

    {3,  EXACT_STRING(" "),               4, -1                                          },
    
    {4,  SINGLE_CHARACTER("0123456789"),  5, -1                                          },
    {4,  EXACT_STRING(" "),               5, -1                                          },
    {5,  SINGLE_CHARACTER("0123456789"),  6, -1                                          },

    {6,  EXACT_STRING(" "),               7, -1                                          },
    {7,  FSM(time_fsm),                   8, -1                                          },
    
    {8,  EXACT_STRING(" "),               9, -1                                          },
    {9,  SINGLE_CHARACTER("0123456789"), 10, -1                                          },
    {10, SINGLE_CHARACTER("0123456789"), 11, -1                                          },
    {11, SINGLE_CHARACTER("0123456789"), 12, -1                                          },
    {12, SINGLE_CHARACTER("0123456789"), -1, -1, ACCEPT                                  },
    
    {-1}
  };


/* example: Monday, 01-01-10 12:34:56 GMT */
transition rfc850_date_fsm[] = 
  {
    /* weekday */
    {0,  EXACT_STRING("Monday"),    1, -1, NORMAL, set_weekday, (void*)&(struct setweekday_args){1} },
    {0,  EXACT_STRING("Tuesday"),   1, -1, NORMAL, set_weekday, (void*)&(struct setweekday_args){2} },
    {0,  EXACT_STRING("Wednesday"), 1, -1, NORMAL, set_weekday, (void*)&(struct setweekday_args){3} },
    {0,  EXACT_STRING("Thursday"),  1, -1, NORMAL, set_weekday, (void*)&(struct setweekday_args){4} },
    {0,  EXACT_STRING("Friday"),    1, -1, NORMAL, set_weekday, (void*)&(struct setweekday_args){5} },
    {0,  EXACT_STRING("Saturday"),  1, -1, NORMAL, set_weekday, (void*)&(struct setweekday_args){6} },
    {0,  EXACT_STRING("Sunday"),    1, -1, NORMAL, set_weekday, (void*)&(struct setweekday_args){0} },

    {1,  EXACT_STRING(", "), 2, -1                                                                  },

    /* date2 */
    {2,  SINGLE_CHARACTER("0123456789"), 3, -1                                                      },
    {3,  SINGLE_CHARACTER("0123456789"), 4, -1                                                      },
    {4,  EXACT_STRING("-"),              5, -1                                                      },
    {5,  FSM(month_fsm),                 6, -1                                                      },
    {6,  EXACT_STRING("-"),              7, -1                                                      },
    {7,  SINGLE_CHARACTER("0123456789"), 8, -1                                                      },
    {8,  SINGLE_CHARACTER("0123456789"), 9, -1                                                      },

    {9,  EXACT_STRING(" "),             10, -1                                                      },
    
    /* time */
    {10, FSM(time_fsm),                11, -1                                                       },

    {11, EXACT_STRING(" GMT"),         -1, -1, ACCEPT                                               },

    {-1}
  };


/* example: Mon, 01 Jan 2010 12:34:56 GMT */
transition rfc1123_date_fsm[] =
  {
    {0, FSM(wkday_fsm),                   1, -1        },

    {1, EXACT_STRING(", "),               2, -1        },

    /* date1 */
    {2, SINGLE_CHARACTER("0123456789"),   3, -1        },
    {3, SINGLE_CHARACTER("0123456789"),   4, -1        },
    {4, EXACT_STRING(" "),                5, -1        },
    {5, FSM(month_fsm),                   6, -1        },
    {6, EXACT_STRING(" "),                7, -1        },
    {7, SINGLE_CHARACTER("0123456789"),   8, -1        },
    {8, SINGLE_CHARACTER("0123456789"),   9, -1        },
    {9, SINGLE_CHARACTER("0123456789"),  10, -1        },
    {10, SINGLE_CHARACTER("0123456789"), 11, -1        },

    {11, EXACT_STRING(" "),              12, -1        },
    
    /* time */
    {12, FSM(time_fsm),                  13, -1        },

    {13, EXACT_STRING(" GMT"),           -1, -1, ACCEPT},

    {-1}
  };
       

transition http_date_fsm[] = 
  {
    /* http date, as per RFC 2616 section 3.1.1 */
    {0, FSM(rfc1123_date_fsm), -1, -1, ACCEPT, NULL, NULL, "rfc1123"},
    {0, FSM(rfc850_date_fsm),  -1, -1, ACCEPT, NULL, NULL, "rfc850" },
    {0, FSM(asctime_date_fsm), -1, -1, ACCEPT, NULL, NULL, "asctime"},
    {-1}
  };


int main(int argc, char **argv)
{
  char *str;
  int ret;
  struct tm parsed_date = {0};

  /* read a string from the user */
  str = calloc(MAX_INPUT+1, 1);
  if(str == NULL) {
    printf("Unable to allocate string storage space.\n");
    return 1;
  }
  printf("Please enter a http-style date:\n");
  fgets(str, MAX_INPUT, stdin);

  printf("Processing %d byte string...\n", (int)strlen(str));
  /* process string through FSM */
  ret = run_fsm(http_date_fsm, &str, (void*)&parsed_date);
  if(ret < 0) {
    printf("Unable to execute FSM on string: %s\n", str);
  } else {
    printf("\nFSM Done - processed %d characters.\n", ret);
  }

  return 0;
}
