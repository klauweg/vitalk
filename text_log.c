#include <stdio.h>
#include "vito_parameter.h"

// Debug Ausgabe aller Parameter auf stream
// ggf. mit Ausgabe des Fehlerspeichers:
void print_all( FILE *file, int error_flag )
{
  fprintf(file, "ALLGEMEIN:\n");
  fprintf(file, " Device Id: %s, Modus Numerisch: %s, Modus: %s         \n",
	 read_deviceid(), read_mode_numeric(), read_mode() );

  fprintf(file, "KESSEL:\n");
  fprintf(file, " Kessel Soll Temperatur: %s °C     \n", read_K_soll_temp() );
  fprintf(file, " Kessel ist: %s °C, ist TP: %s °C      \n", read_K_ist_temp(), read_K_istTP_temp() );
  fprintf(file, " Kessel Abgastemperatur: %s °C       \n", read_K_abgas_temp() );

  fprintf(file, "WARMWASSER:\n");
  fprintf(file, " Solltemperatur: %s °C    \n", read_WW_soll_temp() );
  fprintf(file, " Vorlaufoffset: %s K     \n", read_WW_offset() );
  fprintf(file, " ist: %s °C, ist Tiefpass: %s °C     \n", read_WW_ist_temp(), read_WW_istTP_temp() );
  
  fprintf(file, "AUSSENTEMPERATUR\n");
  fprintf(file, " ist: %s °C, ist Tiefpass: %s °C, ist gedämpft: %s °C      \n",
	 read_outdoor_temp(), read_outdoor_TP_temp(), read_outdoor_smooth_temp() );

  fprintf(file, "BRENNER:\n");
  fprintf(file, " Starts: %s, Laufzeit: %s s, Laufzeit: %s h     \n",
	 read_starts(), read_runtime(), read_runtime_h() );
  fprintf(file, " Leistung: %s %%     \n", read_power() );
  
  fprintf(file, "HYDRAULIK:\n");
  fprintf(file, " Ventilstellung Numerisch: %s, Stellung: %s       \n",
	 read_ventil_numeric(), read_ventil() );
  fprintf(file, " Pumpe: %s %%        \n", read_pump_power() );
  fprintf(file, " Volumenstrom: %s l/h        \n", read_flow() );
      
  fprintf(file, "HEIZKREISTEMPERATUREN:\n");
  fprintf(file, " Vorlaufsoll: %s °C, Raumsoll: %s °C, red. Raumsoll %s °C     \n",
	 read_VL_soll_temp(), read_raum_soll_temp(), read_red_raum_soll_temp() );
  fprintf(file, " Neigung: %s,  Niveau: %s   \n", read_neigung(), read_niveau() );
      
  if ( error_flag )
    {
      fprintf(file, "FEHLERSPEICHER:\n");
      fprintf(file,  "%s", read_error_history() );
    }
}

