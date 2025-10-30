## Secure Client–Server Communication (C and Python)

This project implements a secure(ish) client–server system with TCP sockets in both C and Python. Messages are encrypted using a simple XOR stream cipher with a shared key. The server handles multiple clients concurrently (threads), decrypts incoming messages, displays the plaintext, and returns an encrypted acknowledgment.

### Features
- TCP client/server in C and Python
- Bi-directional encrypted messaging
- Length-prefixed protocol for reliable transmission
- Multi-client concurrency with threads
- Modular code: crypto, framing (send/recv), networking
- Error handling with clear messages

### Algorithm
Simple XOR with a shared ASCII key. Same function is used for encryption/decryption.

Key (default): `MY_SECRET_KEY`

---

## C Implementation

### Build
```bash
cd c
make clean && make
```

Build outputs: `server`, `client`.

### Run Server
```bash
./server 5000
```

### Run Client
Send a message provided as an argument:
```bash
./client 127.0.0.1 5000 "Hello from C"
```

Or interactive mode (no message argument):
```bash
./client 127.0.0.1 5000
```

### Sample Output
Server:
```
[INFO] Listening on 0.0.0.0:5000
[INFO] Client connected: 127.0.0.1:54321
[RECV] Plaintext: Hello from C
[SEND] ACK sent to client
```

Client:
```
[SEND] Encrypted bytes: 12 bytes
[RECV] ACK: Received 12 bytes
```

---

## Python Implementation

### Run Server
```bash
cd python
python3 server.py --port 5000
```

### Run Client
Argument message:
```bash
python3 client.py --host 127.0.0.1 --port 5000 --message "Hello from Python"
```

Interactive mode:
```bash
python3 client.py --host 127.0.0.1 --port 5000
```

### Sample Output
Server:
```
[INFO] Listening on 0.0.0.0:5000
[INFO] Client connected: 127.0.0.1:54322
[RECV] Plaintext: Hello from Python
[SEND] ACK sent to client
```

Client:
```
[SEND] Encrypted bytes: 18 bytes
[RECV] ACK: Received 18 bytes
```

---

## Demo Guide (What to show your instructor)
- Start C server, run multiple C clients sending different messages; show server prints plaintext and clients receive ACK.
- Start Python server, run multiple Python clients similarly.
- Cross-language: Use C client with Python server and vice versa (works due to shared protocol and key).
- Show code modularity: `crypto` and send/recv framing utilities.
- Point out error handling: invalid args, connection errors, short reads/writes.

---

## C vs Python: Key Differences
- Concurrency: C uses `pthreads`; Python uses `threading` with simpler API.
- Memory and I/O: C uses manual buffers and `send/recv` loops; Python leverages exceptions and higher-level methods.
- Build: C requires compilation (`Makefile`), Python runs directly.
- Error handling: Explicit return-code checks in C vs. exceptions in Python.

---

## Security Note
XOR with a static key is for educational purposes only and not secure for real-world use. For actual security, use TLS or proven crypto libraries (AES/GCM, libsodium, etc.).


