#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <syslog.h>
#include <netdb.h>
#include <signal.h>
#include <string.h>

#define BUFFER_SIZE 20000
#define SOCKET_PORT "9000"
#define ALLOWED_PENDING_CONNECTIONS 10
#define FILE_LOCATION "/var/tmp/aesdsocketdata"

int sockfd;
FILE * file_pointer = NULL;

int get_logfile_des(){
    if (access(FILE_LOCATION, F_OK) != -1){
        file_pointer = fopen(FILE_LOCATION, "a");
    }else{
        file_pointer = fopen(FILE_LOCATION, "w");
    }
    if(file_pointer == NULL){
        syslog(LOG_ERR, "could not open %s.", FILE_LOCATION);
        printf("could not open %s.\n", FILE_LOCATION);
        return 1;
    }
    printf("Opened log file descriptor\n");
    return 0;
}

int init_socket(){
    // Create a socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(sockfd == -1){
        syslog(LOG_ERR, "socket file descriptor could not be created.");
        printf("socket file descriptor could not be created.\n");
        return 1;
    }
    // create socket address info
    struct addrinfo * address_info;
    struct addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;        // Use IPv4
    hints.ai_socktype = SOCK_STREAM;  // Use stream socket
    hints.ai_flags = AI_PASSIVE;      // Use my IP
    int res = getaddrinfo(NULL, SOCKET_PORT, &hints, &address_info);
    if(res != 0){
        syslog(LOG_ERR, "socket adress info could not be created.");
        printf("socket adress info could not be created.\n");
        return 1;
    }
    // loop over linked list, try to bind
    for(struct addrinfo * p = address_info; p != NULL; p = p->ai_next){
        res = bind(sockfd, p->ai_addr, p->ai_addrlen);
        if (res == -1) {
            continue;
        }
        break;
    }
    freeaddrinfo(address_info);
    if(res != 0){
        syslog(LOG_ERR, "socket could not be bound.");
        printf("socket could not be bound: %i.\n", res);
        return 1;        
    }
    // start to listen
    res = listen(sockfd, ALLOWED_PENDING_CONNECTIONS);
    if(res == -1){
        syslog(LOG_ERR, "failed to listen.");
        printf("failed to listen.\n");
    }
    return 0;
}

void close_filedesc(){
    close(sockfd);
    fclose(file_pointer);
}

void init_syslog(){
    openlog(NULL, LOG_CONS | LOG_PID, LOG_USER);
}

// Signal handler function
void sig_handler(int signum) {
    syslog(LOG_INFO, "Caught signal, exiting\n");
    printf("Caught signal, exiting\n");
    close_filedesc();
    exit(0);
}


int setup(){
    int ret;
    init_syslog();
    ret = get_logfile_des();
    ret += init_socket();
    // Attach the signal handlers
    signal(SIGINT, sig_handler);
    signal(SIGTERM, sig_handler);
    signal(SIGABRT, sig_handler);
    return ret;
}

void loop(){
    int res;
    struct sockaddr_in client_addr;
    socklen_t client_addr_len = sizeof(client_addr);
    // receive stuff
    char buffer[BUFFER_SIZE];
    ssize_t received_bytes;
    char client_host[NI_MAXHOST];
    char client_service[NI_MAXSERV];
    while(1){
        // accept connections
        int client_fd = accept(sockfd, &client_addr, &client_addr_len);
        if(client_fd == -1){
            syslog(LOG_ERR, "failed to accept.");
            printf("failed to accept.\n");
            close(sockfd);      
        }else{
            res = getnameinfo((struct sockaddr*)&client_addr, client_addr_len, client_host, NI_MAXHOST, client_service, NI_MAXSERV, 0);
            if (res == 0) {
                syslog(LOG_INFO, "Accepted connection from %s:%s\n", client_host, client_service);
                printf("Accepted connection from %s:%s\n", client_host, client_service);
            }    
        }
        printf("start to receive\n");
        int count = 0;
        // receive
        received_bytes = recv(client_fd, buffer, BUFFER_SIZE - 1, 0);
        while(received_bytes > 0){
            buffer[received_bytes] = "\0";
            char *found_ptr = strchr(buffer, '\n');
            if (found_ptr != NULL) {
                buffer[found_ptr - buffer + 1] = '\0';
                buffer[found_ptr - buffer + 2] = '\0';
                printf("received package nr %i: %s", count, buffer);
                count++;
                fprintf(file_pointer, buffer);
                break;
            }
            printf("received: %s", buffer);
            fprintf(file_pointer, buffer);
            received_bytes = recv(client_fd, buffer, BUFFER_SIZE - 1, 0);
        }
        printf("start to send back\n");
        // send back
        fclose(file_pointer);
        file_pointer = fopen(FILE_LOCATION, "r");
        if(file_pointer == NULL){
            syslog(LOG_ERR, "failed to reopen file");
            printf("failed to reopen file\n");
            close(client_fd);
            continue;;
        }
        while (fgets(buffer, sizeof(buffer), file_pointer) != NULL) {
            printf("sending: %s", buffer);
            ssize_t bytes_sent = send(client_fd, buffer, strlen(buffer), 0);
            if (bytes_sent == -1) {
                syslog(LOG_ERR, "failed to send.");
                printf("failed to send.\n");
            }
        }
        // close connections
        close(client_fd);
        syslog(LOG_INFO, "Closed connection from %s:%s\n", client_host, client_service);
        printf("Closed connection from %s:%s\n", client_host, client_service);       
        fclose(file_pointer); 
        // reopen appending file descriptor
        get_logfile_des();
    }
}

int main(int argc, char * argv[]){
    if(setup() != 0){
        return 1;
    }
    if(argc == 1){
        printf("start single process\n");
        loop();
    }else if(argc == 2 && strcmp(argv[1], "-d") == 0){
        printf("start daemon process\n");
        int pid = fork();
        if(pid == 0){
            loop();
        }
    }
    

    return 0;
}

