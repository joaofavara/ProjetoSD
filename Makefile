JCC = javac

all:
	$(CC) cliente.c -o client
	$(CC) -pthread diretorio.c -o diretorio
	$(CC) -pthread servidor1.c -o server1
	$(CC) -pthread servidor2.c -o server2
	$(JCC) Principal.java

clean:
	rm client
	rm diretorio
	rm server1
	rm server2
	rm *.class

