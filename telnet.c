#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#define PORT 83

static int fd_listener = 0; /* listening socket descriptor */
static int fd_telnet = 0; /* File Descriptor for Telnet */
static struct sockaddr_in serveraddr; /* server address */
static struct sockaddr_in clientaddr; /* client address */

void telnet_init(fd_set *master_fds )
{
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
      fprintf( stderr, "Error bind Socket: %s\n", strerror(errno));
      exit(1);
    }
  /* listen */
  if(listen(fd_listener, 5) == -1) // "5" = listen backlog
   {
      fprintf( stderr, "Error listen Socket!\n");
      exit(1);
   }
  /* add the listener to the master set */
  FD_SET(fd_listener, master_fds);
}

void telnet_task(fd_set *master_fds, fd_set *read_fds)
{
  static char telnet_buf[2048]; /* buffer for client data */
  static int telnet_buf_ptr;
  int fd_new;
  
  // Gibt es eine neue Verbindungsanfrage?
  if ( FD_ISSET( fd_listener, read_fds ) )
    {
      socklen_t addrlen = sizeof(clientaddr); // wird als pointer gebraucht
      if((fd_new = accept(fd_listener, (struct sockaddr *)&clientaddr, &addrlen)) == -1)
	{
	  fprintf( stderr, "Error accepting Connection!\n");
	  exit(1);
	}
      else
	{
	  if ( fd_telnet == 0 )
	    { // Verbindung akzeptieren, wenn es noch keine gibt:
	      fd_telnet = fd_new;
	      FD_SET(fd_telnet , master_fds); /* add to master set */
	      printf("New Telnet connection from %s on socket %d\n",
		     inet_ntoa(clientaddr.sin_addr), fd_telnet);
	      telnet_buf_ptr = 0;
	    }
	  else
	    { // Neue Verbindung ablehnen, wenn schon eine existiert:
	      printf("New Telnet connection declined.\n");
	      close( fd_new );
	    }
	}
    }

  // Gibt es Daten aus der Telnet Verbindung?
  if ( FD_ISSET( fd_telnet, read_fds) )
    {
      // Wir lesen hier immer nur ein Zeichen. Das ist zwar wohl nicht sehr
      // effizient aber vereinfacht das Pufferhandling erheblich.
      if( recv(fd_telnet, telnet_buf + telnet_buf_ptr, 1, 0 ) <= 0 )
	{ /* got error or connection closed by client */
	  close(fd_telnet); /* close it */
	  FD_CLR(fd_telnet, master_fds); /* remove from master set */
	  fd_telnet = 0; // Damit neue Verbindungen angenommen werden können
	  printf("Telnet Connection closed.\n");
	}
      else
	{ // Hier wurde ein Zeichen gelesen:
	  telnet_buf_ptr++;
	  telnet_buf[telnet_buf_ptr]='\0';
	  if ( ( strchr(telnet_buf, '\n') ) || // Newline im Puffer?
	       ( telnet_buf_ptr > sizeof(telnet_buf) - 4 ) ) // Oder Puffer ziemlich voll?
	    { // Dann werten wir das aus:
//		      sscanf(telnet_buf, "%s %d", command, &value
	      printf("#%s\n", telnet_buf);
		      
	      telnet_buf_ptr = 0;
	    }
	}
    }
}