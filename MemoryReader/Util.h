#pragma once

#include <wdm.h>

#define DebugLog(x, ...) DbgPrint("MemoryReader: " x "\n", __VA_ARGS__)