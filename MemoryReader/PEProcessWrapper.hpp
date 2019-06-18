#pragma once
#include <ntddk.h>
#include "Util.h"

struct PEProcessWrapper
{
	PEPROCESS pe_process = nullptr;

	~PEProcessWrapper()
	{
		if (pe_process == nullptr)
		{
			DebugLog("PEProcessWrapper: pe_process was null");
			return;
		}

		ObDereferenceObject(pe_process);
		pe_process = nullptr;
	}
};