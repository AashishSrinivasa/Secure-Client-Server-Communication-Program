#ifndef CRYPTO_H
#define CRYPTO_H

#include <stddef.h>
#include <stdint.h>

// XOR cipher with repeating key. In-place transformation.
// Returns 0 on success, -1 on invalid params.
int xor_crypt(uint8_t *buffer, size_t length, const uint8_t *key, size_t key_length);

#endif // CRYPTO_H


