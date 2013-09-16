#include <my_global.h>
#include <mysql.h>
#include <string.h>
#include "vito_parameter.h"

char *my_hostname = NULL;
char *my_username = NULL;
char *my_password = NULL;
char *my_database = NULL;

// Minimales Loggingintervall für zeit-hochauflösenden Parameter in Sekunden:
// die anderen Tasks werden aber auch nicht häufiger aufgerufen.
#define MIN_LOG 10

// Minimales Loggingintervall für die langsamen Tabellen:
#define SLOW_LOG 120

// Hier wird ein querystring an die Datenbank geschickt. Die Verbindung wird
// dazu jedesmal wieder geöffnet und geschlossen. Ich gehe davon aus dass keine größeren
// Performanceprobleme dadurch auftreten. Damit ist aber hoffentlich auch die Frage geklärt,
// was passiert, wenn die Datenbank mal nicht erreichbar sein sollte.
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

  mysql_close(con);
  return 0;
}

// Diese funktion wird bei jedem Timeout der select() Funktion
// der main event loop aufgerufen. (ca. jede Sekunde)
void my_log_task( void )
{
  static time_t last_log = 0;
  char query[2000];
  
  // Zeit noch nicht abgelaufen, dann geben wir gleich wieder zurück:
  if ( time(NULL) / MIN_LOG <= last_log )
    return;

  // Merken für nächstes mal:
  last_log = time(NULL) / MIN_LOG;

  /////////////////////// vitolog_kessel /////////////////////////////////////
  static int last_runtime = 0, this_runtime = 0;
  // Wenn Brenner an ist oder Pumpe läuft oder Volumenstrom gemessen wird,
  // loggen wir die Kesselrelevanten Parameter alle MIN_LOG Sekunden:
  if ( atoi(read_pump_power()) > 0 ||
       atoi(read_flow()) > 3 ||
       strcmp( read_power(), "0.0" ) )
    {
      this_runtime = atoi( read_runtime() );
      if ( last_runtime ) // Haben wir schon einen Wert der letzen Kessellaufzeit?
	{
	  sprintf( query, "INSERT INTO vitolog_kessel ("
		   "start_count, delta_runtime, ventil, pumpe, flow,"
	           "leistung, kessel_soll, kessel_ist, abgas) VALUES ("
	           "%s, %d, %s, %s, %s,"
	           "%s, %s * 10, %s * 10, %s * 10 )",
	       read_starts(), this_runtime - last_runtime, read_ventil_numeric(), read_pump_power(), read_flow(),
	       read_power(), read_K_soll_temp(), read_K_istTP_temp(), read_K_abgas_temp() );
	  
	  my_query( query );
	}
      last_runtime = this_runtime;
    }
  
  ////////////////////// vitolog_config //////////////////////////////////////
  static char last_config_query[2000] = "";
  
  sprintf( query, "INSERT INTO vitolog_config ("
	          "mode, warmwasser_soll, warmwasser_offset,"
	          "raum_soll, raum_soll_red, neigung, niveau) VALUES ("
	          "%s, %s, %s,"
	          "%s, %s, %s * 10, %s)",
	     read_mode_numeric(), read_WW_soll_temp(), read_WW_offset(),
	     read_raum_soll_temp(), read_red_raum_soll_temp(), read_neigung(), read_niveau() );
  
  // hat sich was verändert?
  if ( strcmp( last_config_query, query ) )
    {
      my_query( query ); // Dann loggen wir das
      strcpy( last_config_query, query );
    }
  
  ////////////////////// vitolog_error //////////////////////////////////////
  static char last_error_query[2000] = "";
  
  sprintf( query, "INSERT INTO vitolog_errors ("
	   "error1, error2, error3, error4, error5,"
	   "error6, error7, error8, error9, error10, errortext) VALUES ("
	   "%s,'%s' )", read_error_history_numeric(), read_error_history() );
  
  // hat sich was verändert?
  if ( strcmp( last_error_query, query ) )
    {
      my_query( query ); // Dann loggen wir das
      strcpy( last_error_query, query );
    }
	   
  /////////////////////// vitolog_slow //////////////////////////////////////
  static time_t last_slow_log = 0;

  if ( time(NULL) / SLOW_LOG > last_slow_log )
    {
      last_slow_log = time(NULL) / SLOW_LOG;
      sprintf( query, "INSERT INTO vitolog_slow ("
	       "runtime, warmwasser_ist, aussentemp, aussentemp_ged,"
	       "vorlauf_soll, kessel_ist) VALUES ("
	       "%s, %s * 10, %s * 10, %s * 10,"
	       "%s * 10, %s * 10 )",
	       read_runtime(), read_WW_istTP_temp(), read_outdoor_TP_temp(), read_outdoor_smooth_temp(),
	       read_VL_soll_temp(), read_K_istTP_temp() );
      
      my_query( query );
    }
}
