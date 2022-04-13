all: clean server hub sensors attuatori

server: server.o list.o
	gcc -Wall -o $@ $^

hub: hub.o list.o
	gcc -Wall -o $@ $^

sensors: sensors.o list.o
	gcc -Wall -o $@ $^

attuatori: attuatori.o list.o
	gcc -Wall -o $@ $^

clean:
	rm -f server hub sensors attuatori list *.o *~

