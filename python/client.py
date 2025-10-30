import argparse
import socket

from common import recv_with_length, send_with_length
from crypto import xor_crypt, DEFAULT_KEY
from logger import info, err, recv as log_recv, send as log_send


def send_message_and_get_ack(sock: socket.socket, message: str) -> None:
    payload = bytearray(message.encode("utf-8"))
    xor_crypt(payload, DEFAULT_KEY)
    send_with_length(sock, bytes(payload))
    log_send(f"Encrypted bytes: {len(payload)}")

    enc_ack = recv_with_length(sock)
    buf = bytearray(enc_ack)
    xor_crypt(buf, DEFAULT_KEY)
    log_recv(f"{buf.decode('utf-8', errors='replace')}")


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("--host", required=True)
    parser.add_argument("--port", type=int, required=True)
    parser.add_argument("--message", help="Send one message and exit")
    args = parser.parse_args()

    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
        s.connect((args.host, args.port))
        if args.message:
            send_message_and_get_ack(s, args.message)
            return
        info("Enter messages (empty line to quit):")
        while True:
            try:
                line = input("> ")
            except EOFError:
                break
            if not line:
                break
            send_message_and_get_ack(s, line)


if __name__ == "__main__":
    main()


