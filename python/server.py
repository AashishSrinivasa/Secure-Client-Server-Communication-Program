import argparse
import socket
import threading
from typing import Tuple

from common import recv_with_length, send_with_length
from crypto import xor_crypt, DEFAULT_KEY, to_str

# ANSI color codes
COLOR_RESET = "\033[0m"
COLOR_GREEN = "\033[32m"
COLOR_BLUE = "\033[34m"
COLOR_CYAN = "\033[36m"
COLOR_YELLOW = "\033[33m"
COLOR_MAGENTA = "\033[35m"
COLOR_BOLD = "\033[1m"


def handle_client(conn: socket.socket, addr: Tuple[str, int]) -> None:
    ip, port = addr
    print(f"{COLOR_GREEN}{COLOR_BOLD}\n╔══════════════════════════════════════════════════════════╗{COLOR_RESET}", flush=True)
    print(f"{COLOR_GREEN}{COLOR_BOLD}║{COLOR_RESET}{COLOR_CYAN}{COLOR_BOLD}              NEW CLIENT CONNECTION{COLOR_RESET}{COLOR_GREEN}{COLOR_BOLD}                 ║{COLOR_RESET}", flush=True)
    print(f"{COLOR_GREEN}{COLOR_BOLD}╠══════════════════════════════════════════════════════════╣{COLOR_RESET}", flush=True)
    print(f"{COLOR_GREEN}{COLOR_BOLD}║{COLOR_RESET} {COLOR_BOLD}Client:{COLOR_RESET} {ip:<50s} {COLOR_GREEN}{COLOR_BOLD}║{COLOR_RESET}", flush=True)
    print(f"{COLOR_GREEN}{COLOR_BOLD}║{COLOR_RESET} {COLOR_BOLD}Port:{COLOR_RESET} {port:<52d} {COLOR_GREEN}{COLOR_BOLD}║{COLOR_RESET}", flush=True)
    print(f"{COLOR_GREEN}{COLOR_BOLD}╚══════════════════════════════════════════════════════════╝{COLOR_RESET}", flush=True)
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
            
            print(f"{COLOR_BLUE}{COLOR_BOLD}┌─ MESSAGE RECEIVED ────────────────────────────────────┐{COLOR_RESET}", flush=True)
            print(f"{COLOR_BLUE}│ {COLOR_RESET}{COLOR_BOLD}From:{COLOR_RESET} {ip:<47s} {COLOR_BLUE}│{COLOR_RESET}", flush=True)
            print(f"{COLOR_BLUE}│ {COLOR_RESET}{COLOR_BOLD}Size:{COLOR_RESET} {len(enc):<47d} bytes {COLOR_BLUE}│{COLOR_RESET}", flush=True)
            print(f"{COLOR_BLUE}│ {COLOR_RESET}{COLOR_BOLD}Status:{COLOR_RESET}{COLOR_GREEN} ✓ Decrypted Successfully{COLOR_RESET}            {COLOR_BLUE}│{COLOR_RESET}", flush=True)
            print(f"{COLOR_BLUE}├──────────────────────────────────────────────────────────┤{COLOR_RESET}", flush=True)
            print(f"{COLOR_BLUE}│ {COLOR_RESET}{COLOR_BOLD}Message:{COLOR_RESET} {msg:<45s} {COLOR_BLUE}│{COLOR_RESET}", flush=True)
            print(f"{COLOR_BLUE}└──────────────────────────────────────────────────────────┘{COLOR_RESET}", flush=True)

            ack_text = f"ACK: Received {len(enc)} bytes"
            ack = bytearray(ack_text.encode("utf-8"))
            xor_crypt(ack, DEFAULT_KEY)
            send_with_length(conn, bytes(ack))
            
            print(f"{COLOR_MAGENTA}{COLOR_BOLD}┌─ ACKNOWLEDGMENT SENT ──────────────────────────────────┐{COLOR_RESET}", flush=True)
            print(f"{COLOR_MAGENTA}│ {COLOR_RESET}{COLOR_BOLD}Response:{COLOR_RESET} {ack_text:<42s} {COLOR_MAGENTA}│{COLOR_RESET}", flush=True)
            print(f"{COLOR_MAGENTA}│ {COLOR_RESET}{COLOR_BOLD}Status:{COLOR_RESET}{COLOR_GREEN} ✓ Encrypted & Sent{COLOR_RESET}                    {COLOR_MAGENTA}│{COLOR_RESET}", flush=True)
            print(f"{COLOR_MAGENTA}└──────────────────────────────────────────────────────────┘{COLOR_RESET}", flush=True)
    except (ConnectionError, OSError) as e:
        print(f"{COLOR_BOLD}[✗ ERROR] {COLOR_RESET}Client {ip}:{port} error: {e}")
    finally:
        conn.close()
        print(f"{COLOR_YELLOW}{COLOR_BOLD}\n╔══════════════════════════════════════════════════════════╗{COLOR_RESET}")
        print(f"{COLOR_YELLOW}{COLOR_BOLD}║{COLOR_RESET} {COLOR_BOLD}Client Disconnected:{COLOR_RESET} {ip:<37s} {COLOR_YELLOW}{COLOR_BOLD}║{COLOR_RESET}")
        print(f"{COLOR_YELLOW}{COLOR_BOLD}╚══════════════════════════════════════════════════════════╝{COLOR_RESET}")


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("--host", default="0.0.0.0")
    parser.add_argument("--port", type=int, default=5000)
    args = parser.parse_args()

    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
        s.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        s.bind((args.host, args.port))
        s.listen(16)
        print(f"{COLOR_CYAN}{COLOR_BOLD}\n╔══════════════════════════════════════════════════════════╗{COLOR_RESET}", flush=True)
        print(f"{COLOR_CYAN}{COLOR_BOLD}║{COLOR_RESET}{COLOR_BOLD}              SECURE SERVER STARTED{COLOR_RESET}{COLOR_CYAN}{COLOR_BOLD}                  ║{COLOR_RESET}", flush=True)
        print(f"{COLOR_CYAN}{COLOR_BOLD}╠══════════════════════════════════════════════════════════╣{COLOR_RESET}", flush=True)
        print(f"{COLOR_CYAN}{COLOR_BOLD}║{COLOR_RESET} {COLOR_BOLD}Status:{COLOR_RESET}{COLOR_GREEN} ✓ Listening{COLOR_RESET}                                      {COLOR_CYAN}{COLOR_BOLD}║{COLOR_RESET}", flush=True)
        addr_str = f"{args.host}:{args.port}"
        print(f"{COLOR_CYAN}{COLOR_BOLD}║{COLOR_RESET} {COLOR_BOLD}Address:{COLOR_RESET} {addr_str:<43s} {COLOR_CYAN}{COLOR_BOLD}║{COLOR_RESET}", flush=True)
        print(f"{COLOR_CYAN}{COLOR_BOLD}║{COLOR_RESET} {COLOR_BOLD}Protocol:{COLOR_RESET} TCP with XOR Encryption                      {COLOR_CYAN}{COLOR_BOLD}║{COLOR_RESET}", flush=True)
        print(f"{COLOR_CYAN}{COLOR_BOLD}╚══════════════════════════════════════════════════════════╝{COLOR_RESET}", flush=True)
        print(f"{COLOR_YELLOW}\nWaiting for clients...{COLOR_RESET}", flush=True)
        while True:
            conn, addr = s.accept()
            t = threading.Thread(target=handle_client, args=(conn, addr), daemon=True)
            t.start()


if __name__ == "__main__":
    main()


