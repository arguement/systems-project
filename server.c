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

int main(int argc, char *argv[]){
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

            
    sock_recv=socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock_recv < 0){
        printf("socket() failed\n");
        exit(0);
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
    
        puts("before select");
        select_ret=select(FD_SETSIZE,&readfds,NULL,NULL,NULL);
        puts("after select");

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
        puts("before loop");
        for (i = 0; i < CLIENT_SIZE; i++)   
        {   
            puts("inside loop");
            sd = client_socket[i];   
                 
            if (FD_ISSET( sd , &readfds))   
            {   
                puts("inside isset ");
                //Check if it was for closing , and also read the  
                //incoming message  
                int valread;
                puts("arrives at read");
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
                    printf("%s\n",buffer);
                }   
            }   
        }     
        
    }

    close(sock_recv);
}


