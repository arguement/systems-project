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

void sendMsg(char msg[],int sock_send){
    
    char buffer[BUF_SIZE];
    int send_len=strlen(msg);
    strcpy(buffer,msg);

    send(sock_send,buffer,send_len,0);
    
}

void handleMsg(int client_sock,char msg[]){

    // puts("handle msg");
    char *splitter = strtok(msg,"|");
    if (strcmp(msg,"request acknowledged")==0){
        puts("enter your registration info...");

        char name[30];
        printf("enter your name: ");
        // fgets(name,sizeof(name),stdin);
        fscanf(stdin,"%s",name);

        char strToSend[40] ;
        strcpy(strToSend,"register-data|");
        strcat(strToSend,name);
        printf("toSend in request ack: $%s$",strToSend);
        char buffer[BUF_SIZE];
        int send_len;
        sendMsg(strToSend,client_sock);
    }
    else if (strcmp(msg,"you have been registered")==0){
        puts("you are now registered to the system");
        // char pause[30];
        printf("click enter to view list of all users: \n");
        getchar();

        sendMsg("get user list",client_sock);
    }
    else if(strcmp(splitter,"registered users list") == 0){
        splitter = strtok(NULL,"|");
        char name[40],toSend[50] = "user name|";

        printf("the list of registered users below\n");
        puts(splitter);
        printf("enter the name of the client you want to connect to: ");
        
        // fgets(name,sizeof(name),stdin);
        fscanf(stdin,"%s",name);
        strcat(toSend,name);

        sendMsg(toSend,client_sock);


    }
    /* else if (strcmp(splitter,"request message send") == 0){

        puts("request mess send");

    } */
    else if (strcmp(splitter,"request message send") == 0 || strcmp(msg,"menu")==0 ){
        
        puts("this is the menu");
        puts("1 ----- see friend requests");
        puts("2 ----- see all accepted friends");
        puts("1 ----- see friend requests");
        puts("1 ----- see friend requests");

        int response;
        // scanf("%d",&response);
        fscanf(stdin,"%d",&response);
        switch (response)
        {
        case 1:
            sendMsg("see friend requests",client_sock);
            break;
        case 2:
            sendMsg("see all accepted friends",client_sock);
            break;
        default:
            break;
        }
    }
    else if (strcmp(splitter,"list of friend requests") == 0){
        splitter = strtok(NULL,"|");
        puts("friend request below");
        printf("%s\n",splitter);

        puts("accept a friend: ");
        char name[40],toSend[50];
        // fgets(name,sizeof(name),stdin);
        fscanf(stdin,"%s",name);
        sprintf(toSend,"accepted Request|%s",name);
        sendMsg(toSend,client_sock);
    }
    else if(strcmp(splitter,"list of accepted friends") == 0){
        splitter = strtok(NULL,"|");
        printf("arrives\n");
        printf("list of accepted request below\n");
        puts(splitter);

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
    sendMsg("register",sock_send);

    while(1){
        // printf("Send? ");
        // scanf("%s",text);
        // puts("before revieve");
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


