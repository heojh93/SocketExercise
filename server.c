#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h> // For using "inet_nota"

#include <unistd.h>
#include <fcntl.h>

typedef int SOCKET;
int PORT;

int main(int argc, char *argv[]){

    SOCKET server_socket, client_socket;
    struct sockaddr_in server_addr, client_addr;
    int addrlen;

    PORT = 9000;

    // TCP socket
    server_socket = socket(PF_INET, SOCK_STREAM, 0);
    if(server_socket == -1){
        printf("Socket Error!\n");
        return -1;
    }

    // Set IP Addr & Port
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    // Bind
    if(bind(server_socket, (struct sockaddr*) &server_addr, sizeof(server_addr)) == -1){
        printf("Bind Error!\n");
        return -1;
    }

    // Listen
    if(listen(server_socket, 5) == -1){
        printf("Listen Error!\n");
        return -1;
    }


    // Accept
    while(1){
        printf("Listening...\n");

        // Initialize addrlen to 0
        // it will contain the actual length (in bytes) of the address returned.
        addrlen = sizeof(client_addr);
        client_socket = accept(server_socket, (struct sockaddr *) &client_addr, (socklen_t *)&addrlen);
        if(client_socket == -1){
            printf("Accept Error!\n");
            return -1;
        }
        printf("Connected with Client!\n");
        printf("[TCP] Client IP addr : %s, PORT = %d\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));


        // Close Client
        close(client_socket);
    }
 
    // Close Server   
    close(server_socket);

}

