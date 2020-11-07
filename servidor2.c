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
#include <stdatomic.h>

int socketPrincipal;
int socketReplicacao;
int socketInterface;
atomic_int dadoLocal = 0;

struct args{
	int ns;
};

struct dado {
    int tipoRequisicao;
    int dado;
    int idCliente;
};

pthread_t threadReplicacao;
pthread_t threadRecebimento;
pthread_t threadInterface;

void converterDado(int valor, unsigned char resultado[]) {
    resultado[0] = (valor >> 24) & 0xFF;
    resultado[1] = (valor >> 16) & 0xFF;
    resultado[2] = (valor >> 8) & 0xFF;
    resultado[3] = valor & 0xFF;
}

void encerraCliente() {
  int encerrar = -1;
  send(socketReplicacao, &encerrar, sizeof(encerrar), 0);
  close(socketReplicacao);
  close(socketInterface);
  close(socketPrincipal);
  system("clear");
  exit(0);
}

void enviarDado(int dado) {
    if (send(socketReplicacao, &dado, sizeof(dado), 0) <= 0){
      close(socketReplicacao);
    }
}

void sendInterface(struct dado dado) {
    unsigned char bytes[4];

    while (socketInterface == 0) {
      sleep(1);
    }

    converterDado(dado.idCliente, bytes);
    if (send(socketInterface, &bytes, sizeof(bytes), 0) <= 0){
      close(socketInterface);
    }

    converterDado(dado.tipoRequisicao, bytes);
    if (send(socketInterface, &bytes, sizeof(bytes), 0) <= 0){
      close(socketInterface);
    }

    if (dado.tipoRequisicao == 1) {
      converterDado(dado.dado, bytes);
      if (send(socketInterface, &bytes, sizeof(bytes), 0) <= 0){
        close(socketInterface);
      }
    }
}

void *checkpoint() {
  int backup;

  while(1) {
    sleep(5);
    if (!dadoLocal) {
      dadoLocal = backup;
    }
    else {
      backup = dadoLocal;
    }
  }
}

void *atender_cliente(void* parametros){
	struct args arg = *((struct args*) parametros);
	struct dado dado;

	if (recv(arg.ns, &dado, sizeof(dado), 0) <= 0){
    close(arg.ns);
	  pthread_exit(NULL);
  }

  if(dado.tipoRequisicao == 1) {
    dadoLocal = dado.dado;
    enviarDado(dadoLocal);
    sendInterface(dado);
	  printf("Dado Recebido: %d Cliente: %d\n", dado.dado, dado.idCliente);
  }
  else if(dado.tipoRequisicao == 2) {
    if (send(arg.ns, &dadoLocal, sizeof(dadoLocal), 0) <= 0){
		  printf("Erro ao enviar dados para %d\n", dado.idCliente);
      close(arg.ns);
	    pthread_exit(NULL);
	  }
    sendInterface(dado);
	  printf("Dado Enviado: %d Cliente: %d\n",dadoLocal, dado.idCliente);
  }

	close(arg.ns);
	pthread_exit(NULL);
}

void *receberDado() {
  int valor;
  struct dado dado;

  while(1)
  {
    if (recv(socketReplicacao, &valor, sizeof(valor), 0) <= 0){
      close(socketReplicacao);
      break;
    }

    if (valor < 0) {
      close(socketReplicacao);
      break;
    }

    dadoLocal = valor;
    dado.dado = valor;
    dado.idCliente = 0;
    dado.tipoRequisicao = 1;
    sendInterface(dado);
    printf("Dado replicado: %d\n", valor);
  }
}

void *updateData() {

  int sock;
	struct sockaddr_in idCliente; 
	struct sockaddr_in server; 
	int namelen;
  int valor;

  if ((sock = socket(PF_INET, SOCK_STREAM, 0)) < 0)
  {
    perror("Socket()");
    exit(2);
  }

  server.sin_family = AF_INET;   
  server.sin_port   = htons((unsigned short)4001);       
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

  namelen = sizeof(idCliente);
  
  while(1) {
    if ((socketReplicacao = accept(sock, (struct sockaddr *) &idCliente, (socklen_t *) &namelen)) != -1) {
      enviarDado(dadoLocal);
      receberDado();
      close(socketReplicacao);
    }
  }

  close(sock);
	pthread_exit(NULL);
}

void *updateInterface() {

  int sock;
	struct sockaddr_in idCliente; 
	struct sockaddr_in server; 
	int namelen;
  int valor;

  if ((sock = socket(PF_INET, SOCK_STREAM, 0)) < 0)
  {
    perror("Socket()");
    exit(2);
  }

  server.sin_family = AF_INET;   
  server.sin_port   = htons((unsigned short)6001);       
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

  namelen = sizeof(idCliente);
  
  while(1) {
    if ((socketInterface = accept(sock, (struct sockaddr *) &idCliente, (socklen_t *) &namelen)) == -1) {
      close(socketInterface);
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

    if ((socketReplicacao = socket(PF_INET, SOCK_STREAM, 0)) < 0){
        return false;
    }

    if (connect(socketReplicacao, (struct sockaddr *)&server, sizeof(server)) < 0){
        return false;
    }

    pthread_create(&threadRecebimento, NULL, receberDado, NULL);

    return true;
}

int main()
{
	int ns;
	struct sockaddr_in idCliente; 
	struct sockaddr_in server;
  struct args parametros;
  pthread_t thread; 
	int namelen;
  unsigned short porta;

	signal(SIGINT,encerraCliente);
  signal(SIGPIPE, SIG_IGN);

  porta = (unsigned short) 8001;

  if ((socketPrincipal = socket(PF_INET, SOCK_STREAM, 0)) < 0)
  {
    perror("Socket()");
    exit(2);
  }

  server.sin_family = AF_INET;   
  server.sin_port   = htons(porta);       
  server.sin_addr.s_addr = INADDR_ANY;

  fprintf(stderr,"\nServidor 2 executando na porta: %d\n", ntohs(server.sin_port));

  if (bind(socketPrincipal, (struct sockaddr *)&server, sizeof(server)) < 0)
  {
    perror("Bind()");
    exit(3);
  }

  if (listen(socketPrincipal, 1) != 0)
  {
    perror("Listen()");
    exit(4);
  }

  connectServer("localhost", 4000);
  pthread_create(&threadInterface, NULL, updateInterface, NULL);
  pthread_create(&threadReplicacao, NULL, updateData, NULL);
  pthread_create(&thread, NULL, checkpoint, NULL);

  while(1)
  {
    ns = 0;
	  namelen = sizeof(idCliente);

	  if ((ns = accept(socketPrincipal, (struct sockaddr *) &idCliente, (socklen_t *) &namelen)) == -1)
	  {
      perror("Accept()");
      exit(5);
	  }
		parametros.ns = ns;
		if (pthread_create(&thread, NULL, atender_cliente, (void* )&parametros))
    {
        printf("ERRO: impossivel criar uma thread\n");
        exit(-1);
    }
	}
}