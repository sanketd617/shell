#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <readline/readline.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <netdb.h>
#include <fcntl.h>
#include "shell_defs.c"
#include "shell_utils.c"
#include "shell_hist.c"


int getInput(char * ip){
    char prompt[CWD_LEN];
    getPrompt(prompt);
    char * buff = readline(prompt);
    if(strlen(buff) != 0){
        add_history(buff);
        strcpy(ip, buff);
        return 1;
    }
    return 0;
}

struct pipedInput {
    int numInputs;
    char inputs[MAX_PIPES+1][CMD_LEN];
};

struct argSeparatedInput {
    int numTokens;
    char* tokens[MAX_ARG_LEN+1];
};



struct pipedInput getPipeSeparatedInput(char *ip){
    struct pipedInput pip = {0};
    char *match;

    while((match = strsep(&ip, "|")) != NULL){
        if(strlen(match) > 0){
            strcpy(pip.inputs[pip.numInputs], trim(match));
            pip.numInputs++;
        }
    }

    return pip;
}

struct argSeparatedInput separateArgs(char *ip){
    struct argSeparatedInput asip = {0};
    char *match;
    while((match = strsep(&ip, " ")) != NULL){
        if(strlen(match) > 0){
            asip.tokens[asip.numTokens] = (char *) malloc(sizeof(char *)*MAX_ARG_LEN);
            strcpy(asip.tokens[asip.numTokens], trim(match));
            asip.numTokens++;
        }
    }
    return asip;
}

int connectTo(char *ipAddress, int portno){
    int sockfd, n;
    char buffer[256];
    struct sockaddr_in serv_addr;
    struct hostent *server;

        sockfd = socket(AF_INET, SOCK_STREAM, 0);
        if (sockfd < 0)
            perror("ERROR opening socket");
        server = gethostbyname(ipAddress);
        if (server == NULL) {
            fprintf(stderr,"ERROR, no such host\n");
            exit(0);
        }
        bzero((char *) &serv_addr, sizeof(serv_addr));
        serv_addr.sin_family = AF_INET;
        bcopy((char *)server->h_addr,
             (char *)&serv_addr.sin_addr.s_addr,
             server->h_length);
        serv_addr.sin_port = htons(portno);
        if (connect(sockfd,(struct sockaddr *)&serv_addr,sizeof(serv_addr)) < 0)
            perror("ERROR connecting");
        printf("Please enter the message: ");
        bzero(buffer,256);
        fgets(buffer,255,stdin);
        n = write(sockfd,buffer,strlen(buffer));
        if (n < 0)
             perror("ERROR writing to socket");
        bzero(buffer,256);
        n = read(sockfd,buffer,255);
        if (n < 0)
             perror("ERROR reading from socket");
        printf("%s\n",buffer);
        return 0;
}

int listenOnPort(int portNum){

    int server, new_server, client_address_length;
    char buffer[256];
    struct sockaddr_in server_address, client_address;
    int n;

    server = socket(AF_INET, SOCK_STREAM, 0);
    if (server < 0)
       perror("ERROR: opening socket");

    bzero((char *) &server_address, sizeof(server_address));
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = INADDR_ANY;
    server_address.sin_port = htons(portNum);
    if (bind(server, (struct sockaddr *) &server_address, sizeof(server_address)) < 0)
        perror("ERROR: on binding");
    listen(server,5);
    client_address_length = sizeof(client_address);
    printf("Listening on port %d\n", portNum);
    new_server = accept(server, (struct sockaddr *) &client_address, &client_address_length);
    if (new_server < 0)
        perror("ERROR: on accept");
    bzero(buffer,256);
    n = read(new_server,buffer,255);
    if (n < 0) perror("ERROR: reading from socket");
        printf("Here is the message: %s\n",buffer);
    n = write(new_server,"I got your message",18);
    if (n < 0) perror("ERROR: writing to socket");
        return 0;

    return 0;
}

void executeCommand(struct argSeparatedInput asip, char * ipSource, char *opTarget){

    if(strcmp(asip.tokens[0], "exit") == 0){
        printf("Bye bye..!!\n");
        exit(0);
    }

    pid_t pid = fork();

    if (pid == -1) {
        printf("Something went wrong!\n");
        return;
    }
    else if (pid == 0) {
        int fdIn, fdOut;
        int stdIn = 0, stdOut = 0;
        if(strcmp(ipSource, "stdin") == 0){
            stdIn = 1;
        }
        else{
            fdIn = open(ipSource, O_RDONLY);
            printf("%s\n", fdIn);
        }
        //
        // if(strcmp(ipSource, "stdout") == 0){
        //   stdOut = 1;
        // }
        // else{
        //   fdOut = open(ipSource, O_WRONLY);
        // }
        if(strcmp(asip.tokens[0], "history") == 0){
            displayHistory();
        }
        else if(strcmp(asip.tokens[0], "cd") == 0){

        }
        else if(strcmp(asip.tokens[0], "connect") == 0){
            connectTo(asip.tokens[1], atoi(asip.tokens[2]));
        }
        else if(strcmp(asip.tokens[0], "listen") == 0){
            listenOnPort(atoi(asip.tokens[1]));
        }
        else if (execvp(asip.tokens[0], asip.tokens) < 0) {
            printf("Invalid command!\n");
        }

        if(!stdIn){
          close(fdIn);
        }
        //
        // if(!stdOut){
        //   close(fdOut);
        // }
        exit(0);
    } else {
        wait(NULL);
        return;
    }
}

int main(int argc, const char* argv[]) {
    clearScreen();

    while(1){
        char ip[CMD_LEN];
        if(getInput(ip)){
            struct pipedInput pip = getPipeSeparatedInput(ip);
            struct argSeparatedInput argSeparatedInputs[MAX_PIPES];
            char fifos[MAX_PIPES][50];
            for(int i=0; i<pip.numInputs; i++){
                char pipeNum[MAX_PIPES];
                snprintf(pipeNum, MAX_PIPES, "%d", i);
                char myfifo[50] = "/tmp/myfifo";
                strcat(myfifo, pipeNum);

                argSeparatedInputs[i] = separateArgs(pip.inputs[i]);
                strcpy(fifos[i], myfifo);
            }

            if(pip.numInputs == 1){
              executeCommand(argSeparatedInputs[0], "stdin", "stdout");
            }
            else if(pip.numInputs == 2){
              executeCommand(argSeparatedInputs[0], "stdin", fifos[0]);
              executeCommand(argSeparatedInputs[0], fifos[0], "stdout");
            }
            else{
              executeCommand(argSeparatedInputs[0], "stdin", fifos[0]);
              for(int i=1; i<pip.numInputs-1; i++){
                  if( access(fifos[i], F_OK ) != -1 ) {
                      // file exists
                  } else {
                      mkfifo(fifos[i], 0666);
                  }
                  executeCommand(argSeparatedInputs[i], fifos[i-1], fifos[i]);
              }
              executeCommand(argSeparatedInputs[pip.numInputs-1], fifos[pip.numInputs-2], "stdout");
            }
        }
    }

    return 0;
}
