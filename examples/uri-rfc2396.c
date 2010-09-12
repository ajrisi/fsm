/**
 * @file   uri-rfc2396.c
 * @author Adam Risi <ajrisi@gmail.com>
 * @date   Mon Aug 30 21:54:17 2010
 * 
 * @brief  An example of how to do URI (RFC 2396 style) parsing
 * 
 * 
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fsm.h>

#define MAX_INPUT 2048

/*
      URI-reference = [ absoluteURI | relativeURI ] [ "#" fragment ]
      absoluteURI   = scheme ":" ( hier_part | opaque_part )
      relativeURI   = ( net_path | abs_path | rel_path ) [ "?" query ]

      hier_part     = ( net_path | abs_path ) [ "?" query ]
      opaque_part   = uric_no_slash *uric

      uric_no_slash = unreserved | escaped | ";" | "?" | ":" | "@" |
                      "&" | "=" | "+" | "$" | ","

      net_path      = "//" authority [ abs_path ]
      abs_path      = "/"  path_segments
      rel_path      = rel_segment [ abs_path ]

      rel_segment   = 1*( unreserved | escaped |
                          ";" | "@" | "&" | "=" | "+" | "$" | "," )

      scheme        = alpha *( alpha | digit | "+" | "-" | "." )

      authority     = server | reg_name

      reg_name      = 1*( unreserved | escaped | "$" | "," |
                          ";" | ":" | "@" | "&" | "=" | "+" )

      server        = [ [ userinfo "@" ] hostport ]
      userinfo      = *( unreserved | escaped |
                         ";" | ":" | "&" | "=" | "+" | "$" | "," )

      hostport      = host [ ":" port ]
      host          = hostname | IPv4address
      hostname      = *( domainlabel "." ) toplabel [ "." ]
      domainlabel   = alphanum | alphanum *( alphanum | "-" ) alphanum
      toplabel      = alpha | alpha *( alphanum | "-" ) alphanum
      IPv4address   = 1*digit "." 1*digit "." 1*digit "." 1*digit
      port          = *digit

      path          = [ abs_path | opaque_part ]
      path_segments = segment *( "/" segment )
      segment       = *pchar *( ";" param )
      param         = *pchar
      pchar         = unreserved | escaped |
                      ":" | "@" | "&" | "=" | "+" | "$" | ","

      query         = *uric

      fragment      = *uric

      uric          = reserved | unreserved | escaped
      reserved      = ";" | "/" | "?" | ":" | "@" | "&" | "=" | "+" |
                      "$" | ","
      unreserved    = alphanum | mark
      mark          = "-" | "_" | "." | "!" | "~" | "*" | "'" |
                      "(" | ")"

      escaped       = "%" hex hex
      hex           = digit | "A" | "B" | "C" | "D" | "E" | "F" |
                              "a" | "b" | "c" | "d" | "e" | "f"

      alphanum      = alpha | digit
      alpha         = lowalpha | upalpha

      lowalpha = "a" | "b" | "c" | "d" | "e" | "f" | "g" | "h" | "i" |
                 "j" | "k" | "l" | "m" | "n" | "o" | "p" | "q" | "r" |
                 "s" | "t" | "u" | "v" | "w" | "x" | "y" | "z"
      upalpha  = "A" | "B" | "C" | "D" | "E" | "F" | "G" | "H" | "I" |
                 "J" | "K" | "L" | "M" | "N" | "O" | "P" | "Q" | "R" |
                 "S" | "T" | "U" | "V" | "W" | "X" | "Y" | "Z"
      digit    = "0" | "1" | "2" | "3" | "4" | "5" | "6" | "7" |
                 "8" | "9"

*/

transition digit_fsm[] = 
  {
    { 0, SINGLE_CHARACTER("0123456789"), -1, -1, ACCEPT },
    {-1}
  };

transition upalpha_fsm[] = 
  {
    { 0, SINGLE_CHARACTER("ABCDEFGHIJKLMNOPQRSTUVWXYZ"), -1, -1, ACCEPT },
    {-1}
  };

transition lowalpha_fsm[] = 
  {
    { 0, SINGLE_CHARACTER("abcdefghijklmnopqrstuvwxyz"), -1, -1, ACCEPT },
    {-1}
  };

transition alpha_fsm[] = 
  {
    {0, FSM(lowalpha_fsm), -1, -1, ACCEPT },
    {0, FSM(upalpha_fsm),  -1, -1, ACCEPT },
    {-1}
  };

transition alphanum_fsm[] = 
  {
    {0, FSM(alpha_fsm), -1, -1, ACCEPT },
    {0, FSM(digit_fsm), -1, -1, ACCEPT },
    {-1}
  };

transition hex_fsm[] = 
  {
    {0, FSM(digit_fsm), -1, -1, ACCEPT },
    {0, SINGLE_CHARACTER("ABCDEFabcdef"), -1, -1, ACCEPT },
    {-1}
  };

transition escaped_fsm[] = 
  {
    {0, EXACT_STRING("%"), 1, -1 },
    {1, FSM(hex_fsm),      2, -1 },
    {2, FSM(hex_fsm),     -1, -1, ACCEPT },
    {-1}
  };

transition mark_fsm[] =
  {
    {0, SINGLE_CHARACTER("-_.!~*\'()"), -1, -1, ACCEPT },
    {-1}
  };

transition unreserved_fsm[] =
  {
    {0, FSM(alphanum_fsm), -1, -1, ACCEPT },
    {0, FSM(mark_fsm),     -1, -1, ACCEPT },
    {-1}
  };

transition reserved_fsm[] =
  {
    {0, SINGLE_CHARACTER(";/?:@&=+$,"), -1, -1, ACCEPT },
    {-1}
  };

transition uric_fsm[] =
  {
    {0, FSM(reserved_fsm),   -1, -1, ACCEPT },
    {0, FSM(unreserved_fsm), -1, -1, ACCEPT },
    {0, FSM(escaped_fsm),    -1, -1, ACCEPT },
    {-1}
  };

transition fragment_fsm[] = 
  {
    {0, FSM(uric_fsm), 0, -1, ACCEPT },
    {-1}
  };

transition query_fsm[] = 
  {
    {0, FSM(uric_fsm), 0, -1, ACCEPT },
    {-1}
  };

transition pchar_fsm[] =
  {
    {0, FSM(unreserved_fsm), -1, -1, ACCEPT },
    {0, FSM(escaped_fsm),    -1, -1, ACCEPT },
    {0, SINGLE_CHARACTER(":@&=+$,"), -1, -1, ACCEPT },
    {-1}
  };

transition param_fsm[] = 
  {
    {0, FSM(pchar_fsm), 0, -1, ACCEPT },
    {-1}
  };

transition segment_fsm[] =
  {
    {0, FSM(pchar_fsm),    0, -1, ACCEPT },

    /* ; param */
    {0, EXACT_STRING(";"), 1, -1 },
    {1, FSM(param_fsm),    2, -1, ACCEPT },
    
    {2, EXACT_STRING(";"), 3, -1 },
    {3, FSM(param_fsm),    2, -1, ACCEPT },

    {-1}
  };
      
transition path_segments_fsm[] =
  {
    {0, FSM(segment_fsm), 1, -1, ACCEPT },
    
    /* / segment */
    {1, EXACT_STRING("/"), 2, -1 },
    {2, FSM(segment_fsm), 1, -1, ACCEPT },
    {-1}
  };

transition uric_no_slash_fsm[] =
  {
    {0, FSM(unreserved_fsm),           -1, -1, ACCEPT },
    {0, FSM(escaped_fsm),              -1, -1, ACCEPT },
    {0, SINGLE_CHARACTER(";?:@&=+$,"), -1, -1, ACCEPT },
    {-1}
  };

transition abs_path[] =
  {
    {0, EXACT_STRING("/"), 1, -1},
    {1, FSM(path_segments_fsm), -1, -1, ACCEPT },
    {-1}
  };

transition opaque_part_fsm[] =
  {
    {0, FSM(uric_no_slash_fsm), 1, -1, ACCEPT },
    {1, FSM(uric_fsm), 1, -1, ACCEPT },
    {-1}
  };

transition abs_path_fsm[] =
  {
    {0, EXACT_STRING("/"), 1, -1 },
    {1, FSM(path_segments_fsm), -1, -1, ACCEPT },
    {-1}
  };

transition path_fsm[] =
  {
    {0, FSM(abs_path_fsm),    -1, -1, ACCEPT },
    {0, FSM(opaque_part_fsm), -1, -1, ACCEPT },
    {0, NOTHING,              -1, -1, ACCEPT }, 
    {-1}
  };

transition port_fsm[] =
  {
    {0, FSM(digit_fsm), 0, -1, ACCEPT },
    {0, NOTHING, -1, -1, ACCEPT },
    {-1}
  };

transition ipv4address_fsm[] =
  {
    {0, FSM(digit_fsm), 1, -1 },
    {1, FSM(digit_fsm), 1, -1 },
    {1, EXACT_STRING("."), 2, -1 },

    {2, FSM(digit_fsm), 3, -1 },
    {3, FSM(digit_fsm), 3, -1 },
    {3, EXACT_STRING("."), 4, -1 },

    {4, FSM(digit_fsm), 5, -1 },
    {5, FSM(digit_fsm), 5, -1 },
    {5, EXACT_STRING("."), 6, -1 },

    {6, FSM(digit_fsm), 7, -1, ACCEPT },
    {7, FSM(digit_fsm), 7, -1, ACCEPT },

    {-1}
  };

transition toplabel_fsm[] =
  {
    /* alpha */
    {0, FSM(alpha_fsm), 1, -1, ACCEPT },

    /* alpha *( alphanum | "-" ) alphanum */
    {1, FSM(alphanum_fsm), 1, -1, ACCEPT }, 
    {1, EXACT_STRING("-"), 1, -1 },
    {-1}
  };

transition domainlabel_fsm[] =
  {
    /* alpha */
    {0, FSM(alphanum_fsm), 1, -1, ACCEPT },

    /* alpha *( alphanum | "-" ) alphanum */
    {1, FSM(alphanum_fsm), 1, -1, ACCEPT }, 
    {1, EXACT_STRING("-"), 1, -1 },
    {-1}
  };

transition hostname_fsm[] =
  {
    {0, FSM(domainlabel_fsm), 1, -1 },
    {0, FSM(toplabel_fsm), 2, -1, ACCEPT },

    {1, EXACT_STRING("."), 0, -1 },

    {2, EXACT_STRING("."), -1, -1, ACCEPT },
    {-1}
  };

transition host_fsm[] =
  {
    {0, FSM(hostname_fsm), -1, -1, ACCEPT },
    {0, FSM(ipv4address_fsm), -1, -1, ACCEPT},
    {-1}
  };

transition hostport_fsm[] =
  {
    {0, FSM(host_fsm), 1, -1, ACCEPT },
    {1, EXACT_STRING(":"), 2, -1 },
    {2, FSM(port_fsm), -1, -1, ACCEPT },
    {-1}
  };


transition userinfo_fsm[] =
  {
    {0, FSM(unreserved_fsm), 0, -1, ACCEPT },
    {0, FSM(escaped_fsm), 0, -1, ACCEPT },
    {0, SINGLE_CHARACTER(";:&=+$,"), 0, -1, ACCEPT },
    {0, NOTHING, -1, -1, ACCEPT},
    {-1}
  };

/*   server        = [ [ userinfo "@" ] hostport ] */
transition server_fsm[] =
  {
    {0, FSM(userinfo_fsm), 1,  2 },
    {1, EXACT_STRING("@"), 2, -1 },
    {2, FSM(hostport_fsm), -1, -1, ACCEPT},
    {2, NOTHING, -1, -1, ACCEPT },
    {-1}
  };

transition reg_name_fsm[] =
  {
    {0, FSM(unreserved_fsm), 0, -1, ACCEPT },
    {0, FSM(escaped_fsm), 0, -1, ACCEPT },
    {0, SINGLE_CHARACTER("$,;:@&=+"), 0, -1, ACCEPT },
    {-1}
  };

transition authority_fsm[] =
  {
    {0, FSM(server_fsm), -1, -1, ACCEPT },
    {0, FSM(reg_name_fsm), -1, -1, ACCEPT },
    {-1}
  };

transition scheme_fsm[] =
  {
    {0, FSM(alpha_fsm), 1, -1, ACCEPT },
    {1, FSM(alpha_fsm), 1, -1, ACCEPT },
    {1, FSM(digit_fsm), 1, -1, ACCEPT },
    {1, SINGLE_CHARACTER("+-."), 1, -1, ACCEPT },
    {-1}
  };


transition rel_segment_fsm[] =
  {
    {0, FSM(unreserved_fsm), 0, -1, ACCEPT },
    {0, FSM(escaped_fsm), 0, -1, ACCEPT },
    {0, SINGLE_CHARACTER(";@&=+$,"), 0, -1, ACCEPT },
    {-1}
  };

transition rel_path_fsm[] =
  {
    {0, FSM(rel_segment_fsm), 1, -1, ACCEPT},
    {0, FSM(abs_path_fsm), -1, -1, ACCEPT},
    {-1}
  };

transition net_path_fsm[] =
  {
    {0, EXACT_STRING("//"), 1, -1 },
    {1, FSM(authority_fsm), 2, -1, ACCEPT },
    {2, FSM(abs_path_fsm), -1, -1, ACCEPT},
    {-1}
  };

transition hier_part_fsm[] =
  {
    {0, FSM(net_path_fsm), 1, -1, ACCEPT},
    {0, FSM(abs_path_fsm), 1, -1, ACCEPT},
    {1, EXACT_STRING("?"), 2, -1},
    {2, FSM(query_fsm), -1, -1, ACCEPT},
    {-1}
  };

transition relativeuri_fsm[] =
  {
    {0, FSM(net_path_fsm), 1, -1, ACCEPT},
    {0, FSM(abs_path_fsm), 1, -1, ACCEPT},
    {0, FSM(rel_path_fsm), 1, -1, ACCEPT},
    {1, EXACT_STRING("?"), 2, -1},
    {2, FSM(query_fsm), -1, -1, ACCEPT},
    {-1}
  };

transition absoluteuri_fsm[] =
  {
    {0, FSM(scheme_fsm), 1, -1},
    {1, EXACT_STRING(":"), 2, -1},
    {2, FSM(hier_part_fsm), -1, -1, ACCEPT},
    {2, FSM(opaque_part_fsm), -1, -1, ACCEPT},
    {-1}
  };

transition uri_reference_fsm[] =
  {
    {0, FSM(absoluteuri_fsm), 1, -1, ACCEPT},
    {0, FSM(relativeuri_fsm), 1, -1, ACCEPT},
    {1, EXACT_STRING("#"), 2, -1 },
    {2, FSM(fragment_fsm), -1, -1, ACCEPT},
    {-1}
  };


int main(int argc, char **argv)
{
  char *str;
  int ret;
 
  /* read a string from the user */
  str = calloc(MAX_INPUT+1, 1);
  if(str == NULL) {
    printf("Unable to allocate string storage space.\n");
    return 1;
  }
  printf("Please enter a URI:\n");
  fgets(str, MAX_INPUT, stdin);

  printf("Processing %d byte string...\n", (int)strlen(str));
  /* process string through FSM */
  ret = run_fsm(uri_reference_fsm, &str, NULL, NULL, NULL);
  if(ret < 0) {
    printf("Unable to execute FSM on string: %s\n", str);
    return EXIT_FAILURE;
  }
  
  printf("\nFSM Done - processed %d characters.\n", ret);
 
  return 0;
}



