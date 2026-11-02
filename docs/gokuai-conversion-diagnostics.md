# Conversion scheduling diagnostics (private test candidates)

The default behavior remains the existing relative GPU priority 7 and native
D3D11 NVENC input. The process-level `nvenc_realtime_hags` guard is unchanged.

On a specifically owned Windows test machine, an administrator may create one
of these files under the existing private diagnostic directory:

- `C:/ProgramData/GouKuai/Diagnostics/conversion-soft-priority.enable`: request
  DXGI absolute priority 16 (soft realtime) on both capture and conversion devices. Log API
  acceptance and the relative getter value; that getter is not proof of an
  absolute scheduling class. Failed or unsupported requests restore relative
  priority 7. This does not request process realtime or hard GPU realtime.
- `C:/ProgramData/GouKuai/Diagnostics/conversion-force-cuda.enable`: request the
  existing CUDA interop implementation for its supported 10-bit 4:4:4 input only.
  Other input formats log a rejection and retain native D3D11. The original
  unrestricted diagnostic attempt failed its format guard in the owned test;
  it is not evidence that CUDA encoding is slow or unsupported by the GPU.

Use one switch at a time for controlled comparisons. Changing a switch causes
the existing capture reinitialization path to rebuild the encoder; it does not
restart Windows, Sunshine or the game. The files are checked at most once per
second. Reinitialization can briefly interrupt frame delivery, so exclude the
transition from performance samples and verify that the same session recovers.

These switches are experimental diagnostic inputs, not production defaults.
Do not infer success from a successful API call, and do not claim compatibility
from a short run. Compare frame timing, game Present rate, physical-client
rendering, dropped frames, and long-running stability on the exact test machine.

Microsoft documents absolute priorities 16–29 as soft realtime, periodically
yielding to lower-priority work:
https://learn.microsoft.com/windows/win32/api/dxgi/nf-dxgi-idxgidevice-setgputhreadpriority
