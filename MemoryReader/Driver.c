#include <stdio.h>

#include "Driver.h"


ParentProcessPair* processes;

PHYSICAL_ADDRESS maxaddr;

NTSTATUS DriverEntry(
	_In_ PDRIVER_OBJECT     DriverObject,
	_In_ PUNICODE_STRING    RegistryPath)
{
	PAGED_CODE();


	// NTSTATUS variable to record success or failure
	NTSTATUS status = STATUS_SUCCESS;

	UNICODE_STRING routineName;

	RtlInitUnicodeString(&routineName, L"ZwQueryInformationProcess");

	ZwQueryInformationProcess =
		(QUERY_INFO_PROCESS)MmGetSystemRoutineAddress(&routineName);

	if (ZwQueryInformationProcess == NULL)
	{
		DbgPrint("MemoryReader: Could not get ZwQueryInformationProcess\n");
		return STATUS_ABANDONED;
	}

	// Allocate the driver configuration object
	WDF_DRIVER_CONFIG config;

	// Print "Hello World" for DriverEntry
	DebugLog("Loading driver");

	
	maxaddr.QuadPart = MAXULONG64;

	processes = MmAllocateContiguousMemory(sizeof(ParentProcessPair) * PROCESSES_MAX, maxaddr);

	if (processes == NULL)
	{
		DebugLog("Failed to allocate processes array");
		return STATUS_INSUFFICIENT_RESOURCES;
	}

	RtlSecureZeroMemory(processes, sizeof(ParentProcessPair) * PROCESSES_MAX);
	PsSetCreateProcessNotifyRoutine(CreateProcessNotifyRoutine, FALSE);

	


	// Initialize the driver configuration object to register the
	// entry point for the EvtDeviceAdd callback, KmdfHelloWorldEvtDeviceAdd
	WDF_DRIVER_CONFIG_INIT(&config,
		DriverDeviceAdd
	);

	config.EvtDriverUnload = DriverUnload;

	// Finally, create the driver object
	status = WdfDriverCreate(DriverObject,
		RegistryPath,
		WDF_NO_OBJECT_ATTRIBUTES,
		&config,
		WDF_NO_HANDLE
	);
	return status;
}


VOID CreateProcessNotifyRoutine(HANDLE ParentId, HANDLE ProcessId, BOOLEAN Create)
{
	PAGED_CODE();

	if (Create)
	{
		for (int i = 0; i < PROCESSES_MAX; i++)
		{
			if (processes[i].ParentId == NULL && processes[i].ProcessId == NULL)
			{
				NTSTATUS status = GetImageName(ProcessId, processes[i].ImageName, 256);

				if (status != STATUS_SUCCESS) return;

				processes[i].ParentId = ParentId;
				processes[i].ProcessId = ProcessId;
				DebugLog("PrintProcesses returned with: %02x", PrintProcesses());
				if (GetProcessHandle("cmd.exe")) DebugLog("cmd.exe is alive!");
				return;
			}
		}
		DebugLog("No more space for processes");
	}
	else
	{
		for (int i = 0; i < PROCESSES_MAX; i++)
		{
			if (processes[i].ParentId == ParentId && processes[i].ProcessId == ProcessId)
			{
				processes[i].ParentId = NULL;
				processes[i].ProcessId = NULL;
				DebugLog("PrintProcesses returned with: %02x", PrintProcesses());
				return;
			}
		}
		char string[256] = { 0 };
		const NTSTATUS status = GetImageName(ProcessId, string, sizeof(string));
		if (status == STATUS_SUCCESS)
		{
			DebugLog("Did not have the process %p with imagename %s", ProcessId, string);
		}
		else
		{
			DebugLog("Did not have the process %p and GetImageName failed with %02x", ProcessId, status);
		}

		
	}

	
	
	//DbgPrint("Process created with ParentId: %p ProcessId: %p and Create: %d\n", ParentId, ProcessId, Create);
}

NTSTATUS DriverDeviceAdd(
	_In_    WDFDRIVER       Driver,
	_Inout_ PWDFDEVICE_INIT DeviceInit
)
{
	// We're not using the driver object,
	// so we need to mark it as unreferenced
	UNREFERENCED_PARAMETER(Driver);

	PAGED_CODE();
	
	NTSTATUS status;

	// Allocate the device object
	WDFDEVICE hDevice;

	// Print "Hello World"
	DebugLog("Device Add");


	// Create the device object
	status = WdfDeviceCreate(&DeviceInit,
		WDF_NO_OBJECT_ATTRIBUTES,
		&hDevice
	);
	return status;
}


VOID DriverUnload(IN WDFDRIVER Driver)
{
	UNREFERENCED_PARAMETER(Driver);

	PAGED_CODE();

	DebugLog("Unloading driver special");

	if (processes != NULL)
	{
		PsSetCreateProcessNotifyRoutine(CreateProcessNotifyRoutine, TRUE);
		MmFreeContiguousMemory(processes);
		processes = NULL;
	}
	
}

NTSTATUS PrintProcesses()
{
	PAGED_CODE();

	DebugLog("****** Processes ******");

	for (int i = 0; i < PROCESSES_MAX; i++)
	{
		if (processes[i].ParentId == NULL || processes[i].ProcessId == NULL) continue;
		DebugLog("%s %p %p", processes[i].ImageName, processes[i].ParentId, processes[i].ProcessId);
	}

	DebugLog("***********************");

	return STATUS_SUCCESS;
}

NTSTATUS GetImageName(HANDLE ProcessId, char* string, SIZE_T size)
{
	PAGED_CODE();

	NTSTATUS status = STATUS_ACCESS_DENIED;
	PEPROCESS pe_process = NULL;

	status = PsLookupProcessByProcessId(ProcessId, &pe_process);

	if (status != STATUS_SUCCESS) return status;
	if (pe_process == NULL) return STATUS_FWP_NULL_POINTER;

	HANDLE process_handle = NULL;

	status = ObOpenObjectByPointer(pe_process, 0, NULL, 0, 0, KernelMode, &process_handle);

	if (status != STATUS_SUCCESS)
	{
		ObDereferenceObject(pe_process);
		return status;
	}
	if (process_handle == NULL)
	{
		ObDereferenceObject(pe_process);
		return status;
	}

	ULONG returned_length = 0;

	status = ZwQueryInformationProcess(process_handle, ProcessImageFileName, NULL,
		0, &returned_length);

	if (STATUS_INFO_LENGTH_MISMATCH != status)
	{
		ZwClose(process_handle);
		ObDereferenceObject(pe_process);
		return status;
	}

	PVOID buffer = ExAllocatePoolWithTag(PagedPool, returned_length, 'ipgD');
	if (buffer == NULL)
	{
		ZwClose(process_handle);
		ObDereferenceObject(pe_process);
		return STATUS_INSUFFICIENT_RESOURCES;
	}

	status = ZwQueryInformationProcess(process_handle, ProcessImageFileName, buffer,
		returned_length, &returned_length);

	ZwClose(process_handle);
	ObDereferenceObject(pe_process);

	if (status != STATUS_SUCCESS)
	{
		ExFreePool(buffer);
		return status;
	}

	char ansi_string[512] = { 0 };

	const int chars_written = sprintf_s(ansi_string, sizeof(ansi_string), "%wZ", (PUNICODE_STRING)buffer);

	ExFreePool(buffer);

	if (chars_written == -1) return STATUS_ACCESS_DENIED;

	char* p = strrchr(ansi_string, '\\');

	if (p == NULL)
	{
		p = ansi_string;
	}
	else if (*p == '\\')
	{
		p++;
	}
	

	if (strcpy_s(string, size, p) != 0) return STATUS_ABANDONED;

	return STATUS_SUCCESS;
}

HANDLE GetProcessHandle(const char* module_name)
{
	PAGED_CODE();

	for (int i = 0; i < PROCESSES_MAX; ++i)
	{
		if (processes[i].ParentId == NULL || processes[i].ProcessId == NULL) continue;

		if (strcmp(module_name, processes[i].ImageName) == 0) return processes[i].ProcessId;
	}

	return NULL;
}