"""One outstanding request over POSIX subprocess pipes; no worker threads."""

import math
import os
import re
import selectors
import subprocess
import time
from dataclasses import dataclass


class ControllerError(RuntimeError):
    """A process, transport, timeout, or response-format failure."""


@dataclass(frozen=True)
class Response:
    raw: str
    positions: tuple[float, ...] | None = None


ERRORS = {
    "LINE_TOO_LONG",
    "BAD_ENCODING",
    "EMPTY",
    "UNKNOWN_COMMAND",
    "ARITY",
    "BAD_NUMBER",
    "STEP_RANGE",
}
POSITION = re.compile(r"-?[0-9]+\.[0-9]{6}\Z")


def parse_response(line: str, command: str) -> Response:
    """Check response structure and its relationship to the request command."""
    fields = line.split(" ")
    request = command.strip(" \t").split()[0:1]
    name = request[0] if request else ""
    if len(fields) == 2 and fields[0] == "ERR" and fields[1] in ERRORS:
        return Response(line)
    if (
        len(fields) == 3
        and fields[:2] == ["ERR", "JOINT_LIMIT"]
        and fields[2] in {"1", "2", "3", "4", "5", "6"}
    ):
        return Response(line)
    if fields == ["ACK", name] and name in {"SET", "STOP", "QUIT"}:
        return Response(line)
    if (
        len(fields) == 7
        and fields[0] == "STATE"
        and name in {"GET", "STEP"}
        and all(POSITION.fullmatch(value) for value in fields[1:])
    ):
        positions = tuple(float(value) for value in fields[1:])
        if all(math.isfinite(value) and -360 <= value <= 360 for value in positions):
            return Response(line, positions)
    raise ControllerError(f"malformed or unexpected response: {line!r}")


class ControllerClient:
    """Launch, exchange commands, and reap a local controller process.

    Response waits and shutdown are bounded. Protocol ERR replies are ordinary
    responses; a transport/response failure invalidates this connection.
    """

    def __init__(self, executable, *, config=None, timeout=2.0):
        if os.name != "posix":
            raise ControllerError("this pipe-selector host requires a POSIX platform")
        if not math.isfinite(timeout) or timeout <= 0:
            raise ValueError("timeout must be positive and finite")
        self.timeout = timeout
        self._closed = False
        self._failed = False
        self._quit = False
        self._stderr = bytearray()
        self._stdout = bytearray()
        args = [os.fspath(executable)]
        if config is not None:
            args.extend(["--config", os.fspath(config)])
        self.process = subprocess.Popen(
            args,
            stdin=subprocess.PIPE,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            bufsize=0,
        )
        self._selector = selectors.DefaultSelector()
        for stream in (self.process.stdin, self.process.stdout, self.process.stderr):
            os.set_blocking(stream.fileno(), False)
        self._selector.register(self.process.stdout, selectors.EVENT_READ, "stdout")
        self._selector.register(self.process.stderr, selectors.EVENT_READ, "stderr")

    @property
    def diagnostics(self):
        return self._stderr.decode("utf-8", errors="replace")

    def _events(self, deadline):
        remaining = deadline - time.monotonic()
        if remaining <= 0:
            raise ControllerError("controller response timeout")
        events = self._selector.select(remaining)
        if not events:
            raise ControllerError("controller response timeout")
        return events

    def request(self, command: str) -> Response:
        if self._closed or self._failed or self._quit:
            raise ControllerError("controller connection is closed or unusable")
        if "\n" in command or "\r" in command:
            raise ValueError("send one command without a line ending")
        payload = command.encode("ascii")
        if len(payload) > 256:
            raise ValueError("command exceeds 256 bytes")
        pending = memoryview(payload + b"\n")
        deadline = time.monotonic() + self.timeout
        self._selector.register(self.process.stdin, selectors.EVENT_WRITE, "stdin")
        try:
            while True:
                for key, _ in self._events(deadline):
                    if key.data == "stdin":
                        sent = os.write(key.fd, pending)
                        pending = pending[sent:]
                        if not pending:
                            self._selector.unregister(self.process.stdin)
                        continue
                    data = os.read(key.fd, 4096)
                    if key.data == "stderr":
                        if not data:
                            self._selector.unregister(key.fileobj)
                        self._stderr.extend(data)
                        del self._stderr[:-4096]
                        continue
                    if not data:
                        raise ControllerError(
                            "controller closed stdout before a complete response"
                        )
                    self._stdout.extend(data)
                    if len(self._stdout) > 257:
                        raise ControllerError("oversized controller response")
                    if b"\n" not in self._stdout:
                        continue
                    line, _, extra = self._stdout.partition(b"\n")
                    if extra or pending:
                        raise ControllerError(
                            "unsolicited or multiple controller responses"
                        )
                    self._stdout.clear()
                    response = parse_response(line.decode("ascii"), command)
                    self._quit = response.raw == "ACK QUIT"
                    return response
        except (OSError, UnicodeError, ControllerError) as error:
            self._failed = True
            raise ControllerError(str(error)) from error

    def close(self):
        if self._closed:
            return
        failure = None
        try:
            if not self._failed and not self._quit:
                self.request("QUIT")
            if not self._failed:
                self.process.wait(timeout=self.timeout)
                if self.process.returncode != 0:
                    raise ControllerError(
                        f"controller exited with status {self.process.returncode}"
                    )
        except (OSError, subprocess.TimeoutExpired, ControllerError) as error:
            failure = ControllerError(f"controller shutdown failed: {error}")
        finally:
            if self.process.poll() is None:
                self.process.terminate()
                try:
                    self.process.wait(timeout=self.timeout)
                except subprocess.TimeoutExpired:
                    self.process.kill()
                    self.process.wait()
            # Collect any final stderr without blocking, including startup errors.
            try:
                while True:
                    data = os.read(self.process.stderr.fileno(), 4096)
                    if not data:
                        break
                    self._stderr.extend(data)
                    del self._stderr[:-4096]
            except BlockingIOError:
                pass
            self._selector.close()
            for stream in (
                self.process.stdin,
                self.process.stdout,
                self.process.stderr,
            ):
                stream.close()
            self._closed = True
        if failure is not None:
            raise failure

    def __enter__(self):
        return self

    def __exit__(self, exc_type, exc_value, traceback):
        try:
            self.close()
        except ControllerError:
            if exc_type is None:
                raise
