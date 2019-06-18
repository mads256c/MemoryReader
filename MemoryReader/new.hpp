#pragma once
#include <ntddk.h>
#include <cstddef>


#include "Util.h"


void* operator new(size_t size){
	void* p = ExAllocatePoolWithTag(PagedPool, size, 'wenO');

	DebugLog("Creating %p", p);

	return p;
}

void* operator new[](size_t size) {
	return operator new(size);
}

void operator delete(void* address){
	DebugLog("Deleting %p", address);
	ExFreePoolWithTag(address, 'wenO');
}

void operator delete(void* address, size_t size){
	UNREFERENCED_PARAMETER(size);
	operator delete(address);
}

void operator delete[](void* address)
{
	operator delete(address);
}