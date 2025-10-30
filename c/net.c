#include "net.h"

#include <arpa/inet.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

int send_all(int sockfd, const uint8_t *buffer, size_t length) {
	size_t total_sent = 0;
	while (total_sent < length) {
		ssize_t sent = send(sockfd, buffer + total_sent, length - total_sent, 0);
		if (sent < 0) {
			if (errno == EINTR) continue;
			return -1;
		}
		if (sent == 0) {
			return -1; // should not happen unless disconnected
		}
		total_sent += (size_t)sent;
	}
	return 0;
}

int recv_all(int sockfd, uint8_t *buffer, size_t length) {
	size_t total_read = 0;
	while (total_read < length) {
		ssize_t r = recv(sockfd, buffer + total_read, length - total_read, 0);
		if (r < 0) {
			if (errno == EINTR) continue;
			return -1;
		}
		if (r == 0) {
			return 1; // clean disconnect
		}
		total_read += (size_t)r;
	}
	return 0;
}

int send_with_length(int sockfd, const uint8_t *buffer, uint32_t length) {
	uint32_t net_len = htonl(length);
	if (send_all(sockfd, (const uint8_t *)&net_len, sizeof(net_len)) != 0) {
		return -1;
	}
	if (length == 0) return 0;
	return send_all(sockfd, buffer, length);
}

int recv_with_length(int sockfd, uint8_t **out_buffer, uint32_t *out_length) {
	uint32_t net_len = 0;
	int rc = recv_all(sockfd, (uint8_t *)&net_len, sizeof(net_len));
	if (rc != 0) return rc; // -1 error or 1 disconnect
	uint32_t length = ntohl(net_len);
	uint8_t *buf = NULL;
	if (length > 0) {
		buf = (uint8_t *)malloc(length);
		if (buf == NULL) return -1;
		rc = recv_all(sockfd, buf, length);
		if (rc != 0) {
			free(buf);
			return rc;
		}
	}
	*out_buffer = buf;
	*out_length = length;
	return 0;
}


