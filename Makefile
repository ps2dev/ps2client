
 GCC = gcc -O3 -Wall -D__CHATTY__ ## -D__DEBUG__
 INC = -I/usr/include -I/usr/local/include
 LIB = 

 PREFIX = $(PS2DEV)

 all: bin/ps2client

 clean:
	rm obj/*.o
	rm bin/ps2client*

 install: bin/ps2client
	mkdir -p $(PREFIX)/bin
	cp bin/ps2client* $(PREFIX)/bin

 ####################
 ## PS2CLIENT LIBS ##
 ####################

 HFILES += src/network.h src/ps2link.h src/ps2netfs.h
 CFILES += src/network.c src/ps2link.c src/ps2netfs.c
 OFILES += obj/network.o obj/ps2link.o obj/ps2netfs.o

 obj/network.o: src/network.h src/network.c
	@mkdir -p obj
	$(GCC) $(INC) -c src/network.c -o obj/network.o

 obj/ps2link.o: src/ps2link.h src/ps2link.c
	@mkdir -p obj
	$(GCC) $(INC) -c src/ps2link.c -o obj/ps2link.o

 obj/ps2netfs.o: src/ps2netfs.h src/ps2netfs.c
	@mkdir -p obj
	$(GCC) $(INC) -c src/ps2netfs.c -o obj/ps2netfs.o

 #######################
 ## PS2CLIENT PROGRAM ##
 #######################

 bin/ps2client: $(HFILES) $(CFILES) $(OFILES)
	@mkdir -p bin
	$(GCC) $(INC) $(LIB) $(OFILES) src/ps2client.c -o bin/ps2client
