/*
João Pedro Favara 16061921
Marcelino Noguero Souza 16011538
*/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdbool.h>
#include <sys/ipc.h>
#include <pthread.h>
#include <limits.h>
#include <dirent.h>
#include <libgen.h>
#include <sys/syscall.h>
#include <netdb.h>
#include <sys/stat.h>
#include <signal.h>

/*
 * Servidor TCP
 */

#define MaxArray 10
int s;                     /* Socket para aceitar conexoes       */

struct args{
	int ns;
	int s;
  char servername[10];
  unsigned short port;
	int thread_id;
};

struct serverInfo {
    char servername[10];
    unsigned short port;
};

struct args parameters;
pthread_t thread_id[MaxArray];
unsigned short port;
int countClients = 0;

void encerraCliente() {
    close(s);
    system("clear");
    exit(0);
}

void *atender_cliente(void* parameters){
	struct args args = *((struct args*) parameters);
  struct serverInfo serverDirectory;

  strcpy(serverDirectory.servername, args.servername);
  serverDirectory.port = args.port;

	if (send(args.ns, &serverDirectory, sizeof(serverDirectory), 0) <= 0){
		printf("Erro ao receber dados de %d\n", args.thread_id);
	}

	close(args.ns);
	pthread_exit(NULL);
}

int main(int argc, char **argv)
{
	int ns;
	struct sockaddr_in client; 
	struct sockaddr_in server; 
	int namelen;

	signal(SIGINT,encerraCliente);

  if (argc != 2)
  {
    fprintf(stderr, "\nUse: %s porta\n", argv[0]);
    exit(1);
  }

  port = (unsigned short) atoi(argv[1]);

  if ((s = socket(PF_INET, SOCK_STREAM, 0)) < 0)
  {
    perror("Socket()");
    exit(2);
  }

  server.sin_family = AF_INET;   
  server.sin_port   = htons(port);       
  server.sin_addr.s_addr = INADDR_ANY;

  printf("\nPorta utilizada: %d", ntohs(server.sin_port));
  printf("\nIP utilizado: %d\n\n", ntohs(server.sin_addr.s_addr));

  if (bind(s, (struct sockaddr *)&server, sizeof(server)) < 0)
  {
    perror("Bind()");
    exit(3);
  }

  if (listen(s, 1) != 0)
  {
    perror("Listen()");
    exit(4);
  }

  while(1)
  {
	  namelen = sizeof(client);
	  if ((ns = accept(s, (struct sockaddr *) &client, (socklen_t *) &namelen)) == -1)
	  {
      perror("Accept()");
      exit(5);
	  }
		parameters.ns = ns;
		parameters.s = s;
		parameters.thread_id = countClients;
    strcpy(parameters.servername, "localhost");
    parameters.port = 8001;

		if (pthread_create(&thread_id[countClients], NULL, atender_cliente, (void* )&parameters))
    {
        printf("ERRO: impossivel criar uma thread\n");
        exit(-1);
    }

		countClients++;
	}
}