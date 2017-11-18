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

    char send_msg[1000];
    char recv_msg[1000];

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

    
    while(1){
        memset(&send_msg, 0, sizeof(send_msg));
        memset(&recv_msg, 0, sizeof(recv_msg));

        // Send
        scanf("%s", send_msg);
        if(send(client_socket, send_msg, strlen(send_msg), 0) == -1){
            printf("Sending Message Error!\n");
            return -1;
        }

        // List of file
        if(!strcmp(send_msg, "list")){
            if(recv(client_socket, recv_msg, 1000, 0) == -1){
                printf("Receiving Message Error for List!\n");
                break;
            }
            printf("%d\n",strlen(recv_msg));
            printf("%s\n",recv_msg);
        }




        // File Transfer 
    }

    close(client_socket);

    return 0;
}
