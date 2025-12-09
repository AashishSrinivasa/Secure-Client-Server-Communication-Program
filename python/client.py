import argparse
import socket

from common import recv_with_length, send_with_length
from crypto import xor_crypt, DEFAULT_KEY

# ANSI color codes
COLOR_RESET = "\033[0m"
COLOR_GREEN = "\033[32m"
COLOR_BLUE = "\033[34m"
COLOR_CYAN = "\033[36m"
COLOR_YELLOW = "\033[33m"
COLOR_BOLD = "\033[1m"


def send_message_and_get_ack(sock: socket.socket, message: str) -> None:
    payload = bytearray(message.encode("utf-8"))
    xor_crypt(payload, DEFAULT_KEY)
    send_with_length(sock, bytes(payload))
    
    print(f"{COLOR_CYAN}{COLOR_BOLD}┌─ MESSAGE SENT ─────────────────────────────────────┐{COLOR_RESET}")
    print(f"{COLOR_CYAN}│ {COLOR_RESET}{COLOR_BOLD}Message:{COLOR_RESET} {message:<45s} {COLOR_CYAN}│{COLOR_RESET}")
    print(f"{COLOR_CYAN}│ {COLOR_RESET}{COLOR_BOLD}Size:{COLOR_RESET} {len(payload):<47d} bytes {COLOR_CYAN}│{COLOR_RESET}")
    print(f"{COLOR_CYAN}│ {COLOR_RESET}{COLOR_BOLD}Status:{COLOR_RESET}{COLOR_GREEN} ✓ Encrypted & Sent{COLOR_RESET}                    {COLOR_CYAN}│{COLOR_RESET}")
    print(f"{COLOR_CYAN}└──────────────────────────────────────────────────────────┘{COLOR_RESET}")

    enc_ack = recv_with_length(sock)
    buf = bytearray(enc_ack)
    xor_crypt(buf, DEFAULT_KEY)
    ack_msg = buf.decode('utf-8', errors='replace')
    
    print(f"{COLOR_GREEN}{COLOR_BOLD}┌─ ACKNOWLEDGMENT RECEIVED ────────────────────────────┐{COLOR_RESET}")
    print(f"{COLOR_GREEN}│ {COLOR_RESET}{COLOR_BOLD}Response:{COLOR_RESET} {ack_msg:<42s} {COLOR_GREEN}│{COLOR_RESET}")
    print(f"{COLOR_GREEN}│ {COLOR_RESET}{COLOR_BOLD}Status:{COLOR_RESET}{COLOR_GREEN} ✓ Decrypted Successfully{COLOR_RESET}              {COLOR_GREEN}│{COLOR_RESET}")
    print(f"{COLOR_GREEN}└──────────────────────────────────────────────────────────┘{COLOR_RESET}")


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
        print(f"{COLOR_BLUE}{COLOR_BOLD}\n╔══════════════════════════════════════════════════════════╗{COLOR_RESET}")
        print(f"{COLOR_BLUE}{COLOR_BOLD}║{COLOR_RESET}{COLOR_CYAN}{COLOR_BOLD}          SECURE CLIENT - INTERACTIVE MODE{COLOR_RESET}{COLOR_BLUE}{COLOR_BOLD}          ║{COLOR_RESET}")
        print(f"{COLOR_BLUE}{COLOR_BOLD}╚══════════════════════════════════════════════════════════╝\n{COLOR_RESET}")
        print(f"{COLOR_YELLOW}Enter messages (empty line to quit):{COLOR_RESET}")
        while True:
            try:
                line = input(f"{COLOR_CYAN}{COLOR_BOLD}→ {COLOR_RESET}")
            except EOFError:
                break
            if not line:
                break
            print()
            send_message_and_get_ack(s, line)
            print()


if __name__ == "__main__":
    main()


