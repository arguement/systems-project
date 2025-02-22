// Jordan Williams 620108502
// Malik Curriah 620108120
// Andryck Rodgers 620118382

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
void input(char str[],int size);

void handleMsg(int client_sock,char msg[]){

    // puts(msg); // to remove
    char *splitter = strtok(msg,"|");
    if (strcmp(msg,"request acknowledged")==0){
        puts("enter your registration info...");

        char name[30];
        printf("enter your name: ");
        // fgets(name,sizeof(name),stdin);
        // fscanf(stdin,"%s",name);
        input(name,sizeof(name)); // -- here

        char strToSend[40] ;
        strcpy(strToSend,"register-data|");
        strcat(strToSend,name);
        // printf("toSend in request ack: $%s$",strToSend);
        char buffer[BUF_SIZE];
        int send_len;
        sendMsg(strToSend,client_sock);
    }
    else if (strcmp(msg,"registering")==0){
        puts("you have been registered");
        sendMsg("menu",client_sock);
    }
    else if (strcmp(msg,"you have been registered")==0){
        // puts("you are now registered to the system");
        // char pause[30];
        // printf("click enter to view list of all users: \n");
        
        // char c = getchar();
        // while (getchar() != '\n');

        sendMsg("get user list",client_sock);
    }
    else if(strcmp(splitter,"registered users list") == 0){
        splitter = strtok(NULL,"|");
        char name[40],toSend[50] = "user name|";

        printf("the list of registered users below\n");
        puts(splitter);
        printf("enter the name of the client you want to connect to: ");
        
        // fgets(name,sizeof(name),stdin);
        // fscanf(stdin,"%s",name);
        input(name,sizeof(name)); // -- here
        strcat(toSend,name);

        sendMsg(toSend,client_sock);
        puts("");
        printf("resquest to user %s sent\n",name);
        printf("accept request in other client by clicking option 2...");
        puts("");


    }
    /* else if (strcmp(splitter,"request message send") == 0){

        puts("request mess send");

    } */
    else if (strcmp(splitter,"request message send") == 0 || strcmp(msg,"menu")==0 ){
        sleep(1);
        puts("");
        puts("Main Menu");
        puts("---------");

        puts("");
        puts("");
        puts("1 ----- See connected users and make connection request (other users need to be registered first)");
        puts("");
        puts("2 ----- See friend requests");
        puts("");
        puts("3 ----- See all accepted friends");
        puts("");
        puts("4 ----- Select and message a friend (after connection request accepted)");
        puts("");
        puts("5 ----- Join work group");
        puts("");
        puts("6 ----- Join friend group");
        puts("");
        puts("7 ----- Exit");
        puts("");
        

        printf("select option > ");
        int response;
        char temp[100];
        // scanf("%d",&response);
        // fscanf(stdin,"%d",&response);
        fgets(temp,sizeof(temp),stdin); // - here
        sscanf(temp,"%d",&response);

        switch (response)
        {
        case 1:
            sendMsg("see connected users to connect to",client_sock);
            break;
        case 2:
            sendMsg("see friend requests",client_sock);
            break;
        case 3:
            sendMsg("see all accepted friends",client_sock);
            break;
        case 4:
            sendMsg("select and message a friend",client_sock);
            break;
        case 5:
            sendMsg("work",client_sock);
            break;
        case 6:
            sendMsg("friend",client_sock);
            break;
        case 7:
            sendMsg("exit",client_sock);
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
        // fscanf(stdin,"%s",name);
        input(name,sizeof(name)); // -- here
        sprintf(toSend,"accepted Request|%s",name);
        sendMsg(toSend,client_sock);
        printf("you can now connect to that friend in option 4\n");
        
    }
    else if(strcmp(splitter,"list of accepted friends") == 0){
        splitter = strtok(NULL,"|");
        printf("arrives\n");
        printf("list of accepted request below\n");
        puts(splitter);
        sendMsg("menu",client_sock);

    }
    else if (strcmp(splitter,"select and message a friend")==0){
        splitter = strtok(NULL,"|");
        printf("list of users you can connect to\n");
        puts(splitter);

        printf("select the user you want to connect to: ");
        char user[30],toSend[50];
        
        // fscanf(stdin,"%s",user);
        input(user,sizeof(user)); // -- here

        sprintf(toSend,"client to send messages to|%s",user);
        sendMsg(toSend,client_sock);
    }
    else if (/*sending messages*/strcmp(splitter,"messaging")==0){
        
        splitter = strtok(NULL,"|");
        
        char msg[30],senderName[50],toSend[50];
        strcpy(senderName,splitter);
        // fscanf(stdin,"%s",msg);
        splitter = strtok(NULL,"|");

        
        /* if (splitter != NULL){
            printf("respondant recieved your messages\n");
            sleep(1);
        }  */

        printf("previous messages other user: \n%s\n",splitter);
        puts("\ntype 'end' to end conversation");
        printf("type msg > ");

        
        fgets(msg,sizeof(msg),stdin);
        if (strcmp(msg,"end\n") == 0){
            printf("...............ending message\n");
            sendMsg("menu",client_sock);
            return;
        }
        printf("message send: %s\n",msg);
        // input(msg,sizeof(msg)); // -- here

        // msg[strcspn(msg,"\n")] = 0;
        sprintf(toSend,"message to someone|%s|%s",senderName,msg);
        sendMsg(toSend,client_sock);
        printf("\n");
    }
    else if (strcmp(msg,"work group registering") == 0){
        char name[50],toSend[100];
        printf("enter your name for the work group: ");
        input(name,sizeof(name));
        sprintf(toSend,"work group registration|%s",name);
        sendMsg(toSend,client_sock);
    }
    else if (strcmp(msg,"join work group")==0){
        sendMsg("register for work group",client_sock);
    }
    else if (strcmp(splitter,"send message in work group")==0){
        splitter = strtok(NULL,"|");

        printf("previous meesages: \n%s\n",splitter);
        puts("\ntype 'end' to end conversation");
        printf("type msg (work group) > ");
        char msg[30],toSend[50];

        fgets(msg,sizeof(msg),stdin);
        // input(msg,sizeof(msg)); // -- here
        if (strcmp(msg,"end\n") == 0){
            printf("...............ending message\n");
            sendMsg("menu",client_sock);
            return;
        }
        puts(msg);
        // printf("\n");
        sprintf(toSend,"message to work group|%s",msg);
        
        sendMsg(toSend,client_sock);

    }
    else if (strcmp(msg,"friend group registering") == 0){
        char name[50],toSend[100];
        printf("enter your name for the friend group: ");
        input(name,sizeof(name));
        sprintf(toSend,"friend group registration|%s",name);
        sendMsg(toSend,client_sock);
    }
    else if (strcmp(msg,"join friend group")==0){
        sendMsg("register for friend group",client_sock);
    }
    else if (strcmp(splitter,"send message in friend group")==0){
        splitter = strtok(NULL,"|");

        printf("previous meesages: \n%s\n",splitter);
        puts("\ntype 'end' to end conversation");
        printf("type msg (friend group) > ");
        char msg[30],toSend[50];

        fgets(msg,sizeof(msg),stdin);
        // input(msg,sizeof(msg)); // -- here
        if (strcmp(msg,"end\n") == 0){
            printf("...............ending message\n");
            sendMsg("menu",client_sock);
            return;
        }
        puts(msg);
        // printf("\n");
        sprintf(toSend,"message to friend group|%s",msg);
        
        sendMsg(toSend,client_sock);

    }
    else if (strcmp(splitter,"errors")==0){
        splitter = strtok(NULL,"|");
        printf("error message: %s\n\n",splitter);
        sendMsg("menu",client_sock);
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

    
    sendMsg("register",sock_send);

    while(1){
       
        int valread = read( sock_send, buf, BUF_SIZE);
        buf[valread] = '\0'; 
        if (strcmp(buf,"exit") == 0){

            puts("exiting client......");
            break;
        }

        
        handleMsg(sock_send,buf);
        
        }

    close(sock_send);
}

void input(char str[],int size){
  fgets(str,size,stdin);
  sscanf(str,"%s",str);
}


