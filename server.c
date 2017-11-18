#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h> // For using "inet_nota"

#include <unistd.h>
#include <fcntl.h>

#include <dirent.h> // For visualize directories
typedef int SOCKET;

void listDirTree(const char *name, int indent, char *buf, int recursive);
int str2int(char* str);

int main(int argc, char *argv[]){


    if(argc < 2){
        printf("Usage : ./server [PORT]\n");
        return 0;
    }

    SOCKET server_socket, client_socket;
    struct sockaddr_in server_addr, client_addr;
    int addrlen;

    // TCP socket
    server_socket = socket(PF_INET, SOCK_STREAM, 0);
    if(server_socket == -1){
        printf("Socket Error!\n");
        return -1;
    }

    // Prevent Bind Error
    int option = 1;          // SO_REUSEADDR option : TRUE
    setsockopt( server_socket, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option) );


    // Set IP Addr & Port
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(str2int(argv[1]));
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

        int read_size;
        char recv_msg[1000];
        char send_msg[1000];

        while(1){
            memset(&send_msg, 0, sizeof(send_msg));
            memset(&recv_msg, 0, sizeof(recv_msg));

            // Recv
            read_size = recv(client_socket, recv_msg, 1000, 0);
            if(read_size == -1){
                printf("Receive Failed!\n");
            }
            else if(read_size == 0){
                printf("Disconnected with Client\n");
                break;
            }

            // Send
            if(!strcmp(recv_msg, "list")){
                listDirTree("./server_folder", 0, send_msg, 0);    
               
                if(send(client_socket, send_msg, strlen(send_msg), 0) == -1){
                    printf("Return Message Error for List!\n");
                    break;
                }
            }
    

            // File Send
            if(!strncmp(recv_msg, "file", 4)){
                char fileName[100] = "./server_folder/";

                // Parse recv_msg
                int len = strlen(fileName);
                for(int i=5; recv_msg[i]!='\0' ; i++){
                    fileName[len + i - 5] = recv_msg[i];
                }
                fileName[strlen(fileName)] = '\0';

                printf("Requested File Name : %s\n",fileName);
                
                // File Open
                FILE *fd;
                int fileExist = 1;
                fd = fopen(fileName, "rb");
                if(!fd){
                    printf("File doesn't Exist\n");
                    strcpy(send_msg, "NO_SUCH_FILE");
                    fileExist = 0;
                }
                else{
                    // Transfer file size using file offset
                    char fileSize[20];
                    fseek(fd, 0, SEEK_END);
                    sprintf(fileSize, "%ld", ftell(fd));
                    fseek(fd, 0, SEEK_SET);
                    strcpy(send_msg, fileSize);
                }
            
                // Notice client whether file exists
                if(send(client_socket, send_msg, strlen(send_msg), 0) == -1){
                    printf("Sending Message Error!\n");
                    return -1;
                }
                if(!fileExist) continue;


                // Send File Data to Buffer
                int sendByte;
                char buf[1024];
                while((sendByte = fread(buf, sizeof(char), sizeof(buf), fd)) > 0){
                    send(client_socket, buf, sendByte, 0);
                }
               
                // File close 
                fclose(fd);
            }

            // Receive Message if client got file correctly
            if(recv(client_socket, recv_msg, 1000, 0) == -1){
                printf("Receiving Message Error!\n");
                return -1;
            }
            if(!strcmp(recv_msg,"THANK YOU!")){
                printf("Got Thanks Message From Client ^-^\n");
            }

        }

        // Close Client
        close(client_socket);
    }
 
    // Close Server   
    close(server_socket);

}


// Make directory tree
void listDirTree(const char *name, int indent, char *buf, int recursive){
    DIR *dir;
    struct dirent *entry;

    if(!(dir = opendir(name))){
        return;
    }
 
    // file/dir on current directory    
    while((entry = readdir(dir)) != NULL){
        if(entry->d_type == DT_DIR){
            char path[1024];
            memset(&path, 0, sizeof(path));

            if(strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name,"..") == 0){
                continue;
            }
            // make path
            strcat(path, name);
            strcat(path, "/");
            strcat(path, entry->d_name);

            printf("%s\n",path);
 
            // buffer
            if(recursive){ 
                // indentation
                for(int i=0; i<indent - 2; i++){
                    strcat(buf, " ");
                }
                strcat(buf, "ㄴ");

            }
            strcat(buf, entry->d_name);
            strcat(buf, "\n");
            
            // search recursively
            listDirTree(path, indent + 2, buf, 1);
        }
        else{
            // buffer
            if(recursive){ 
                // indentation
                for(int i=0; i<indent - 2; i++){
                    strcat(buf, " ");
                }
                strcat(buf, "ㄴ");
            }

            strcat(buf,entry->d_name);
            strcat(buf, "\n");
        }
    }
    closedir(dir);
}

int str2int(char *str){

    int ret;
    while((*str >= '0') && (*str <= '9')){
        ret = (ret * 10) + ((*str) - '0');
        str++;
    }
    return ret;
}
