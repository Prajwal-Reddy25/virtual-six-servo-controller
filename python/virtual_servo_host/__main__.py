"""Run with PYTHONPATH=python python3 -m virtual_servo_host."""

import argparse
import sys
from pathlib import Path

from .client import ControllerClient, ControllerError

DEMO = (
    "GET",
    "SET 120 60 150 30 180 0",
    "GET",
    "STEP 250",
    "SET 0 0 0 0 0 181",
    "STEP 250",
    "STOP",
    "STEP 1000",
    "QUIT",
)


def main():
    parser = argparse.ArgumentParser(
        description="Host for a software-only six-servo controller"
    )
    parser.add_argument(
        "--controller", type=Path, default=Path("build/servo_controller")
    )
    parser.add_argument("--config", type=Path)
    parser.add_argument(
        "--timeout", type=float, default=2.0, help="response timeout in seconds"
    )
    parser.add_argument(
        "--demo", action="store_true", help="run a fixed six-joint demonstration"
    )
    args = parser.parse_args()
    client = None
    try:
        client = ControllerClient(
            args.controller.resolve(), config=args.config, timeout=args.timeout
        )
        with client:
            if args.demo:
                commands = DEMO
            else:
                if sys.stdin.isatty():
                    print(
                        "Enter SET, GET, STEP, STOP, or QUIT (EOF also closes).",
                        file=sys.stderr,
                    )
                commands = (line.rstrip("\r\n") for line in sys.stdin)
            for command in commands:
                if args.demo:
                    print(f"> {command}")
                response = client.request(command)
                print(f"< {response.raw}" if args.demo else response.raw, flush=True)
                if response.raw == "ACK QUIT":
                    break
        return 0
    except KeyboardInterrupt:
        return 130
    except (OSError, ValueError, ControllerError) as error:
        print(f"host: {error}", file=sys.stderr)
        if client is not None and client.diagnostics:
            print(client.diagnostics, file=sys.stderr, end="")
        return 1


if __name__ == "__main__":
    sys.exit(main())
