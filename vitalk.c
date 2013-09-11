#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include "vito_parameter.h"
#include "vito_io.h"
#include "mysql.h"
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
  int log_intervall = 10;
  int text_log_flag = 0;
  int error_log_flag = 0;
  
  // Main event loop (select()):
  struct timeval *timeout;
  timeout = (struct timeval *) malloc( sizeof(struct timeval) );
  fd_set master_fds, read_fds; /* file descriptor list for select() */
  
  // Option processing with GNU getopt
  while ((c = getopt (argc, argv, "H:U:P:D:T:hft:se")) != -1)
    switch(c)
      {
      case 'h':
	printf("Vitalk, Viessmann Vitodens 300 (B3HA) Interface\n"
	       " (c) by KWS, 2013\n\n"
	       "Usage: vitalk [option...]\n"
	       "  -h            give this help list\n"
	       "  -H <host>     set MySQL Hostname for Logging\n"
	       "  -U <user>     set MySQL Username for Logging\n"
	       "  -P <password> set MySQL Password for Logging\n"
	       "  -D <database> set MySQL Database Name for Logging\n"
	       "                wenn nicht angegeben, wird nicht in Datenbank geloggt.\n"
	       "  -s            aktiviere Parameterwertausgabe auf stdout\n"
	       "  -e            Parameterausgabe auf stdout mit Fehlerspeicher\n"
	       "  -T <log_t>    set LogIntervall in sec.\n"
	       "  -f            activate framedebugging\n"
	       "  -t <tty_dev>  set tty Devicename to Vitodens\n"
               );
	exit(1);
      case 'H':
	my_hostname = optarg;
	break;
      case 'U':
	my_username = optarg;
	break;
      case 'P':
	my_password = optarg;
	break;
      case 'D':
	my_database = optarg;
	break;
      case 's':
	text_log_flag = 1;
	break;
      case 'e':
	error_log_flag = 1;
	break;
      case 'T':
	sscanf( optarg, "%d", &log_intervall);
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

  if ( log_intervall < 1 ||
       log_intervall > 600 )
    {
      printf("ERROR: Log Intervall must between 1 and 600 sec.!\n");
      exit(5);
    }
  
  if ( text_log_flag )
    {
      // Wenn die (wiederholte) Textausgabe der Parameter erfolgen soll,
      // löschen wir vorher besser den Bildschirm:
      printf("\033[2J\033[;H");
    }

  /////////////////////////////////////////////////////////////////////////////
  
  signal(SIGINT, exit_handler);
  signal(SIGHUP, exit_handler);

  // Clear File Descriptor Set for select()
  FD_ZERO(&master_fds);
  
  telnet_init(&master_fds);

//  opentty( tty_devicename );
//  vito_init();

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
	{ // Timeout bearbeiten:
	  if ( text_log_flag )
	    {
	      printf("TEST!\n");
//	      printf("\033[H"); // "HOME"
//	      print_all( stdout, error_log_flag ); // Parameter auf stdout ausgeben
	    }
	  if ( my_database )
	    {
	      ;
//	      my_log(); // Einen Datensatz in die SQL Datenbank schreiben
	    }
	}
    }

}

