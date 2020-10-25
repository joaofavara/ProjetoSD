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

int mainSocket;                     /* Socket para aceitar conexoes       */
int updateSocket;
int valorDado = 0;
pthread_mutex_t locker;

struct args{
	int ns;
	int thread_id;
};

struct data {
    int requestType;
    int data;
};

pthread_t updateThread;
pthread_t receiveThread;
unsigned short port;
int countClients = 0;

void encerraCliente() {
  if(pthread_mutex_destroy(&locker)!= 0){
    fprintf(stderr,"Error while destroying muxex\n");
    exit(-1);
  }
  close(updateSocket);
  close(mainSocket);
  system("clear");
  exit(0);
}

int sendData(int dado) {
    if (send(updateSocket, &dado, sizeof(dado), 0) <= 0){
      return 0;
    }
    return 1;
}

void *atender_cliente(void* parameters){
	struct args arg = *((struct args*) parameters);
	struct data dado;

	if (recv(arg.ns, &dado, sizeof(dado), 0) <= 0){
	  printf("Erro ao receber dados de %d (%d)\n", arg.thread_id, arg.ns);
  }

  if(dado.requestType == 1) {
    pthread_mutex_lock(&locker);
    valorDado = dado.data;
    sendData(valorDado);
	  printf("Dado Recebido: %d Cliente: %d (%d)\n", dado.data, arg.thread_id, arg.ns);
    pthread_mutex_unlock(&locker);
  }
  else if(dado.requestType == 2) {
    pthread_mutex_lock(&locker);
    if (send(arg.ns, &valorDado, sizeof(valorDado), 0) <= 0){
      pthread_mutex_unlock(&locker);
		  printf("Erro ao enviar dados para %d (%d)\n", arg.thread_id, arg.ns);
      close(arg.ns);
	    pthread_exit(NULL);
	  }
	  printf("Dado Enviado: %d Cliente: %d (%d)\n",dado.data, arg.thread_id, arg.ns);
    pthread_mutex_unlock(&locker);
  }

	close(arg.ns);
	pthread_exit(NULL);
}

void *receiveData() {
  int valor;

  while(1)
  {
    if (recv(updateSocket, &valor, sizeof(valor), 0) <= 0){
      break;
    }
    pthread_mutex_lock(&locker);
    valorDado = valor;
    printf("Dado recebido: %d\n", valor);
    pthread_mutex_unlock(&locker);
  }
}

void *updateData() {

  int sock;
	struct sockaddr_in client; 
	struct sockaddr_in server; 
	int namelen;
  int valor;

  if ((sock = socket(PF_INET, SOCK_STREAM, 0)) < 0)
  {
    perror("Socket()");
    exit(2);
  }

  server.sin_family = AF_INET;   
  server.sin_port   = htons((unsigned short)4000);       
  server.sin_addr.s_addr = INADDR_ANY;

  if (bind(sock, (struct sockaddr *)&server, sizeof(server)) < 0)
  {
    perror("Bind()");
    exit(3);
  }

  if (listen(sock, 1) != 0)
  {
    perror("Listen()");
    exit(4);
  }

  namelen = sizeof(client);
  
  while(1) {
    if ((updateSocket = accept(sock, (struct sockaddr *) &client, (socklen_t *) &namelen)) == -1) {
      break;
    }
    else {

      if (valorDado != 0) {
        sendData(valorDado);
      }

      receiveData();

      close(updateSocket);
    }
  }

  close(sock);
	pthread_exit(NULL);
}

bool connectServer(char *nomeServidor, unsigned short portaServidor)
{
    unsigned short port;
    struct hostent *hostnm;
    struct sockaddr_in server;
    int valor;

    hostnm = gethostbyname(nomeServidor);

    if (hostnm == (struct hostent *)0){
        return false;
    }

    port = portaServidor;

    server.sin_family = AF_INET;
    server.sin_port = htons(port);
    server.sin_addr.s_addr = *((unsigned long *)hostnm->h_addr);

    if ((updateSocket = socket(PF_INET, SOCK_STREAM, 0)) < 0){
        return false;
    }

    if (connect(updateSocket, (struct sockaddr *)&server, sizeof(server)) < 0){
        return false;
    }

    pthread_create(&receiveThread, NULL, receiveData, NULL);

    return true;
}

int main(int argc, char **argv)
{
	int ns;
	struct sockaddr_in client; 
	struct sockaddr_in server;
  struct args parameters;
  pthread_t thread; 
	int namelen;

	signal(SIGINT,encerraCliente);

  if (argc != 2)
  {
    fprintf(stderr, "\nUse: %s porta\n", argv[0]);
    exit(1);
  }

  if(pthread_mutex_init(&locker,NULL) != 0){
    fprintf(stderr,"Erro ao iniciar o semaforo\n");
    exit(-1);
  }

  port = (unsigned short) atoi(argv[1]);

  if ((mainSocket = socket(PF_INET, SOCK_STREAM, 0)) < 0)
  {
    perror("Socket()");
    exit(2);
  }

  server.sin_family = AF_INET;   
  server.sin_port   = htons(port);       
  server.sin_addr.s_addr = INADDR_ANY;

  printf("\nPorta utilizada: %d", ntohs(server.sin_port));
  printf("\nIP utilizado: %d\n\n", ntohs(server.sin_addr.s_addr));

  if (bind(mainSocket, (struct sockaddr *)&server, sizeof(server)) < 0)
  {
    perror("Bind()");
    exit(3);
  }

  if (listen(mainSocket, 1) != 0)
  {
    perror("Listen()");
    exit(4);
  }

  if(!connectServer("localhost", 4001)) {
    if (pthread_create(&updateThread, NULL, updateData, NULL)) {
      printf("ERRO: impossivel criar uma thread\n");
      exit(-1);    
    }
  }

  while(1)
  {
    ns = 0;
	  namelen = sizeof(client);

	  if ((ns = accept(mainSocket, (struct sockaddr *) &client, (socklen_t *) &namelen)) == -1)
	  {
      perror("Accept()");
      exit(5);
	  }

		parameters.ns = ns;
		parameters.thread_id = countClients;
		if (pthread_create(&thread, NULL, atender_cliente, (void* )&parameters))
    {
        printf("ERRO: impossivel criar uma thread\n");
        exit(-1);
    }
		// printf("Thread[%i] criada\n", countClients);
		countClients++;
	}
}