"""Real C++ process integration; no emulation of the controller implementation."""

import os
import subprocess
import sys
import tempfile
import unittest
from pathlib import Path

from virtual_servo_host import ControllerClient, ControllerError

ROOT = Path(__file__).resolve().parents[1]
CONTROLLER = Path(
    os.environ.get("SERVO_CONTROLLER", ROOT / "build/servo_controller")
).resolve()


class IntegrationTests(unittest.TestCase):
    def client(self, **kwargs):
        client = ControllerClient(CONTROLLER, **kwargs)
        self.addCleanup(client.close)
        return client

    def positions(self, client, command, expected):
        response = client.request(command)
        self.assertIsNotNone(response.positions)
        for actual, target in zip(response.positions, expected):
            self.assertAlmostEqual(actual, target, delta=0.000001)

    def raw(self, payload, *args):
        return subprocess.run(
            [str(CONTROLLER), *args],
            input=payload,
            capture_output=True,
            timeout=3,
            check=False,
        )

    def test_six_joint_motion_atomic_rejection_and_stop(self):
        with self.client() as client:
            self.positions(client, "GET", [90] * 6)
            self.assertEqual(client.request("SET 180 0 150 30 120 60").raw, "ACK SET")
            self.positions(client, "GET", [90] * 6)
            for bad, expected in (
                ("SET 0 180 0 180 0 181", "ERR JOINT_LIMIT 6"),
                ("SET 0 180 0 180 0 nan", "ERR BAD_NUMBER"),
                ("SET 1 2", "ERR ARITY"),
                ("STEP 0", "ERR STEP_RANGE"),
            ):
                self.assertEqual(client.request(bad).raw, expected)
                self.positions(client, "GET", [90] * 6)
            self.positions(client, "STEP 250", [105, 75, 105, 75, 105, 75])
            self.positions(client, "STEP 250", [120, 60, 120, 60, 120, 60])
            self.assertEqual(client.request("STOP").raw, "ACK STOP")
            self.positions(client, "STEP 1000", [120, 60, 120, 60, 120, 60])
            client.request("SET 90 90 90 90 90 90")
            self.positions(client, "STEP 1000", [90] * 6)
        self.assertEqual(client.process.returncode, 0)

    def test_individual_configuration(self):
        with self.client(config=ROOT / "config/example.conf") as client:
            self.positions(client, "GET", [0] * 6)
            client.request("SET 90 -90 90 -90 90 -90")
            self.positions(client, "STEP 500", [5, -10, 15, -20, 25, -30])
            self.assertEqual(
                client.request("SET 0 0 0 0 0 -91").raw, "ERR JOINT_LIMIT 6"
            )

    def test_step_partition_is_deterministic(self):
        with self.client() as whole, self.client() as parts:
            for client in (whole, parts):
                client.request("SET 180 0 180 0 180 0")
            expected = whole.request("STEP 1000").positions
            for _ in range(10):
                actual = parts.request("STEP 100").positions
            self.assertEqual(actual, expected)

    def test_raw_framing_error_recovery_and_crlf(self):
        payload = (
            b"\nGET extra\nSET 90 90oops 90 90 90 90\nSTEP 1001\n"
            + b"x" * 100000
            + b"\xff\n"  # Drain oversized input, length wins over encoding.
            + b"GET\xff\nGET\x00\nGET\r\nQUIT\nGET\n"
        )
        result = self.raw(payload)
        self.assertEqual(result.returncode, 0)
        self.assertEqual(result.stderr, b"")
        lines = result.stdout.decode("ascii").splitlines()
        self.assertEqual(
            lines[:7],
            [
                "ERR EMPTY",
                "ERR ARITY",
                "ERR BAD_NUMBER",
                "ERR STEP_RANGE",
                "ERR LINE_TOO_LONG",
                "ERR BAD_ENCODING",
                "ERR UNKNOWN_COMMAND",
            ],
        )
        self.assertTrue(lines[7].startswith("STATE 90.000000"))
        self.assertEqual(lines[8:], ["ACK QUIT"])

    def test_maximum_line_length_for_lf_and_crlf(self):
        valid = b" " * 253 + b"GET"
        result = self.raw(valid + b"\n" + valid + b"\r\n" + b" " + valid + b"\nGET\n")
        self.assertEqual(result.returncode, 0)
        lines = result.stdout.splitlines()
        self.assertEqual(len(lines), 4)
        self.assertEqual(lines[0], lines[1])
        self.assertEqual(lines[2], b"ERR LINE_TOO_LONG")
        self.assertEqual(lines[0], lines[3])

    def test_eof_discards_unterminated_line(self):
        for payload in (b"", b"GET", b"x" * 1000):
            with self.subTest(payload_length=len(payload)):
                result = self.raw(payload)
                self.assertEqual(
                    (result.returncode, result.stdout, result.stderr), (0, b"", b"")
                )
        result = self.raw(b"GET\nSET 0 0 0 0 0 0")
        self.assertEqual(result.returncode, 0)
        self.assertEqual(len(result.stdout.splitlines()), 1)

    def test_bad_startup_arguments_and_configuration(self):
        result = self.raw(b"GET\n", "--unknown")
        self.assertNotEqual(result.returncode, 0)
        self.assertEqual(result.stdout, b"")
        self.assertIn(b"usage:", result.stderr)
        with tempfile.TemporaryDirectory(prefix="virtual-servo-config-") as directory:
            path = Path(directory) / "bad.conf"
            path.write_text("0 180 90 0\n" * 6, encoding="ascii")
            result = self.raw(b"GET\n", "--config", str(path))
            self.assertNotEqual(result.returncode, 0)
            self.assertEqual(result.stdout, b"")
            with self.client(config=path) as client, self.assertRaises(ControllerError):
                client.request("GET")
            self.assertIn("configuration", client.diagnostics)

    def test_cli_demo_and_piped_input(self):
        demo = subprocess.run(
            [
                sys.executable,
                "-m",
                "virtual_servo_host",
                "--controller",
                str(CONTROLLER),
                "--demo",
            ],
            capture_output=True,
            timeout=5,
            check=False,
        )
        self.assertEqual(demo.returncode, 0, demo.stderr.decode())
        self.assertEqual(demo.stderr, b"")
        self.assertIn(b"< ERR JOINT_LIMIT 6\n", demo.stdout)
        self.assertTrue(demo.stdout.endswith(b"< ACK QUIT\n"))
        piped = subprocess.run(
            [
                sys.executable,
                "-m",
                "virtual_servo_host",
                "--controller",
                str(CONTROLLER),
            ],
            input=b"GET\n",
            capture_output=True,
            timeout=5,
            check=False,
        )
        self.assertEqual(piped.returncode, 0, piped.stderr.decode())
        self.assertTrue(piped.stdout.startswith(b"STATE "))
        self.assertEqual(len(piped.stdout.splitlines()), 1)
