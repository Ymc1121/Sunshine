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
  frame_timing::context = {};
}
TEST(FrameTiming, OptInWindowPersistsCorrelatedRowsWithoutOverwriting) {
  auto root = std::filesystem::temp_directory_path() / ("gokuai-timing-test-" + std::to_string(frame_timing::now_ns()));
  std::filesystem::create_directory(root);
  auto request = root / "frame-timing-v1.request";
  auto output = root / "frame-timing-baseline.csv";
  {
    frame_timing::recorder recorder(root);
    EXPECT_FALSE(recorder.active());
  }
  { std::ofstream f(request); f << "baseline 5\n"; }
  {
    frame_timing::recorder recorder(root);
    ASSERT_TRUE(recorder.active());
    auto t = frame_timing::now_ns();
    recorder.add({t, t + 10, "test_stage", {7, 11, 1234}, 42});
    recorder.finish();
  }
  std::ifstream input(output, std::ios::binary);
  std::string content((std::istreambuf_iterator<char>(input)), std::istreambuf_iterator<char>());
  EXPECT_NE(content.find("window_complete=0"), std::string::npos);
  EXPECT_NE(content.find("overflow=0,rows=1"), std::string::npos);
  EXPECT_NE(content.find("test_stage,7,11,1234,42"), std::string::npos);
  input.close();
  {
    frame_timing::recorder recorder(root);
    EXPECT_FALSE(recorder.active()); // The existing result is immutable.
  }
  EXPECT_EQ(std::filesystem::file_size(output), content.size());
  std::filesystem::remove(request);
  std::filesystem::remove(output);
  std::filesystem::remove(root);
}
