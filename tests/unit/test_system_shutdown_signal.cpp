#ifdef _WIN32
#include "../../src/platform/windows/system_shutdown_signal.h"
#include "../tests_common.h"

TEST(SystemShutdownSignal, OnlyConfirmedOperatingSystemEndSuppressesRecovery) {
  EXPECT_FALSE(gokuai_shutdown::confirmed_system_end(false, 0));
  EXPECT_FALSE(gokuai_shutdown::confirmed_system_end(false, ENDSESSION_CRITICAL));
  EXPECT_FALSE(gokuai_shutdown::confirmed_system_end(true, ENDSESSION_LOGOFF));
  EXPECT_FALSE(gokuai_shutdown::confirmed_system_end(true, ENDSESSION_CLOSEAPP));
  EXPECT_FALSE(gokuai_shutdown::confirmed_system_end(true, ENDSESSION_LOGOFF | ENDSESSION_CRITICAL));
  EXPECT_TRUE(gokuai_shutdown::confirmed_system_end(true, 0));
  EXPECT_TRUE(gokuai_shutdown::confirmed_system_end(true, ENDSESSION_CRITICAL));
}

TEST(SystemShutdownSignal, ObjectOwnerMustMatchLocalSystem) {
  auto event = CreateEventW(nullptr, TRUE, FALSE, nullptr);
  ASSERT_NE(event, nullptr);
  HANDLE token = nullptr;
  ASSERT_TRUE(OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &token));
  alignas(TOKEN_USER) unsigned char bytes[512];
  DWORD needed = 0;
  ASSERT_TRUE(GetTokenInformation(token, TokenUser, bytes, sizeof(bytes), &needed));
  EXPECT_EQ(gokuai_shutdown::owned_by_system(event),
            !!IsWellKnownSid(reinterpret_cast<TOKEN_USER *>(bytes)->User.Sid, WinLocalSystemSid));
  CloseHandle(token);
  CloseHandle(event);
}
#endif
