#pragma once
#include <ntifs.h>
#include <ntddk.h>
#include <wdf.h>
#include <sal.h>

#include "String.hpp"


#define PROCESSES_MAX 256

//The driver's entrypoint.
extern "C" DRIVER_INITIALIZE DriverEntry;

//Called when a device is added.
extern "C" EVT_WDF_DRIVER_DEVICE_ADD DriverDeviceAdd;

//Called every time a process is created or destroyed.
extern "C" VOID CreateProcessNotifyRoutine(HANDLE ParentId, HANDLE ProcessId, BOOLEAN Create);

//Creates and closes IoCtl.
_Dispatch_type_(IRP_MJ_CREATE)
_Dispatch_type_(IRP_MJ_CLOSE)
extern "C" DRIVER_DISPATCH IoCtlCreateClose;

_Dispatch_type_(IRP_MJ_DEVICE_CONTROL)
extern "C" DRIVER_DISPATCH IoCtlDeviceControl;

//Called when the driver unloads.
extern "C" EVT_WDF_DRIVER_UNLOAD DriverUnload;


//Prints all processes saved in a linked list.
NTSTATUS PrintProcesses();

//Gets the exe name from a process id.
NTSTATUS GetImageName(HANDLE ProcessId, String* string);

//Gets a handle to a process with the name specified in module_name.
HANDLE GetProcessHandle(const char* module_name);


typedef NTSTATUS(*QUERY_INFO_PROCESS) (
	__in HANDLE ProcessHandle,
	__in PROCESSINFOCLASS ProcessInformationClass,
	__out_bcount(ProcessInformationLength) PVOID ProcessInformation,
	__in ULONG ProcessInformationLength,
	__out_opt PULONG ReturnLength
	);

QUERY_INFO_PROCESS ZwQueryInformationProcess;



