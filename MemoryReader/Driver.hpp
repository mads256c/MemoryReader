#pragma once
#include <ntifs.h>
#include <ntddk.h>
#include <wdf.h>
#include <sal.h>

#include "String.hpp"


#define PROCESSES_MAX 256

extern "C" DRIVER_INITIALIZE DriverEntry;
extern "C" EVT_WDF_DRIVER_DEVICE_ADD DriverDeviceAdd;
extern "C" VOID CreateProcessNotifyRoutine(HANDLE ParentId, HANDLE ProcessId, BOOLEAN Create);
extern "C" EVT_WDF_DRIVER_UNLOAD DriverUnload;
NTSTATUS PrintProcesses();
NTSTATUS GetImageName(HANDLE ProcessId, String* string);
HANDLE GetProcessHandle(const char* module_name);


typedef NTSTATUS(*QUERY_INFO_PROCESS) (
	__in HANDLE ProcessHandle,
	__in PROCESSINFOCLASS ProcessInformationClass,
	__out_bcount(ProcessInformationLength) PVOID ProcessInformation,
	__in ULONG ProcessInformationLength,
	__out_opt PULONG ReturnLength
	);

QUERY_INFO_PROCESS ZwQueryInformationProcess;



