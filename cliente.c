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

//variaveis para a criacao do socket de dados
int s_dados;

int connectServer(char *nomeServidor, char *portaServidor)
{
    unsigned short port;
    struct hostent *hostnm;
    struct sockaddr_in server;

    hostnm = gethostbyname(nomeServidor);

    if (hostnm == (struct hostent *)0){
        return 0;
    }

    port = (unsigned short)atoi(portaServidor);

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

void encerraCliente() {

    close(s_dados);
    system("clear");
    exit(0);
}

int enviarDado(int dado) {
    if (send(s_dados, &dado, sizeof(dado), 0) <= 0){
        return 0;
    }

    return 1;
}

// MAIN FUNCTION
int main(){

    signal(SIGINT,encerraCliente);
    connectServer("localhost", "8000");
    enviarDado(10);

    encerraCliente();
}