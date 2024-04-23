#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#define CHUNK_SIZE 1024

int main(int argc, char **argv) {
	
	if (argc < 5) {
		printf("Usage <ip> <port> <uri> <filename>\n");
		return -1;
	}

	FILE *file;	

	// create info struct
	struct sockaddr_in sockaddr_info = {0};
	sockaddr_info.sin_family = AF_INET;
	sockaddr_info.sin_port = htons(atoi(argv[2]));

	// ensure valid ip is passed=>push to info struct
	if (inet_pton(AF_INET, argv[1], &sockaddr_info.sin_addr) <= 0) {
		perror("Invalid address");
		return -1;
	}

	socklen_t addrlen = sizeof(sockaddr_info);

	// create socket
	int sock_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (sock_fd < 0) {
		perror("Could not create socket");
		return -1;
	}

	// connect to server
	int connect_fd = connect(sock_fd, (struct sockaddr*)&sockaddr_info, addrlen);

	if (connect_fd < 0) {
		perror("Could not connect to server");
		return -1;
	}

	// create req header=>send
	char* request = (char*)malloc(strlen("GET ") + strlen(argv[3]) + 1);
	strcpy(request, "GET ");
	strcat(request, argv[3]);
	strcat(request, " ");
	printf("REQUEST: \n%s\n", request);
	
	send(sock_fd, request, strlen(request), 0);
	free(request);

	// recv response
	char *file_buff = malloc(CHUNK_SIZE);

	size_t total = 0;
	size_t current_size = CHUNK_SIZE;
	char chunk[CHUNK_SIZE];

	while (1) {
		ssize_t recvd = recv(sock_fd, chunk, CHUNK_SIZE, 0);
		if (recvd < 0) {
        	    perror("Failed to receive data");
	            free(file_buff);
        	    return -1;
        	}

		if (recvd == 0) {
			break;
		}

		if (total + recvd > current_size) {
			current_size = total + recvd;
			char* new_buffer = realloc(file_buff,current_size);
			if (!new_buffer) {
				perror("Failed to reallocate buffer");
				free(file_buff);
				return -1;
			}
			file_buff = new_buffer;
		}	

		memcpy(file_buff + total, chunk, recvd);
	        total += recvd;

	}
	
	file = fopen(argv[4], "r");
	if (fopen(argv[4], "r")) { 
		printf("File already exists, exiting...\n");
		fclose(file);
		return -1;
	}

	file = fopen(argv[4], "w");
	if (file == NULL) {
		perror("Failed to write to file");
		return -1;
	}

	if (fputs(file_buff, file) == EOF) {
		perror("Failed to write to file");
		fclose(file);
		return -1;
	}

	fclose(file);		
	printf("Data written succesfully to '%s'.\n", argv[4]);

	//printf("%s", file_buff);

	close(sock_fd);
	close(connect_fd);

	return 0;
}
