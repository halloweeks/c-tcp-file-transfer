#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>

#define ADDRESS "127.0.0.1"
#define PORT 8888
#define BUFFER_SIZE 1024

int main(int argc, char const *argv[]) {
	if (argc != 2) {
		printf("Usage %s filename.txt\n", argv[0]);
		exit(1);
	}
	
	char chunk[BUFFER_SIZE] = {0};
	char filename[1024] = {0};
	
	int socket_fd = 0;
	ssize_t len = 0;
	
	struct sockaddr_in serv_addr;
	
	memset(&serv_addr, 0, sizeof(serv_addr));
		
	if ((socket_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		printf("[ERROR] CAN'T CREATE SOCKET\n");
		exit(1);
	}
	
	// assign IP, PORT
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(PORT);
	
	// Convert IPv4 and IPv6 addresses from text to binary form
	if (inet_pton(AF_INET, ADDRESS, &serv_addr.sin_addr) == -1) {
		printf("[ERROR] INVALID ADDRESS/ ADDRESS NOT SUPPORTED\n");
		exit(1);
	}
	
	// connect
	if (connect(socket_fd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) == -1) {
		printf("[ERROR] CAN'T CONNECT TO THE HOST %s:%d\n", ADDRESS, PORT);
		exit(1);
	}
	
	int fd = open(argv[1], O_RDONLY);
	
	if (fd == -1) {
		printf("Failed to open file %s\n", argv[1]);
		exit(1);
	}
	
	strcpy(filename, argv[1]);
	strcat(filename, "\n");
	
	if (send(socket_fd, filename, strlen(filename), 0) > 0) {
		printf("[INFO] Starting to transfer file %s\n", argv[1]);
	} else {
		printf("Failed to transfer file %s\n", argv[1]);
		exit(1);
	}
	
	while ((len = read(fd, chunk, BUFFER_SIZE)) > 0) {
		if (send(socket_fd, chunk, len, 0) != len) {
			printf("Failed to transfer file %s\n", argv[1]);
			exit(1);
		}
	}
	
	
	if (len == -1) {
		printf("Failed to read file %s\n", argv[1]);
		exit(1);
	} else if (len == 0) {
		printf("File transfer done\n");
	}
	
	// close file
	close(fd);
	
	// close connection
	close(socket_fd);
	return 0; 
}
