#include <stdio.h>
#include <sys/types.h>	/* system type defintions */
#include <sys/socket.h>	/* network system functions */
#include <netinet/in.h>	/* protocol & struct definitions */
#include <stdlib.h>	/* exit() warnings */
#include <string.h>	/* memset warnings */
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>

#define BUF_SIZE	1024
#define LISTEN_PORT	60000
#define CLIENT_SIZE 30

struct Store{
  char data[30][256];
};
struct Accepted_id{
    int id1; // person who sent the request
    int id2; // recieved and accepted the request 
};
// keeps track of all information
struct State{
    char register_users_names[30][25] ;
    int register_users_sock_id[30];
    struct Store request_messages[30];
    struct Accepted_id accepted_id[30]; // all conections that were accepted
    struct Accepted_id connection_requests[30]; // all requests sent to a client

  };

void viewAcceptedFriendYouCanConnectTo(int lookup_id,struct State* state,char accepted_names[]);
void getFriendRequests(int id,char result[],struct State *state);
void addRequestToAnotherUser(int client_sock,int pid,struct State *state);

// functions to test state array
void printMatchingIds(struct State *state);
void printAllNames(struct State *state);

  // gets user name from socket id 
void getUserName(int id,char name[],struct State *state){

    for (size_t i = 0; i < 30; i++)
    {
        if (state->register_users_sock_id[i] == id)
        {
            
            strcpy(name,state->register_users_names[i]);
            break;
        }
        
    }
    
}

void viewAcceptedFriendRequestForAUser(int lookup_id,struct State* state,char accepted_names[]){

    char result[200] = "";
    for (size_t i = 0; i < CLIENT_SIZE; i++)
    {
        if (state->accepted_id[i].id1 == lookup_id )
        {
            char temp[30];
            int accepted_user = state->accepted_id[i].id2; // person accepted the request
            printf("viewAcceptedFriendRequestForAUser -- acc_user: %d",accepted_user);
            getUserName(accepted_user,temp,state);
            strcat(result,temp);
            strcat(result,"\n");
        }
        
        
    }
    strcpy(accepted_names,result);
    
}

void storeAcceptedFriendRequest(int id, int id2,struct State *state){
    
    static int i = 0;
    i %= 30;

    state->accepted_id[i].id1 = id;
    state->accepted_id[i].id2 = id2;

    i+= 1;


}

 // finds the certain users list of friend requests 
void getRegisterMessage(int id,struct State *state,char output[]){

    char result[200] = "";

    for (size_t i = 0; i < 30; i++)
    {
        if ( state->register_users_sock_id[i] == id )
        {
            
            for (size_t in = 0; in < 30; in++){
                
                if(!strcmp(state->request_messages[i].data[in],"") == 0){

                    strcat(result,state->request_messages[i].data[in]);
                    strcat(result,"\n");

                    
                }
            }
            break;
        }
        
    }

    strcpy(output,result);

} 

// adds name of person to get the friend request to and also stores there message
void addRegisterMessage(char name[],char mess[],struct State *state){

    for (size_t i = 0; i < 30; i++)
    {
        if (strcmp(state->register_users_names[i],name) == 0)
        {
            
            for (size_t in = 0; in < 30; in++){
                
                if(strcmp(state->request_messages[i].data[in],"") == 0){

                    strcpy(state->request_messages[i].data[in],mess);

                    break;
                }
            }
            break;
        }
        
    }

} 



// get socket process id from user name
int getUserId(char name[],struct State *state){

    for (size_t i = 0; i < 30; i++)
    {
        if (strcmp(state->register_users_names[i],name) == 0)
        {
            
            return state->register_users_sock_id[i];
        }
        
    }
    return -1;
}

// gets list of all connected/registered users 
char* getClientList(struct State *state){

    char userListTemp[1024] = "";

    for (size_t i = 0; i < 30; i++)
    {
        if (state->register_users_sock_id[i] != 0)
        {
            
            strcat(userListTemp,state->register_users_names[i]);
            strcat(userListTemp,"\n");
        }
        
    }

   
    static char userList[1024];
    strcpy(userList,userListTemp);

    return userList;

}  

// adds a newly registered users name and socket id to state
void updateRegisteredClients(int sock,struct State *state,char name[]){

    


    for (size_t i = 0; i < 30; i++)
    {
        if (state->register_users_sock_id[i] == 0)
        {
            state->register_users_sock_id[i] = sock;
            strcpy(state->register_users_names[i],name);
            break;
        }
        
    }
    



}
// sends message to a user
void sendMsg(char msg[],int sock_send){
    
    char buffer[BUF_SIZE];
    int send_len=strlen(msg);
    strcpy(buffer,msg);

    send(sock_send,buffer,send_len,0);
    
}
// handles all incoming messages
void handleMsg(int client_sock,char msg[], struct State *state){

    char *splitter = strtok(msg,"|");
    if (strcmp(msg,"register")==0){
        puts("a user is trying to register...");
        char buffer[BUF_SIZE];
        int send_len;
        sendMsg("request acknowledged",client_sock);
    }
    
    else if(strcmp(splitter,"register-data") == 0){
        splitter = strtok(NULL,"|");
        printf("%s is now registered in the system\n",splitter);
        updateRegisteredClients(client_sock,state,splitter);
        sendMsg("you have been registered",client_sock);
    }
    else if(strcmp(msg,"get user list")==0){

        char users[1024];
        strcpy( users,getClientList(state));
        // puts("before list");
        // puts(users);
        char toSend[1024]="registered users list|";
        strcat(toSend,users);
        // puts(toSend);
        sendMsg(toSend,client_sock);


    }
    else if (strcmp(splitter,"user name") == 0){// need to fix
        printf("arrive\n");
        splitter = strtok(NULL,"|");
        printf("the splitter is: $%s$",splitter);
        int pid = getUserId(splitter,state);
        printAllNames(state);
        printf("pid from username: %d\n",pid);
        char name[50],toSend[200];
        
        // getUserName(client_sock,name,state);
        // sprintf(toSend,"list of friend requests|%s send a request",name);
        // addRegisterMessage(splitter,"",state); // chaange name
        addRequestToAnotherUser(client_sock,pid,state); // arg1 - sender of request arg2 - reciever of request
        // sendMsg("",pid);
        sendMsg("request message send",client_sock);
        printf("done\n");
    }
    else if(strcmp(msg,"see friend requests")==0){
        char result[300];
        // updateRegisteredClients(client_sock,state,result);
        getFriendRequests(client_sock,result,state);
        printf("results below from friend req\n");
        puts(result);
        char toSend[320];
        sprintf(toSend,"list of friend requests|%s",result);
        sendMsg(toSend,client_sock);
    }

    else if (strcmp(splitter,"accepted Request") == 0){
        splitter = strtok(NULL,"|");
        printf("accepted request name: %s$\n",splitter);
        int userid = getUserId(splitter,state);
        printf("userid === %d and clientSock == %d",userid,client_sock);
        storeAcceptedFriendRequest(userid,client_sock,state);
        printMatchingIds(state);
        sendMsg("menu",client_sock);
    }

    else if (/* strcmp(splitter,"see all accepted friends") == 0 */strcmp(msg,"see all accepted friends") == 0){
        printf("in accpted friend");
        // splitter = strtok(NULL,"|");
        char nameList[200],toSend[256];
        printf("client sock is: %d\n",client_sock);
        viewAcceptedFriendRequestForAUser(client_sock,state,nameList);
        puts(nameList);
        puts("after");
        sprintf(toSend,"list of accepted friends|%s",nameList);
        sendMsg(toSend,client_sock);
    }
    else if (strcmp(msg,"menu")==0){
        sendMsg("menu",client_sock);
    }
    else if (strcmp(msg,"select and message a friend")==0){
        char nameList[200],toSend[256];
        
        viewAcceptedFriendYouCanConnectTo(client_sock,state,nameList);
        
        sprintf(toSend,"select and message a friend|%s",nameList);
        sendMsg(toSend,client_sock);
    }
    
    else if (strcmp(splitter,"client to send messages to")==0){
        splitter = strtok(NULL,"|");
        char toSend[50];

        sprintf(toSend,"messaging|%s",splitter);
        sendMsg(toSend,client_sock);
    }
    else if (strcmp(splitter,"message to someone")==0){
        splitter = strtok(NULL,"|");
        char toSend[50],name[50];
        strcpy(name,splitter);

        splitter = strtok(NULL,"|");
        printf("message: %s\n",splitter);

        // need to store messages

        sprintf(toSend,"messaging|%s",name);
        sendMsg(toSend,client_sock);
    }
    
    

}

int main(int argc, char *argv[]){
    int opt = 1;
    int			sock_recv;
    struct sockaddr_in	my_addr;
    int			i;
    fd_set	readfds,active_fd_set,read_fd_set;
    struct timeval		timeout={0,0};
    int			incoming_len;
    struct sockaddr_in	remote_addr;
    int			recv_msg_size;
    char			buf[BUF_SIZE];
    int			select_ret;
    int client_socket[30] = {};
    int sd,new_socket;
    int addrlen;
    char buffer[BUF_SIZE]; 
    struct State state;


    // turns all sockets id's in staet to 0
    int temp[30] = {};
    memcpy(state.register_users_sock_id, temp, sizeof(temp));

            
    sock_recv=socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock_recv < 0){
        printf("socket() failed\n");
        exit(0);
    }

    if( setsockopt(sock_recv, SOL_SOCKET, SO_REUSEADDR, (char *)&opt,  
          sizeof(opt)) < 0 )   
    {   
        perror("setsockopt");   
        exit(EXIT_FAILURE);   
    }   
        /* make local address structure */
    memset(&my_addr, 0, sizeof (my_addr));	/* zero out structure */
    my_addr.sin_family = AF_INET;	/* address family */
    my_addr.sin_addr.s_addr = htonl(INADDR_ANY);  /* current machine IP */
    my_addr.sin_port = htons((unsigned short)LISTEN_PORT);
        /* bind socket to the local address */
    addrlen = sizeof (my_addr);    
    i=bind(sock_recv, (struct sockaddr *) &my_addr, addrlen);
    if (i < 0){
        printf("bind() failed\n");
        exit(0);
    }

    //try to specify maximum of 3 pending connections for the master socket  
    if (listen(sock_recv, 5) < 0)   
    {   
        perror("listen");   
        exit(EXIT_FAILURE);   
    }   
       
        /* listen ... */
    while (1){
        FD_ZERO(&readfds);		
        FD_SET(sock_recv,&readfds);	
        // read_fd_set = active_fd_set;


        for (size_t i = 0; i < CLIENT_SIZE; i++)
        {
            
            sd = client_socket[i]; 
            if(sd > 0)   
                FD_SET( sd , &readfds); 
        } 
    
        // puts("before select");
        select_ret=select(FD_SETSIZE,&readfds,NULL,NULL,NULL);
        // puts("after select");

        if ((select_ret < 0) && (errno!=EINTR))   
        {   
            printf("select error");   
        }

         if (FD_ISSET(sock_recv, &readfds))   
        {   
            if ((new_socket = accept(sock_recv,  
                    (struct sockaddr *)&my_addr, (socklen_t*)&addrlen))<0)   
            {   
                perror("accept");   
                exit(EXIT_FAILURE);   
            }   
             
            //inform user of socket number - used in send and receive commands  
            printf("New connection , socket fd is %d , ip is : %s , port : %d  \n" , new_socket , inet_ntoa(my_addr.sin_addr) , ntohs (my_addr.sin_port));   
           
            //send new connection greeting message  

            // if( send(new_socket, message, strlen(message), 0) != strlen(message) )   
            // {   
            //     perror("send");   
            // }   
                 
            // puts("Welcome message sent successfully");   
                 
            //add new socket to array of sockets  
            for (i = 0; i < CLIENT_SIZE; i++)   
            {   
                //if position is empty  
                if( client_socket[i] == 0 )   
                {   
                    client_socket[i] = new_socket;   
                    printf("Adding to list of sockets as %d\n" , i);   
                         
                    break;   
                }   
            }   
        }
        //else its some IO operation on some other socket 
        // puts("before loop");
        for (i = 0; i < CLIENT_SIZE; i++)   
        {   
            // puts("inside loop");
            sd = client_socket[i];   
                 
            if (FD_ISSET( sd , &readfds))   
            {   
                // puts("inside isset ");
                //Check if it was for closing , and also read the  
                //incoming message  
                int valread;
                // puts("arrives at read");
                if ((valread = read( sd , buffer, BUF_SIZE)) == 0)   
                {   
                    //Somebody disconnected , get his details and print  
                    getpeername(sd , (struct sockaddr*)&my_addr , (socklen_t*)&addrlen);   
                    printf("Host disconnected , ip %s , port %d \n" ,  
                          inet_ntoa(my_addr.sin_addr) , ntohs(my_addr.sin_port));   
                         
                    //Close the socket and mark as 0 in list for reuse  
                    close( sd );   
                    client_socket[i] = 0;   
                }   
                     
                //Echo back the message that came in  
                else 
                {   
                    //set the string terminating NULL byte on the end  
                    //of the data read  
                    buffer[valread] = '\0';   
                    // send(sd , buffer , strlen(buffer) , 0 );

                    handleMsg(sd,buffer,&state);  

                    printf("%s\n",buffer);
                }   
            }   
        }     
        
    }

    close(sock_recv);
}

void printMatchingIds(struct State *state){
    for (size_t i = 0; i < CLIENT_SIZE; i++)
    {
        printf("%d ---- %d\n",state->accepted_id[i].id1,state->accepted_id[i].id2);
    }
    
}

void printAllNames(struct State *state){
    for (size_t i = 0; i < CLIENT_SIZE; i++)
    {
        printf("$%s$ - %d\n",state->register_users_names[i],state->register_users_sock_id[i]);
    }
}

void addRequestToAnotherUser(int client_sock,int pid,struct State *state){
    static int i = 0;
    i %= 30;

    
    state->connection_requests[i].id1 = client_sock;
    state->connection_requests[i].id2 = pid;

    i+= 1;
}

void getFriendRequests(int id,char result[],struct State *state){
    char temp[200]= "";
    for (size_t i = 0; i < CLIENT_SIZE; i++)
    {
        if (state->connection_requests[i].id2 == id){

            char name[30];
            getUserName( state->connection_requests[i].id1,name,state);
            sprintf(temp,"%s\n",name);
        }
    }


    strcpy(result,temp);
    
}
void viewAcceptedFriendYouCanConnectTo(int lookup_id,struct State* state,char accepted_names[]){

    char result[200] = "";
    for (size_t i = 0; i < CLIENT_SIZE; i++)
    {
        if (state->accepted_id[i].id1 == lookup_id )
        {
            char temp[30];
            int accepted_user = state->accepted_id[i].id2; // person accepted the request
            printf("viewAcceptedFriendRequestForAUser -- acc_user: %d",accepted_user);
            getUserName(accepted_user,temp,state);
            strcat(result,temp);
            strcat(result,"\n");
        }
        else if (state->accepted_id[i].id2 == lookup_id )
        {
            char temp[30];
            int accepted_user = state->accepted_id[i].id1; // person accepted the request
            printf("viewAcceptedFriendRequestForAUser -- acc_user: %d",accepted_user);
            getUserName(accepted_user,temp,state);
            strcat(result,temp);
            strcat(result,"\n");
        }
        
        
    }
    strcpy(accepted_names,result);
    
}

