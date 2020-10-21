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


/*
 * Cliente TCP
 */
 
#define COMMAND 200
#define FILESIZE 4096

int s;

//variaveis para a criacao do socket de dados
int s_dados, ns_dados, port_dados;
struct sockaddr_in client_dados; 
struct sockaddr_in server_dados; 
int namelen_dados;

char command[COMMAND];

long file_size(char *name) {
    FILE *fp = fopen(name, "rb"); //must be binary read to get bytes

    long size;
    if(fp) {
        fseek(fp, 0, SEEK_END);
        size = ftell(fp);
        fclose(fp);
    }
    return size;
}

void create_socket() {
    // int s_dados;                     /* Socket para aceitar conexoes       */
	// int *ns_dados = 0;                /* Socket conectado ao cliente        */
    unsigned short port;

    /*
     * O primeiro argumento (argv[1]) e a porta
     * onde o servidor aguardara por conexoes
     */

    port = (unsigned short) 0;

    /*
     * Cria um socket TCP (stream) para aguardar conexoes
     */
    if ((s_dados = socket(PF_INET, SOCK_STREAM, 0)) < 0)
    {
	  perror("Socket()");
	  exit(2);
    }

   /*
    * Define a qual endereco IP e porta o servidor estara ligado.
    * IP = INADDDR_ANY -> faz com que o servidor se ligue em todos
    * os enderecos IP
    */
    server_dados.sin_family = AF_INET;   
    server_dados.sin_port   = htons(port);       
    server_dados.sin_addr.s_addr = INADDR_ANY;

    /*
     * Liga o servidor a porta definida anteriormente.
     */
    if (bind(s_dados, (struct sockaddr *)&server_dados, sizeof(server_dados)) < 0)
    {
		perror("Bind() aqui ... ");
		exit(3);
    }

    /* Consulta qual porta foi utilizada. */
    namelen_dados = sizeof(server_dados);
    if (getsockname(s_dados, (struct sockaddr *) &server_dados, &namelen_dados) < 0) {
        perror("getsockname()");
        exit(1);
    }

	/* Imprime qual porta E IP foram utilizados. */
    port_dados = ntohs(server_dados.sin_port);
    printf("\nPorta utilizada (enviar): %d", ntohs(server_dados.sin_port));
    printf("\nIP utilizado (enviar): %d\n", ntohs(server_dados.sin_addr.s_addr));

    /*
     * Prepara o socket para aguardar por conexoes e
     * cria uma fila de conexoes pendentes.
     */
    if (listen(s_dados, 1) != 0)
    {
	  perror("Listen()");
	  exit(4);
    }


    //  while(ns_dados == 0)
    // {
	//   /*
	//   * Aceita uma conexao e cria um novo socket atraves do qual
	//   * ocorrera a comunicacao com o cliente.
	//   */
	//   namelen_dados = sizeof(client_dados);
	//   if ((ns_dados = accept(s_dados, (struct sockaddr *) &client_dados, (socklen_t *) &namelen_dados)) == -1) {
	// 	perror("Accept()");
	// 	exit(5);
	//   }
	// }

    // return ntohs(server_dados.sin_port);
}

void enviar(char comando[], char nome_local[], char nome_remoto[]) {
    FILE *fp;
    char str_aux[COMMAND];
    char bufsize[100];
    char file_name[strlen(nome_local)+1];
    int i, len, accum;
    long sizeFile = file_size(nome_local);
    char line[FILESIZE];

    strcpy(str_aux, comando);
    // enviar o comando enviar ...
    if (send(s, &str_aux, (sizeof(str_aux)), 0) < 0) {
        perror("Send() 1");
        exit(5);
    }

    // envia o nome que o arquivo sera salvo
    strcpy(str_aux, nome_remoto);
    if (send(s, &str_aux, (sizeof(str_aux)), 0) < 0) {
        perror("Send() 2");
        exit(5);
    }

    //enviar a porta onde sera conectado o servidor ...
    if (send(s, &port_dados, (sizeof(port_dados)), 0) < 0)
    {
        perror("Send()");
        exit(5);
    }

    while(ns_dados == 0)
    {
	  /*
	  * Aceita uma conexao e cria um novo socket atraves do qual
	  * ocorrera a comunicacao com o cliente.
	  */
	  namelen_dados = sizeof(client_dados);
	  if ((ns_dados = accept(s_dados, (struct sockaddr *) &client_dados, (socklen_t *) &namelen_dados)) == -1) {
		perror("Accept()");
		exit(5);
	  }
	}

    // le o nome do arquivo
    snprintf(file_name, strlen(nome_local)+1, "%s", nome_local);
    fp = fopen(file_name, "rb");
    // printf("file_name: %s\n", file_name);

    // enviar qual o tamanho do arquivo
    if (send(ns_dados, &sizeFile, (sizeof(sizeFile)), 0) < 0) {
        perror("Send() 3");
        exit(5);
    }

    if(fp) {
        int sent_bytes = 0;
        while((sent_bytes = fread(line, 1, FILESIZE, fp)) && (sizeFile > 0)) {
            // printf("sizeFile: %li - %i\n", sizeFile, sent_bytes);
            sizeFile -= sent_bytes;

            if (send(ns_dados, &line, sent_bytes, 0) < 0) {
                perror("Send()");
                exit(5);
            }
            memset(line, 0, sizeof(line));
        }
        printf("Finalizou o processo de envio ... \n");
    }

    fclose(fp);
    close(ns_dados);
    ns_dados = 0;
    printf("Fechou tudo!\n");
}

void receber(char comando[], char nome_remoto[], char nome_local[]) {
    FILE *fp;
    char str_aux[COMMAND];
    char bufsize[100];
    char file_name[200];
    int i, len, accum;
    long sizeFile = 0;
    char line[FILESIZE];

    strcpy(str_aux, comando);
    // enviar o comando receber ...
    if (send(s, &str_aux, (sizeof(str_aux)), 0) < 0) {
        perror("Send() 1");
        exit(5);
    }

    // envia o nome que o arquivo sera salvo
    strcpy(str_aux, nome_remoto);
    printf("str_aux: %s\n", str_aux);
    if (send(s, &str_aux, (sizeof(str_aux)), 0) < 0) {
        perror("Send() 2");
        exit(5);
    }

    //enviar a porta para o servidor se conectar
    printf("port: %i\n", port_dados);
    if (send(s, &port_dados, (sizeof(port_dados)), 0) < 0)
    {
        perror("Send()");
        exit(5);
    }

    while(ns_dados == 0)
    {
	  /*
	  * Aceita uma conexao e cria um novo socket atraves do qual
	  * ocorrera a comunicacao com o cliente.
	  */
	  namelen_dados = sizeof(client_dados);
	  if ((ns_dados = accept(s_dados, (struct sockaddr *) &client_dados, (socklen_t *) &namelen_dados)) == -1) {
		perror("Accept()");
		exit(5);
	  }
	}
    
    // recebe qual o tamanho do arquivo
    if (recv(ns_dados, &sizeFile, (sizeof(sizeFile)), 0) < 0) {
        perror("Send() 3");
        exit(5);
    }
    // printf("sizeFile: %li\n", sizeFile);

    // le o nome do arquivo
    // printf("nome_local: %s\n", nome_local);
    snprintf(file_name, strlen(nome_local), "%s", nome_local);
    // printf("file_name: %s\n", file_name);
    // printf("(sizeof(line)): %lu\n", (sizeof(line)));
    fp = fopen(file_name, "wb");

    if(fp) {
		accum = 0; // quantidade de dados acumuladods
		int sent_bytes = 0; // quantidade de dados recebidos
		while(accum < sizeFile) {
			if ((sent_bytes = recv(ns_dados, &line, (sizeof(line)), 0)) < 0) {
				perror("Send() 4");
				exit(5);
			}

            if(sent_bytes > 0) {
                printf("sent_bytes = %i\n", sent_bytes);
            }

			accum += sent_bytes;//track size of growing file

			fwrite(line, sent_bytes, 1, fp);
		}
	}

    fclose(fp);
    close(ns_dados);
    ns_dados = 0;
    printf("Fechou tudo!\n");
}

void encerrar(const char list_command[]) {
    char comando_encerrar[COMMAND], nomeFile[256];
    strcpy(comando_encerrar, list_command);
    if (send(s, &comando_encerrar, (sizeof(comando_encerrar)), 0) < 0)
    {
        perror("Send()");
        exit(5);
    }
    close(s);
    close(s_dados);
    printf("Finalizando o cliente ...\n");
    exit(0);
}

void listar(const char list_command[]) {
    char comando_listar[COMMAND], nomeFile[256];
    DIR *dir;
    struct dirent *dp;
    int count = 0;

    //envia o comando "listar"
    strcpy(comando_listar, list_command);
    if (send(s, &comando_listar, (sizeof(comando_listar)), 0) < 0)
    {
        perror("Send()");
        exit(5);
    }

    printf("port: %i\n", port_dados);
    // enviar para o servidor a porta q ocorerra a comunicacao com o servidor
    if (send(s, &port_dados, (sizeof(port_dados)), 0) < 0)
    {
        perror("Send()");
        exit(5);
    }

    while(ns_dados == 0)
    {
	  /*
	  * Aceita uma conexao e cria um novo socket atraves do qual
	  * ocorrera a comunicacao com o cliente.
	  */
	  namelen_dados = sizeof(client_dados);
	  if ((ns_dados = accept(s_dados, (struct sockaddr *) &client_dados, (socklen_t *) &namelen_dados)) == -1) {
		perror("Accept()");
		exit(5);
	  }
	}

    // Recebe a lista de arquivos que estao no servidor...
    do {
        if (recv(ns_dados, &nomeFile, (sizeof(nomeFile)), 0) < 0)
        {
            perror("Recv()");
            exit(6);
        }

        if((strcmp(nomeFile, "stop")) != 0) {
            printf("> %s\n", nomeFile);
        }

    }while((strcmp(nomeFile, "stop")) != 0);

    //close(s_dados);
    close(ns_dados);
    ns_dados = 0;
}

void conectar(char hostname[], char porta[]) {
    unsigned short port;
    struct hostent *hostnm;
    struct sockaddr_in server;

    printf("hostname: %s\n", hostname);
    printf("porta: %s\n", porta);

    /*
     * Obtendo o endereco IP do servidor
     */
    hostnm = gethostbyname(hostname);
    if (hostnm == (struct hostent *) 0)
    {
        fprintf(stderr, "Gethostbyname failed\n");
        exit(2);
    }
    port = (unsigned short) atoi(porta);

    /*
     * Define o endereco IP e a porta do servidor
     */
    server.sin_family      = AF_INET;
    server.sin_port        = htons(port);
    server.sin_addr.s_addr = *((unsigned long *)hostnm->h_addr);

    /*
     * Cria um socket TCP (stream)
     */
    if ((s = socket(PF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("Socket()");
        exit(3);
    }

    /* Estabelece conexao com o servidor */
    if (connect(s, (struct sockaddr *)&server, sizeof(server)) < 0)
    {
        perror("Connect()");
        exit(4);
    }
}

int baixar(char list_command[]){
    char localizacao[200], pacote[BUFSIZ],
     nomeDoArquivo[200], mensagem[200];
    int qntd = 0;
	ssize_t len;

//verifica se existe nome do arquivo para enviar
    if (list_command[1] == NULL)
    {
        printf("Digite o nume do arquivo para receber\n");
        exit(6);
    }
    
    //define nome do arquivo a ser criado
    if(list_command[2] != NULL) {
        strcpy(nomeDoArquivo, list_command[2]);
    } else {
        strcpy(nomeDoArquivo, list_command[1]);
    }
    
    strncpy(mensagem, list_command[0], 1024);
    strcat(mensagem," ");
    strcat(mensagem, list_command[1]);

    //enviar comando e nome do arquivo
    if (send(s, &mensagem, (strlen(mensagem)), 0) < 0)
    {
        perror("Send()");
        exit(5);
    }

    //receber arquivo 
    //primeiro recebo validacao e tamanho
    if (recv(s, &pacote, BUFSIZ, 0) < 0)
    {
		fprintf(stderr, "Arquivo nao encontrado no servirdor\n");
        perror("Recv()");
        exit(6);
    }

    strtok(pacote,"\n");
    char *token = strtok(pacote, " ");
    if (strcmp(token, "404") == 0)
    {
        fprintf(stderr, "Arquivo nao encontrado no servirdor\n");
        return(1);
    }
    
    token = strtok(NULL, " ");
    qntd = atoi(token);
	FILE *received_file;
    received_file = fopen(nomeDoArquivo, "w");

    //segundo recebo os pacotes que formam o arquivo
    while (len = recv(s, &pacote, BUFSIZ, 0) > 0 && qntd > 0)
    {
        fwrite(&pacote,sizeof(char),len, received_file);
        qntd -= len;
        fprintf(stdout, "Recebidos %d bytes e aguardamos :- %d bytes\n", len, qntd);
        if (qntd <= 0) {
            break;
        }
    }

	fclose(received_file);
    return(0);
}

// MAIN FUNCTION
int main(int argc, char **argv){
    bool variavelLoop = true;
    create_socket();

    do {
        variavelLoop = false;
        printf("Insira um comando:\n");
        printf("opcoes:\n");
        printf("conectar <nome do servidor> [<porta do servidor>]\n");
        printf("listar");
        printf("receber <arquivo remoto> [<arquivo local>]\n");
        printf("enviar <arquivo local> [<arquivo remoto>]\n");
        printf("encerrar\n");

        memset(command, 0, sizeof(command));
        __fpurge(stdin);
        fgets(command,sizeof(command),stdin);

        char *token = strtok(command, " ");

        int i = 0;
        char value[3][COMMAND];

        while( token != NULL ) {
            strcpy(value[i],token);
            // printf("Token[%i] = %s\n", i, token);
            token = strtok(NULL, " ");
            i++;
        }

        //printf("command: %s\n", value[0]);
        strtok(value[0],"\n");

        if ((strcmp(value[0], "conectar")) == 0) {
            if((strcmp(value[1],"localhost")) == 0) {
                conectar(value[1], value[2]);
                create_socket(); // cria o socket de dados
            }
        } else if ((strcmp(value[0], "listar")) == 0) {
            printf("Entrou aqui para listar ...\n");
            listar(value[0]);
        } else if ((strcmp(value[0], "encerrar")) == 0) {
            encerrar(value[0]);
        } else if ((strcmp(value[0], "enviar")) == 0) {
            enviar(value[0], value[1], value[2]);
        } else if ((strcmp(value[0], "receber")) == 0) {
            receber(value[0], value[1], value[2]);
        } else {
            printf("Comando inexistente!\n");
        }

    } while(!variavelLoop);

    /* Fecha o socket */
    close(s);
    printf("Cliente terminou com sucesso.\n");
    exit(0);
}