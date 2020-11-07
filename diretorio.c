/*
Nome: João Pedro Favara RA: 16061921
Nome: Marcelino Noguero RA: Souza 16011538
Opcionais funcionando: Projeto Basico, Opcionais: 1, 2, 3, 6, 7,8
Observações: Não conseguimos encontrar uma maneira de testar o opcional 6; 
Quando muitos escritores executam juntos, os dados demoram um pouco para replicar os dados.
Valor do Projeto: 
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

int s;

struct args{
	int ns;
  char nomeServidor[10];
  unsigned short porta;
};

struct infoServidor {
    char nomeServidor[10];
    unsigned short porta;
};

void encerraCliente() {
    close(s);
    system("clear");
    exit(0);
}

bool testarServidor(char *nomeServidor, unsigned short portaServidor)
{
    unsigned short porta;
    struct hostent *hostnm;
    struct sockaddr_in server;
    int ns;

    hostnm = gethostbyname(nomeServidor);

    if (hostnm == (struct hostent *)0){
        return false;
    }

    porta = portaServidor;

    server.sin_family = AF_INET;
    server.sin_port = htons(porta);
    server.sin_addr.s_addr = *((unsigned long *)hostnm->h_addr);

    if ((ns = socket(PF_INET, SOCK_STREAM, 0)) < 0){
        return false;
    }

    if (connect(ns, (struct sockaddr *)&server, sizeof(server)) < 0){
        return false;
    }

    close(ns);

    return true;
}

void *atender_cliente(void* parametros){
	struct args args = *((struct args*) parametros);
  struct infoServidor serverDirectory;

  strcpy(serverDirectory.nomeServidor, args.nomeServidor);
  serverDirectory.porta = args.porta;

	if (send(args.ns, &serverDirectory, sizeof(serverDirectory), 0) <= 0){
		printf("Erro ao enviar dados\n");
	}
  
  printf("Informacao de servidor enviada!\n");

	close(args.ns);
	pthread_exit(NULL);
}

int proximoIp(int indice){
  indice += 1;
  if (indice == 2) {
    indice = 0;
  }

  return indice;
}

int main()
{
	int ns;
	struct sockaddr_in client; 
	struct sockaddr_in server;
  struct args parametros;
  pthread_t thread;
	int namelen;
  unsigned short porta;
  int portas[2] = {8000, 8001};
  int indice = 0; 

	signal(SIGINT,encerraCliente);

  porta = (unsigned short) 5000;

  if ((s = socket(PF_INET, SOCK_STREAM, 0)) < 0)
  {
    perror("Socket()");
    exit(2);
  }

  server.sin_family = AF_INET;   
  server.sin_port   = htons(porta);       
  server.sin_addr.s_addr = INADDR_ANY;

  fprintf(stderr,"\nDiretorio executando na porta: %d\n", ntohs(server.sin_port));

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

    if (testarServidor("localhost", portas[indice])) {
      parametros.porta = portas[indice];
    } else {
      indice = proximoIp(indice);
      if (testarServidor("localhost", portas[indice])) {
        parametros.porta = portas[indice];
      } else {
        parametros.porta = 0;
      }
    }

		parametros.ns = ns;
    strcpy(parametros.nomeServidor, "localhost");

		if (pthread_create(&thread, NULL, atender_cliente, (void* )&parametros))
    {
        printf("ERRO: impossivel criar uma thread\n");
        exit(-1);
    }

    indice = proximoIp(indice);
	}
}