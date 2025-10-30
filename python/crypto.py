from typing import ByteString

DEFAULT_KEY = b"MY_SECRET_KEY"


def xor_crypt(data: bytearray, key: bytes = DEFAULT_KEY) -> None:
    if not key:
        raise ValueError("Key must not be empty")
    klen = len(key)
    for i in range(len(data)):
        data[i] ^= key[i % klen]


def to_bytes(s: str) -> bytes:
    return s.encode("utf-8")


def to_str(b: ByteString) -> str:
    return bytes(b).decode("utf-8", errors="replace")


