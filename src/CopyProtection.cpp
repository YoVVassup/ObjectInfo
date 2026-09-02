// ============================================================================
// CopyProtection.cpp - Debug-only DRM bypass hooks
//
// These hooks disable the game's copy protection checks in Debug builds
// to allow running the debugger without the launcher present.
// Only compiled when _DEBUG is defined.
// ============================================================================

#include <YRpp.h>

#ifdef _DEBUG

// Bypasses the "IsLauncherRunning" check - always returns true.
DEFINE_HOOK(0x49F5C0, CopyProtection_IsLauncherRunning, 8)
{
	R->AL(1);
	return 0x49F61A;
}

// Bypasses the "NotifyLauncher" check - always returns true.
DEFINE_HOOK(0x49F620, CopyProtection_NotifyLauncher, 5)
{
	R->AL(1);
	return 0x49F733;
}

// Bypasses the "CheckProtectedData" check - always returns true.
DEFINE_HOOK(0x49F7A0, CopyProtection_CheckProtectedData, 8)
{
	R->AL(1);
	return 0x49F8A7;
}

#endif
