#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>

#define BUFFER_SIZE 2048

int main(int argc, char *argv[]) {
    if (argc != 5) {
        printf("Usage: %s -ip <server_ip> -p <port>\n", argv[0]);
        return 1;
    }

    char* server_ip = NULL;
    int port = 0;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-ip") == 0) {
            server_ip = argv[++i];
        } else if (strcmp(argv[i], "-p") == 0) {
            port = atoi(argv[++i]);
        }
    }

    if (!server_ip || port == 0) {
        printf("Invalid arguments. Please provide the server IP and port.\n");
        return 1;
    }

    int client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket < 0) {
        perror("socket");
        return 1;
    }

    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(server_ip);
    server_addr.sin_port = htons(port);

    if (connect(client_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("connect");
        return 1;
    }

    printf("Connected to server\n");

    while (1) {
    printf("Enter a command for the server: ");
    char buffer[BUFFER_SIZE];
    fgets(buffer, sizeof(buffer), stdin);
    printf("After Fgets \n");
    buffer[strcspn(buffer, "\n")] = '\0';  // Remove trailing newline

    char *datafilename = NULL;

    

    send(client_socket, buffer, strlen(buffer), 0);
    printf("Command sent to server: %s\n", buffer);

    if (strstr(buffer, "kmeanspar") != NULL) {
            if (sscanf(buffer, "%*s -f %ms", &datafilename) == 1) {
            printf("Filename: %s\n", datafilename);
            FILE *file = fopen(datafilename, "r");
            if (file) {
                char buffer[2048];
                int bytes_read;
                while (fgets(buffer,2048,file)!=NULL) {
                    send(client_socket, buffer,sizeof(buffer),0);
                    bzero(buffer,2048);        
                    }
                send(client_socket,"SALAAR",strlen("SALAAR"),0);    
                fclose(file);
            }
            free(datafilename);
            } else {
                // Handle error if sscanf fails to extract the filename
                fprintf(stderr, "Error: Could not extract filename.\n");
            }

        } 


    // Receive the filename
    char filename[256];
    ssize_t bytes_received = recv(client_socket, filename, sizeof(filename) - 1, 0);  // -1 to leave space for null terminator
    if (bytes_received <= 0) {
        perror("Failed to receive filename from server");
        close(client_socket);
        exit(1);
    }
    filename[bytes_received] = '\0';
    printf("Received the solution file name: %s\n", filename);

    char temp_str1[256];  // Adjust the size as needed
    strcpy(temp_str1, "./results/");
    strcat(temp_str1,filename);

    FILE *file = fopen(temp_str1, "wb");
    if (!file) {
        perror("Failed to open file");
        close(client_socket);
        exit(1);
    }
    int n;
    char file_data[BUFFER_SIZE];

    while (1){
        n=recv(client_socket,file_data,2048,0);
        if(strcmp(file_data,"SV")==0){
        break;
        }
        fprintf(file,"%s",file_data);
        bzero(file_data,2048);
    }

    fclose(file);
    printf("File %s received and saved.\n", filename);
}
    

    close(client_socket);
    return 0;
}