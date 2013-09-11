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
  
  // Main event loop:
  struct timeval *timeout;
  timeout = (struct timeval *) malloc( sizeof(struct timeval) );

  // This is for the telnet Socket
  fd_set master_fds, read_fds; /* file descriptor list for select() */
  int fd_listener; /* listening socket descriptor */
  int fd_telnet; /* File Descriptor for Telnet */
  struct sockaddr_in serveraddr; /* server address */
  struct sockaddr_in clientaddr; /* client address */
  socklen_t addrlen = sizeof(clientaddr); // wird als pointer gebraucht
  char telnet_buf[80]; /* buffer for client data */
  int telnet_buf_ptr;
  int nbytes;

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
  
  /////////////////////////////////////////////////////////////////////////////
  
  signal(SIGINT, exit_handler);
  signal(SIGHUP, exit_handler);

//  opentty( tty_devicename );
//  vito_init();

  if ( text_log_flag )
    {
      // Wenn die (wiederholte) Textausgabe der Parameter erfolgen soll,
      // löschen wir vorher besser den Bildschirm:
      printf("\033[2J\033[;H");
    }

  // Clear File Descriptor Set for select()
  FD_ZERO(&master_fds);
  /* create socket */
  if((fd_listener = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0)) == -1)
    {
      fprintf( stderr, "Error creating Socket!\n");
      exit(1);
    }

  // Set REUSEADDR option
  // Sonst kann man nach einem Programmabbruch längere Zeit nicht neu starten
  const int optVal = 1;
  if ( setsockopt(fd_listener, SOL_SOCKET, SO_REUSEADDR, (void*) &optVal, sizeof(int)) == -1 )
    {
      fprintf( stderr, "Error setting Socket Options!\n");
      exit(1);
    }
  
  /* bind */
  serveraddr.sin_family = AF_INET;
  serveraddr.sin_addr.s_addr = INADDR_ANY;
  serveraddr.sin_port = htons(PORT);
  memset(&(serveraddr.sin_zero), '\0', sizeof(serveraddr.sin_zero) );
  if(bind(fd_listener, (struct sockaddr *)&serveraddr, sizeof(serveraddr)) == -1)
    {
      fprintf( stderr, "Error bind Socket!\n");
      exit(1);
    }
  /* listen */
  if(listen(fd_listener, 5) == -1)
   {
      fprintf( stderr, "Error listen Socket!\n");
      exit(1);
   }
  /* add the listener to the master set */
  FD_SET(fd_listener, &master_fds);

  // Main Event-Loop. Kann nur durch die Signalhandler beendet werden.
  for (;;)
    {
      timeout->tv_sec = 1; // select timeout immer nach einer Sekunde
      timeout->tv_usec = 0;
      read_fds = master_fds;
      if ( select ( 10, &read_fds, NULL, NULL, timeout ) > 0 )
	{
	  if ( FD_ISSET( fd_listener, &read_fds ) ) // neue Verbindung?
	    {
	      if((fd_telnet = accept(fd_listener, (struct sockaddr *)&clientaddr, &addrlen)) == -1)
		{
		  fprintf( stderr, "Error accepting Connection!\n");
		  exit(1);
		}
	      else
		{
		  FD_SET(fd_telnet , &master_fds); /* add to master set */
		  printf("New Telnet connection from %s on socket %d\n",
			 inet_ntoa(clientaddr.sin_addr), fd_telnet);
		  telnet_buf_ptr = 0;
		}
	    }
	  if ( FD_ISSET( fd_telnet, &read_fds) ) // Daten per Telnet?
	    {
	      if((nbytes = recv(fd_telnet,
				telnet_buf + telnet_buf_ptr,
				sizeof(telnet_buf) - telnet_buf_ptr - 1, 0)) <= 0)
		{ /* got error or connection closed by client */
		  //  if(nbytes == 0)
		  //    /* connection closed */
		  //     printf("socket %d hung up\n", i);
		  close(fd_telnet); /* close it */
		  FD_CLR(fd_telnet, &master_fds); /* remove from master set */
		  printf("Telnet Connection closed.\n");
		}
	      else
		{ // Hier wurden Zeichen gelesen:
		  telnet_buf_ptr += nbytes;
		  telnet_buf[telnet_buf_ptr]='\0';
		  if ( ( strchr(telnet_buf, '\n') ) || // Newline im Puffer?
		       ( telnet_buf_ptr > sizeof(telnet_buf) - 4 ) ) // // Puffer ziemlich voll?
		    { // Dann werten wir das aus:
		      printf("#%s\n", telnet_buf);
		      
		      telnet_buf_ptr = 0;
		    }
		}
	    }
	}
      else
	{
	  // Timeout
	  if ( text_log_flag )
	    {
	      printf("\033[H"); // "HOME"
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

