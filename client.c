#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h> //For using "inet_addr"

#include <unistd.h>
#include <fcntl.h>

typedef int SOCKET;
int PORT;
char* IP = "127.0.0.1";

int main(int argc, char *argv[]){
    
    SOCKET client_socket;
    struct sockaddr_in client_addr;
    int addrlen;

    // TCP SOCKET
    client_socket = socket(PF_INET, SOCK_STREAM, 0);
    if(client_socket == -1){
        printf("Socket Error!\n");
        return -1;
    }

    PORT = 9000;
    

    // SET IP Addr & Port
    memset(&client_addr, 0, sizeof(client_addr));
    client_addr.sin_family = AF_INET;
    client_addr.sin_port = htons(PORT);
    client_addr.sin_addr.s_addr = inet_addr(IP);
 

    // Connect
    if(connect(client_socket, (struct sockaddr *) &client_addr, sizeof(client_addr)) == -1){
        printf("Connect Error!\n");
        close(client_socket);
        return -1;
    }
    printf("connect complete!\n");

    // File Transfer 

    close(client_socket);

    return 0;
}
