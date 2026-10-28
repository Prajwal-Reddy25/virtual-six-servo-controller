# Verified command sessions

Run after building:

```sh
PYTHONPATH=python python3 -m virtual_servo_host \
  --controller build-release/servo_controller --demo
```

The [captured output](evidence/demo.txt) shows:

1. `SET 120 60 150 30 180 0` returns `ACK SET`; GET still reports six 90-degree
   positions because targets and positions are separate.
2. `STEP 250` reports `105 75 105 75 105 75` degrees.
3. `SET 0 0 0 0 0 181` returns `ERR JOINT_LIMIT 6`.
4. Another `STEP 250` reports `120 60 120 60 120 60`, showing old targets survived
   the rejected command.
5. STOP followed by STEP 1000 leaves those positions unchanged.
6. QUIT receives ACK QUIT; both programs finish successfully.

For exploration, omit `--demo`. For the custom configuration:

```sh
PYTHONPATH=python python3 -m virtual_servo_host \
  --controller build-release/servo_controller --config config/example.conf
```

Enter `SET 90 -90 90 -90 90 -90` and `STEP 500`. Integration tests verify positions
`5 -10 15 -20 25 -30`, reflecting each joint's configured maximum speed.
