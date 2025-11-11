#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include "crypto.h"
#include "net.h"

static const uint8_t DEFAULT_KEY[] = "MY_SECRET_KEY";

static int connect_to_server(const char *host, int port) {
	int fd = socket(AF_INET, SOCK_STREAM, 0);
	if (fd < 0) {
		perror("socket");
		return -1;
	}
	struct sockaddr_in addr;
	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons((uint16_t)port);
	if (inet_pton(AF_INET, host, &addr.sin_addr) <= 0) {
		perror("inet_pton");
		close(fd);
		return -1;
	}
	if (connect(fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
		fprintf(stderr, "[ERROR] connect failed\n");
		close(fd);
		return -1;
	}
	return fd;
}

static int send_message_and_get_ack(int fd, const char *message) {
	size_t len = strlen(message);
	uint8_t *payload = (uint8_t *)malloc(len);
	if (!payload) return -1;
	memcpy(payload, message, len);
	if (xor_crypt(payload, len, DEFAULT_KEY, sizeof(DEFAULT_KEY) - 1) != 0) {
		free(payload);
		return -1;
	}
	if (send_with_length(fd, payload, (uint32_t)len) != 0) {
		fprintf(stderr, "[ERROR] send_with_length failed\n");
		free(payload);
		return -1;
	}
	printf("[SEND] Encrypted bytes: %zu\n", len);
	free(payload);

	uint8_t *ack_enc = NULL;
	uint32_t ack_len = 0;
	int rc = recv_with_length(fd, &ack_enc, &ack_len);
	if (rc != 0) {
		if (rc == 1) printf("[INFO] Disconnected before ACK.\n");
		else fprintf(stderr, "[ERROR] recv_with_length failed\n");
		return -1;
	}
	if (ack_len > 0 && xor_crypt(ack_enc, ack_len, DEFAULT_KEY, sizeof(DEFAULT_KEY) - 1) == 0) {
		char *ack = (char *)malloc(ack_len + 1);
		if (ack) {
			memcpy(ack, ack_enc, ack_len);
			ack[ack_len] = '\0';
			printf("[RECV] %s\n", ack);
			free(ack);
		}
	}
	free(ack_enc);
	return 0;
}

int main(int argc, char **argv) {
	if (argc < 3) {
		fprintf(stderr, "Usage: %s <server-ip> <port> [message]\n", argv[0]);
		return 1;
	}
	const char *host = argv[1];
	int port = atoi(argv[2]);
	if (port <= 0 || port > 65535) {
		fprintf(stderr, "Invalid port: %s\n", argv[2]);
		return 1;
	}

	int fd = connect_to_server(host, port);
	if (fd < 0) return 1;

	if (argc >= 4) {
		// Single-shot send using argument
		if (send_message_and_get_ack(fd, argv[3]) != 0) {
			close(fd);
			return 1;
		}
	} else {
		// Interactive mode
		char line[1024];
		printf("Enter messages (empty line to quit):\n");
		for (;;) {
			printf("> ");
			fflush(stdout);
			if (!fgets(line, sizeof(line), stdin)) break;
			size_t L = strlen(line);
			while (L > 0 && (line[L - 1] == '\n' || line[L - 1] == '\r')) {
				line[--L] = '\0';
			}
			if (L == 0) break;
			if (send_message_and_get_ack(fd, line) != 0) break;
		}
	}

	close(fd);
	return 0;
}


