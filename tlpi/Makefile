script: from_book/script.o from_book/pty_fork.o from_book/tty_functions.o from_book/pty_master_open.o from_book/lib/error_functions.o
	gcc -g -Og from_book/script.o from_book/pty_fork.o from_book/tty_functions.o from_book/pty_master_open.o from_book/lib/error_functions.o -o from_book/script

from_book/script.o: from_book/script.c
	gcc -c -g -Og from_book/script.c -o from_book/script.o

from_book/tty_functions.o: from_book/tty_functions.c
	gcc -c -g -Og from_book/tty_functions.c -o from_book/tty_functions.o

from_book/pty_fork.o: from_book/lib/error_functions.o
	gcc -c -g -Og from_book/pty_fork.c -o from_book/pty_fork.o from_book/lib/error_functions.o

from_book/lib/error_functions.o: from_book/lib/error_functions.c
	gcc -c -g -Og from_book/lib/error_functions.c -o from_book/lib/error_functions.o

from_book/pty_master_open.o: from_book/pty_master_open.c
	gcc -c -g -Og from_book/pty_master_open.c -o from_book/pty_master_open.o
	 
