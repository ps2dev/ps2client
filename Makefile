
 GCC = gcc -Wall
 INC = -I/usr/include -I/usr/local/include
 LIB = 

 PREFIX = $(PS2DEV)

 all: bin/ps2client

 clean:
	rm -f obj/*.o
	rm -f bin/*

 install: bin/ps2client
	mkdir -p $(PREFIX)/bin
	cp bin/ps2client* $(PREFIX)/bin

 ####################
 ## PS2CLIENT LIBS ##
 ####################

 HFILES = src/ps2link.h src/network.h
 CFILES = src/ps2link.c src/network.c
 OFILES = obj/ps2link.o obj/network.o

 obj/ps2link.o: src/ps2link.h src/ps2link.c
	$(GCC) $(INC) -c src/ps2link.c -o obj/ps2link.o

 obj/network.o: src/network.h src/network.c
	$(GCC) $(INC) -c src/network.c -o obj/network.o

 #######################
 ## PS2CLIENT PROGRAM ##
 #######################

 bin/ps2client: $(HFILES) $(CFILES) $(OFILES)
	$(GCC) $(INC) $(LIB) $(OFILES) src/ps2client.c -o bin/ps2client
