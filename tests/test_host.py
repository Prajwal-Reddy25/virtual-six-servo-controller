"""Host response checks and deliberately faulty subprocess communication."""

import math
import sys
import tempfile
import unittest
from pathlib import Path

from virtual_servo_host import ControllerClient, ControllerError
from virtual_servo_host.client import parse_response


class ResponseTests(unittest.TestCase):
    def test_valid_responses(self):
        self.assertEqual(parse_response("ACK STOP", "STOP").raw, "ACK STOP")
        self.assertEqual(
            parse_response("ERR JOINT_LIMIT 6", "SET bad").raw, "ERR JOINT_LIMIT 6"
        )
        self.assertEqual(parse_response("ERR EMPTY", "").raw, "ERR EMPTY")
        response = parse_response(
            "STATE 0.000000 -1.000000 2.000000 3.000000 4.000000 5.000000", "GET"
        )
        self.assertEqual(response.positions, (0, -1, 2, 3, 4, 5))

    def test_malformed_or_mismatched_responses(self):
        for line, command in (
            ("ACK STOP", "GET"),
            ("ACK GET", "GET"),
            ("ERR UNKNOWN", "GET"),
            ("ERR JOINT_LIMIT 0", "SET"),
            ("ERR JOINT_LIMIT 12", "SET"),
            ("STATE 1 2 3 4 5 6", "GET"),
            ("STATE nan 0.000000 0.000000 0.000000 0.000000 0.000000", "GET"),
            ("STATE 361.000000 0.000000 0.000000 0.000000 0.000000 0.000000", "GET"),
            ("STATE 0.000000 0.000000 0.000000 0.000000 0.000000 0.000000", "SET"),
            ("ACK STOP ", "STOP"),
            ("", "GET"),
        ):
            with self.subTest(line=line), self.assertRaises(ControllerError):
                parse_response(line, command)


class FaultyProcessTests(unittest.TestCase):
    def setUp(self):
        self.directory = tempfile.TemporaryDirectory(prefix="virtual-servo-test-")
        self.addCleanup(self.directory.cleanup)

    def child(self, body, timeout=1.0):
        executable = Path(self.directory.name) / "faulty_controller"
        executable.write_text(
            f"#!{sys.executable}\nimport sys, time, signal\n{body}\n", encoding="utf-8"
        )
        executable.chmod(0o700)
        client = ControllerClient(executable, timeout=timeout)
        self.addCleanup(client.close)
        return client

    def test_invalid_response_bytes_and_framing(self):
        for reply in (
            b"garbage\n",
            b"\xff\n",
            b"x" * 300 + b"\n",
            b"ACK SET\nACK STOP\n",
            b"STATE 90",
        ):
            with self.subTest(reply=reply):
                client = self.child(
                    f"sys.stdin.buffer.readline()\nsys.stdout.buffer.write({reply!r})\nsys.stdout.buffer.flush()"
                )
                with self.assertRaises(ControllerError):
                    client.request("SET 90 90 90 90 90 90")
                client.close()
                self.assertIsNotNone(client.process.returncode)

    def test_unexpected_nonzero_exit(self):
        client = self.child("sys.stderr.write('deliberate failure\\n')\nsys.exit(7)")
        with self.assertRaises(ControllerError):
            client.request("GET")
        client.close()
        self.assertEqual(client.process.returncode, 7)
        self.assertIn("deliberate failure", client.diagnostics)

    def test_timeout_and_forced_cleanup(self):
        client = self.child(
            "signal.signal(signal.SIGTERM, signal.SIG_IGN)\nsys.stdin.readline()\ntime.sleep(10)",
            timeout=0.3,
        )
        with self.assertRaisesRegex(ControllerError, "timeout"):
            client.request("GET")
        client.close()
        self.assertIsNotNone(client.process.returncode)
        self.assertTrue(client.process.stdin.closed)
        with self.assertRaises(ControllerError):
            client.request("GET")

    def test_stderr_does_not_block_response(self):
        client = self.child(
            "sys.stdin.readline()\nsys.stderr.write('diagnostic\\n' * 10000)\nsys.stderr.flush()\n"
            "sys.stdout.write('ACK '); sys.stdout.flush(); time.sleep(0.01)\n"
            "sys.stdout.write('STOP\\n'); sys.stdout.flush()\n"
            "sys.stdin.readline()\nprint('ACK QUIT', flush=True)"
        )
        self.assertEqual(client.request("STOP").raw, "ACK STOP")
        client.close()
        self.assertLessEqual(len(client.diagnostics.encode()), 4096)

    def test_nonzero_exit_after_quit_acknowledgment(self):
        client = self.child(
            "sys.stdin.readline()\nprint('ACK QUIT', flush=True)\nsys.exit(7)"
        )
        self.assertEqual(client.request("QUIT").raw, "ACK QUIT")
        with self.assertRaises(ControllerError):
            client.close()

    def test_invalid_local_requests_do_not_poison_connection(self):
        client = self.child("sys.stdin.readline()\nprint('ACK QUIT', flush=True)")
        for command in ("GET\nQUIT", "GET\r", "x" * 257, "é"):
            with self.subTest(command=command), self.assertRaises(ValueError):
                client.request(command)
        client.close()
        self.assertEqual(client.process.returncode, 0)

    def test_invalid_timeout(self):
        for timeout in (0, -1, math.inf, math.nan):
            with self.subTest(timeout=timeout), self.assertRaises(ValueError):
                ControllerClient("unused", timeout=timeout)
