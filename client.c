#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h> // mkdir
#include <netinet/in.h>
#include <arpa/inet.h> //For using "inet_addr"

#include <unistd.h>
#include <fcntl.h>
#include <libgen.h> // basename

typedef int SOCKET;
char* IP = "127.0.0.1";

int str2int(char* str);

int main(int argc, char *argv[]){
   
    if(argc < 2){
        printf("Usage : ./client [PORT]\n");
        return 0;
    }

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


    // SET IP Addr & Port
    memset(&client_addr, 0, sizeof(client_addr));
    client_addr.sin_family = AF_INET;
    client_addr.sin_port = htons(str2int(argv[1]));
    client_addr.sin_addr.s_addr = inet_addr(IP);
 

    // Connect
    if(connect(client_socket, (struct sockaddr *) &client_addr, sizeof(client_addr)) == -1){
        printf("Connect Error!\n");
        close(client_socket);
        return -1;
    }
    printf("connect complete!\n\n");

    
    while(1){
        memset(&send_msg, 0, sizeof(send_msg));
        memset(&recv_msg, 0, sizeof(recv_msg));

        printf("              USAGE               \n");
        printf("#1. List Server Directory : \"list\"\n");
        printf("#2. File Transfer : \"file\"\n");
        printf("#3. Quit : \"q\"\n");
        printf("Input : ");

        // Send type
        scanf("%s", send_msg);

        if(!strcmp(send_msg, "q")) break;

        // List of file
        if(!strcmp(send_msg, "list")){
 
            // Send
            if(send(client_socket, send_msg, strlen(send_msg), 0) == -1){
                printf("Sending Message Error!\n");
                return -1;
            }

            // Receive
            if(recv(client_socket, recv_msg, 1000, 0) == -1){
                printf("Receiving Message Error for List!\n");
                break;
            }
            printf("\n###  server_folder ###\n");
            printf("%s\n",recv_msg);
        }

        // File Transfer 
        if(!strcmp(send_msg, "file")){
            
            char fileName[100]; 
            memset(&fileName, 0, sizeof(fileName));
            
            printf("Input file name : ");
            scanf("%s", fileName);
            
            strcat(send_msg, " ");
            strcat(send_msg, fileName);
            
            // Send File Name
            if(send(client_socket, send_msg, strlen(send_msg), 0) == -1){
                printf("Sending Message Error!\n");
                return -1;
            }
        
            // Receive Message about whether file exists or not
            if(recv(client_socket, recv_msg, 1000, 0) == -1){
                printf("Receiving Message Error for List!\n");
                break;
            }
            if(!strcmp(recv_msg, "NO_SUCH_FILE")){
                printf("No Such File!\n");
                continue;
            }


            
            char fileName_[100] = "./client_folder/";
            strcat(fileName_,fileName);

            char *dirc = strdup(fileName_);
            char *dname = dirname(dirc);
            mkdir(dname, 0755);

            // File Open
            FILE *fd = fopen(fileName_, "wb");

            int bufferNum, totalBufferNum;
            long readByte, totalReadByte;
            long fileSize;
            
            char buf[1024];

            fileSize = str2int(recv_msg);
            totalBufferNum = fileSize / 1024 + 1;
            bufferNum = 0;
            totalReadByte = 0;

            memset(&recv_msg, 0, sizeof(recv_msg));

            // Read Buffer from Socket & Write to client file
            while(bufferNum != totalBufferNum){
                readByte = recv(client_socket, buf, 1024, 0);
                bufferNum++;
                totalReadByte += readByte;
                printf("In progress : %ld/%ld Byte(s) [%d%%]\n\n", totalReadByte, fileSize, (bufferNum*100/totalBufferNum));
                fwrite(buf, sizeof(char), readByte, fd);
                if(readByte == -1){
                    printf("File Receive Error!\n");
                    return -1;
                }
            } 

            fclose(fd);
            // Fin message 
            memset(&send_msg, 0, sizeof(send_msg));
            strcpy(send_msg, "THANK YOU!");
            if(send(client_socket, send_msg, strlen(send_msg), 0) == -1){
                printf("Message Error for Thx!\n");
                return -1;
            }

        }
    }

    close(client_socket);

    return 0;
}

int str2int(char* str)
{
    int ret;
    while ((*str >= '0') && (*str <= '9')){
        ret = (ret * 10) + ((*str) - '0');
        str++;
    }
    return ret;
}

