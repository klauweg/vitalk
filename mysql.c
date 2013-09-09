#include <my_global.h>
#include <mysql.h>
#include "vito_parameter.h"

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
      return -1;
    }
  
  return 0;
}

void my_log( void )
{
  char query[2000];
  
  sprintf( query,
	   "INSERT INTO vitolog (mode, kessel_soll, kessel_ist, kessel_abgas,"
	   "                     warmwasser_soll, warmwasser_offset, warmwasser_ist,"
	   "                     aussentemp, aussentemp_ged, starts, runtime, leistung,"
	   "                     ventil, pumpe, flow, vorlauf_soll, raum_soll,"
	   "                     raum_soll_red, neigung, niveau ) VALUES ("
	   "                     %s, %s * 10, %s * 10, %s * 10,"
           "                     %s, %s, %s * 10,"
	   "                     %s * 10, %s * 10, %s, %s, %s,"
	   "                     %s, %s, %s, %s, %s,"
           "                     %s, %s * 10, %s )",
	   read_mode_numeric(), read_K_soll_temp(), read_K_istTP_temp(), read_K_abgas_temp(),
	   read_WW_soll_temp(), read_WW_offset(), read_WW_istTP_temp(),
	   read_outdoor_TP_temp(), read_outdoor_smooth_temp(), read_starts(), read_runtime(), read_power(),
	   read_ventil_numeric(), read_pump_power(), read_flow(), read_VL_soll_temp(), read_raum_soll_temp(),
	   read_red_raum_soll_temp(), read_neigung(), read_niveau() );

  my_query( query );
}
