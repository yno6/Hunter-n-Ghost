#list of all object files
OBJ = main.o room.o hunter.o ghost.o helpers.o

#default target: build the final executable
all: $(OBJ)
	gcc -pthread -o project $(OBJ)

#compilation targets
main.o: main.c defs.h room.h hunter.h ghost.h helpers.h
	gcc -pthread -c main.c

room.o: room.c room.h defs.h
	gcc -pthread -c room.c

hunter.o: hunter.c hunter.h room.h defs.h helpers.h
	gcc -pthread -c hunter.c

ghost.o: ghost.c ghost.h room.h defs.h helpers.h
	gcc -pthread -c ghost.c

helpers.o: helpers.c helpers.h defs.h
	gcc -pthread -c helpers.c

#clean target/executable file
clean:
	rm -f $(OBJ) project