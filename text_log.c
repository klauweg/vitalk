#include <stdio.h>
#include "vito_parameter.h"

// Debug Ausgabe aller Parameter auf stream
// ggf. mit Ausgabe des Fehlerspeichers:
void print_all( void )
{
  fprintf(stdout, "ALLGEMEIN:\n");
  fprintf(stdout, " Device Id: %s, Modus Numerisch: %s, Modus: %s         \n",
	 read_deviceid(), read_mode_numeric(), read_mode() );

  fprintf(stdout, "KESSEL:\n");
  fprintf(stdout, " Kessel Soll Temperatur: %s °C     \n", read_K_soll_temp() );
  fprintf(stdout, " Kessel ist: %s °C, ist TP: %s °C      \n", read_K_ist_temp(), read_K_istTP_temp() );
  fprintf(stdout, " Kessel Abgastemperatur: %s °C       \n", read_K_abgas_temp() );

  fprintf(stdout, "WARMWASSER:\n");
  fprintf(stdout, " Solltemperatur: %s °C    \n", read_WW_soll_temp() );
  fprintf(stdout, " Vorlaufoffset: %s K     \n", read_WW_offset() );
  fprintf(stdout, " ist: %s °C, ist Tiefpass: %s °C     \n", read_WW_ist_temp(), read_WW_istTP_temp() );
  
  fprintf(stdout, "AUSSENTEMPERATUR\n");
  fprintf(stdout, " ist: %s °C, ist Tiefpass: %s °C, ist gedämpft: %s °C      \n",
	 read_outdoor_temp(), read_outdoor_TP_temp(), read_outdoor_smooth_temp() );

  fprintf(stdout, "BRENNER:\n");
  fprintf(stdout, " Starts: %s, Laufzeit: %s s, Laufzeit: %s h     \n",
	 read_starts(), read_runtime(), read_runtime_h() );
  fprintf(stdout, " Leistung: %s %%     \n", read_power() );
  
  fprintf(stdout, "HYDRAULIK:\n");
  fprintf(stdout, " Ventilstellung Numerisch: %s, Stellung: %s       \n",
	 read_ventil_numeric(), read_ventil() );
  fprintf(stdout, " Pumpe: %s %%        \n", read_pump_power() );
  fprintf(stdout, " Volumenstrom: %s l/h        \n", read_flow() );
      
  fprintf(stdout, "HEIZKREISTEMPERATUREN:\n");
  fprintf(stdout, " Vorlaufsoll: %s °C, Raumsoll: %s °C, red. Raumsoll %s °C     \n",
	 read_VL_soll_temp(), read_raum_soll_temp(), read_red_raum_soll_temp() );
  fprintf(stdout, " Neigung: %s,  Niveau: %s   \n", read_neigung(), read_niveau() );
  fprintf(stdout, "FEHLERSPEICHER:\n");
  fprintf(stdout,  "%s", read_error_history_numeric() );
}

