#pragma once
#include <cstdint>

namespace platf::dxgi::gpu_reserve {
  // Qualified on desktop RTX 4060. Do not silently extend the policy to
  // untested GPUs, non-HAGS scheduling, or explicit process-realtime setups.
  constexpr bool enabled(std::uint32_t vendor, std::uint32_t device,
                         bool hags, bool process_realtime_requested) {
    return vendor == 0x10de && device == 0x2882 && hags && !process_realtime_requested;
  }
}
