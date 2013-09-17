#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include "vito_parameter.h"
#include "telnet.h"

#define PORT 83
#define MAX_TELNET_SERVER 10
#define TELNET_BUFFER_SIZE 70

extern fd_set master_fds; /* file descriptor list for select() */
extern fd_set read_fds;   /* ergebnis des select() Aufrufs */
extern int fdmax; /* größter benutzter Filedescriptor */

static int fd_listener; /* listening socket descriptor */
static struct sockaddr_in serveraddr; /* server address */
static struct sockaddr_in clientaddr; /* client address */

// Per telnet aufrufbare Befehle:
const char *commands[] =
{ "help",              // 0
  "h",                 // 1
  "get",               // 2
  "g",                 // 3
  "set",               // 4
  "s",                 // 5
  "\0"
};

#if 0
// Hilfstext
static void print_help( void )
{
  dprintf(fd_telnet,
	  "Short Help Text:\n"
	  "  h, help        - This Help Text\n"
	  "  s, state       - Summary of Parameter Values\n"
	  "  e, errors      - Error History\n"
          "  numeric_errors - Error History in numeric Format\n"
	  "Changeable Parameters:\n"
	  "  set_mode       - Betriebsmodus setzen\n"
	  "        0 == Abschaltbetrieb\n"
	  "        1 == Nur Warmwasser\n"
	  "        2 == Heizen und Warmwasser\n"
	  "  set_ww         - Warmwassertemperatur in Grad Celsius setzen\n"
	  "  set_raum       - Raumtemperatur in Grad Celsius setzen\n"
	  "  set_red_raum   - reduzierte Raumtemperatur in Grad Celsius setzen\n"
	  "\n"
    );
}
# endif

// Init Telnet Socket:
void telnet_init( void )
{
  FD_ZERO(&master_fds);
  
  /* create socket */
  if((fd_listener = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0)) == -1)
    {
      fprintf( stderr, "Error creating Telnet Socket!\n");
      exit(1);
    }

  // Set REUSEADDR option
  // Sonst kann man nach einem Programmabbruch längere Zeit nicht neu starten:
  const int optVal = 1;
  if ( setsockopt(fd_listener, SOL_SOCKET, SO_REUSEADDR, (void*) &optVal, sizeof(int)) == -1 )
    {
      fprintf( stderr, "Error setting Telnet Socket Options!\n");
      exit(1);
    }
  
  /* bind */
  serveraddr.sin_family = AF_INET;
  serveraddr.sin_addr.s_addr = INADDR_ANY;
  serveraddr.sin_port = htons(PORT);
  memset(&(serveraddr.sin_zero), '\0', sizeof(serveraddr.sin_zero) );
  if(bind(fd_listener, (struct sockaddr *)&serveraddr, sizeof(serveraddr)) == -1)
    {
      fprintf( stderr, "Error bind Telnet Socket: %s\n", strerror(errno));
      exit(1);
    }
  
  /* listen */
  if(listen(fd_listener, 10) == -1) // "10" = listen backlog
    {
      fprintf( stderr, "Error listen on Telnet Socket!\n");
      exit(1);
    }
  else
    {
      fprintf( stderr, "Now listening to telnet Port %d\n", PORT );
    }
  
  /* add the listener to the master set */
  FD_SET(fd_listener, &master_fds);
  fdmax = fd_listener; // Vorerst ist das nur der Listener
}

// Folgende Funktion wird periodisch nach der select() Rückkehr aufgerufen.
// Es muss die von select() zurückgegebene Descriptormenge übergeben werden:
void telnet_task( void )
{
  int i;
  int fd_new;
  static char buffers[MAX_TELNET_SERVER][TELNET_BUFFER_SIZE];
  static int buf_ptr[MAX_TELNET_SERVER];
  int result;

  // Traverse over all possible Filedescriptors:
  for(i = 0; i <= fdmax; i++)
    {
      if( FD_ISSET(i, &read_fds) )
	{ // Neue Verbindung?
	  if( i == fd_listener )
	    { // Neue Verbindung!
	      socklen_t addrlen = sizeof(clientaddr);
	      if( ( fd_new = accept(fd_listener, (struct sockaddr *)&clientaddr, &addrlen)) == -1)
		  fprintf(stderr, "Error accepting Connection!\n");
	      else
		{
		  if ( fd_new > MAX_TELNET_SERVER )
		    {
		      fprintf(stderr, "Max Number of Telnet Connections reached!\n");
		      close( fd_new );
		    }
		  else
		    {
		      fprintf(stderr, "New connection from %s on socket %d\n",
			      inet_ntoa(clientaddr.sin_addr), fd_new);
		      FD_SET(fd_new, &master_fds); /* add to master set */
		      buffers[i][0]='\0';
		      buf_ptr[i] = 0;
		      if(fd_new > fdmax) // neuer größter Filedescriptor?
			fdmax = fd_new;
		      dprintf( fd_new, "Welcome at vitalk, the Vitodens telnet Interface. (nr. %u)\n", fd_new);
		    }
		}
	    }
	  else
	    { // Daten von einem Client:
	      result = recv(i, &buffers[i][buf_ptr[i]], 1, 0);
	      if ( result <= 0 )
		{
		  if ( result < 0 )
		    fprintf( stderr, "recv() error: %s\n", strerror(errno));
		  // Verbindung wurde beendet:
		  printf("Socket %d hung up.\n", i);
		  close(i);
		  FD_CLR(i, &master_fds);
		}
	      else
		{ // Echte Daten auswerten:
		  buf_ptr[i]++;
		  fprintf(stderr,"%s\n", buffers[i]);
		}
	    }
	}
    }
}

#if 0
	{ // Hier wurde ein Zeichen gelesen:
	  telnet_buf_ptr++;
	  telnet_buf[telnet_buf_ptr]='\0';
	  if ( ( strchr(telnet_buf, '\n') ) || // Newline im Puffer?
	       ( telnet_buf_ptr > sizeof(telnet_buf) - 4 ) ) // Oder Puffer ziemlich voll?
	    { // Dann werten wir das aus:
	      telnet_buf_ptr = 0; // Wird erst für neue Zeile wieder gebraucht
	      telnet_buf[75]='\0'; // Länger kann ein Befehl eh nicht sein
	      command[0] = '\0';
	      value = 0;
	      sscanf(telnet_buf, "%s %d", command, &value);
	      // Suche nach bekanntem Befehl:
	      for ( i=0; commands[i][0]; i++ ) 
		{
		  if ( strcmp( commands[i], command ) == 0 )
		    switch( i )
		      {
		      case 0:
		      case 1:
			print_help();
			break;
		      case 2:
		      case 3:
			print_all();
			break;
		      case 4:
		      case 5:
			dprintf(fd_telnet, "FEHLERSPEICHER:\n");
			dprintf(fd_telnet,  "%s", read_error_history() );
			dprintf(fd_telnet, "\n");
			break;
		      case 6:
			dprintf(fd_telnet, "FEHLERSPEICHER (numerisch):\n");
			dprintf(fd_telnet,  "%s", read_error_history_numeric() );
			dprintf(fd_telnet, "\n");
			break;
		      case 7:
			dprintf(fd_telnet, "Setting mode: ");
			if ( value >= 0 && value <= 2 )
			  {
			    write_mode_numeric( value );
			    dprintf(fd_telnet, "done.\n");
			  }
			else
			  dprintf(fd_telnet, "error!\n");
			break;
		      case 8:
			dprintf(fd_telnet, "Setting WW temp: ");
			if ( value >= 5 && value <= 60 )
			  {
			    write_WW_soll_temp( value );
			    dprintf(fd_telnet, "done.\n");
			  }
			else
			  dprintf(fd_telnet, "error!\n");
			break;
		      case 9:
			dprintf(fd_telnet, "Setting room temp: ");
			if ( value >= 10 && value <= 30 )
			  {
			    write_raum_soll_temp( value );
			    dprintf(fd_telnet, "done.\n");
			  }
			else
			  dprintf(fd_telnet, "error!\n");
			break;
		      case 10:
			dprintf(fd_telnet, "Setting reduced room temp: ");
			if ( value >= 10 && value <= 30 )
			  {
			    write_red_raum_soll_temp( value );
			    dprintf(fd_telnet, "done.\n");
			  }
			else
			  dprintf(fd_telnet, "error!\n");
			break;
		      }
		}
	    }
	}
    }
}
#endif