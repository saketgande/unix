#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/epoll.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>


#define MAX_CLIENTS 1024
#define EPOLL_SIZE 1024
#define BUFFER_SIZE 2048





// Placeholder functions for matrix inversion and k-means clustering
int compute_matrix_inversion(char *command, char *result_file) {
    // Construct the command to execute with redirection to save the output to result_file
    char full_command[1024];
    snprintf(full_command, sizeof(full_command), "./%s > ../computed_results/%s", command, result_file);

    // Execute the command
    system(full_command);
    return system(full_command);
}

int compute_kmeans(char *command, char *result_file) {
    // Construct the command to execute with redirection to save the output to result_file
    char full_command[1024];
    snprintf(full_command, sizeof(full_command), "./%s > ../computed_results/%s", command, result_file);

    // Execute the command
    system(full_command);
    return system(full_command);
}


void handle_client_request(int client_socket, int client_id) {
    char buffer[BUFFER_SIZE];

    while (1) {
        ssize_t bytes_received = recv(client_socket, buffer, sizeof(buffer), 0);
        if (bytes_received <= 0) {
            break;
        }
        buffer[bytes_received] = '\0';

        static int command_counter = 0;
        command_counter++;

        printf("Client %d command: %s\n", client_id, buffer);

        char result_filename[256];
        if (strstr(buffer, "matinvpar") != NULL) {
            sprintf(result_filename, "matinv_client%d_soln%d.txt", client_id, command_counter);
            compute_matrix_inversion(buffer, result_filename);
            if (compute_matrix_inversion(buffer, result_filename) != 0) {
                perror("Error executing matrix inversion");
            }

        } else if (strstr(buffer, "kmeanspar") != NULL) {
            sprintf(result_filename, "kmeans_client%d_soln%d.txt", client_id, command_counter);

            char *datafilename;
            char *flag_position = strstr(buffer, "-f");

            if (flag_position != NULL) {
            // Move the pointer to the character after the space following "-f"
            flag_position += strlen("-f") + 1;

            // Find the position of the next space after the filename
            char *space_position = strchr(flag_position, ' ');

            if (space_position != NULL) {
                // Calculate the length of the filename
                size_t filename_length = space_position - flag_position;

                // Allocate memory for the filename and copy it
                datafilename = malloc(filename_length + 1);
                strncpy(datafilename, flag_position, filename_length);
                datafilename[filename_length] = '\0';

            }
            if (datafilename != NULL) {
                printf("File name in server received print %s \n", datafilename);
                FILE *file = fopen(datafilename, "wb");
                int n;
                char file_data[BUFFER_SIZE];
                while (1){
                    n=recv(client_socket,file_data,2048,0);
                    
                    if(strcmp(file_data,"SALAAR")==0){
                        break;
                    }
                    fprintf(file,"%s",file_data);
                    bzero(file_data,2048);
                    }
                    fclose(file);
                free(datafilename);
            }
            }
            compute_kmeans(buffer, result_filename);
            if (compute_kmeans(buffer, result_filename) != 0) {
                perror("Error executing k-means");
            }
        } else {
            strcpy(result_filename, "error.txt");
            char *temp_str = "../computed_results/";
            strcat(temp_str,result_filename);
            FILE *error_file = fopen(temp_str, "w");
            fprintf(error_file, "Invalid command received.");
            fclose(error_file);
        }

        char temp_str1[256];
        strcpy(temp_str1, "../computed_results/");
        strcat(temp_str1,result_filename);
        printf("%s",temp_str1);

        printf("Sending solution: %s\n", result_filename);
        

        send(client_socket, result_filename, strlen(result_filename), 0);

        
        FILE *file = fopen(temp_str1, "r");


        if (file) {
            char buffer[2048];
            int bytes_read;
            while (fgets(buffer,1024,file)!=NULL) {
                send(client_socket, buffer,sizeof(buffer),0);
                bzero(buffer,1024);        
                }
            send(client_socket,"SV",strlen("SV"),0);    
            fclose(file);
        }
    }
    printf("socket_close handle _client_request \n");
    close(client_socket);
}


void start_server_fork(int server_socket) {
    printf("Server running in 'fork' mode...\n");
    int client_id = 0;

    while (1) {
        int client_socket = accept(server_socket, NULL, NULL);
        if (client_socket < 0) {
            perror("accept");
            continue;
        }
        client_id++;
        printf("Connected with client %d\n", client_id);

        if (fork() == 0) {
            // close(server_socket);
            handle_client_request(client_socket, client_id);
            // close(client_socket);
            exit(0);
        }
        printf("client socket closed after fork \n");
        close(client_socket); 
    }
}


void start_server_muxbasic(int server_socket) {
    printf("Server running in 'muxbasic' mode using select()...\n");
    fd_set master_set, read_set;
    FD_ZERO(&master_set);
    FD_SET(server_socket, &master_set);
    int max_sd = server_socket;
    int client_id = 0;

    while (1) {
        read_set = master_set;
        if (select(max_sd + 1, &read_set, NULL, NULL, NULL) < 0) {
            perror("select");
            continue;
        }

        for (int fd = 0; fd <= max_sd; fd++) {
            if (FD_ISSET(fd, &read_set)) {
                if (fd == server_socket) {
                    int client_socket = accept(server_socket, NULL, NULL);
                    if (client_socket < 0) {
                        perror("accept");
                        continue;
                    }
                    client_id++;
                    printf("Connected with client %d\n", client_id);
                    FD_SET(client_socket, &master_set);
                    if (client_socket > max_sd) {
                        max_sd = client_socket;
                    }
                } else {
                    handle_client_request(fd, client_id);
                    FD_CLR(fd, &master_set);
                }
            }
        }
    }
}



void start_server_muxscale(int server_socket) {
    printf("Server running in 'muxscale' mode using epoll()...\n");
    int epoll_fd = epoll_create(EPOLL_SIZE);
    struct epoll_event ev, events[EPOLL_SIZE];
    ev.events = EPOLLIN;
    ev.data.fd = server_socket;
    epoll_ctl(epoll_fd, EPOLL_CTL_ADD, server_socket, &ev);
    int client_id = 0;

    while (1) {
        int nfds = epoll_wait(epoll_fd, events, EPOLL_SIZE, -1);
        for (int i = 0; i < nfds; i++) {
            if (events[i].data.fd == server_socket) {
                int client_socket = accept(server_socket, NULL, NULL);
                if (client_socket < 0) {
                    perror("accept");
                    continue;
                }
                client_id++;
                printf("Connected with client %d\n", client_id);
                ev.events = EPOLLIN;
                ev.data.fd = client_socket;
                epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_socket, &ev);
            } else {
                handle_client_request(events[i].data.fd, client_id);
                epoll_ctl(epoll_fd, EPOLL_CTL_DEL, events[i].data.fd, &ev);
                close(events[i].data.fd);
            }
        }
    }
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("Usage: %s <port> <mode>\n", argv[0]);
        printf("Modes: fork, muxbasic, muxscale\n");
        return 1;
    }

    int port = atoi(argv[1]);
    char* mode = argv[2];

    int server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0) {
        perror("socket");
        return 1;
    }

    int opt = 1;
    setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);

    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("bind");
        return 1;
    }

    if (listen(server_socket, 5) < 0) {
        perror("listen");
        return 1;
    }

    printf("Server listening on port %d...\n", port);

    if (strcmp(mode, "fork") == 0) {
        start_server_fork(server_socket);
    } else if (strcmp(mode, "muxbasic") == 0) {
        start_server_muxbasic(server_socket);
    } else if (strcmp(mode, "muxscale") == 0) {
        start_server_muxscale(server_socket);
    } else {
        printf("Invalid mode specified.\n");
        return 1;
    }

    printf("server socket closed main \n");
    close(server_socket);
    return 0;
}