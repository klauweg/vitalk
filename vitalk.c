#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include "vito_io.h"
#include "telnet.h"

#define NO_VITOCOM

// Global:
fd_set master_fds;  // Aktive Filedeskriptoren für select()
fd_set read_fds;    // Ergebnis des select() - Aufrufs
int fdmax;         // maximaler benutzer Filedescriptor

// Signal Handler:
void exit_handler( int exitcode )
{
  printf("\n");
  fprintf(stderr, "\nAbort caught!\n" );
#ifdef VITOCOM
  sleep(3);
  vito_close();
  closetty();
#endif
  exit( exitcode );
}

int main(int argc, char **argv)
{
  // Option processing
  int c; 
  // Diverse Options:
  char *tty_devicename = NULL;
  
  // Option processing with GNU getopt
  while ((c = getopt (argc, argv, "hft:")) != -1)
    switch(c)
      {
      case 'h':
	printf("Vitalk, Viessmann Vitodens 300 (B3HA) Interface\n"
	       " (c) by KWS, 2013\n\n"
	       "Usage: vitalk [option...]\n"
	       "  -h            give this help list\n"
	       "  -f            activate framedebugging\n"
	       "  -t <tty_dev>  set tty Devicename\n"
               );
	exit(1);
      case 'f':
	frame_debug = 1;
	break;
      case 't':
	tty_devicename = optarg;
	break;
      case '?':
	exit (8);
      }
  
  // Do some checks:
  if ( !tty_devicename )
    {
      printf("ERROR: Need tty Devicename!\n");
      exit(5);
    }


  /////////////////////////////////////////////////////////////////////////////

  signal(SIGINT, exit_handler);
  signal(SIGHUP, exit_handler);

#ifdef VITOCOM
  opentty( tty_devicename );
  vito_init();
#endif
  
  // Das machen wir sicherheitshalber erst nach vito_init(), für den Fall dass
  // ein client sehr schnell ist:
  telnet_init();

  // Main Event-Loop. (Kann nur durch die Signalhandler beendet werden.)
  for (;;)
    {
      read_fds = master_fds;
      
      if ( select ( fdmax+1, &read_fds, NULL, NULL, NULL ) > 0 )  // SELECT
	  telnet_task();
    }

  
#if 0  
  int i=0;
  
  while( parameter_liste[i].p_name[0] )
    {
      printf("%s: %s %s (%s)\n",
	     parameter_liste[i].p_name,
	     get_v(parameter_liste[i].p_name),
	     get_u(parameter_liste[i].p_name),
	     parameter_liste[i].p_description
	     );
      i++;
    }
#endif
  
#ifdef VITOCOM
  vito_close();
  closetty();
#endif
  return 0;
}

