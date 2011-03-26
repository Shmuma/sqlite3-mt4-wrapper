all: sqlite3_wrapper.dll test.ex4 test2.ex4

CCOPTS=-I/usr/i586-mingw32msvc/include -O2
LDOPTS=-L/usr/i586-mingw32msvc/lib -shared -Wl,--add-stdcall-alias

sqlite3_wrapper.dll: sqlite3_wrapper.o sqlite3.o
	i586-mingw32msvc-gcc $(LDOPTS) -o sqlite3_wrapper.dll sqlite3_wrapper.o sqlite3.o -lshlwapi -lshell32

sqlite3.o: sqlite3.c sqlite3.h
	i586-mingw32msvc-gcc $(CCOPTS) -c -o sqlite3.o sqlite3.c

sqlite3_wrapper.o: sqlite3_wrapper.c
	i586-mingw32msvc-gcc $(CCOPTS) -c -o sqlite3_wrapper.o sqlite3_wrapper.c

clean:
	rm -f test.ex4 test2.ex4


test.ex4: test.mq4
	ml4.sh test.mq4

test2.ex4: test2.mq4
	ml4.sh test2.mq4
