#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>	/* exit() warnings */
#include <string.h>	/* memset warnings */
#include <unistd.h>
#include <arpa/inet.h>


#define BUF_SIZE	1024
#define SERVER_IP	"127.0.0.1"
#define	SERVER_PORT	60000

void sendMsg(char msg[],int sock_send,char buffer[],int* send_len){
    
    
    *send_len=strlen(msg);
    strcpy(buffer,msg);

    send(sock_send,buffer,*send_len,0);
    printf("\nafter send");
}

void handleMsg(int client_sock,char msg[]){

    puts("handle msg");

    if (strcmp(msg,"request acknowledged")==0){
        puts("enter your registration info...");

        char name[30];
        printf("enter your name: ");
        scanf("%s",name);

        char strToSend[40] ;
        strcpy(strToSend,"register-data|");
        strcat(strToSend,name);

        char buffer[BUF_SIZE];
        int send_len;
        sendMsg(strToSend,client_sock,buffer,&send_len);
    }

}



int main(int argc, char *argv[]){
    int			sock_send;
    struct sockaddr_in	addr_send;
    char			text[80],buf[BUF_SIZE];
    int			send_len,bytes_sent;
    int i;
        /* create socket for sending data */
    sock_send=socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock_send < 0){
        printf("socket() failed\n");
        exit(0);
    }

            /* fill the address structure for sending data */
    memset(&addr_send, 0, sizeof(addr_send));  /* zero out structure */
    addr_send.sin_family = AF_INET;  /* address family */
    addr_send.sin_addr.s_addr = inet_addr(SERVER_IP);
    addr_send.sin_port = htons((unsigned short)SERVER_PORT);

    i=connect(sock_send, (struct sockaddr *) &addr_send, sizeof (addr_send));
    if (i < 0){
        printf("connect() failed\n");
        exit(0);
    }

    // char initial[] = "regsiter";
    // send_len=strlen(text);
    // strcpy(buf,initial);

    // send(sock_send,buf,send_len,0);
    sendMsg("register",sock_send,buf,&send_len);

    while(1){
        // printf("Send? ");
        // scanf("%s",text);
        puts("before revieve");
        int valread = read( sock_send, buf, BUF_SIZE);
        buf[valread] = '\0'; 
        if (strcmp(text,"quit") == 0)
            break;

        // strcpy(buf,text);
        // send_len=strlen(text);
        handleMsg(sock_send,buf);
        // bytes_sent=send(sock_send,buf,send_len,0);
        }

    close(sock_send);
}


