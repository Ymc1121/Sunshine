# RTX 4060 GPU reserve policy

On the qualified desktop RTX 4060 (NVIDIA PCI device 0x2882), with HAGS active
and the existing process-High policy selected, capture and conversion request
DXGI soft realtime priority 16. If either request fails, the automatic encoder
policy remains disabled and the request restores the prior relative priority.
The qualified path disables the optional NVENC preliminary pass, leaving more
GPU scheduling time for frame conversion and delivery. The process-level HAGS
Realtime freeze guard remains intact.

The policy is independent of game name and does not cap game FPS. It needs no
additional Agent INI fields or per-session diagnostic files. Other device IDs,
vendors, HAGS-off systems and explicitly selected process-Realtime setups retain
their previous behavior. Extending GPU coverage requires separate qualification.

The tradeoff is single-pass rate control/compression versus two-pass; validate
delivered image quality, bitrate peaks, loss and latency in the actual stream.
This is not a promise to reserve a fixed percentage of physical GPU utilization.

The earlier private diagnostic switches remain available for controlled testing.
Deployment and full-load device acceptance must be recorded separately from
source and unit-test results.
