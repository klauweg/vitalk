vitalk: vitalk.c vito_io.c vito_io.h vito_parameter.c vito_parameter.h mysql.c mysql.h text_log.c text_log.h
	gcc -Wall -o vitalk -I/usr/include/mysql  -DBIG_JOINS=1 \
	    -fno-strict-aliasing   -DUNIV_LINUX -DUNIV_LINUX \
	    -rdynamic -L/usr/lib/mysql -lmysqlclient \
	    vitalk.c vito_io.c vito_parameter.c mysql.c text_log.c

copy:
	scp *.c root@lupo:vitalk/
	scp *.h root@lupo:vitalk/
	scp Makefile root@lupo:vitalk/
	
