
  GCC = gcc -O3
  INC = -I/usr/include -I/usr/local/include
  LIB = -lpthread

  ifeq "x$(PREFIX)" "x"
   PREFIX = $(PS2DEV)
  endif

  all: bin/ps2client

  clean:
	rm -f obj/*.o bin/ps2client*

  install: bin/ps2client
	mkdir -p $(PREFIX)/bin
	cp bin/ps2client* $(PREFIX)/bin

 ####################
 ## PS2CLIENT LIBS ##
 ####################

  OFILES += obj/network.o
  obj/network.o: src/network.c src/network.h
	@mkdir -p obj
	$(GCC) $(INC) -c src/network.c -o obj/network.o

  OFILES += obj/ps2client.o
  obj/ps2client.o: src/ps2client.c
	@mkdir -p obj
	$(GCC) $(INC) -c src/ps2client.c -o obj/ps2client.o

  OFILES += obj/ps2link.o
  obj/ps2link.o: src/ps2link.c src/ps2link.h
	@mkdir -p obj
	$(GCC) $(INC) -c src/ps2link.c -o obj/ps2link.o

  OFILES += obj/utility.o
  obj/utility.o: src/utility.c src/utility.h
	@mkdir -p obj
	$(GCC) $(INC) -c src/utility.c -o obj/utility.o

 #######################
 ## PS2CLIENT PROGRAM ##
 #######################

  bin/ps2client: $(OFILES)
	@mkdir -p bin
	$(GCC) $(INC) $(LIB) $(OFILES) -o bin/ps2client
