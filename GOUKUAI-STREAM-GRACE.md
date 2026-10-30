# GouKuai stream interruption grace candidate

Candidate only; no production default, image publication, or physical acceptance.

Baseline: downstream commit 6448abcd0b245967ff430abe29f8b02130c9fcce,
with the deployed encoder-probe and immediate-service-launch source preserved
from complete archive SHA256
1ce3433aa7b2610fec121296e7e8db52175d0198f8aebd251524c63c07f0a0bd.

This source is modified directly and rebuilt. There is no patch application
step in the candidate build or deployment.

* The default control ping timeout and connected ENet peer timeout are 60s.
  Candidate configuration must not override ping_timeout below 60000 ms.
* Existing stream objects are not resurrected after STOPPING. A short packet
  outage can retain a live connection; an ended protocol needs a new stream.
* The cleanup watchdog remains bounded at 10s. During Windows shutdown it
  records an explicit cleanup timeout with recovery suppressed, rather than
  generating a debug trap which could be mistaken for a runtime crash.
* Normal Windows shutdown is not a reason to restart Sunshine. Agent and
  terminal orchestration retain exact session, generation and Boot boundaries.
  A confirmed WM_ENDSESSION publishes a LocalSystem-owned boot-local event
  before process exit. The service wrapper and Agent honor it before recovery;
  an ordinary user-owned object is rejected. A cancelled WM_ENDSESSION keeps
  Sunshine running, and logoff/Restart Manager do not publish OS-shutdown proof.
* Network recovery and process rebuild share the original client deadline;
  repeated events cannot create an additional minute.

Build: MSYS2 UCRT64 GCC 16.1, CMake/Ninja, RelWithDebInfo, at most two workers;
Boost 1.89.0 source pinned by upstream SHA, build-deps v2026.221.143859 FFmpeg
AMD64 archive SHA256 f362e4e0d8706befecfa8b2974e14089003066ffeeaaf11c4637cd7385df7148.
BUILD_VERSION=2026.1030.2-gokuai.streamgrace60 for the shutdown-order correction.

Local upstream tests: 338 passed, 7 environment-dependent skips, 0 failed.
These checks do not establish real GPU/encoder, outage, shutdown, or customer
acceptance. The original physical link fault is not repaired by this candidate.

Private 959-6 validation of the preceding build: 5s and 55s streaming-only UDP
loss recovered in the same client process; 65s loss expired at 60.004s without
another grace budget. Its final normal-shutdown audit found an Agent restart
before SCM PRESHUTDOWN. The confirmed-end-session handoff above fixes that
ordering gap in source and still requires a new physical shutdown acceptance.
