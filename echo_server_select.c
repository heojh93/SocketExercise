#include <stdio.h>  
#include <string.h>   //strlen  
#include <stdlib.h>  
#include <errno.h>  
#include <unistd.h>   //close  
#include <arpa/inet.h>    //close  
#include <sys/types.h>  
#include <sys/socket.h>  
#include <netinet/in.h>  
#include <sys/time.h> //FD_SET, FD_ISSET, FD_ZERO macros  

     
#define TRUE   1  
#define FALSE  0  
#define PORT 8888 

#define MAX_CLIENT 30

struct sockaddr_in server;
int client_socket[MAX_CLIENT];

int server_listen(int port);
void handle_event(int server_socket);
int block_for_event(int server_socket, fd_set *readfds);
void accept_client(int server_socket);
void echo_message(int client_socket);

int main(int argc , char *argv[]){

    int server_socket = server_listen(PORT);
    handle_event(server_socket);
    
    return 0;   
} 


int server_listen(int port){
    int opt = TRUE;
    int server_socket;
    
    // initialize all client sockets
    for(int i=0; i<MAX_CLIENT; i++){
        client_socket[i] = 0;
    }

    if( (server_socket = socket(AF_INET, SOCK_STREAM, 0)) == 0 ){
        perror("socket failed");
        exit(EXIT_FAILURE);
    }
    printf("server socket = %d\n", server_socket);

    // set master socket to allow multiple connections
    if( setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, (char*)&opt, sizeof(opt)) < 0){
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    // type of socket created
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(PORT);

    // bind the socket to localhost port 8888
    if (bind(server_socket, (struct sockaddr *)&server, sizeof(server)) < 0){
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
    printf("Listener on port %d \n", PORT);

    // maximum 5 pending connection
    if(listen(server_socket, 3) < 0){
        perror("listen");
        exit(EXIT_FAILURE);
    }

    puts("Waiting for connections ... ");

    return server_socket;
}

void handle_event(int server_socket){
    
    // socket descripters set
    fd_set readfds;

    while(TRUE){
        if(block_for_event(server_socket, &readfds) < 0 && (errno != EINTR)){
            printf("error while block");
        }
        for (int sd = 0; sd < MAX_CLIENT; sd++){
            if(sd == server_socket){
                if(FD_ISSET(server_socket, &readfds)){
                    accept_client(server_socket);
                }
            }else{
                if(FD_ISSET(client_socket[sd], &readfds)){
                    echo_message(sd);
                }
            }
        }
    }

}

int block_for_event(int server_socket, fd_set *readfds){
    // clear the socket set
    FD_ZERO(readfds);

    // add master socket to set
    FD_SET(server_socket, readfds);
    
    for(int i=0; i<MAX_CLIENT; i++){
        // socket descripter
        if(client_socket[i] > 0){
            FD_SET(client_socket[i], readfds);
        }
    }
    
    return select(MAX_CLIENT + 1, readfds, NULL, NULL, NULL);
}

void accept_client(int server_socket){

    char * message = "Hello\n";
    int addrlen = sizeof(server);
    int new_socket;

    if((new_socket = accept(server_socket, (struct sockaddr *)&server, (socklen_t*)&addrlen))<0){
        perror("accept");
        exit(EXIT_FAILURE);   
    }

    printf("New connection , socket fd is %d , ip is : %s , port : %d\n", 
        new_socket , inet_ntoa(server.sin_addr) , ntohs(server.sin_port));
    
    //send greeting message  
    if( send(new_socket, message, strlen(message), 0) != strlen(message) ){   
        perror("send");
    }
    puts("Welcome message sent successfully");

    //add new socket to array of sockets  
    for (int i = 0; i < MAX_CLIENT; i++) {   
        
        if( client_socket[i] == 0 ) {
            client_socket[i] = new_socket;   
            printf("Adding to list of sockets as %d\n" , i);   
            break;   
        }   

    }   
}

void echo_message(int sd){

    char buffer[1025];
    struct sockaddr_in client;
    int addrlen = sizeof(client);
    int rsize;

    if ((rsize = read(sd, buffer, 1024)) == 0) {   
        
        getpeername(sd, (struct sockaddr*)&client, (socklen_t*)&addrlen);   
    
        printf("Host disconnected , ip %s , port %d \n" ,  
            inet_ntoa(client.sin_addr) , ntohs(client.sin_port));

        close(sd);   
        client_socket[sd] = 0;   
    }   

    //Echo back 
    else {   
        buffer[rsize] = '\0';
        send(sd, buffer, strlen(buffer), 0);   
    } 
}