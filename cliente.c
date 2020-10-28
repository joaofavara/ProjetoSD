/*
João Pedro Favara 16061921
Marcelino Noguero Souza 16011538
*/
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <dirent.h>
#include <libgen.h>
#include <sys/stat.h>
#include <signal.h>
#include <time.h>

//variaveis para a criacao do socket de dados

struct serverInfo {
    char servername[10];
    unsigned short port;
};

struct data {
    int requestType;
    int data;
    int client;
};

int s_dados;
struct serverInfo serverDirectory;
// pthread_mutex_t locker;

int connectServer(char *nomeServidor, unsigned short portaServidor)
{
    unsigned short port;
    struct hostent *hostnm;
    struct sockaddr_in server;

    hostnm = gethostbyname(nomeServidor);

    if (hostnm == (struct hostent *)0){
        return 0;
    }

    port = portaServidor;

    server.sin_family = AF_INET;
    server.sin_port = htons(port);
    server.sin_addr.s_addr = *((unsigned long *)hostnm->h_addr);

    if ((s_dados = socket(PF_INET, SOCK_STREAM, 0)) < 0){
        return 0;
    }

    if (connect(s_dados, (struct sockaddr *)&server, sizeof(server)) < 0){
        return 0;
    }

    return 1;
}

void encerraConexao() {
    close(s_dados);
}

void encerraCliente() {
    encerraConexao();
    exit(0);
}

int sendData(int dado, int usuario) {

    struct data message;
    message.data = dado;
    message.requestType = 1;
    message.client = usuario;
    
    if (send(s_dados, &message, sizeof(message), 0) <= 0){
        return 0;
    }

    printf("Dado enviado: %d\n", message.data);

    return 1;
}

int receiveData(int usuario) {

    struct data message;
    int receivedData;
    message.requestType = 2;
    message.client = usuario;

    if (send(s_dados, &message, sizeof(message), 0) <= 0){
        return 0;
    }

    if (recv(s_dados, &receivedData, sizeof(receivedData), 0) <= 0){
        return 0;
    }

    printf("Dado recebido: %d\n", receivedData);

    return 1;
}

void receiveDirectory() {

    connectServer("localhost", 5000);

    if (recv(s_dados, &serverDirectory, sizeof(serverDirectory), 0) <= 0){
		printf("Erro ao receber dados\n");
	}

    close(s_dados);
}

// MAIN FUNCTION
int main(int argc, char *argv[]){

    time_t t;
    srand((unsigned) time(&t));

    signal(SIGINT,encerraCliente);

    //while(1) {
        receiveDirectory();
        connectServer(serverDirectory.servername, serverDirectory.port);
        sendData(rand() % 100, atoi(argv[1]));
        encerraConexao();

        //sleep(1);

        receiveDirectory();
        connectServer(serverDirectory.servername, serverDirectory.port);
        receiveData(atoi(argv[1]));

        //sleep(1);
    //}

    encerraCliente();
}