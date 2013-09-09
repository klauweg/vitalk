#include <my_global.h>
#include <mysql.h>

char *my_hostname = NULL;
char *my_username = NULL;
char *my_password = NULL;
char *my_database = NULL;

int my_query( char * querystring )
{
  MYSQL *con = mysql_init(NULL);
  
  if (con == NULL) 
    {
      fprintf(stderr, "%s\n", mysql_error(con));
      exit(1);
    }
  
  if (mysql_real_connect(con, my_hostname, my_username, my_password, 
			     my_database, 0, NULL, 0) == NULL) 
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

