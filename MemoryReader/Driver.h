#pragma once
#include <ntifs.h>
#include <ntddk.h>
#include <wdf.h>
#include <sal.h>


#define PROCESSES_MAX 256

#define DebugLog(x, ...) DbgPrint("MemoryReader: " x "\n", __VA_ARGS__)

DRIVER_INITIALIZE DriverEntry;
EVT_WDF_DRIVER_DEVICE_ADD DriverDeviceAdd;
VOID CreateProcessNotifyRoutine(HANDLE ParentId, HANDLE ProcessId, BOOLEAN Create);
EVT_WDF_DRIVER_UNLOAD DriverUnload;
NTSTATUS PrintProcesses();
NTSTATUS GetImageName(HANDLE ProcessId, char* string, SIZE_T size);
HANDLE GetProcessHandle(const char* module_name);


typedef NTSTATUS(*QUERY_INFO_PROCESS) (
	__in HANDLE ProcessHandle,
	__in PROCESSINFOCLASS ProcessInformationClass,
	__out_bcount(ProcessInformationLength) PVOID ProcessInformation,
	__in ULONG ProcessInformationLength,
	__out_opt PULONG ReturnLength
	);

QUERY_INFO_PROCESS ZwQueryInformationProcess;

typedef struct _ParentProcessPair
{
	char ImageName[256];
	HANDLE ParentId;
	HANDLE ProcessId;
} ParentProcessPair;