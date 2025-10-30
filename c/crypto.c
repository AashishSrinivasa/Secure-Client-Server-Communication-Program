#include "crypto.h"

int xor_crypt(uint8_t *buffer, size_t length, const uint8_t *key, size_t key_length) {
	if (buffer == NULL || key == NULL || key_length == 0) {
		return -1;
	}
	for (size_t i = 0; i < length; ++i) {
		buffer[i] ^= key[i % key_length];
	}
	return 0;
}


