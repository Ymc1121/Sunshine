#pragma once

#include <Windows.h>
#include <aclapi.h>
#include <sddl.h>

namespace gokuai_shutdown {
  constexpr auto signal_name = L"Global\\GouKuaiSunshineSystemShutdownConfirmedV1";

  inline bool confirmed_system_end(bool ending, LPARAM flags) {
    // A cancelled query and Restart Manager/logoff are not a system shutdown.
    return ending && !(flags & (ENDSESSION_LOGOFF | ENDSESSION_CLOSEAPP));
  }

  inline bool owned_by_system(HANDLE event) {
    PSID owner = nullptr;
    PSECURITY_DESCRIPTOR descriptor = nullptr;
    auto error = GetSecurityInfo(event, SE_KERNEL_OBJECT, OWNER_SECURITY_INFORMATION,
                                 &owner, nullptr, nullptr, nullptr, &descriptor);
    bool trusted = error == ERROR_SUCCESS && owner && IsWellKnownSid(owner, WinLocalSystemSid);
    if (descriptor) LocalFree(descriptor);
    return trusted;
  }

  inline HANDLE create_service_signal() {
    PSECURITY_DESCRIPTOR descriptor = nullptr;
    if (!ConvertStringSecurityDescriptorToSecurityDescriptorW(
          L"O:SYG:SYD:P(A;;GA;;;SY)(A;;GR;;;BA)", SDDL_REVISION_1, &descriptor, nullptr)) return nullptr;
    SECURITY_ATTRIBUTES security {sizeof(security), descriptor, FALSE};
    auto event = CreateEventW(&security, TRUE, FALSE, signal_name);
    LocalFree(descriptor);
    if (event && !owned_by_system(event)) {
      CloseHandle(event);
      return nullptr;
    }
    // Do not reset an existing confirmed signal on a service restart. The
    // kernel object is boot-local and retained by the Agent until OS exit.
    return event;
  }

  inline bool publish_confirmed() {
    auto event = OpenEventW(EVENT_MODIFY_STATE | READ_CONTROL, FALSE, signal_name);
    if (!event) return false;
    bool result = owned_by_system(event) && SetEvent(event);
    CloseHandle(event);
    return result;
  }

  inline bool requested(HANDLE event) {
    return event && WaitForSingleObject(event, 0) == WAIT_OBJECT_0;
  }

  inline bool requested() {
    auto event = OpenEventW(SYNCHRONIZE | READ_CONTROL, FALSE, signal_name);
    if (!event) return false;
    bool result = owned_by_system(event) && requested(event);
    CloseHandle(event);
    return result;
  }
}
