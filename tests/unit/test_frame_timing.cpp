#include <gtest/gtest.h>
#include "src/frame_timing.h"

TEST(FrameTiming, RejectsUnboundedAndPathRequests) {
  EXPECT_TRUE(frame_timing::recorder::valid_request("baseline-4060", 60));
  EXPECT_FALSE(frame_timing::recorder::valid_request("../outside", 60));
  EXPECT_FALSE(frame_timing::recorder::valid_request("baseline", 121));
  EXPECT_FALSE(frame_timing::recorder::valid_request("", 60));
}
TEST(FrameTiming, RestoresNestedCorrelation) {
  frame_timing::context = {1, 2, 3};
  {
    frame_timing::context_guard outer(4, 5, 6);
    { frame_timing::context_guard inner(7, 8, 9); EXPECT_EQ(frame_timing::context.frame, 8); }
    EXPECT_EQ(frame_timing::context.frame, 5);
  }
  EXPECT_EQ(frame_timing::context.frame, 2);
}
