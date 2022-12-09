#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>

// server address
#define ADDRESS "0.0.0.0"

// port number
#define PORT 8888

// buffer size 1KB
#define BUFFER_SIZE 1024

char *GetFilename(const char*);
int GetLine(int, char*);

int main(int argc, char *argv[]) {
	char buffer[BUFFER_SIZE] = {0};
	int master_socket, conn_id, len;
	struct sockaddr_in server, client;
	
	memset(&server, 0, sizeof(server));
	memset(&client, 0, sizeof(client));
	
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
    
    if ((conn_id = accept(master_socket, (struct sockaddr*)&client, (socklen_t*)&len)) == -1) {
    	printf("[WARNING] CAN'T ACCEPT NEW CONNECTION\n");
    	exit(1);
    } else {
    	printf("[INFO] NEW CONNECTION ACCEPTED\n");//", inet_ntoa(client.sin.addr), ntohs(client.sin.port));
    }
    
    char MetaData[1024] = {0};
    char filename[1024] = {0};
    
    GetLine(conn_id, MetaData);
    strcpy(filename, GetFilename(MetaData));
    
    if (filename[0] == '\0') {
    	printf("No file name header found\n");
        exit(1);
    }
    
    int fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, 644);
    
	// byte size
	ssize_t byte = 0;
	
	// read response continue
	while ((byte = recv(conn_id, buffer, BUFFER_SIZE, 0)) > 0) {
		if (write(fd, buffer, byte) != byte) {
			printf("Failed to write\n");
			exit(1);
		}
	}
	
	close(fd);
	
	// terminate connection
	close(conn_id);
	
    exit(0);
}

char *GetFilename(const char *str) {
	static char filename[1024] = {0};
	int FileIndex = 0;
	
	for (int index = 0; index < strlen(str); index++) {
		if (filename[0] == '\0') {
			if (str[index-8] == 'f'  && str[index-7] == 'i'  && str[index-6] == 'l'  && str[index-5] == 'e'  && str[index-4] == 'n' && str[index-3] == 'a' && str[index-2] == 'm'  && str[index-1] == 'e' && str[index] == '=') {
				filename[0] = 's';
			}
		} else {
			if ((str[index] == '\r' && str[index +1] == '\n') || (str[index] != '\r' && str[index] == '\n')) {
				break;
			} else {
				filename[FileIndex++] = str[index];
			}
		}
		
	}
	return filename;
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
