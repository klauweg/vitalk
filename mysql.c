#include <my_global.h>
#include <mysql.h>

int my_query( char * querystring,
	      char * hostname,
	      char * username,
	      char * password,
	      char * database )
{
  MYSQL *con = mysql_init(NULL);
  
  if (con == NULL) 
    {
      fprintf(stderr, "%s\n", mysql_error(con));
      exit(1);
    }
  
  if (mysql_real_connect(con, hostname, username, password, 
			     database, 0, NULL, 0) == NULL) 
    {
          fprintf(stderr, "%s\n", mysql_error(con));
          mysql_close(con);
          return -1;
    } 
  
  if (mysql_query(con, querystring)) 
    {
          fprintf(stderr, "%s\n", mysql_error(con));
          mysql_close(con);
          exit(1);
    }
  
  return 0;
}

