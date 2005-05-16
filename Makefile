
  GCC = gcc -O3 -I/usr/include -I/usr/local/include
  LIB = -lpthread

  ifeq "x$(PREFIX)" "x"
   PREFIX = $(PS2DEV)
  endif

  all: bin/fsclient bin/ps2client

  clean:
	rm -f obj/*.o bin/*client*

  install: bin/fsclient bin/ps2client
	strip --strip-unneeded bin/*client*
	@mkdir -p $(PREFIX)/bin
	cp bin/*client* $(PREFIX)/bin

 ####################
 ## CLIENT MODULES ##
 ####################

  OFILES += obj/network.o
  obj/network.o: src/network.c src/network.h
	@mkdir -p obj
	$(GCC) -c src/network.c -o obj/network.o

  OFILES += obj/ps2link.o
  obj/ps2link.o: src/ps2link.c src/ps2link.h
	@mkdir -p obj
	$(GCC) -c src/ps2link.c -o obj/ps2link.o

  OFILES += obj/ps2netfs.o
  obj/ps2netfs.o: src/ps2netfs.c src/ps2netfs.h
	@mkdir -p obj
	$(GCC) -c src/ps2netfs.c -o obj/ps2netfs.o

  OFILES += obj/utility.o
  obj/utility.o: src/utility.c src/utility.h
	@mkdir -p obj
	$(GCC) -c src/utility.c -o obj/utility.o

 #####################
 ## CLIENT PROGRAMS ##
 #####################

  bin/fsclient: $(OFILES) src/fsclient.c
	@mkdir -p bin
	$(GCC) $(OFILES) src/fsclient.c -o bin/fsclient $(LIB)

  bin/ps2client: $(OFILES) src/ps2client.c
	@mkdir -p bin
	$(GCC) $(OFILES) src/ps2client.c -o bin/ps2client $(LIB)
