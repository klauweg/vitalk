vitalk: vitalk.c vito_io.c vito_io.h vito_parameter.c vito_parameter.h telnet.c telnet.h
	gcc -Wall -o vitalk \
	    vitalk.c vito_io.c vito_parameter.c telnet.c

copy:
	scp *.c *.h Makefile root@heizung:Heizung/ViTalk/

