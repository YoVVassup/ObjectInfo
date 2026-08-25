#include <YRPP.h>

#ifdef _DEBUG

DEFINE_HOOK(0x49F5C0, CopyProtection_IsLauncherRunning, 8)
{
	R->AL(1);
	return 0x49F61A;
}

DEFINE_HOOK(0x49F620, CopyProtection_NotifyLauncher, 5)
{
	R->AL(1);
	return 0x49F733;
}

DEFINE_HOOK(0x49F7A0, CopyProtection_CheckProtectedData, 8)
{
	R->AL(1);
	return 0x49F8A7;
}

#endif
