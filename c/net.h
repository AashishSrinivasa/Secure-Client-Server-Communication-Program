#ifndef NET_H
#define NET_H

#include <stddef.h>
#include <stdint.h>

// Sends all bytes in buffer, handling partial writes.
// Returns 0 on success, -1 on failure.
int send_all(int sockfd, const uint8_t *buffer, size_t length);

// Receives exactly length bytes, unless EOF occurs. Returns 0 on success,
// -1 on error, and 1 on clean disconnect (EOF before all bytes read).
int recv_all(int sockfd, uint8_t *buffer, size_t length);

// Length-prefixed messaging (uint32 length in network byte order followed by payload)
int send_with_length(int sockfd, const uint8_t *buffer, uint32_t length);
// Returns 0 on success, -1 on error, 1 on disconnect.
int recv_with_length(int sockfd, uint8_t **out_buffer, uint32_t *out_length);

#endif // NET_H


