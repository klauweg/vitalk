#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <time.h>
#include "vito_io.h"
#include "vito_parameter.h"
#include "text_log.h"
#include "telnet.h"

// Signal Handler:
void exit_handler( int exitcode )
{
  printf("\n");
  fprintf(stderr, "\nAbort caught!\n" );
  sleep(3);
  vito_close();
  closetty();
  exit( exitcode );
}

int main(int argc, char **argv)
{
  // Option processing
  int c; 
  // Diverse Options:
  char *tty_devicename = NULL;
  int text_log_intervall = 0;
  
  // Option processing with GNU getopt
  while ((c = getopt (argc, argv, "hft:s:")) != -1)
    switch(c)
      {
      case 'h':
	printf("Vitalk, Viessmann Vitodens 300 (B3HA) Interface\n"
	       " (c) by KWS, 2013\n\n"
	       "Usage: vitalk [option...]\n"
	       "  -h            give this help list\n"
	       "  -s <log_t>    aktiviere Parameterwertausgabe auf stdout\n"
	       "  -f            activate framedebugging\n"
	       "  -t <tty_dev>  set tty Devicename\n"
               );
	exit(1);
      case 's':
	sscanf( optarg, "%d", &text_log_intervall);
	break;
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

  if ( text_log_intervall < 0 ||
       text_log_intervall > 600 )
    {
      printf("ERROR: Log Intervall must between 1 and 600 sec.!\n");
      exit(5);
    }
  
  if ( text_log_intervall > 0 )
    {
      // Wenn die (wiederholte) Textausgabe der Parameter erfolgen soll,
      // löschen wir vorher besser den Bildschirm:
      printf("\033[2J\033[;H");
    }

  /////////////////////////////////////////////////////////////////////////////
#if 0
  // Main event loop (select()):
  struct timeval *timeout;
  timeout = (struct timeval *) malloc( sizeof(struct timeval) );
  fd_set master_fds, read_fds; /* file descriptor list for select() */
  time_t last_textlog = 0;

  signal(SIGINT, exit_handler);
  signal(SIGHUP, exit_handler);

  // Clear File Descriptor Set for select()
  // Wir machen das nicht in telnet_init, weil u.U. noch weitere TCP Dienste
  // hinzukommen könnten und dann käme es auf die Reihenfolge der Initialisierung an.
  FD_ZERO(&master_fds);
#endif  
  opentty( tty_devicename );
  
  vito_init();
#if 0
  // Das machen wir sicherheitshalber erst nach vito_init(), für den Fall dass
  // ein client sehr schnell ist:
  telnet_init(&master_fds);

  // Main Event-Loop. (Kann nur durch die Signalhandler beendet werden.)
  for (;;)
    {
      timeout->tv_sec = 1; // select timeout immer nach einer Sekunde
      timeout->tv_usec = 0;
      read_fds = master_fds;
      
      if ( select ( 10, &read_fds, NULL, NULL, timeout ) > 0 )  // SELECT
	{ // Filedescriptoren bearbeiten:
	  telnet_task( &master_fds, &read_fds );
	}
      else
	{ // select() Timeout bearbeiten:

	  if ( text_log_intervall )
	    { // Bei Bedarf Statusmeldungen auf stdout ausgeben:
	      if ( time(NULL) / text_log_intervall > last_textlog )
		{
		  last_textlog = time(NULL) / text_log_intervall;
		  printf("\033[H"); // "HOME"
		  print_all(); // Parameter auf stdout ausgeben
		}
	    }
	}
    }
#endif
  
//  printf("%s\n", get_v("deviceid"));
//goto end;
  
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
end:
  
  vito_close();
  closetty();
  return 0;
}

