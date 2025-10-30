from datetime import datetime


COLORS = {
    "INFO": "\x1b[36m",  # cyan
    " OK ": "\x1b[32m",  # green
    "ERR ": "\x1b[31m",  # red
    "RECV": "\x1b[33m",  # yellow
    "SEND": "\x1b[35m",  # magenta
}


def _log(level: str, message: str) -> None:
    color = COLORS.get(level, "")
    reset = "\x1b[0m" if color else ""
    ts = datetime.now().strftime("%H:%M:%S")
    print(f"{color}[{level}]{reset} {ts} {message}", flush=True)


def info(msg: str) -> None:
    _log("INFO", msg)


def ok(msg: str) -> None:
    _log(" OK ", msg)


def err(msg: str) -> None:
    _log("ERR ", msg)


def recv(msg: str) -> None:
    _log("RECV", msg)


def send(msg: str) -> None:
    _log("SEND", msg)


