# Bounded frame pipeline diagnostics

This opt-in diagnostic build preserves the stream-grace60 runtime behavior.
It does not change GPU scheduling, HAGS, frame limits, queue depths or codecs.

An administrator can create the private directory
`C:/ProgramData/GouKuai/Diagnostics/frame-timing` and write a request file named
`frame-timing-v1.request` containing a unique lowercase tag and duration, e.g.
`baseline-4060 60`. Only tags `[a-z0-9-]{1,32}` and 5–120 seconds are accepted.
The request is checked at most once per second; no service restart is needed.
Use the existing ProgramData administrative ACL, never a publicly writable path.

Rows are buffered in memory, capped at 200,000, and written once the observation
window ends to `frame-timing-<tag>.csv`. Existing results are never replaced.
The header records the steady/UTC clock mapping, requested end time, completion,
overflow and row count. Incomplete or overflowing captures are invalid samples.

The measurements are CPU-side elapsed intervals around actual calls:

- `capture_snapshot`: desktop duplication and capture-side work, keyed by the
  returned image's original presentation timestamp (`source_ns`).
- `capture_acquire`: desktop duplication acquire call, including its wait.
- `convert_submit`: image conversion call, including synchronization and GPU
  command submission. It is not a GPU execution timestamp.
- `convert_keyed_mutex`: wait for the shared capture image's keyed mutex.
- `nvenc_input_sync`, `nvenc_map_input`, `nvenc_submit`,
  `nvenc_completion_wait`, `nvenc_lock_bitstream`: NVENC call intervals.
- `encode_total`: complete codec processing call.
- `packetize_and_send`: packet preparation, pacing and send-call interval;
  its return is not proof of receipt or physical display.

Encode and send rows share the stream channel and frame index. `source_ns`
joins the captured image with conversion/encoding even when two streams reuse
one image. A zero source timestamp denotes repeated/static content or missing
source attribution and must not be treated as a newly rendered game frame.

Compare instrumented, unarmed and armed runs on the same machine to measure
probe overhead. Correlate with independently collected game Present events and
physical-client receive/decode/render statistics. Do not add nested spans when
calculating total latency. The data contains no video pixels, input or credentials.
