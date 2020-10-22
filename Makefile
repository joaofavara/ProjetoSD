all:
	$(CC) cliente.c -o client
	$(CC) -pthread servidor1.c -o server1
clean:
	rm client
	rm server1
