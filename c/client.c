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

// ANSI color codes
#define COLOR_RESET   "\033[0m"
#define COLOR_GREEN   "\033[32m"
#define COLOR_BLUE    "\033[34m"
#define COLOR_CYAN    "\033[36m"
#define COLOR_YELLOW  "\033[33m"
#define COLOR_BOLD    "\033[1m"

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
		fprintf(stderr, COLOR_BOLD "[✗ ERROR] " COLOR_RESET "send_with_length failed\n");
		free(payload);
		return -1;
	}
	printf(COLOR_CYAN COLOR_BOLD "┌─ MESSAGE SENT ─────────────────────────────────────┐\n" COLOR_RESET);
	printf(COLOR_CYAN "│ " COLOR_RESET COLOR_BOLD "Message:" COLOR_RESET " %-45s " COLOR_CYAN "│\n" COLOR_RESET, message);
	printf(COLOR_CYAN "│ " COLOR_RESET COLOR_BOLD "Size:" COLOR_RESET " %-47zu bytes " COLOR_CYAN "│\n" COLOR_RESET, len);
	printf(COLOR_CYAN "│ " COLOR_RESET COLOR_BOLD "Status:" COLOR_RESET COLOR_GREEN " ✓ Encrypted & Sent" COLOR_RESET "                    " COLOR_CYAN "│\n" COLOR_RESET);
	printf(COLOR_CYAN "└──────────────────────────────────────────────────────────┘\n" COLOR_RESET);
	free(payload);

	uint8_t *ack_enc = NULL;
	uint32_t ack_len = 0;
	int rc = recv_with_length(fd, &ack_enc, &ack_len);
	if (rc != 0) {
		if (rc == 1) printf(COLOR_YELLOW "[ℹ INFO] " COLOR_RESET "Disconnected before ACK.\n");
		else fprintf(stderr, COLOR_BOLD "[✗ ERROR] " COLOR_RESET "recv_with_length failed\n");
		return -1;
	}
	if (ack_len > 0 && xor_crypt(ack_enc, ack_len, DEFAULT_KEY, sizeof(DEFAULT_KEY) - 1) == 0) {
		char *ack = (char *)malloc(ack_len + 1);
		if (ack) {
			memcpy(ack, ack_enc, ack_len);
			ack[ack_len] = '\0';
			printf(COLOR_GREEN COLOR_BOLD "┌─ ACKNOWLEDGMENT RECEIVED ────────────────────────────┐\n" COLOR_RESET);
			printf(COLOR_GREEN "│ " COLOR_RESET COLOR_BOLD "Response:" COLOR_RESET " %-42s " COLOR_GREEN "│\n" COLOR_RESET, ack);
			printf(COLOR_GREEN "│ " COLOR_RESET COLOR_BOLD "Status:" COLOR_RESET COLOR_GREEN " ✓ Decrypted Successfully" COLOR_RESET "              " COLOR_GREEN "│\n" COLOR_RESET);
			printf(COLOR_GREEN "└──────────────────────────────────────────────────────────┘\n" COLOR_RESET);
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
		printf(COLOR_BLUE COLOR_BOLD "\n╔══════════════════════════════════════════════════════════╗\n" COLOR_RESET);
		printf(COLOR_BLUE COLOR_BOLD "║" COLOR_RESET COLOR_CYAN COLOR_BOLD "          SECURE CLIENT - INTERACTIVE MODE" COLOR_RESET COLOR_BLUE COLOR_BOLD "          ║\n" COLOR_RESET);
		printf(COLOR_BLUE COLOR_BOLD "╚══════════════════════════════════════════════════════════╝\n\n" COLOR_RESET);
		printf(COLOR_YELLOW "Enter messages (empty line to quit):\n" COLOR_RESET);
		for (;;) {
			printf(COLOR_CYAN COLOR_BOLD "→ " COLOR_RESET);
			fflush(stdout);
			if (!fgets(line, sizeof(line), stdin)) break;
			size_t L = strlen(line);
			while (L > 0 && (line[L - 1] == '\n' || line[L - 1] == '\r')) {
				line[--L] = '\0';
			}
			if (L == 0) break;
			printf("\n");
			if (send_message_and_get_ack(fd, line) != 0) break;
			printf("\n");
		}
	}

	close(fd);
	return 0;
}


