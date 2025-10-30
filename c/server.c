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
#include "log.h"

#define DEFAULT_PORT 5000
#define BACKLOG 16

static const uint8_t DEFAULT_KEY[] = "MY_SECRET_KEY";

typedef struct {
	int client_fd;
	struct sockaddr_in addr;
} client_ctx_t;

static void *client_thread(void *arg) {
	client_ctx_t *ctx = (client_ctx_t *)arg;
	char ipstr[INET_ADDRSTRLEN];
	inet_ntop(AF_INET, &ctx->addr.sin_addr, ipstr, sizeof(ipstr));
	log_info("Client connected: %s:%d", ipstr, ntohs(ctx->addr.sin_port));

	for (;;) {
		uint8_t *enc_payload = NULL;
		uint32_t enc_len = 0;
		int rc = recv_with_length(ctx->client_fd, &enc_payload, &enc_len);
		if (rc == 1) {
			log_info("Client disconnected: %s:%d", ipstr, ntohs(ctx->addr.sin_port));
			break;
		} else if (rc == -1) {
			log_err("recv_with_length failed");
			break;
		}
		if (enc_len == 0) {
			free(enc_payload);
			continue;
		}

		// Decrypt in place
		if (xor_crypt(enc_payload, enc_len, DEFAULT_KEY, sizeof(DEFAULT_KEY) - 1) != 0) {
			log_err("Decrypt failed");
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
		log_recv("%s", msg);

		// Prepare ACK plaintext
		char ack_buf[256];
		int n = snprintf(ack_buf, sizeof(ack_buf), "ACK: Received %u bytes", enc_len);
		if (n < 0) n = 0;
		uint32_t ack_len = (uint32_t) (n < 0 ? 0 : n);

		uint8_t *ack_enc = (uint8_t *)malloc(ack_len);
		if (!ack_enc) {
			free(enc_payload);
			free(msg);
			break;
		}
		memcpy(ack_enc, ack_buf, ack_len);
		if (xor_crypt(ack_enc, ack_len, DEFAULT_KEY, sizeof(DEFAULT_KEY) - 1) != 0) {
			log_err("Encrypt failed");
			free(enc_payload);
			free(msg);
			free(ack_enc);
			break;
		}
		if (send_with_length(ctx->client_fd, ack_enc, ack_len) != 0) {
			log_err("send_with_length failed");
			free(enc_payload);
			free(msg);
			free(ack_enc);
			break;
		}
		log_send("ACK sent to client");

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

	log_info("Listening on 0.0.0.0:%d", port);

	while (keep_running) {
		struct sockaddr_in cliaddr;
		socklen_t clilen = sizeof(cliaddr);
		int cfd = accept(server_fd, (struct sockaddr *)&cliaddr, &clilen);
		if (cfd < 0) {
			if (errno == EINTR) continue;
			log_err("accept failed");
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
			log_err("pthread_create failed");
			close(cfd);
			free(ctx);
			continue;
		}
		pthread_detach(tid);
	}

	if (server_fd >= 0) close(server_fd);
	log_info("Server shutting down.");
	return 0;
}


