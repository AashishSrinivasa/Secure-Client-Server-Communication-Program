import argparse
import socket
import threading
from typing import Tuple

from common import recv_with_length, send_with_length
from crypto import xor_crypt, DEFAULT_KEY, to_str
from logger import info, err, recv as log_recv, send as log_send


def handle_client(conn: socket.socket, addr: Tuple[str, int]) -> None:
    ip, port = addr
    info(f"Client connected: {ip}:{port}")
    try:
        while True:
            enc = recv_with_length(conn)
            if enc is None:
                break
            if len(enc) == 0:
                continue
            buf = bytearray(enc)
            xor_crypt(buf, DEFAULT_KEY)
            msg = to_str(buf)
            log_recv(f"{msg}")

            ack_text = f"ACK: Received {len(enc)} bytes"
            ack = bytearray(ack_text.encode("utf-8"))
            xor_crypt(ack, DEFAULT_KEY)
            send_with_length(conn, bytes(ack))
            log_send("ACK sent to client")
    except (ConnectionError, OSError) as e:
        err(f"Client {ip}:{port} error: {e}")
    finally:
        conn.close()
        info(f"Client disconnected: {ip}:{port}")


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("--host", default="0.0.0.0")
    parser.add_argument("--port", type=int, default=5000)
    args = parser.parse_args()

    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
        s.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        s.bind((args.host, args.port))
        s.listen(16)
        info(f"Listening on {args.host}:{args.port}")
        while True:
            conn, addr = s.accept()
            t = threading.Thread(target=handle_client, args=(conn, addr), daemon=True)
            t.start()


if __name__ == "__main__":
    main()


