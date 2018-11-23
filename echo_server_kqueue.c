#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/event.h>
#include <sys/time.h>
#include <errno.h>
     
#define TRUE   1  
#define FALSE  0  
#define PORT 8888 
#define BUFFER_SIZE 1024
#define MAX_EVENT 30

struct sockaddr_in server;

int server_listen(int port);
void handle_event(int server_socket);
int accept_client(int server_socket);
void echo_message(int client_socket);

int main(int argc , char *argv[]){

    int server_socket = server_listen(PORT);
    handle_event(server_socket);

    return 0;   
} 


int server_listen(int port){
    int opt = TRUE;
    int server_socket;

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

    struct kevent change_list[MAX_EVENT];
    struct kevent event_list[MAX_EVENT];
    int kq, event_used = 0;

    // create kqueue
    if((kq = kqueue()) == -1){
        perror("kqueue create failed");
    }

    // put server_socket to change_list
    EV_SET(change_list, server_socket, EVFILT_READ, EV_ADD, 0, 0, NULL);
    if (kevent(kq, change_list, 1, NULL, 0, NULL) == -1) {
        close(server_socket);
        exit(EXIT_FAILURE);
    }

    while(TRUE){
        // block for events
        int new_events = kevent(kq, NULL, 0, event_list, MAX_EVENT, NULL);
        
        for (int i = 0; i < new_events; i++){
            struct kevent *e = &event_list[i];
            if(e->ident == server_socket){
                int new_socket = accept_client(server_socket);
                EV_SET(change_list, new_socket, EVFILT_READ, EV_ADD, 0, 0, NULL);
                if(kevent(kq, change_list, 1, NULL, 0, NULL) == -1){
                    close(new_socket);
                    exit(EXIT_FAILURE);
                }
            }else{
                echo_message(e->ident);
            }
        }
    }

}

int accept_client(int server_socket){

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

    return new_socket;
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
    }   

    //Echo back 
    else {   
        buffer[rsize] = '\0';
        send(sd, buffer, strlen(buffer), 0);   
    } 
}