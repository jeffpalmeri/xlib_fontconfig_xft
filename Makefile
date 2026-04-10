C_FLAGS_EXTRA ?=

main: main.o pty.o ptyFork.o stuff.o moreStuff.o
	# gcc main.o -o main -lX11 -lfontconfig -lXft -lfreetype
	gcc -g -O0 main.o pty.o ptyFork.o stuff.o moreStuff.o -o build/main `pkg-config --libs x11 xft fontconfig`

main.o: main.c
	# gcc -c main.c -o main.o `pkg-config --cflags freetype2`
	gcc -c -g -O0 main.c $(C_FLAGS_EXTRA) -o main.o `pkg-config --cflags x11 xft fontconfig`

pty.o: pty.c
	gcc -c pty.c -o pty.o

ptyFork.o: ptyFork.c
	gcc -c ptyFork.c -o ptyFork.o

clean:
	# rm -rf main.o main .cache pty.o ptyFork.o script.o tty_functions.o
	rm -rf main .cache *.o

stuff.o: stuff.c
	gcc -c -g -O0 stuff.c $(C_FLAGS_EXTRA) -o stuff.o `pkg-config --cflags x11 xft fontconfig`

moreStuff.o: moreStuff.c
	gcc -c -g -O0 moreStuff.c $(C_FLAGS_EXTRA) -o moreStuff.o `pkg-config --cflags x11 xft fontconfig`

test.o: test.c
	gcc -c -g -O0 test.c -o test.o `pkg-config --libs x11 xft fontconfig --cflags x11 xft fontconfig`

.PHONY: test
test: test.o moreStuff.o
	gcc -g -O0 test.o moreStuff.o -o build/test `pkg-config --libs x11 xft fontconfig ` && ./build/test

# script: script.o pty.o ptyFork.o tty_functions.o
# 	gcc script.o pty.o ptyFork.o tty_functions.o -o script
#
# script.o: script.c pty.o ptyFork.o tty_functions.o
# 	gcc -c script.c -o script.o 

# tty_functions.o: tty_functions.c
# 	gcc -c tty_functions.c -o tty_functions.o

############################################

script: script.o pty2.o ptyFork2.o tty_functions.o
	gcc script.o pty2.o ptyFork2.o tty_functions.o from_book/lib/error_functions.o -o build/script

script.o: from_book/script.c pty2.o ptyFork2.o tty_functions.o
	gcc -c from_book/script.c -o script.o 
# script.o: script.c pty.o ptyFork2.o tty_functions.o
# 	gcc -c script.c -o script.o 

pty2.o: from_book/pty_master_open.c
	gcc -c from_book/pty_master_open.c -o pty2.o

ptyFork2.o: from_book/pty_fork.c
	gcc -c from_book/pty_fork.c -o ptyFork2.o

from_book/lib/error_functions.o: from_book/lib/error_functions.c
	gcc -c -g -Og from_book/lib/error_functions.c -o from_book/lib/error_functions.o

tty_functions.o: tty_functions.c
	gcc -c tty_functions.c -o tty_functions.o

.PHONY: clean

clean2:
	rm -f *.o

############################################

nogui: nogui.o from_book/lib/error_functions.o ptyFork.o pty.o
	gcc nogui.o from_book/lib/error_functions.o ptyFork.o pty.o -o build/nogui

nogui.o: nogui.c
	gcc -c nogui.c -o nogui.o
