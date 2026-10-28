# Moving the design to an ESP32

The current link uses local subprocess pipes. An ESP32 receives a serial byte
stream, so the grammar can remain useful while transport and execution change.

- Replace pipes with UART/USB serial access. Choose baud rate, connection/reset
  behavior, and a serial library/OS interface. Python has no portable high-level
  standard-library serial API; that dependency is a hardware-stage decision.
- Use an incremental bounded buffer. Reads can contain partial or multiple
  lines; overflow must discard through a delimiter. Keep debug text separate
  from acknowledgments.
- Retain validation before target changes, but calibrate each joint's limits
  and pulse mapping. Do not assume 0–180 degrees directly maps to a particular
  servo pulse range or linkage motion.
- Replace manual STEP timing with a nonblocking device-side loop and suitable
  clock. Measure timing/jitter rather than inferring real-time behavior from
  deterministic virtual steps.
- Define startup, link-loss, reset, watchdog, timeout, and stop behavior. A missing
  acknowledgment does not prove a command was not executed; retries may need
  sequence IDs and explicit idempotency rules in a revised protocol.
- Engineer supply, grounding, wiring, current demand, travel, load behavior,
  and independent stopping/protection. Simulation does not assess powered joints.

STOP changes software targets only. Hobby servo pulse commands may provide no
measured position feedback, and requested position is not necessarily actual
joint position. Hardware claims require separate physical validation.
