#pragma once
#include <ntddk.h>
#include <cstddef>


void* operator new(size_t size){
	PHYSICAL_ADDRESS max_address;
	max_address.QuadPart = MAXULONG64;

	void * p = MmAllocateContiguousMemory(size, max_address);

	DebugLog("Creating %p", p);

	return p;
}

void* operator new[](size_t size) {
	return operator new(size);
}

void operator delete(void* address){
	DebugLog("Deleting %p", address);
	MmFreeContiguousMemory(address);
}

void operator delete(void* address, size_t size){
	UNREFERENCED_PARAMETER(size);
	operator delete(address);
}

void operator delete[](void* address)
{
	operator delete(address);
}