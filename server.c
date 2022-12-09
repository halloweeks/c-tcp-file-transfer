#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>

// server address
#define ADDRESS "0.0.0.0"

// port number
#define PORT 8888

// buffer size 1KB
#define BUFFER_SIZE 1024

char *GetFilename(const char*);
int GetLine(int, char*);
void signal_callback_handler(int);

int main(int argc, char *argv[]) {
	char buffer[BUFFER_SIZE] = {0};
	char filename[BUFFER_SIZE] = {0};
    ssize_t byte = 0;
	int fd;
	int master_socket, conn_id, len;
	struct sockaddr_in server, client;
	
	memset(&server, 0, sizeof(server));
	memset(&client, 0, sizeof(client));
	
	/* Catch Signal Handler SIGPIPE */
	signal(SIGPIPE, signal_callback_handler);
	
	// creating master socket
	if ((master_socket = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		printf("[ERROR] CAN'T CREATE TO SOCKET\n");
	    exit(1);
	} else {
		printf("[NOTE] SOCKET CREATED DONE\n");
	}
	
     // Prepare the sockaddr_in structure
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = inet_addr(ADDRESS);
	server.sin_port = htons(PORT);
    
	len = sizeof(struct sockaddr_in);
    
	// binding address and port
	if (bind(master_socket, (struct sockaddr *)&server , sizeof(server)) == -1) {
		printf("[ERROR][BIND] %s\n", strerror(errno));
	    exit(1);
	} else {
		printf("[NOTE] BIND %s:%d\n", ADDRESS, PORT);
	}
	
	// Listen on the socket, with 3 max connection requests queued
	if (listen(master_socket, 3) == -1) {
		printf("[ERROR][LISTEN] %s\n", strerror(errno));
		exit(1);
	} else {
		printf("[INFO] WAITING FOR INCOMING CONNECTIONS\n");
	}
	
	while (1) {
		if ((conn_id = accept(master_socket, (struct sockaddr*)&client, (socklen_t*)&len)) == -1) {
        	printf("[WARNING] CAN'T ACCEPT NEW CONNECTION\n");
            exit(1);
        } else {
        	printf("[INFO] NEW CONNECTION ACCEPTED FROM %s:%hu\n", inet_ntoa(client.sin_addr), ntohs(client.sin_port));
        }
        
        memset(filename, 0, 1024);
        GetLine(conn_id, filename);
        
        fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, 644);
        
        if (fd == -1) {
        	printf("[WARNING] FAILED TO CREATE FILE %s\n", strerror(errno));
           close(conn_id);
        }
       
       // read response continue
       while ((byte = recv(conn_id, buffer, BUFFER_SIZE, 0)) > 0) {
       	if (write(fd, buffer, byte) != byte) {
       	    printf("[WARNING] FAILED TO WRITE\n");
           }
       }
       
       close(fd);
	   printf("[INFO] CONNECTION CLOSED %s:%hu\n", inet_ntoa(client.sin_addr), ntohs(client.sin_port));
	
	// terminate connection
	close(conn_id);
	
     }
	
    exit(0);
}

int GetLine(int fd, char* buffer) {
	char ch;
	char Data[1024] = {0};
	int index = 0;
	
	while ((read(fd, &ch, 1) > 0) && ch != '\n') {
		if (index != 1024) {
			Data[index++] = ch;
		}
	}
	
	strcpy(buffer, Data);
	return index;
}

/* Catch Signal Handler functio */
void signal_callback_handler(int signum) {
        printf("Caught signal SIGPIPE %d\n",signum);
}
