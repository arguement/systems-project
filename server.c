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

int top = 0;
int workMsg = 0;
int friendMsg = 0;

// all files used 
struct Logs{
    FILE *reg_users; // all registered users 
    FILE *all_available_friends; // all available potential friends/contacts
    FILE *all_private_messages;
    FILE *all_workgroup_messages;
    FILE *all_friendgroup_messages;
};

// conversation between 2 users
struct Conversation{
  int user1;
  int user2;

  char user1Msg[30][30];
  char user2Msg[30][30];
};
struct Store{
  char data[30][256];
};
struct Accepted_id{
    int id1; // person who sent the request
    int id2; // recieved and accepted the request 
};
// keeps track of all information
struct State{
    char register_users_names[30][25] ; // all resgitered user names
    int register_users_sock_id[30]; // all register users socket id
    struct Store request_messages[30]; // store all friend request
    struct Accepted_id accepted_id[30]; // all conections that were accepted
    struct Accepted_id connection_requests[30]; // all requests sent to a client
    struct Conversation convos[10]; //all conversation between users
    char workGroupConvos[100][70];
    char friendGroupConvos[100][70];
    struct Logs logs;

  };

void logToFile(FILE *fp,char filename[],char filemethod[],char mess[]); // append content to file

void getConvosFromWorkGroup(char msg[],struct State* state); // gets all the work group conversations
void addConvoToWorkGroup(char msg[],struct State* state);// store all work group messages  

void getConvosFromFriendGroup(char msg[],struct State* state); // gets all the friend group conversations
void addConvoToFriendGroup(char msg[],struct State* state);// store all work group messages  


void getConvoMsg(int id,int otherId,char msg[],struct Conversation convos[]); // get conversations for a specific user
void addConversationMsg(int id,int otherId,char msg[],struct Conversation convos[]); // adds a specific user conversation to his personal list
void addConnvo(struct State* state,struct Conversation convo); // adds converation

// adds id to convo
void setUserIds(int id1,int id2, struct Conversation* convo);

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
            // printf("viewAcceptedFriendRequestForAUser -- acc_user: %d",accepted_user);
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
char* getClientList(struct State *state,int id){

    char userListTemp[1024] = "";

    for (size_t i = 0; i < 30; i++)
    {
        if (state->register_users_sock_id[i] != 0 && state->register_users_sock_id[i] != id)
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
    // puts(msg);
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

        // logs to the file about all persons who registered to the system
        char logger[60];
        sprintf(logger,"%s is now registered in the system\n",splitter);
        state->logs.all_available_friends = fopen("all_available_friend","a");
        fprintf(state->logs.all_available_friends,"%s",logger);
        fclose(state->logs.all_available_friends);


        updateRegisteredClients(client_sock,state,splitter);
        sendMsg(/* "you have been registered" */"registering",client_sock);
    }
    else if (strcmp(msg,"see connected users to connect to") ==  0){
        sendMsg("you have been registered",client_sock);


        // log to file registered_users.txt which shows all registered users
        state->logs.reg_users = fopen("registered_users.txt","w");
        char users[1024];
        strcpy( users,getClientList(state,-1));
        fprintf(state->logs.reg_users,"%s",users);
        fclose(state->logs.reg_users);
    }
    else if(strcmp(msg,"get user list")==0){

        char users[1024];
        strcpy( users,getClientList(state,client_sock));
        // puts("before list");
        // puts(users);
        if (strcmp(users,"")==0){
            sendMsg("errors|no registerd clients to see. someone else needs to register",client_sock);
        }
        else{
        char toSend[1024]="registered users list|";
        strcat(toSend,users);
        // puts(toSend);
        sendMsg(toSend,client_sock);
        }


    }
    else if (strcmp(splitter,"user name") == 0){// need to fix
        // printf("arrive\n");
        splitter = strtok(NULL,"|");
        // printf("the splitter is: $%s$",splitter);
        int pid = getUserId(splitter,state);
        // printAllNames(state);
        // printf("pid from username: %d\n",pid);
        char name[50],toSend[200];
        
        // getUserName(client_sock,name,state);
        // sprintf(toSend,"list of friend requests|%s send a request",name);
        // addRegisterMessage(splitter,"",state); // chaange name
        addRequestToAnotherUser(client_sock,pid,state); // arg1 - sender of request arg2 - reciever of request
        // sendMsg("",pid);
        sendMsg("request message send",client_sock);
        // printf("done\n");
    }
    else if(strcmp(msg,"see friend requests")==0){
        char result[300];
        // updateRegisteredClients(client_sock,state,result);
        getFriendRequests(client_sock,result,state);

        if (strcmp(result,"") == 0){
            sendMsg("errors|no resgister user has sent you a friend request",client_sock);
        }
        else{

        
        
        char toSend[320];
        sprintf(toSend,"list of friend requests|%s",result);
        sendMsg(toSend,client_sock);

        }
    }

    else if (strcmp(splitter,"accepted Request") == 0){
        splitter = strtok(NULL,"|");
        printf("accepted request name: %s$\n",splitter);
        int userid = getUserId(splitter,state);
        // printf("userid === %d and clientSock == %d",userid,client_sock);
        storeAcceptedFriendRequest(userid,client_sock,state);
        // printMatchingIds(state);
        sendMsg("menu",client_sock);
    }

    else if (/* strcmp(splitter,"see all accepted friends") == 0 */strcmp(msg,"see all accepted friends") == 0){
       
       
        char nameList[200],toSend[256];
        // printf("client sock is: %d\n",client_sock);
        viewAcceptedFriendRequestForAUser(client_sock,state,nameList);
        puts(nameList);
        if (strcmp(nameList,"") == 0){
            sendMsg("errors|no accepted friend requests. Someone must have accepted a friend request first",client_sock);
        }
        else{
        
            sprintf(toSend,"list of accepted friends|%s",nameList);
            sendMsg(toSend,client_sock);
        }
    }
    else if (strcmp(msg,"menu")==0){
        sendMsg("menu",client_sock);
    }
    else if (strcmp(msg,"select and message a friend")==0){
        char nameList[200],toSend[256];
        
        viewAcceptedFriendYouCanConnectTo(client_sock,state,nameList);
        if (strcmp(nameList,"") ==  0){
            sendMsg("errors|you or a friend needs to send and accept a friend request first",client_sock);
        }
        else{
        
        sprintf(toSend,"select and message a friend|%s",nameList);
        sendMsg(toSend,client_sock);
        }
    }
    
    else if (strcmp(splitter,"client to send messages to")==0){
        splitter = strtok(NULL,"|");
        char toSend[50],name[50];

        strcpy(name,splitter);
        int revieverID = getUserId(splitter,state);

        struct Conversation convo;
        setUserIds(client_sock,revieverID,&convo);

        addConnvo(state,convo); // adds convo that was previously sent

        char store[500];
        getConvoMsg(client_sock,getUserId(name,state),store,state->convos);

        sprintf(toSend,"messaging|%s|%s",name,store);
        sendMsg(toSend,client_sock);
    }
    else if (strcmp(splitter,"message to someone")==0){
        splitter = strtok(NULL,"|");
        char toSend[1150],name[50],senderName[50];
        strcpy(name,splitter);
        
        getUserName(client_sock,senderName,state);
        printf("message from %s\n",senderName);
        printf("message sent to %s\n",name);
        splitter = strtok(NULL,"|");
        addConversationMsg(client_sock,getUserId(name,state),splitter,state->convos);
        printf("message: %s\n",splitter);

        char logger[60];
        sprintf(logger,"from: %s\nto: %s\nmessage: %s\n\n",senderName,name,splitter);
        logToFile( state->logs.all_private_messages,"all_private_messages.txt","a",logger); // saves private message to file

        // need to store messages
        char store[500];
        getConvoMsg(client_sock,getUserId(name,state),store,state->convos);
        puts(store);

        sprintf(toSend,"messaging|%s|%s",name,store);
        sendMsg(toSend,client_sock);
    }
    else if(strcmp(msg,"register for work group")==0){
        char requesterName[50];
        getUserName(client_sock,requesterName,state);
        printf("registering for work group\n");
        printf("%s now registered in the group\n",requesterName);

        char allWorkMsg[550],toSend[700];
        getConvosFromWorkGroup(allWorkMsg,state);
        sprintf(toSend,"send message in work group|%s",allWorkMsg);

        // sendMsg("send message in work group",client_sock);
        sendMsg(toSend,client_sock);
    }
    else if(strcmp(splitter,"message to work group")==0){
        splitter = strtok(NULL,"|");
        char message[100];
        char senderName[30];
        char toSend[700];
        getUserName(client_sock,senderName,state);

        sprintf(message,"fr %s:- %s\n",senderName,splitter);
        puts(message);
        addConvoToWorkGroup(message,state);
        logToFile(state->logs.all_workgroup_messages,"all_workgroup_messages.txt","a",message); // saves to friend group file
        char allWorkMsg[550];
        getConvosFromWorkGroup(allWorkMsg,state);
        printf("all work messages: \n%s",allWorkMsg);

        sprintf(toSend,"send message in work group|%s",allWorkMsg);
        sendMsg(toSend,client_sock);
    }




    else if(strcmp(msg,"register for friend group")==0){
        char requesterName[50];
        getUserName(client_sock,requesterName,state);
        printf("registering for friend group\n");
        printf("%s now registered in the group\n",requesterName);

        // get all messages from friend group
        char allWorkMsg[550],toSend[700];
        addConvoToFriendGroup(allWorkMsg,state);
        sprintf(toSend,"send message in friend group|%s",allWorkMsg);

        // sendMsg("send message in friend group",client_sock);
        sendMsg(toSend,client_sock);
    }
    else if(strcmp(splitter,"message to friend group")==0){
        splitter = strtok(NULL,"|");
        char message[100];
        char senderName[30];
        char toSend[700];
        getUserName(client_sock,senderName,state);

        sprintf(message,"fr %s:- %s\n",senderName,splitter);
        puts(message);
        addConvoToFriendGroup(message,state);
        logToFile(state->logs.all_friendgroup_messages,"all_friendgroup_messages.txt","a",message); // saves to friend group file
        char allWorkMsg[550];
        getConvosFromFriendGroup(allWorkMsg,state);
        printf("all work messages: \n%s",allWorkMsg);

        sprintf(toSend,"send message in friend group|%s",allWorkMsg);
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
    
       
        select_ret=select(FD_SETSIZE,&readfds,NULL,NULL,NULL);
    

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
      
        for (i = 0; i < CLIENT_SIZE; i++)   
        {   
            
            sd = client_socket[i];   
                 
            if (FD_ISSET( sd , &readfds))   
            {   
                
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


                    // removes user after disconnecting
                    for (size_t i = 0; i < 30; i++) 
                    {
                        if (state.register_users_sock_id[i] == sd)
                        {
                            state.register_users_sock_id[i] = 0;
                            break;
                        }
                        
                    }      
                         
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
                    

                    handleMsg(sd,buffer,&state);  

                   
                }   
            }   
        }     
        
    }

    close(sock_recv);
}

// for testing
void printMatchingIds(struct State *state){
    for (size_t i = 0; i < CLIENT_SIZE; i++)
    {
        printf("%d ---- %d\n",state->accepted_id[i].id1,state->accepted_id[i].id2);
    }
    
}
// for test
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
            
            getUserName(accepted_user,temp,state);
            strcat(result,temp);
            strcat(result,"\n");
        }
        else if (state->accepted_id[i].id2 == lookup_id )
        {
            char temp[30];
            int accepted_user = state->accepted_id[i].id1; // person accepted the request
            // printf("viewAcceptedFriendRequestForAUser -- acc_user: %d",accepted_user);
            getUserName(accepted_user,temp,state);
            strcat(result,temp);
            strcat(result,"\n");
        }
        
        
    }
    strcpy(accepted_names,result);
    
}

void addConnvo(struct State* state,struct Conversation convo){
  
  top  %= 10;


  state->convos[top] = convo;

  

  top++;
}

void setUserIds(int id1,int id2, struct Conversation* convo){
  convo->user1 = id1;
  convo->user2 = id2;
  
  for(int i =0; i < 30;i++){
    strcpy(convo->user1Msg[i],"$");
    strcpy(convo->user2Msg[i],"$");

  }
}


void addConversationMsg(int id,int otherId,char msg[],struct Conversation convos[]){
  for(int i =0; i < 30;i++){
    
    
    if((convos->user1 == id && convos->user2 == otherId) || (convos->user1 == otherId && convos->user2 == id) ){

      if (id == convos->user1){
        for(int j=0; j < 30; j++){
          if (strcmp(convos->user1Msg[j],"$") ==  0){
            strcpy(convos->user1Msg[j],msg);
            break;
          }
        }
      }
      else if (id == convos->user2){
        for(int j=0; j < 30; j++){
          if (strcmp(convos->user2Msg[j],"$") ==  0){
            strcpy(convos->user2Msg[j],msg);
            break;
          }
        }
      }
  break;
    }
    
  }
}
void getConvoMsg(int id,int otherId,char msg[],struct Conversation convos[]){

  char allMsg[200]="";
  for(int i =0; i < 30;i++){
    
    
    if((convos->user1 == id && convos->user2 == otherId) || (convos->user1 == otherId && convos->user2 == id) ){

      if (id == convos->user1){
        for(int j=0; j < 30; j++){
          if (strcmp(convos->user2Msg[j],"$") !=  0){
         
            strcat(allMsg, convos->user2Msg[j]);
            strcat(allMsg, "\n");
          }
        }
        break;
      }
      else if (id == convos->user2){
        for(int j=0; j < 30; j++){
          if (strcmp(convos->user1Msg[j],"$") !=  0){

            strcat(allMsg, convos->user1Msg[j]);
            strcat(allMsg, "\n");
          }
        }
        break;
      }
  break;
    }
    
  }
  strcpy(msg, allMsg);
}

void addConvoToWorkGroup(char msg[],struct State* state){
    workMsg %= 100;

    strcpy(state->workGroupConvos[workMsg],msg);

    workMsg++; 
}

void getConvosFromWorkGroup(char msg[],struct State* state){

    char allCovos[500] = "";

    for (size_t i = 0; i < workMsg; i++)
    {
        strcat(allCovos,state->workGroupConvos[i]);
        strcat(allCovos,"\n");
    }
    
    strcpy(msg,allCovos);
}


void addConvoToFriendGroup(char msg[],struct State* state){

    friendMsg %= 100;

    strcpy(state->friendGroupConvos[friendMsg]/* workFriendConvos[workMsg] */,msg);

    friendMsg++; 
}

void getConvosFromFriendGroup(char msg[],struct State* state){

    char allCovos[500] = "";

    for (size_t i = 0; i < friendMsg; i++)
    {
        strcat(allCovos,state->friendGroupConvos[i]);
        strcat(allCovos,"\n");
    }
    
    strcpy(msg,allCovos);
}

void logToFile(FILE *fp,char filename[],char filemethod[],char mess[]){


    fp = fopen(filename,filemethod);
    fprintf(fp,"%s",mess);
    fclose(fp);
        
}

