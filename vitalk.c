#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "vito_parameter.h"
#include "vito_io.h"
#include "mysql.h"
#include "text_log.h"

#define PORT 83

// Signal Handler:
void exit_handler( int exitcode )
{
  printf("\n");
  fprintf(stderr, "\nAbort caught!\n" );
  sleep(5);
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
  
  // Server:
  struct timeval *timeout;
  timeout = (struct timeval *) malloc( sizeof(struct timeval) );
  
  fd_set master; /* master file descriptor list */
  fd_set read_fds; /* temp file descriptor list for select() */
  struct sockaddr_in serveraddr; /* server address */
  struct sockaddr_in clientaddr; /* client address */
  int fdmax; /* maximum file descriptor number */
  int listener; /* listening socket descriptor */
  int newfd; /* newly accept()ed socket descriptor */
  int yes = 1; /* for setsockopt() SO_REUSEADDR, below */
  socklen_t addrlen;
  int i;
  char buf[1024]; /* buffer for client data */
  int nbytes;
  
  // Option processing with GNU getopt
  while ((c = getopt (argc, argv, "H:U:P:D:T:hft:c:se")) != -1)
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
	       "  -c <cache_t>  set parameter cache intervall in sec.\n"
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
      case 'c':
	sscanf( optarg, "%d", &param_cache_time );
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
  
  if ( param_cache_time < 1 ||
       param_cache_time > 600 )
    {
      printf("ERROR: Cache Time must between 1 and 600 sec.!\n");
      exit(5);
    }

  /////////////////////////////////////////////////////////////////////////////
  
  signal(SIGINT, exit_handler);
  signal(SIGHUP, exit_handler);

  opentty( tty_devicename );
  vito_init();

  timeout->tv_sec = 1; // Den ersten Timeout immer nach einer Sekunde!
  timeout->tv_usec = 0;
  
  if ( text_log_flag )
    {
      // Wenn die (wiederholte) Textausgabe der Parameter erfolgen soll,
      // löschen wir vorher besser den Bildschirm:
      printf("\033[2J\033[;H");
    }

  /* clear the master and temp sets */
  FD_ZERO(&master);
  FD_ZERO(&read_fds);
  /* create socket */
  if((listener = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
      fprintf( stderr, "Error creating Socket!\n");
      exit(1);
    }
   if(setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1)
    {
      fprintf( stderr, "Error configuring Socket!\n");
      exit(1);
    }
  /* bind */
  serveraddr.sin_family = AF_INET;
  serveraddr.sin_addr.s_addr = INADDR_ANY;
  serveraddr.sin_port = htons(PORT);
  memset(&(serveraddr.sin_zero), '\0', 8);
  if(bind(listener, (struct sockaddr *)&serveraddr, sizeof(serveraddr)) == -1)
    {
      fprintf( stderr, "Error bind Socket!\n");
      exit(1);
    }
  /* listen */
  if(listen(listener, 10) == -1)
   {
      fprintf( stderr, "Error listen Socket!\n");
      exit(1);
   }
  /* add the listener to the master set */
  FD_SET(listener, &master);
  /* keep track of the biggest file descriptor */
  fdmax = listener; /* so far, it's this one*/

  
  // Main Event-Loop. Kann nur durch die Signalhandler beendet werden.
  for (;;)
    {
      read_fds = master;
      if ( select ( fdmax+1, &read_fds, NULL, NULL, timeout ) > 0 )
	{
	  /*run through the existing connections looking for data to be read*/
	  for(i = 0; i <= fdmax; i++)
	    {
	      if(FD_ISSET(i, &read_fds))
		{ /* we got one... */
		  if(i == listener)
		    { /* handle new connections */
		      addrlen = sizeof(clientaddr);
		      if((newfd = accept(listener, (struct sockaddr *)&clientaddr, &addrlen)) == -1)
			{
			  fprintf( stderr, "Error accepting Connection!\n");
			  exit(1);
			}
		      else
			{
			  FD_SET(newfd, &master); /* add to master set */
			  if(newfd > fdmax) /* keep track of the maximum */
			    fdmax = newfd;
			  printf("New connection from %s on socket %d\n", inet_ntoa(clientaddr.sin_addr), newfd);
			}
		    }
		  else
		    { /* handle data from a client */
		      if((nbytes = recv(i, buf, sizeof(buf), 0)) <= 0)
			{ /* got error or connection closed by client */
			  if(nbytes == 0)
			    /* connection closed */
			     printf("socket %d hung up\n", i);
			  close(i); /* close it */
			  FD_CLR(i, &master); /* remove from master set */
			}
		      else
			{
			  printf("%s", buf);
			}
		    }
		}
	    }
	}
      else
	{
	  // Timeout. Die Linuximplementierung modifiziert die Restzeit
	  // falls der Timeout bei Rückkehr aus select() noch nicht abgelaufen
	  // war. War er abgelaufen, muss er in jedem Fall neu gesetzt werden:
	  timeout->tv_sec = log_intervall;
	  timeout->tv_usec = 0;
	  if ( text_log_flag )
	    {
	      printf("\033[H"); // "HOME"
	      print_all( stdout, error_log_flag ); // Parameter auf stdout ausgeben
	    }
	  if ( my_database )
	    {
	      my_log(); // Einen Datensatz in die SQL Datenbank schreiben
	    }
	}
    }

}

