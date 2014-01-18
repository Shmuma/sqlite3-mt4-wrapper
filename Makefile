CC=i686-pc-mingw32-gcc
METALANG=metalang
CFLAGS=-O2
LDFLAGS=-shared -Wl,--add-stdcall-alias
LDFLAGS_LIBS=-lshlwapi -lshell32

SQLITE3_OBJS=sqlite3.o
SQLITE3_EXT_OBJS=extension-functions.o
WRAPPER_OBJS=sqlite3_wrapper.o
OBJS=$(SQLITE3_OBJS) $(SQLITE3_EXT_OBJS) $(WRAPPER_OBJS)
TARGET_DLL=sqlite3_wrapper.dll
EX4_FILES=test.ex4 test_extra.ex4 test_journal.ex4 test2.ex4


all: $(TARGET_DLL) $(EX4_FILES)

%.o : $.c
	$(CC) $(CFLAGS) -c -o $@ $<

sqlite3.o: sqlite3.c sqlite3.h

$(TARGET_DLL): $(OBJS)
	$(CC) $(LDFLAGS) -o $(TARGET_DLL) $(OBJS) $(LDFLAGS_LIBS)

%.ex4 : %.mq4
	$(METALANG) $<

clean:
	rm -f $(TARGET_DLL) $(WRAPPER_OBJS)

clean-ex4:
	rm -f *.ex4

distclean: clean clean-ex4
	rm -f *.o
