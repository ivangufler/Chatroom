server: server.o colors.o client
	gcc -o server.out server/server.o colors/colors.o

server.o: server/server.c server/server.h
	gcc -c -o server/server.o server/server.c

client: client.o colors.o
	gcc -o client.out client/client.o colors/colors.o

client.o: client/client.c
	gcc -c -o client/client.o client/client.c

colors.o: colors/colors.c colors/colors.h
	gcc -c -o colors/colors.o colors/colors.c

clean:
	rm -rf *.out
	rm -rf client/*.o
	rm -rf server/*.o
	rm -rf colors/*.o
