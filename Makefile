main: main.o
	gcc main.o -o main -lX11 -lfontconfig -lXft -lfreetype

main.o: main.c
	gcc -c main.c -o main.o `pkg-config --cflags freetype2`

clean:
	rm -rf main.o main .cache

