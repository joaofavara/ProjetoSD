all:
	$(CC) cliente.c -o client
	$(CC) -pthread diretorio.c -o diretorio
	$(CC) -pthread servidor1.c -o server1
clean:
	rm client
	rm diretorio
	rm server1
