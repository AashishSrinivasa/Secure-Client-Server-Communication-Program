#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <pthread.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include "crypto.h"
#include "net.h"

#define DEFAULT_PORT 5000
#define BACKLOG 16

static const uint8_t DEFAULT_KEY[] = "MY_SECRET_KEY";

typedef struct {
	int client_fd;
	struct sockaddr_in addr;
} client_ctx_t;

// ANSI color codes
#define COLOR_RESET   "\033[0m"
#define COLOR_GREEN   "\033[32m"
#define COLOR_BLUE    "\033[34m"
#define COLOR_CYAN    "\033[36m"
#define COLOR_YELLOW  "\033[33m"
#define COLOR_MAGENTA "\033[35m"
#define COLOR_BOLD    "\033[1m"

static void *client_thread(void *arg) {
	client_ctx_t *ctx = (client_ctx_t *)arg;
	char ipstr[INET_ADDRSTRLEN];
	inet_ntop(AF_INET, &ctx->addr.sin_addr, ipstr, sizeof(ipstr));
	printf(COLOR_GREEN COLOR_BOLD "\n╔══════════════════════════════════════════════════════════╗\n" COLOR_RESET);
	printf(COLOR_GREEN COLOR_BOLD "║" COLOR_RESET COLOR_CYAN COLOR_BOLD "              NEW CLIENT CONNECTION" COLOR_RESET COLOR_GREEN COLOR_BOLD "                 ║\n" COLOR_RESET);
	printf(COLOR_GREEN COLOR_BOLD "╠══════════════════════════════════════════════════════════╣\n" COLOR_RESET);
	printf(COLOR_GREEN COLOR_BOLD "║" COLOR_RESET " " COLOR_BOLD "Client:" COLOR_RESET " %-50s " COLOR_GREEN COLOR_BOLD "║\n" COLOR_RESET, ipstr);
	printf(COLOR_GREEN COLOR_BOLD "║" COLOR_RESET " " COLOR_BOLD "Port:" COLOR_RESET " %-52d " COLOR_GREEN COLOR_BOLD "║\n" COLOR_RESET, ntohs(ctx->addr.sin_port));
	printf(COLOR_GREEN COLOR_BOLD "╚══════════════════════════════════════════════════════════╝\n" COLOR_RESET);
	fflush(stdout);

	for (;;) {
		uint8_t *enc_payload = NULL;
		uint32_t enc_len = 0;
		int rc = recv_with_length(ctx->client_fd, &enc_payload, &enc_len);
		if (rc == 1) {
			printf(COLOR_YELLOW COLOR_BOLD "\n╔══════════════════════════════════════════════════════════╗\n" COLOR_RESET);
			printf(COLOR_YELLOW COLOR_BOLD "║" COLOR_RESET " " COLOR_BOLD "Client Disconnected:" COLOR_RESET " %-37s " COLOR_YELLOW COLOR_BOLD "║\n" COLOR_RESET, ipstr);
			printf(COLOR_YELLOW COLOR_BOLD "╚══════════════════════════════════════════════════════════╝\n" COLOR_RESET);
			break;
		} else if (rc == -1) {
			fprintf(stderr, COLOR_BOLD "[✗ ERROR] " COLOR_RESET "recv_with_length failed\n");
			break;
		}
		if (enc_len == 0) {
			free(enc_payload);
			continue;
		}

		// Decrypt in place
		if (xor_crypt(enc_payload, enc_len, DEFAULT_KEY, sizeof(DEFAULT_KEY) - 1) != 0) {
			fprintf(stderr, COLOR_BOLD "[✗ ERROR] " COLOR_RESET "Decrypt failed\n");
			free(enc_payload);
			break;
		}

		// Ensure message is printable string for display
		char *msg = (char *)malloc(enc_len + 1);
		if (!msg) {
			free(enc_payload);
			break;
		}
		memcpy(msg, enc_payload, enc_len);
		msg[enc_len] = '\0';
		printf(COLOR_BLUE COLOR_BOLD "┌─ MESSAGE RECEIVED ────────────────────────────────────┐\n" COLOR_RESET);
		printf(COLOR_BLUE "│ " COLOR_RESET COLOR_BOLD "From:" COLOR_RESET " %-47s " COLOR_BLUE "│\n" COLOR_RESET, ipstr);
		printf(COLOR_BLUE "│ " COLOR_RESET COLOR_BOLD "Size:" COLOR_RESET " %-47u bytes " COLOR_BLUE "│\n" COLOR_RESET, enc_len);
		printf(COLOR_BLUE "│ " COLOR_RESET COLOR_BOLD "Status:" COLOR_RESET COLOR_GREEN " ✓ Decrypted Successfully" COLOR_RESET "            " COLOR_BLUE "│\n" COLOR_RESET);
		printf(COLOR_BLUE "├──────────────────────────────────────────────────────────┤\n" COLOR_RESET);
		printf(COLOR_BLUE "│ " COLOR_RESET COLOR_BOLD "Message:" COLOR_RESET " %-45s " COLOR_BLUE "│\n" COLOR_RESET, msg);
		printf(COLOR_BLUE "└──────────────────────────────────────────────────────────┘\n" COLOR_RESET);
		fflush(stdout);

		// Prepare ACK plaintext
		char ack_buf[256];
		int n = snprintf(ack_buf, sizeof(ack_buf), "ACK: Received %u bytes", enc_len);
		if (n < 0 || n >= (int)sizeof(ack_buf)) n = 0;
		uint32_t ack_len = (uint32_t)n;

		uint8_t *ack_enc = (uint8_t *)malloc(ack_len);
		if (!ack_enc) {
			free(enc_payload);
			free(msg);
			break;
		}
		memcpy(ack_enc, ack_buf, ack_len);
		if (xor_crypt(ack_enc, ack_len, DEFAULT_KEY, sizeof(DEFAULT_KEY) - 1) != 0) {
			fprintf(stderr, "[ERROR] Encrypt failed\n");
			free(enc_payload);
			free(msg);
			free(ack_enc);
			break;
		}
		if (send_with_length(ctx->client_fd, ack_enc, ack_len) != 0) {
			fprintf(stderr, COLOR_BOLD "[✗ ERROR] " COLOR_RESET "send_with_length failed\n");
			free(enc_payload);
			free(msg);
			free(ack_enc);
			break;
		}
		printf(COLOR_MAGENTA COLOR_BOLD "┌─ ACKNOWLEDGMENT SENT ──────────────────────────────────┐\n" COLOR_RESET);
		printf(COLOR_MAGENTA "│ " COLOR_RESET COLOR_BOLD "Response:" COLOR_RESET " %-42s " COLOR_MAGENTA "│\n" COLOR_RESET, ack_buf);
		printf(COLOR_MAGENTA "│ " COLOR_RESET COLOR_BOLD "Status:" COLOR_RESET COLOR_GREEN " ✓ Encrypted & Sent" COLOR_RESET "                    " COLOR_MAGENTA "│\n" COLOR_RESET);
		printf(COLOR_MAGENTA "└──────────────────────────────────────────────────────────┘\n" COLOR_RESET);
		fflush(stdout);

		free(enc_payload);
		free(msg);
		free(ack_enc);
	}

	close(ctx->client_fd);
	free(ctx);
	return NULL;
}

static volatile sig_atomic_t keep_running = 1;
static int g_server_fd = -1;
static void handle_sigint(int sig) {
	(void)sig;
	keep_running = 0;
	if (g_server_fd >= 0) {
		close(g_server_fd); // force accept() to unblock
	}
}

int main(int argc, char **argv) {
	int port = DEFAULT_PORT;
	if (argc >= 2) {
		port = atoi(argv[1]);
		if (port <= 0 || port > 65535) {
			fprintf(stderr, "Invalid port: %s\n", argv[1]);
			return 1;
		}
	}

	signal(SIGINT, handle_sigint);

	int server_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (server_fd < 0) {
		perror("socket");
		return 1;
	}
	g_server_fd = server_fd;

	int opt = 1;
	setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

	struct sockaddr_in servaddr;
	memset(&servaddr, 0, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port = htons((uint16_t)port);

	if (bind(server_fd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
		perror("bind");
		close(server_fd);
		return 1;
	}
	if (listen(server_fd, BACKLOG) < 0) {
		perror("listen");
		close(server_fd);
		return 1;
	}

	printf(COLOR_CYAN COLOR_BOLD "\n╔══════════════════════════════════════════════════════════╗\n" COLOR_RESET);
	printf(COLOR_CYAN COLOR_BOLD "║" COLOR_RESET COLOR_BOLD "              SECURE SERVER STARTED" COLOR_RESET COLOR_CYAN COLOR_BOLD "                  ║\n" COLOR_RESET);
	printf(COLOR_CYAN COLOR_BOLD "╠══════════════════════════════════════════════════════════╣\n" COLOR_RESET);
	printf(COLOR_CYAN COLOR_BOLD "║" COLOR_RESET " " COLOR_BOLD "Status:" COLOR_RESET COLOR_GREEN " ✓ Listening" COLOR_RESET "                                      " COLOR_CYAN COLOR_BOLD "║\n" COLOR_RESET);
	printf(COLOR_CYAN COLOR_BOLD "║" COLOR_RESET " " COLOR_BOLD "Address:" COLOR_RESET " 0.0.0.0:%-43d " COLOR_CYAN COLOR_BOLD "║\n" COLOR_RESET, port);
	printf(COLOR_CYAN COLOR_BOLD "║" COLOR_RESET " " COLOR_BOLD "Protocol:" COLOR_RESET " TCP with XOR Encryption                      " COLOR_CYAN COLOR_BOLD "║\n" COLOR_RESET);
	printf(COLOR_CYAN COLOR_BOLD "╚══════════════════════════════════════════════════════════╝\n" COLOR_RESET);
	printf(COLOR_YELLOW "\nWaiting for clients...\n" COLOR_RESET);
	fflush(stdout);

	while (keep_running) {
		struct sockaddr_in cliaddr;
		socklen_t clilen = sizeof(cliaddr);
		int cfd = accept(server_fd, (struct sockaddr *)&cliaddr, &clilen);
		if (cfd < 0) {
			if (errno == EINTR) continue;
			fprintf(stderr, "[ERROR] accept failed\n");
			break;
		}
		client_ctx_t *ctx = (client_ctx_t *)malloc(sizeof(client_ctx_t));
		if (!ctx) {
			close(cfd);
			continue;
		}
		ctx->client_fd = cfd;
		ctx->addr = cliaddr;

		pthread_t tid;
		if (pthread_create(&tid, NULL, client_thread, ctx) != 0) {
			fprintf(stderr, "[ERROR] pthread_create failed\n");
			close(cfd);
			free(ctx);
			continue;
		}
		pthread_detach(tid);
	}

	if (server_fd >= 0) close(server_fd);
	printf(COLOR_YELLOW COLOR_BOLD "\n╔══════════════════════════════════════════════════════════╗\n" COLOR_RESET);
	printf(COLOR_YELLOW COLOR_BOLD "║" COLOR_RESET " " COLOR_BOLD "Server shutting down..." COLOR_RESET "                              " COLOR_YELLOW COLOR_BOLD "║\n" COLOR_RESET);
	printf(COLOR_YELLOW COLOR_BOLD "╚══════════════════════════════════════════════════════════╝\n" COLOR_RESET);
	return 0;
}


