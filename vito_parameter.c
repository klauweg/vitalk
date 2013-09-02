#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <string.h>

#include "vito_parameter.h"
#include "vito_io.h"

     
/////////////////// ALLGEMEIN
void read_deviceid( char *valuestr )
{
  unsigned char content[2];
  int value;
  
  if ( vito_read(0x00f8, 2, content) < 0 )
    sprintf( valuestr, "NULL" );
  else
    {
      // Normalerweise sind die Parameter in Little Endian
      // Byteorder, aber bei der Deviceid hat sich offenbar
      // die umgekehrte Interpretation durchgesetzt:
      value = (content[0] << 8) + content[1];
      sprintf( valuestr, "0x%4x", value );
    }
}


//////////////////// KESSEL
void read_abgas_temp( char *valuestr )
{
  unsigned char content[2];
  float value;
  
  if ( vito_read(0x0808, 2, content) < 0 )
    sprintf( valuestr, "NULL" );
  else
    {
      value = (content[1] << 8) + content[0];
      value = value / 10;
      sprintf( valuestr, "%3.2f", value );
    }
}
void read_Kist_temp( char *valuestr )
{
  unsigned char content[2];
  float value;
  
  if ( vito_read(0x0802, 2, content) < 0 )
    sprintf( valuestr, "NULL" );
  else
    {
      value = (content[1] << 8) + content[0];
      value = value / 10;
      sprintf( valuestr, "%3.2f", value );
    }
}
getTempKistTP 0810 TD 2
getTempKSoll 555A TD 2


//////////////////// WARMWASSER
void read_WW_soll_temp( char *valuestr )
{
  unsigned char content[1];
  float value;
  
  if ( vito_read(0x6300, 1, content) < 0 )
    sprintf( valuestr, "NULL" );
  else
    {
      value = content[0];
      sprintf( valuestr, "%3.2f", value );
    }
}
getTempWWsoll 6300 TD 2
getTempWWistTP 0812 UT 2
getTempWWist 0804 TD 2
getTempWWOffset 6760 UTI 1

/////////////////// AUSSENTEMPERATUR
getTempATP 5525 (TD 2)
getTempAGedaempft 5527 ( TD 2)
getTempA 0800 ( TD 2 )

/////////////////// BRENNER
getBrennerstarts 088A CO 4
getStundenBrenner a38f HS 4
getValueLeistung a38f PC1 1
getStatusFlamme 55d3 B_STF 1
getVerriegelung 55d3 B_STV 1

/////////////////// VENTILE UND RELAIS
getStatusUmschaltVentil 0a10 E_STUV2 1


struct vito_parameter parameter_list[] = {
// ALLGEMEIN
 { "deviceid", "Geraeteidentifikation", &read_deviceid, NULL, "" },
// KESSEL
 { "abgas_temp", "Abgastemperatur", &read_abgas_temp, NULL, "°C" },
// WARMWASSER
 { "WW_soll_temp", "Warmwasser Solltemperatur", &read_WW_soll_temp, NULL, "°C" },

// ende
 { "", "", NULL, NULL, "" }
};

