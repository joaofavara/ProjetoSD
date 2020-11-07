/*
Nome: João Pedro Favara RA: 16061921
Nome: Marcelino Noguero RA: Souza 16011538
Opcionais funcionando: Projeto Basico, Opcionais: 1, 2, 3, 6, 7,8
Observações: Não conseguimos encontrar uma maneira de testar o opcional 6; 
Quando muitos escritores executam juntos, os dados demoram um pouco para replicar os dados.
Valor do Projeto: 
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

struct infoServidor {
    char nomeServidor[10];
    unsigned short porta;
};

struct tipoDado {
    int tipoRequisicao;
    int dado;
    int idCliente;
};

struct infoServidor recebidoDiretorio;
int socketDiretorio;
int socketCliente;

int conetarServidor(int *sock, char *nomeServidor, unsigned short portaServidor)
{
    unsigned short porta;
    struct hostent *hostnm;
    struct sockaddr_in server;

    hostnm = gethostbyname(nomeServidor);

    if (hostnm == (struct hostent *)0){
        return 0;
    }

    porta = portaServidor;

    server.sin_family = AF_INET;
    server.sin_port = htons(porta);
    server.sin_addr.s_addr = *((unsigned long *)hostnm->h_addr);

    if ((*sock = socket(PF_INET, SOCK_STREAM, 0)) < 0){
        return 0;
    }

    if (connect(*sock, (struct sockaddr *)&server, sizeof(server)) < 0){
        return 0;
    }
    return 1;
}

int enviarDado(int *sock, int dado, int usuario) {

    struct tipoDado menssagem;
    menssagem.dado = dado;
    menssagem.tipoRequisicao = 1;
    menssagem.idCliente = usuario;
    
    if (send(*sock, &menssagem, sizeof(menssagem), 0) <= 0){
        return 0;
    }

    printf("Dado enviado: %d\n", menssagem.dado);

    return 1;
}

int receberDado(int *sock, int usuario) {

    struct tipoDado menssagem;
    int receivedData;
    menssagem.tipoRequisicao = 2;
    menssagem.idCliente = usuario;

    if (send(*sock, &menssagem, sizeof(menssagem), 0) <= 0){
        return 0;
    }

    if (recv(*sock, &receivedData, sizeof(receivedData), 0) <= 0){
        return 0;
    }

    printf("Dado recebido: %d\n", receivedData);

    return 1;
}

void receberServidor() {

    if (!conetarServidor(&socketDiretorio, "localhost", 5000)) {
        printf("Erro ao se conectar ao diretorio\n");
        exit(3);
    }

    if (recv(socketDiretorio, &recebidoDiretorio, sizeof(recebidoDiretorio), 0) <= 0){
		printf("Erro ao receber dados\n");
	}

    close(socketDiretorio);
}

int main(int argc, char *argv[]){

    int sock = 0;
    time_t t;
    srand((unsigned) time(&t));

    if (argc < 3) {
        printf("Use: ./cliente clienteID[numero] acao[1(escrita)-2(leitura)]\n");
        exit(-1);
    }

    while(1) {
        if (atoi(argv[2]) == 1) {
            receberServidor();
            if (recebidoDiretorio.porta != 0) {
                conetarServidor(&sock, recebidoDiretorio.nomeServidor, recebidoDiretorio.porta);
                enviarDado(&sock, rand() % 100, atoi(argv[1]));
                close(sock);
            } else {
                printf("Os servidores estão offline\n");
            }
        } else if (atoi(argv[2]) == 2) {
            receberServidor();
            if (recebidoDiretorio.porta != 0) {
                conetarServidor(&sock, recebidoDiretorio.nomeServidor, recebidoDiretorio.porta);
                receberDado(&sock, atoi(argv[1]));
                close(sock);
            } else {
                printf("Os servidores estão offline\n");
            }
        } else {
            printf("Acao invalida\n");
            exit(-2);
        }

        sleep(2);
    }

    exit(1);
}