all: sqlite3_wrapper.dll test.ex4 test2.ex4


sqlite3_wrapper.dll: sqlite3_wrapper.c
	i586-mingw32msvc-gcc -I/usr/i586-mingw32msvc/include -shared -o sqlite3_wrapper.dll -Wl,--add-stdcall-alias sqlite3_wrapper.c -lsqlite3


clean:
	rm -f test.ex4 test2.ex4


test.ex4: test.mq4
	ml4 test.mq4

test2.ex4: test2.mq4
	ml4 test2.mq4