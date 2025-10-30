### C Version

Build:
```bash
make clean && make
```

Run server:
```bash
./server 5000
```

Run client (one-shot):
```bash
./client 127.0.0.1 5000 "Hello from C"
```

Run client (interactive):
```bash
./client 127.0.0.1 5000
```

Protocol: 4-byte big-endian length + payload (XOR-encrypted). Key is `MY_SECRET_KEY`.


