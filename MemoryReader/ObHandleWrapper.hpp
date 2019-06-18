#pragma once
#include <ntddk.h>

#include "Util.h"

struct ObHandleWrapper
{
	HANDLE handle = nullptr;

	~ObHandleWrapper()
	{
		if (handle == nullptr)
		{
			DebugLog("ObHandleWrapper: handle was null");
			return;
		}

		if (NTSTATUS status = ZwClose(handle); status != STATUS_SUCCESS)
		{
			DebugLog("ObHandleWrapper: Could not close handle %p", handle);
		}

		handle = nullptr;
	}
};
