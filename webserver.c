#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/sendfile.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include "status_codes.h"

int main(int argc, char **argv) {

	bool server_running = true;
	int backlog = 5;
	int port = 8080;

	// first argument is optional
	if (argc > 1) {
		port = atoi(argv[1]);
	}	

	int sfd, cfd;

	// info struct
	struct sockaddr_in sockaddr_info = {0};
	sockaddr_info.sin_family = AF_INET;
	sockaddr_info.sin_port = htons(port);
	socklen_t sockaddr_len = sizeof(sockaddr_info);

	// create fd -> bind
	sfd = socket(AF_INET, SOCK_STREAM, 0);
	if ( 0 > bind(sfd, (struct sockaddr*)&sockaddr_info, sockaddr_len) ){
		perror("Bind Failed");
		return -1;
	}
	
	printf("listening for requests...\n");
	// listen for incoming connections
	while(server_running) {
		listen(sfd, backlog);
		cfd = accept(sfd, (struct sockaddr*)&sockaddr_info, &sockaddr_len);
		printf("\n====================\n");
		if ( 0 > cfd) {
			perror("Receive Error");
			return -1;
		}

		// get request from user
		char req_buff[256] = {0};
		ssize_t recvd = recv(cfd, (void*)req_buff, sizeof(req_buff), 0);
		printf("%s", req_buff);

		
		// atm GET is the only supported header
		if (strncmp(req_buff, "GET ", 4) != 0) {
			send_404_response(cfd);
			close(cfd);
			continue;
		}	

		
		// GET /file.html ....
		char *uri_start = req_buff + 5;
		char *uri_end = strchr(uri_start, ' ');
		if (!uri_end) {
			send_404_response(cfd);
			close(cfd);
			continue;
		}
		*uri_end = '\0'; // null terminate empty space		

		// open file
		int opened_fd = open(uri_start, O_RDONLY);
		if ( 0 > opened_fd) { 
			send_404_response(cfd);
			close(cfd);
			printf("====================\n");
			continue;
		}

		// get file size of file to send
		struct stat stat_buff;
		stat(uri_start, &stat_buff);
		printf("Response File Size: %ldbytes\n", stat_buff.st_size);

		sendfile(cfd, opened_fd, 0, (ssize_t)stat_buff.st_size);
		close(opened_fd);
		close(cfd);

		printf("====================\n");
		
	}

	// clean up
	close(sfd);
		
	printf("Server has exited\n");
	return 0;
}
