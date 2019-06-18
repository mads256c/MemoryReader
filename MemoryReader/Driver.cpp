#include <stdio.h>

#include "Driver.hpp"
#include "ListNode.hpp"
#include "LinkedList.hpp"
#include "ProcessInfo.h"
#include "PEProcessWrapper.hpp"
#include "Util.h"
#include "ObHandleWrapper.hpp"
#include "BufferWrapper.hpp"
#include "String.hpp"

#include "new.hpp"

LinkedList<ProcessInfo*>* process_list;


extern "C" NTSTATUS DriverEntry(
	_In_ PDRIVER_OBJECT     DriverObject,
	_In_ PUNICODE_STRING    RegistryPath)
{
	PAGED_CODE();


	// NTSTATUS variable to record success or failure
	NTSTATUS status = STATUS_SUCCESS;

	UNICODE_STRING routineName;

	RtlInitUnicodeString(&routineName, L"ZwQueryInformationProcess");

	ZwQueryInformationProcess =
		static_cast<QUERY_INFO_PROCESS>(MmGetSystemRoutineAddress(&routineName));

	if (ZwQueryInformationProcess == nullptr)
	{
		DbgPrint("MemoryReader: Could not get ZwQueryInformationProcess\n");
		return STATUS_ABANDONED;
	}

	// Allocate the driver configuration object
	WDF_DRIVER_CONFIG config;

	// Print "Hello World" for DriverEntry
	DebugLog("Loading driver");


	process_list = new LinkedList<ProcessInfo*>();


	status = PsSetCreateProcessNotifyRoutine(CreateProcessNotifyRoutine, FALSE);
	if (status != STATUS_SUCCESS)
	{
		DebugLog("Could not set CreateProcessNotifyRoutine");
		return status;
	}




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


extern "C" VOID CreateProcessNotifyRoutine(HANDLE ParentId, HANDLE ProcessId, BOOLEAN Create)
{
	PAGED_CODE();

	if (Create)
	{
		auto process_info = new ProcessInfo(ParentId, ProcessId);

		NTSTATUS status = GetImageName(ProcessId, &process_info->image_name);
		if (status != STATUS_SUCCESS)
		{
			process_info->image_name.allocate("GetImageName failed!");
		}

		process_list->insert(process_info);

		PrintProcesses();
	}
	else
	{
		for (auto node = process_list->head(); node != nullptr; node = node->next)
		{
			if (node->data->parent_id() == ParentId && node->data->process_id() == ProcessId)
			{
				process_list->remove(node);
				PrintProcesses();
				return;
			}
		}

		String str;
		const NTSTATUS status = GetImageName(ProcessId, &str);
		if (status == STATUS_SUCCESS)
		{
			DebugLog("Did not have the process %p with imagename %s", ProcessId, str.buffer());
		}
		else
		{
			DebugLog("Did not have the process %p and GetImageName failed with %02x", ProcessId, status);
		}
	}
}

extern "C" NTSTATUS DriverDeviceAdd(
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


extern "C" VOID DriverUnload(IN WDFDRIVER Driver)
{
	UNREFERENCED_PARAMETER(Driver);

	PAGED_CODE();

	DebugLog("Unloading driver special");

	if (process_list != NULL)
	{
		PsSetCreateProcessNotifyRoutine(CreateProcessNotifyRoutine, TRUE);
		delete process_list;
	}
}


NTSTATUS PrintProcesses()
{
	PAGED_CODE();

	DebugLog("****** Processes ******");

	for (auto node = process_list->head(); node != nullptr; node = node->next)
	{
		DebugLog("%s %p %p", node->data->image_name.buffer(), node->data->parent_id(), node->data->process_id());
	}
	DebugLog("%zu processes in list", process_list->count());
	DebugLog("***********************");

	return STATUS_SUCCESS;
}


NTSTATUS GetImageName(HANDLE ProcessId, String* string)
{
	PAGED_CODE();

	NTSTATUS status = STATUS_ACCESS_DENIED;

	PEProcessWrapper process_wrapper;

	status = PsLookupProcessByProcessId(ProcessId, &process_wrapper.pe_process);

	if (status != STATUS_SUCCESS) return status;
	if (process_wrapper.pe_process == nullptr) return STATUS_FWP_NULL_POINTER;

	ObHandleWrapper process_handle_wrapper;

	status = ObOpenObjectByPointer(process_wrapper.pe_process, 0, nullptr, 0, nullptr, KernelMode,
	                               &process_handle_wrapper.handle);


	if (status != STATUS_SUCCESS) return status;
	if (process_handle_wrapper.handle == nullptr) return STATUS_FWP_NULL_POINTER;

	ULONG returned_length = 0;

	status = ZwQueryInformationProcess(process_handle_wrapper.handle, ProcessImageFileName, nullptr, 0, &returned_length);

	if (STATUS_INFO_LENGTH_MISMATCH != status) return status;

	const BufferWrapper buffer_wrapper(returned_length);

	if (buffer_wrapper.buffer<void>() == nullptr) return STATUS_INSUFFICIENT_RESOURCES;

	status = ZwQueryInformationProcess(process_handle_wrapper.handle, ProcessImageFileName,
	                                   buffer_wrapper.buffer<void>(), returned_length, &returned_length);

	if (status != STATUS_SUCCESS) return status;



	string->allocate(buffer_wrapper.buffer<UNICODE_STRING>());

	auto p = strrchr(string->buffer(), '\\');
	if (*p == '\\') p++;

	string->allocate(p);

	return STATUS_SUCCESS;
}

HANDLE GetProcessHandle(const char* module_name)
{
	PAGED_CODE();

	for (auto node = process_list->head(); node != nullptr; node = node->next)
	{
		if (strcmp(module_name, node->data->image_name.buffer()) == 0) return node->data->process_id();
	}

	return nullptr;
}
