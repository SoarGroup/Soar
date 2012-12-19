#include <stdlib.h>
#include <stdio.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <errno.h>
#include <unistd.h>

#include "viewer.h"

int get_stdin(char *buf, int n);
int get_domain_socket(char *buf, int n);
int init_domain_socket(char *path);
int init_file(char *path);

enum Input_type { REGULAR_FILE, FILE_SOCKET };

static enum Input_type input_type;
static int socket_fd = -1;
static FILE *file = NULL;

int init_input(int argc, char *argv[]) {
	int i;
	
	for (i = 1; i < argc; ++i) {
		if (strcmp(argv[i], "-u") == 0) {
			if (i + 1 >= argc) {
				fprintf(stderr, "specify a socket path\n");
				return 0;
			}
			input_type = FILE_SOCKET;
			return init_domain_socket(argv[i + 1]);
		} else if (strcmp(argv[i], "-f") == 0) {
			if (i + 1 >= argc) {
				fprintf(stderr, "specify file path\n");
				return 0;
			}
			input_type = REGULAR_FILE;
			return init_file(argv[i + 1]);
		}
	}
	file = stdin;
	input_type = REGULAR_FILE;
	return 1;
}

int get_input(char *buf, int n) {
	switch (input_type) {
		case REGULAR_FILE:
			return get_file(buf, n);
		case FILE_SOCKET:
			return get_domain_socket(buf, n);
	}
	return 0;
}

int get_file(char *buf, int n) {
	if (!fgets(buf, n, file)) {
		if (feof(file)) {
			fprintf(stderr, "EOF\n");
		} else if (ferror(file)) {
			perror("get_input: ");
		}
		return -1;
	}
	return strlen(buf);
}

int init_domain_socket(char *path) {
	struct sockaddr_un addr;
	
	if ((socket_fd = socket(AF_UNIX, SOCK_DGRAM, 0)) == -1) {
		perror("socket: ");
		return 0;
	}
	
	memset(&addr, 0, sizeof(addr));
	addr.sun_family = AF_UNIX;
	strcpy(addr.sun_path, path);
	unlink(addr.sun_path);
	
	if (bind(socket_fd, (struct sockaddr *) &addr, sizeof(addr)) == -1) {
		close(socket_fd);
		perror("socket: ");
		return 0;
	}
	return 1;
}

int get_domain_socket(char *buf, int n) {
	return recv(socket_fd, buf, n, 0);
}

int init_file(char *path) {
	if ((file = fopen(path, "r")) == NULL)
		return 0;
	return 1;
}
