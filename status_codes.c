#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>

void send_404_response(int socket_fd) {
        char *msg = "404 Not Found\n";
        send(socket_fd, (void*)msg, strlen(msg), 0);
}
