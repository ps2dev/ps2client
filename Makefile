  DEBUG = 0

  ifndef CC
   CC = gcc
  endif

  CFLAGS = -std=gnu99 -Wall -pedantic -I/usr/include -I/usr/local/include

  ifeq ($(DEBUG), 1)
   CFLAGS += -O0 -g
  else
   CFLAGS += -O2
  endif

  ifeq ($(OS),Windows_NT)
   LIBS = -lwsock32
  else
   LIBS = -lpthread
  endif

  ifeq "x$(PREFIX)" "x"
   PREFIX = $(PS2DEV)
  endif

  ifeq ($(OS),Windows_NT)
   all:
	$(MAKE) -f Makefile.mingw32 all

   clean:
	$(MAKE) -f Makefile.mingw32 clean
  else
   all: bin/fsclient bin/ps2client

   clean:
	rm -f obj/*.o bin/*client*
  endif

  install: bin/fsclient bin/ps2client
	strip bin/*client*
	@mkdir -p $(DESTDIR)$(PREFIX)/bin
	cp bin/*client* $(DESTDIR)$(PREFIX)/bin

 ####################
 ## CLIENT MODULES ##
 ####################

  OFILES += obj/network.o
  obj/network.o: src/network.c src/network.h
	@mkdir -p obj
	$(CC) $(CFLAGS) -c src/network.c -o obj/network.o

  OFILES += obj/ps2link.o
  obj/ps2link.o: src/ps2link.c src/ps2link.h
	@mkdir -p obj
	$(CC) $(CFLAGS) -c src/ps2link.c -o obj/ps2link.o

  OFILES += obj/ps2netfs.o
  obj/ps2netfs.o: src/ps2netfs.c src/ps2netfs.h
	@mkdir -p obj
	$(CC) $(CFLAGS) -c src/ps2netfs.c -o obj/ps2netfs.o

  OFILES += obj/utility.o
  obj/utility.o: src/utility.c src/utility.h
	@mkdir -p obj
	$(CC) $(CFLAGS) -c src/utility.c -o obj/utility.o

 #####################
 ## CLIENT PROGRAMS ##
 #####################

  bin/fsclient: $(OFILES) src/fsclient.c
	@mkdir -p bin
	$(CC) $(CFLAGS) $(OFILES) src/fsclient.c -o bin/fsclient $(LIBS)

  bin/ps2client: $(OFILES) src/ps2client.c
	@mkdir -p bin
	$(CC) $(CFLAGS) $(OFILES) src/ps2client.c -o bin/ps2client $(LIBS)
