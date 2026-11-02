#include <gtest/gtest.h>
#include "src/platform/windows/gpu_reserve_policy.h"

TEST(GpuReservePolicy, EnablesQualified4060HagsHighPath) {
  EXPECT_TRUE(platf::dxgi::gpu_reserve::enabled(0x10de, 0x2882, true, false));
}

TEST(GpuReservePolicy, PreservesOtherGpuModelsAndVendors) {
  EXPECT_FALSE(platf::dxgi::gpu_reserve::enabled(0x10de, 0x2f06, true, false));
  EXPECT_FALSE(platf::dxgi::gpu_reserve::enabled(0x10de, 0x2484, true, false));
  EXPECT_FALSE(platf::dxgi::gpu_reserve::enabled(0x1002, 0x2882, true, false));
}

TEST(GpuReservePolicy, PreservesOtherSchedulerPolicies) {
  EXPECT_FALSE(platf::dxgi::gpu_reserve::enabled(0x10de, 0x2882, false, false));
  EXPECT_FALSE(platf::dxgi::gpu_reserve::enabled(0x10de, 0x2882, true, true));
}
