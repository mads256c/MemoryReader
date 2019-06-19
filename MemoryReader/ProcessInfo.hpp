#pragma once

#include <ntddk.h>

#include "String.hpp"

class ProcessInfo
{
	HANDLE parent_id_;
	HANDLE process_id_;

public:

	String image_name;

	ProcessInfo(HANDLE parent_id, HANDLE process_id) : parent_id_(parent_id), process_id_(process_id)
	{
		
	}

	HANDLE parent_id() const
	{
		return parent_id_;
	}

	HANDLE process_id() const
	{
		return process_id_;
	}
	
};
