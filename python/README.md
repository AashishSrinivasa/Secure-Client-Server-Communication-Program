### Python Version

Run server:
```bash
python3 server.py --port 5000
```

Run client (one-shot):
```bash
python3 client.py --host 127.0.0.1 --port 5000 --message "Hello from Python"
```

Run client (interactive):
```bash
python3 client.py --host 127.0.0.1 --port 5000
```

Protocol: 4-byte big-endian length + payload (XOR-encrypted). Key is `MY_SECRET_KEY`.


