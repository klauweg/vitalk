#!/bin/sh
#
# Vitalk Daemon beendet sich falls eine eingestellt Häufigkeit von Synchronisationsversuchen
# fehl schlägt. Mit diesem Script wird nach einer gewissen Zeit komplett neu gestartet.

# Serielle Schnittstelle zur Vitodens:
TTY="/dev/serial/by-id/usb-ftdi_usb_serial_converter_ftDCC9VC-if00-port0"

while ( [ -h $TTY ] ); do
/root/Heizung/ViTalk/vitalk >/dev/null 2>/tmp/vitalk.err -t $TTY
aplay -q /usr/local/lib/sounds/ups.wav
sleep 600
done

echo "Panic: tty disappeard!!!!"
echo >>/tmp/vitalk_wrapper.err "Panic: tty disappeard!!!!"

