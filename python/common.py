import socket
import struct
from typing import Tuple


def send_all(sock: socket.socket, data: bytes) -> None:
    view = memoryview(data)
    total = 0
    while total < len(data):
        sent = sock.send(view[total:])
        if sent == 0:
            raise ConnectionError("Socket connection broken during send")
        total += sent


def recv_exact(sock: socket.socket, length: int) -> bytes:
    chunks = []
    total = 0
    while total < length:
        chunk = sock.recv(length - total)
        if chunk == b"":
            raise ConnectionError("Socket connection closed during recv")
        chunks.append(chunk)
        total += len(chunk)
    return b"".join(chunks)


def send_with_length(sock: socket.socket, payload: bytes) -> None:
    header = struct.pack("!I", len(payload))
    send_all(sock, header)
    if payload:
        send_all(sock, payload)


def recv_with_length(sock: socket.socket) -> bytes:
    header = recv_exact(sock, 4)
    (length,) = struct.unpack("!I", header)
    if length == 0:
        return b""
    return recv_exact(sock, length)


