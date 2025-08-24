#pragma once

#include <windows.h>
#include <winsvc.h>
#include <iostream>
#pragma comment(lib, "advapi32.lib")

bool StopWindowsService(const wchar_t* serviceName);
